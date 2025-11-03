#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <linux/if_packet.h>

#define MAX_IFS 10

/* Interface data structure */
struct ifs_data {
	struct sockaddr_ll addr[MAX_IFS];
	int      ifn;              /* Number of interfaces */
	int      rsock;            /* Raw socket */
	uint8_t  local_hip_addr;   /* Local HIP address */
};

/* Socket and interface utilities */
int create_raw_socket(void);
void init_ifs(struct ifs_data *ifs, int rsock);
void get_mac_from_ifaces(struct ifs_data *ifs);
int epoll_add_sock(int sock);

/* Helper utilities */
void print_mac_addr(const uint8_t *mac, size_t len);

#endif /* _UTILS_H_ */
