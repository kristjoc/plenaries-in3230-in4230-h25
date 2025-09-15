#ifndef _HIP_H_
#define _HIP_H_

#include <stdint.h>
#include <stddef.h>

#define HIP_HDR_LEN	sizeof(struct hip_hdr)

#define HIP_VERSION	4

#define HIP_TYPE_HI	0x1
#define HIP_TYPE_HEI	0x2
#define HIP_TYPE_HEIHEI	0x3
#define HIP_TYPE_HELLO	0x4
#define HIP_TYPE_ALOHA	0x5

#define HIP_DST_ADDR	0xff

struct hip_hdr {
	uint8_t dst : 8;
	uint8_t src : 8;
	size_t len : 8;
	uint8_t version : 4;
	uint8_t type : 4;
} __attribute__((packed));


#endif /* _HIP_H_ */
