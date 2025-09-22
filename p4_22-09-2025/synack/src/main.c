#include <stdlib.h>		/* free */
#include <stdio.h> 		/* printf */
#include <unistd.h>             /* fgets */
#include <string.h>		/* memset */
#include <fcntl.h>
#include <linux/if_packet.h>	/* AF_PACKET */
#include <net/ethernet.h>	/* ETH_* */
#include <arpa/inet.h>		/* htons */
#include <sys/epoll.h>		/* epoll */

#include "utils.h"
#include "ether.h"
#include "hip.h"

int main(int argc, const char *argv[])
{
	struct ifs_data local_if;
	struct epoll_event events[MAX_EVENTS];
	int    raw_sock, efd, rc;

	if (argc < 2) {
		printf("Too few args: ./hip c/s 'hello'\n");
		exit(EXIT_FAILURE);
	}

	/* Set up a raw AF_PACKET socket without ethertype filtering */
	raw_sock = create_raw_socket();

	/* Initialize interface data */
	init_ifs(&local_if, raw_sock);

	/* Add socket to epoll table */
	efd = epoll_add_sock(raw_sock);

	/* Simply introduce yourself via stdout */
	printf("\n<info> Hi! I am node %u with MAC ", local_if.local_hip_addr);
	print_mac_addr(local_if.addr[0].sll_addr, 6);

	if (strcmp(argv[1], "c") == 0) {
		/* client mode */
		/* Send SYN to the server */
		uint8_t broadcast[] = ETH_DST_MAC;
		send_hip_packet(&local_if, local_if.addr[0].sll_addr, broadcast,
				local_if.local_hip_addr, HIP_DST_ADDR, argv[2], 0);
	}

	while(1) {
		rc = epoll_wait(efd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		} else if (events->data.fd == raw_sock) {
			rc = handle_hip_packet(&local_if, argv[1]);
			if (rc < 0) {
				perror("handle_hip_packet");
				exit(EXIT_FAILURE);
			}
		}
		break;
	}
	close(raw_sock);

	return 0;
}
