#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <unistd.h>             /* fgets */
#include <string.h>		/* memset */
#include <sys/socket.h>		/* socket */
#include <fcntl.h>
#include <sys/epoll.h>          /* epoll */
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */

#include "common.h"

int main(int argc, char *argv[])
{
	struct ifs_data local_if;
	int    raw_sock, rc;
	struct epoll_event ev, events[MAX_EVENTS];
	int epollfd;

	/* Set up a raw AF_PACKET socket without ethertype filtering */
	raw_sock = create_raw_socket();

	/* Initialize interface data */
	init_ifs(&local_if, raw_sock, MIP_ADDR_A);

	/* Create epoll table */
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	/* Add RAW socket to epoll table */
	ev.events = EPOLLIN;
	ev.data.fd = raw_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, raw_sock, &ev) == -1) {
		perror("epoll_ctl: raw_sock");
		exit(EXIT_FAILURE);
	}

	/* Simply introduce yourself via stdout */
	printf("\n<nodeA> Hi! My MAC address is: ");
	print_mac_addr(local_if.addr[0].sll_addr, 6);

	/* Send 'PING' to nodeB */
	uint8_t dst_addr[] = ETH_MAC_B;
	uint8_t packet[] = "PING";
	send_mip_packet(&local_if, dst_addr, MIP_ADDR_B, packet);

	while(1) {
		rc = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		} else if (events->data.fd == raw_sock) {
			printf("<info> nodeB is Ponging\n");

			rc = handle_mip_packet(&local_if);
			if (rc < 1) {
				perror("handle_mip_packet");
				exit(EXIT_FAILURE);
			}
		}
		break;
	}

	close(raw_sock);

	return 0;
}
