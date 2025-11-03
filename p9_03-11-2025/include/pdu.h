#ifndef _PDU_H_
#define _PDU_H_

#include <stdint.h>
#include <stddef.h>

#include "ether.h"
#include "hip.h"
#include "rtp.h"

#define ETH_HDR_LEN     sizeof(struct eth_hdr)
#define HIP_HDR_LEN     sizeof(struct hip_hdr)
#define RTP_HDR_LEN     sizeof(struct rtp_hdr)
#define MAX_BUF_SIZE    1500
#define MAX_SDU_SIZE    (MAX_BUF_SIZE - ETH_HDR_LEN - HIP_HDR_LEN - RTP_HDR_LEN)

struct pdu {
	struct eth_hdr *ethhdr;
	struct hip_hdr *hiphdr;
	struct rtp_hdr *rtphdr;
	uint8_t        *sdu;
} __attribute__((packed));

/* PDU management */
struct pdu * alloc_pdu(void);
void destroy_pdu(struct pdu *);

/* Fill PDU */
void fill_pdu(struct pdu *pdu,
              uint8_t *src_mac_addr,
              uint8_t *dst_mac_addr,
              uint8_t src_hip_addr,
              uint8_t dst_hip_addr,
              uint8_t rtp_type,
              uint32_t seq_num,
              const uint8_t *payload,
              size_t payload_len);

/* Serialization */
size_t serialize_pdu(struct pdu *pdu, uint8_t *buf);
size_t deserialize_pdu(struct pdu *pdu, uint8_t *buf);

/* Debug */
void print_pdu_content(struct pdu *);

#endif /* _PDU_H_ */
