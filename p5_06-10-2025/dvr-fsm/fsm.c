#include "fsm.h"

void print_mac_addr(uint8_t *addr, int len) {
	for (int i = 0; i < len; i++) {
		printf("%02x", addr[i]);
		if (i < len - 1)
			printf(":");
	}
}

/*
 * Send packet over the RAW socket
 */
int send_raw_packet(int sd, struct sockaddr_ll *so_name, uint8_t *buf, size_t len)
{
	struct ether_frame frame_hdr;
	struct msghdr      *msg;
	struct iovec       msgvec[2];
	int                rc;

	/* Fill in Ethernet header */
	/* The dst MAC address is broadcast */
	uint8_t dst_addr[] = BROADCAST_MAC_ADDR;
	memcpy(frame_hdr.dst_addr, dst_addr, 6);
	memcpy(frame_hdr.src_addr, so_name->sll_addr, 6);
	/* Match the ethertype in packet_socket.c: */
	frame_hdr.eth_proto[0] = frame_hdr.eth_proto[1] = 0xFF;

	/* Point to frame header */
	msgvec[0].iov_base = &frame_hdr;
	msgvec[0].iov_len  = sizeof(struct ether_frame);
	/* Point to frame payload */
	msgvec[1].iov_base = buf;
	msgvec[1].iov_len  = len;

	/* Allocate a zeroed-out message info struct */
	msg = (struct msghdr *)calloc(1, sizeof(struct msghdr));

	/* Fill out message metadata struct */
	msg->msg_name    = so_name;
	msg->msg_namelen = sizeof(struct sockaddr_ll);
	msg->msg_iovlen  = 2;
	msg->msg_iov     = msgvec;

	printf("\nSending a broadcast hello ...\n");

	/* Construct and send message */
	rc = sendmsg(sd, msg, 0);
	if (rc == -1) {
		perror("sendmsg");
		free(msg);
		return 1;
	}

	/* Remember that we allocated this on the heap; free it */
	free(msg);

	return rc;
}

/*
 * Receive packet from the RAW socket
 */
int recv_raw_packet(int sd, uint8_t *buf, size_t len)
{
	struct sockaddr_ll so_name;
	struct ether_frame frame_hdr;
	struct msghdr      msg;
	struct iovec       msgvec[2];
	int                rc;

	/* Point to frame header */
	msgvec[0].iov_base = &frame_hdr;
	msgvec[0].iov_len  = sizeof(struct ether_frame);
	/* Point to frame payload */
	msgvec[1].iov_base = buf;
	msgvec[1].iov_len  = len;


	/* Fill out message metadata struct */
	msg.msg_name    = &so_name;
	msg.msg_namelen = sizeof(struct sockaddr_ll);
	msg.msg_iovlen  = 2;
	msg.msg_iov     = msgvec;

	rc = recvmsg(sd, &msg, 0);
	if (rc == -1) {
		perror("recvmsg");
		return -1;
	}

	printf("neighbor with MAC address ");
	print_mac_addr(frame_hdr.src_addr, 6);
	printf(" sent: %s\n", buf);

	return rc;
}

/*
 * Discover all interfaces and store them in the interfaces array
 * Returns the number of interfaces found
 */
int discover_interfaces(interface_t *interfaces) {
	struct ifaddrs *ifaddr, *ifa;
	int num_interfaces = 0;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	/* Walk through linked list */
	for (ifa = ifaddr;
	     ifa != NULL && num_interfaces < MAX_INTERFACES;
	     ifa = ifa->ifa_next) {
		if (ifa->ifa_addr != NULL &&
		    ifa->ifa_addr->sa_family == AF_PACKET &&
		    strcmp("lo", ifa->ifa_name) != 0) {
			struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;

			/* Store interface index */
			interfaces[num_interfaces].ifindex = s->sll_ifindex;

			/* Store MAC address */
			memcpy(interfaces[num_interfaces].mac_addr, s->sll_addr, 6);

			printf("Found interface %s with index %d, MAC: ",
			       ifa->ifa_name, s->sll_ifindex);
			print_mac_addr(interfaces[num_interfaces].mac_addr, 6);
			printf("\n");

			num_interfaces++;
		}
	}

	freeifaddrs(ifaddr);
	return num_interfaces;
}

/*
 * Send hello messages to all interfaces
 */
void send_hello_to_all_interfaces(int raw_sock, interface_t *interfaces,
				  int num_interfaces) {
	struct sockaddr_ll so_name;
	uint8_t hello_msg[] = "HELLO";

	for (int i = 0; i < num_interfaces; i++) {
		/* Set up the sockaddr_ll for this interface */
		memset(&so_name, 0, sizeof(struct sockaddr_ll));
		so_name.sll_family = AF_PACKET;
		so_name.sll_protocol = htons(ETH_P_ALL);
		so_name.sll_ifindex = interfaces[i].ifindex;
		memcpy(so_name.sll_addr, interfaces[i].mac_addr, 6);
		so_name.sll_halen = 6;

		/* Send the hello message */
		send_raw_packet(raw_sock, &so_name, hello_msg,
				strlen((char *)hello_msg) + 1);
	}
}

/*
 * Find a neighbor in the neighbors array by MAC address
 * If not found, add it to the array
 * Returns the index of the neighbor in the array
 */
int find_or_add_neighbor(neighbor_t *neighbors, int *num_neighbors,
			 uint8_t *mac_addr) {
	/* Check if we already know this neighbor */
	for (int i = 0; i < *num_neighbors; i++) {
		if (memcmp(neighbors[i].mac_addr, mac_addr, 6) == 0) {
			return i;
		}
	}

	/* If we have room, add this new neighbor */
	if (*num_neighbors < MAX_NEIGHBORS) {
		memcpy(neighbors[*num_neighbors].mac_addr, mac_addr, 6);
		neighbors[*num_neighbors].last_hello_time = time(NULL);
		neighbors[*num_neighbors].missed_hellos = 0;
		neighbors[*num_neighbors].state = CONNECTED;

		printf("New neighbor discovered: ");
		print_mac_addr(mac_addr, 6);
		printf("\n");

		return (*num_neighbors)++;
	}

	return -1; /* No room for new neighbors */
}

/*
 * Check if any neighbors have timed out
 */
void check_neighbor_timeouts(neighbor_t *neighbors, int num_neighbors) {
	time_t now = time(NULL);

	for (int i = 0; i < num_neighbors; i++) {
		/* Skip neighbors that are already disconnected */
		if (neighbors[i].state == DISCONNECTED) {
			continue;
		}

		/* Check if it's been more than HELLO_INTERVAL seconds since the
                 * last hello */
		if (now - neighbors[i].last_hello_time > HELLO_INTERVAL) {
			neighbors[i].missed_hellos++;

			printf("Neighbor ");
			print_mac_addr(neighbors[i].mac_addr, 6);
			printf(" missed a hello. Count: %d\n",
			       neighbors[i].missed_hellos);

			/* If we've missed too many hellos, mark as disconnected */
			if (neighbors[i].missed_hellos >= TIMEOUT_COUNT) {
				printf("Neighbor ");
				print_mac_addr(neighbors[i].mac_addr, 6);
				printf(" is now DISCONNECTED\n");
				neighbors[i].state = DISCONNECTED;
			}
		}
	}
}

/*
 * Print the current list of neighbors and their states
 */
void print_neighbors(neighbor_t *neighbors, int num_neighbors) {
	printf("\n=== Neighbor List ===\n");
	for (int i = 0; i < num_neighbors; i++) {
		printf("Neighbor %d: MAC=", i);
		print_mac_addr(neighbors[i].mac_addr, 6);
		printf(", State=%s, Last Hello=%ld seconds ago\n",
		       neighbors[i].state == CONNECTED ? "CONNECTED" : "DISCONNECTED",
		       time(NULL) - neighbors[i].last_hello_time);
	}
	printf("=====================\n");
}

int main(void) {
	int raw_sock, epollfd, nfds, protocol = ETH_P_ALL;
	struct epoll_event ev, events[10];
	interface_t interfaces[MAX_INTERFACES];
	neighbor_t neighbors[MAX_NEIGHBORS];
	int num_interfaces = 0;
	int num_neighbors = 0;
	time_t last_hello_time = 0;
	time_t last_timeout_check = 0;
	uint8_t buffer[BUFFER_SIZE];
	node_state_t node_state = INIT;

	/* Set up a raw AF_PACKET socket without ethertype filtering */
	raw_sock = socket(AF_PACKET, SOCK_RAW, htons(protocol));
	if (raw_sock == -1) {
		perror("socket");
		return -1;
	}

	/* Setup epoll */
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = raw_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, raw_sock, &ev) == -1) {
		perror("epoll_ctl: raw_sock");
		exit(EXIT_FAILURE);
	}

	/* Discover all network interfaces */
	num_interfaces = discover_interfaces(interfaces);
	if (num_interfaces == 0) {
		printf("No interfaces found!\n");
		exit(EXIT_FAILURE);
	}

	printf("FSM started in INIT state\n");

	/* Main loop */
	for (;;) {
		/* Check if it's time to send a hello message */
		time_t current_time = time(NULL);
		if (current_time - last_hello_time >= HELLO_INTERVAL) {
			send_hello_to_all_interfaces(raw_sock, interfaces,
						     num_interfaces);
			last_hello_time = current_time;

			/* If in INIT state and we've sent at least one hello,
			 *  move to CONNECTED state */
			if (node_state == INIT) {
				node_state = CONNECTED;
				printf("FSM state changed: INIT -> CONNECTED\n");
			}
		}

		/* Check for timeouts once per second */
		if (current_time - last_timeout_check >= 1) {
			check_neighbor_timeouts(neighbors, num_neighbors);
			last_timeout_check = current_time;

			/* Periodically print neighbor list */
			if (num_neighbors > 0) {
				print_neighbors(neighbors, num_neighbors);
			}
		}

		/* Wait for events with a timeout of 1 second */
		nfds = epoll_wait(epollfd, events, 10, 1000);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		/* Process received messages */
		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == raw_sock) {
				struct sockaddr_ll so_name;
				struct ether_frame frame_hdr;
				struct msghdr msg;
				struct iovec msgvec[2];

				/* Prepare for receiving message */
				memset(buffer, 0, BUFFER_SIZE);

				/* Point to frame header */
				msgvec[0].iov_base = &frame_hdr;
				msgvec[0].iov_len  = sizeof(struct ether_frame);
				/* Point to frame payload */
				msgvec[1].iov_base = buffer;
				msgvec[1].iov_len  = BUFFER_SIZE;

				/* Fill out message metadata struct */
				msg.msg_name    = &so_name;
				msg.msg_namelen = sizeof(struct sockaddr_ll);
				msg.msg_iovlen  = 2;
				msg.msg_iov     = msgvec;

				/* Receive message */
				int rc = recvmsg(raw_sock, &msg, 0);
				if (rc > 0) {

					/* Process HELLO message */
					printf("Received HELLO from ");
					print_mac_addr(frame_hdr.src_addr, 6);
					printf("\n");

					/* Update neighbor information */
					int idx = find_or_add_neighbor(neighbors,
								       &num_neighbors,
								       frame_hdr.src_addr);
					if (idx >= 0) {
						neighbors[idx].last_hello_time = time(NULL);
						neighbors[idx].missed_hellos = 0;

						/* If neighbor was disconnected,
                                                 * mark as connected */
						if (neighbors[idx].state == DISCONNECTED) {
							neighbors[idx].state = CONNECTED;
							printf("Neighbor ");
							print_mac_addr(neighbors[idx].mac_addr, 6);
							printf(" is now CONNECTED\n");
						}
					}
				}
			}
		}
	}

	return 0;
}
