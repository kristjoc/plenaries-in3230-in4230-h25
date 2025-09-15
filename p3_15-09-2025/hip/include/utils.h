#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>		/* uint8_t */
#include <unistd.h>		/* size_t */
#include <linux/if_packet.h>	/* struct sockaddr_ll */

#define MAX_EVENTS 10
#define MAX_IF     3

struct ifs_data {
	struct sockaddr_ll addr[MAX_IF];
	int rsock;
	uint8_t local_hip_addr;
	int ifn;
};

void get_mac_from_ifaces(struct ifs_data *);
void print_mac_addr(uint8_t *, size_t);
void init_ifs(struct ifs_data *, int);
int create_raw_socket(void);
int epoll_add_sock(int);
int send_hip_packet(struct ifs_data *, uint8_t *, uint8_t *, uint8_t, uint8_t,
		    const char *);
int handle_hip_packet(struct ifs_data *, const char *);

#endif /* _UTILS_H */
