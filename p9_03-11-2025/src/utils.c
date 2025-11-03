#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "utils.h"
#include "ether.h"

/* Create raw AF_PACKET socket */
int create_raw_socket(void)
{
	int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sock;
}

/*
 * This function stores struct sockaddr_ll addresses for all interfaces of the
 * node (except loopback interface)
 */
void get_mac_from_ifaces(struct ifs_data *ifs)
{
	struct ifaddrs *ifaces, *ifp;
	int i = 0;

	/* Enumerate interfaces: */
	/* Note in man getifaddrs that this function dynamically allocates
	   memory. It becomes our responsability to free it! */
	if (getifaddrs(&ifaces)) {
		perror("getifaddrs");
		exit(-1);
	}

	/* Walk the list looking for ifaces interesting to us */
	for (ifp = ifaces; ifp != NULL; ifp = ifp->ifa_next) {
		/* We make sure that the ifa_addr member is actually set: */
		if (ifp->ifa_addr != NULL &&
		    ifp->ifa_addr->sa_family == AF_PACKET &&
		    strcmp("lo", ifp->ifa_name)) {

			printf("<debug> Found interface: %s\n", ifp->ifa_name);

			/* Copy the address info into the array of our struct */
			memcpy(&(ifs->addr[i++]),
			       (struct sockaddr_ll*)ifp->ifa_addr,
			       sizeof(struct sockaddr_ll));

			if (i >= MAX_IFS) {
				printf("<warning> Max interfaces reached\n");
				break;
			}
		}
	}
	/* After the for loop, the address info of all interfaces are stored */
	/* Update the counter of the interfaces */
	ifs->ifn = i;

	printf("<debug> Found %d interfaces\n", ifs->ifn);

	/* Free the interface list */
	freeifaddrs(ifaces);

	if (ifs->ifn == 0) {
		fprintf(stderr, "<error> No network interfaces found (except loopback)\n");
		exit(EXIT_FAILURE);
	}
}

/* Initialize interface data */
void init_ifs(struct ifs_data *ifs, int rsock)
{
	uint8_t rand_hip;

	/* Get some info about the local ifaces */
	get_mac_from_ifaces(ifs);

	/* We use one RAW socket per node */
	ifs->rsock = rsock;

	/* One HIP address per node; We name nodes and not interfaces like the
	 * Internet does. Read about RINA Network Architecture for more info
	 * about what's wrong with the current Internet.
	 */

	srand(time(0));
	rand_hip = (uint8_t)(rand() % 256);

	ifs->local_hip_addr = rand_hip;

	printf("<debug> Initialized with HIP address: %u\n", ifs->local_hip_addr);
}

/* Add socket to epoll */
int epoll_add_sock(int sock)
{
	int efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock;

	if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	return efd;
}

/* Print MAC address */
void print_mac_addr(const uint8_t *mac, size_t len)
{
	if (mac == NULL) {
		printf("(null)\n");
		return;
	}

	for (size_t i = 0; i < len; i++) {
		printf("%02x", mac[i]);
		if (i < len - 1)
			printf(":");
	}
	printf("\n");
}
