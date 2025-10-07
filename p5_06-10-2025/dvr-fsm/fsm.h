#ifndef DVR_FSM_H
#define DVR_FSM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <time.h>

#define HELLO_INTERVAL 5    // Send hello every 5 seconds
#define TIMEOUT_COUNT 2     // Consider neighbor dead after 2 missed hellos
#define MAX_NEIGHBORS 20    // Maximum number of neighbors
#define MAX_INTERFACES 10   // Maximum number of interfaces
#define BUFFER_SIZE 1024    // Buffer for receiving messages
#define BROADCAST_MAC_ADDR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

// FSM States
typedef enum {
	INIT,
	CONNECTED,
	DISCONNECTED
} node_state_t;

// Ethernet frame header
struct ether_frame {
	uint8_t dst_addr[6];
	uint8_t src_addr[6];
	uint8_t eth_proto[2];
} __attribute__((packed));

// Interface structure
typedef struct {
	int ifindex;           // Interface index
	uint8_t mac_addr[6];   // MAC address of interface
} interface_t;

// Neighbor structure
typedef struct {
	uint8_t mac_addr[6];      // MAC address of neighbor
	time_t last_hello_time;   // Last time hello was received
	int missed_hellos;        // Count of missed hellos
	node_state_t state;       // Current state of this neighbor
} neighbor_t;

// Function declarations
int send_raw_packet(int sd, struct sockaddr_ll *so_name, uint8_t *buf, size_t len);
int recv_raw_packet(int sd, uint8_t *buf, size_t len);
void print_mac_addr(uint8_t *addr, int len);
int discover_interfaces(interface_t *interfaces);
void send_hello_to_all_interfaces(int raw_sock, interface_t *interfaces, int num_interfaces);
int find_or_add_neighbor(neighbor_t *neighbors, int *num_neighbors, uint8_t *mac_addr);
void check_neighbor_timeouts(neighbor_t *neighbors, int num_neighbors);
void print_neighbors(neighbor_t *neighbors, int num_neighbors);

#endif /* DVR_FSM_H */
