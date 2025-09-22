#ifndef _PDU_H_
#define _PDU_H_

#include <stdint.h>
#include <stddef.h>

#include "ether.h"
#include "hip.h"

#define HIP_HDR_LEN	sizeof(struct hip_hdr)
#define MAX_BUF_SIZE	1024

struct pdu {
	struct eth_hdr *ethhdr;
	struct hip_hdr *hiphdr;
	uint8_t        *sdu;
} __attribute__((packed));

struct pdu * alloc_pdu(void);
void fill_pdu(struct pdu *,
	      uint8_t *,
	      uint8_t *,
	      uint8_t,
	      uint8_t,
	      const char *,
              uint8_t);
size_t hip_serialize_pdu(struct pdu *, uint8_t *);
size_t hip_deserialize_pdu(struct pdu *, uint8_t *);
void print_pdu_content(struct pdu *);
void destroy_pdu(struct pdu *);

#endif /* _PDU_H_ */
