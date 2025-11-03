#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "utils.h"
#include "ether.h"
#include "hip.h"
#include "rtp.h"
#include "pdu.h"

#define MAX_EVENTS 10

/* Sender function */
void sender(int raw_sock, int efd, struct ifs_data *local_if, const char *filename)
{
	struct sender_state sender;
	struct epoll_event events[MAX_EVENTS];
	uint8_t broadcast[] = ETH_DST_MAC;
	int rc;
    
	if (sender_init(&sender, filename) < 0) {
		exit(EXIT_FAILURE);
	}
    
	printf("<info> Sending file: %s\n\n", filename);
    
	/* Keep sending until window is empty and no more data */
	while (1) {
		/* Send packets to fill window */
		int sent = 0;
		while (send_next_packet(raw_sock, local_if, &sender,
					broadcast, HIP_DST_ADDR) > 0) {
			sent = 1;
		}
        
		/* If no packets sent and queue is empty, we're done */
		if (!sent && sender.queue_size == 0)
			break;
        
		/* Wait for ACKs */
		rc = epoll_wait(efd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			break;
		}
        
		/* Process ACKs */
		for (int i = 0; i < rc; i++) {
			if (events[i].data.fd == raw_sock) {
				uint8_t buf[MAX_BUF_SIZE];
				ssize_t len = recv(raw_sock, buf, MAX_BUF_SIZE, 0);
                
				if (len > 0) {
					/* Quick filter: check ethertype before allocating PDU */
					if (len < 14) continue; /* Too short for ethernet header */
                    
					uint16_t ethertype = ntohs(*(uint16_t *)(buf + 12));
					if (ethertype != ETH_P_HIP) continue; /* Not our protocol */
                    
					struct pdu *pdu = alloc_pdu();
					if (!pdu) continue;
                    
					deserialize_pdu(pdu, buf);
                    
					if (pdu->rtphdr && pdu->rtphdr->type == RTP_TYPE_ACK) {
						uint32_t ack = ntohl(pdu->rtphdr->seq_num);
                        
						/* Check for duplicate ACK (trigger Go-Back-N) */
						if (ack < sender.oldest_unacked_seq && ack == sender.oldest_unacked_seq - 1) {
							/* Duplicate ACK - retransmit from oldest_unacked_seq */
							go_back_n(raw_sock, local_if, &sender,
								  broadcast, HIP_DST_ADDR);
						} else {
							/* Normal ACK */
							handle_ack(&sender, ack);
						}
					}
                    
					destroy_pdu(pdu);
				}
			}
		}
	}
    
	printf("\n<info> Transfer complete!\n");
	fclose(sender.fp);
}

/* Receiver function */
void receiver(int raw_sock, int efd, struct ifs_data *local_if)
{
	struct receiver_state receiver;
	struct epoll_event events[MAX_EVENTS];
	int rc;
	char received_filename[256];
    
	receiver.fp = NULL;
	receiver.expected_seq = 0;
    
	printf("<info> Waiting for file transfer...\n\n");
    
	while (1) {
		rc = epoll_wait(efd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			break;
		}
        
		/* Process data packets */
		for (int i = 0; i < rc; i++) {
			if (events[i].data.fd == raw_sock) {
				uint8_t buf[MAX_BUF_SIZE];
				ssize_t len = recv(raw_sock, buf, MAX_BUF_SIZE, 0);
                
				if (len > 0) {
					/* Quick filter: check ethertype BEFORE allocating PDU */
					if (len < 14) {
						/* Too short for ethernet header */
						continue;
					}
                    
					/* Ethertype is at bytes 12-13 in ethernet frame */
					uint16_t ethertype = ntohs(*(uint16_t *)(buf + 12));
                    
					/* Only process our protocol packets */
					if (ethertype != ETH_P_HIP) {
						/* Skip non-HIP packets (ARP, IP, etc.) */
						continue;
					}
                    
					struct pdu *pdu = alloc_pdu();
					if (!pdu) {
						fprintf(stderr, "Failed to allocate PDU\n");
						continue;
					}
                    
					size_t parsed = deserialize_pdu(pdu, buf);
					if (parsed == 0) {
						/* Failed to parse */
						destroy_pdu(pdu);
						continue;
					}
                    
					/* Check if it's an RTP DATA packet */
					if (pdu->rtphdr && pdu->rtphdr->type == RTP_TYPE_DATA) {
                        
						/* Initialize receiver on first packet */
						if (receiver.fp == NULL) {
							/* Create filename with HIP address for uniqueness */
							snprintf(received_filename, sizeof(received_filename),
								 "received_file_%u_received", local_if->local_hip_addr);
                            
							receiver_init(&receiver, received_filename);
							printf("<info> Receiving file: %s\n\n", received_filename);
						}
                        
						handle_data(raw_sock, local_if, &receiver, pdu);
					}
                    
					destroy_pdu(pdu);
				}
			}
		}
	}
}

int main(int argc, const char *argv[])
{
	struct ifs_data local_if;
	int raw_sock, efd;
    
	if (argc < 2) {
		printf("Usage:\n");
		printf("  Receiver: ./rtp -l\n");
		printf("  Sender:   ./rtp -c <filename>\n");
		exit(EXIT_FAILURE);
	}
    
	printf("<debug> Creating raw socket...\n");
    
	/* Set up raw AF_PACKET socket */
	raw_sock = create_raw_socket();
    
	printf("<debug> Socket created: %d\n", raw_sock);
	printf("<debug> Initializing interfaces...\n");
    
	/* Initialize interface data */
	init_ifs(&local_if, raw_sock);
    
	printf("<debug> Interfaces initialized\n");
    
	/* Add socket to epoll */
	efd = epoll_add_sock(raw_sock);
    
	printf("<debug> Epoll fd: %d\n", efd);
    
	/* Print node info */
	printf("\n<info> Hi! I am node %u with MAC ", local_if.local_hip_addr);
    
	if (local_if.ifn > 0) {
		print_mac_addr(local_if.addr[0].sll_addr, 6);
	} else {
		printf("(no interface found)\n");
	}
    
	if (strcmp(argv[1], "-c") == 0) {
		/* CLIENT MODE - SENDER */
		if (argc < 3) {
			printf("Error: -c requires filename\n");
			exit(EXIT_FAILURE);
		}
        
		sender(raw_sock, efd, &local_if, argv[2]);
        
	} else if (strcmp(argv[1], "-l") == 0) {
		/* LISTEN MODE - RECEIVER */
        
		receiver(raw_sock, efd, &local_if);
        
	} else {
		printf("Unknown option: %s\n", argv[1]);
		printf("Use -l for receiver or -c <filename> for sender\n");
		exit(EXIT_FAILURE);
	}
    
	close(raw_sock);
	close(efd);
    
	return 0;
}
