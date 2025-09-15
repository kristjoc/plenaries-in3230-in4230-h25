#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>		/* uint8_t */
#include <unistd.h>		/* size_t */
#include <linux/if_packet.h>	/* struct sockaddr_ll */

#define MAX_EVENTS 10
#define MAX_IF     3
#define ETH_MAC_A {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
#define ETH_MAC_B {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}
#define MIP_ADDR_A 10
#define MIP_ADDR_B 20


struct ether_frame {
	uint8_t dst_addr[6];
	uint8_t src_addr[6];
	uint8_t eth_proto[2];
	uint8_t contents[0];
} __attribute__((packed));

struct hello_header {
	uint8_t dest;
	uint8_t src;
	uint8_t sdu_len : 4;
} __attribute__((packed));

struct ifs_data {
	struct sockaddr_ll addr[MAX_IF];
	int rsock;
	uint8_t local_mip_addr;
	int ifn;
};

void get_mac_from_interfaces(struct ifs_data *);
void print_mac_addr(uint8_t *, size_t);
void init_ifs(struct ifs_data *, int, uint8_t);
int create_raw_socket(void);
int send_mip_packet(struct ifs_data *, uint8_t *, uint8_t, uint8_t *);
int handle_mip_packet(struct ifs_data *);

#endif
