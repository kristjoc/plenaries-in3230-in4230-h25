#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include <stdint.h>

#define ETH_DST_MAC {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} // broadcast MAC addr.
#define ETH_P_HIP 0xBB88
#define ETH_HDR_LEN sizeof(struct eth_hdr)

struct eth_hdr {
	uint8_t  dst_mac[6];
	uint8_t  src_mac[6];
	uint16_t ethertype;
} __attribute__((packed));

#endif /* _ETHERNET_H_*/
