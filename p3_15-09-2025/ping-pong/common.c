#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>		/* getifaddrs */
#include <linux/if_packet.h>    /* AF_PACKET  */
#include <arpa/inet.h>          /* htons      */
#include <stdint.h>

#include "common.h"

/*
 * Print MAC address in hex format
 */
void print_mac_addr(uint8_t *addr, size_t len)
{
        size_t i;

        for (i = 0; i < len - 1; i++) {
                printf("%02x:", addr[i]);
        }
        printf("%02x\n", addr[i]);
}


/*
 * This function stores struct sockaddr_ll addresses for all interfaces of the
 * node (except loopback interface)
 */
void get_mac_from_interfaces(struct ifs_data *ifs)
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
                    strcmp("lo", ifp->ifa_name))
			/* Copy the address info into the array of our struct */
                        memcpy(&(ifs->addr[i++]),
                               (struct sockaddr_ll*)ifp->ifa_addr,
                               sizeof(struct sockaddr_ll));
        }
        /* After the for loop, the address info of all interfaces are stored */
        /* Update the counter of the interfaces */
        ifs->ifn = i;

        /* Free the interface list */
        freeifaddrs(ifaces);
}


void init_ifs(struct ifs_data *ifs, int rsock, uint8_t local) {
	/* Get some info about the local ifaces */
	get_mac_from_interfaces(ifs);

	/* We use one RAW socket per node */
	ifs->rsock = rsock;
	
	/* One MIP address per node; We name nodes and not interfaces like the
	 * Internet does. Read about RINA Network Architecture for more info
	 * about what's wrong with the current Internet.
	 */
	ifs->local_mip_addr = local;
}

int create_raw_socket(void)
{
	int sd;
	short unsigned int protocol = 0xFFFF;

	/* Set up a raw AF_PACKET socket without ethertype filtering */
	sd = socket(AF_PACKET, SOCK_RAW, htons(protocol));
	if (sd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	return sd;
}

int send_mip_packet(struct ifs_data *ifs,
		    uint8_t *dst_mac_addr,
		    uint8_t mip_dest,
		    uint8_t *packet)
{
	struct ether_frame  frame_hdr;
	struct hello_header hello_hdr;
	struct msghdr      *msg;
	struct iovec        msgvec[3];
	int                 rc;

	/* Fill in Ethernet header */
	memcpy(frame_hdr.dst_addr, dst_mac_addr, 6);
	memcpy(frame_hdr.src_addr, ifs->addr[0].sll_addr, 6);
	/* Match the ethertype in packet_socket.c: */
	frame_hdr.eth_proto[0] = frame_hdr.eth_proto[1] = 0xFF;

	/* Fill in MIP header */
	hello_hdr.dest = mip_dest;
	hello_hdr.src = ifs->local_mip_addr;
	hello_hdr.sdu_len = sizeof(*packet);

	/* Point to frame header */
	msgvec[0].iov_base = &frame_hdr;
	msgvec[0].iov_len  = sizeof(struct ether_frame);

	/* Point to hello header */
	msgvec[1].iov_base = &hello_hdr;
	msgvec[1].iov_len  = sizeof(struct hello_header);

	/* Point to ping/pong message */
	msgvec[2].iov_base = (void *)packet;
	msgvec[2].iov_len  = strlen((const char *)packet) + 1;


	/* Allocate a zeroed-out message info struct */
	msg = (struct msghdr *)calloc(1, sizeof(struct msghdr));

	/* Fill out message metadata struct */
	msg->msg_name    = &(ifs->addr[0]);
	msg->msg_namelen = sizeof(struct sockaddr_ll);
	msg->msg_iovlen  = 3;
	msg->msg_iov     = msgvec;

	printf("Sending a MIP pkt. with content '%s' to node %u with MAC addr.: \n",
	       (char *)packet, hello_hdr.dest);
	print_mac_addr(frame_hdr.dst_addr, 6);

	/* Send message via RAW socket */
	rc = sendmsg(ifs->rsock, msg, 0);
	if (rc == -1) {
		perror("sendmsg");
		free(msg);
		return -1;
	}

	/* Remember that we allocated this on the heap; free it */
	free(msg);

	return rc;
}

int handle_mip_packet(struct ifs_data *ifs)
{
	struct sockaddr_ll  so_name;
	struct ether_frame  frame_hdr;
	struct hello_header hello_hdr;
	struct msghdr       msg = {0};
	struct iovec        msgvec[3];
	uint8_t             packet[256];
	int                 rc;

	/* Point to frame header */
	msgvec[0].iov_base = &frame_hdr;
	msgvec[0].iov_len  = sizeof(struct ether_frame);

	/* Point to hello header */
	msgvec[1].iov_base = &hello_hdr;
	msgvec[1].iov_len  = sizeof(struct hello_header);
	
	/* Point to ping/pong packet */
	msgvec[2].iov_base = (void *)packet;
	/* We can read up to 256 characters. Who cares? PONG is only 5 bytes */
	msgvec[2].iov_len  = 256;

	/* Fill out message metadata struct */
	msg.msg_name    = &so_name;
	msg.msg_namelen = sizeof(struct sockaddr_ll);
	msg.msg_iovlen  = 3;
	msg.msg_iov     = msgvec;

	rc = recvmsg(ifs->rsock, &msg, 0);
	if (rc <= 0) {
		perror("sendmsg");
		return -1;
	}

	printf("<info>: We got a MIP pkt. with content '%s' from node %d with MAC addr.: ",
	       (char *)packet, hello_hdr.src);
	print_mac_addr(frame_hdr.src_addr, 6);

	return rc;
}
