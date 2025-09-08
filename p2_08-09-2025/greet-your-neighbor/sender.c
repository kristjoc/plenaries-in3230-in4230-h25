#include <stdlib.h>		/* free */
#include <stdio.h>		/* printf */
#include <unistd.h>		/* fgets */
#include <string.h>		/* memset */
#include <sys/socket.h>	/* socket */
#include <fcntl.h>
#include <sys/epoll.h>	/* epoll */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */

#include "common.h"

int main(int argc, char *argv[])
{
	struct ifs_data local_if;
	int	raw_sock, rc;

	struct epoll_event ev, events[MAX_EVENTS];
	int epollfd;

	/* Set up a raw AF_PACKET socket without ethertype filtering */
	raw_sock = create_raw_socket();

	/* Initialize interface data and store the interface metadata (MAC addr.
	 * interface index, etc.) to local_if
	 */
	init_ifs(&local_if, raw_sock);

	/* Create epoll table */
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		close(raw_sock);
		exit(EXIT_FAILURE);
	}

	/* Add RAW socket to epoll table */
	ev.events = EPOLLIN;
	ev.data.fd = raw_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, raw_sock, &ev) == -1) {
		perror("epoll_ctl: raw_sock");
		close(raw_sock);
		exit(EXIT_FAILURE);
	}

	/* Introduce yourself in the stdout - this is just locally */
	printf("\n<%s> Hi! I am %s with MAC ", argv[0], argv[0]);
	print_mac_addr(local_if.addr[0].sll_addr, 6);

	/* Send ARP request to know the neighbor next door */
	send_arp_request(&local_if);

	while(1) {
		rc = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			break;
		} else if (events->data.fd == raw_sock) {
			printf("<info> The neighbor is responding to the handshake\n");
			rc = handle_arp_packet(&local_if);
			if (rc < 1) {
				perror("handle_arp_packet");
				break;
			}
		}
		/* we're done here - it was just a handshake */
		break;
	}

	close(raw_sock);
	return 0;
}
