#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "ether.h"
#include "hip.h"
#include "rtp.h"
#include "pdu.h"
#include "utils.h"

struct pdu * alloc_pdu(void)
{
	struct pdu *pdu = (struct pdu *)malloc(sizeof(struct pdu));
	
	pdu->ethhdr = (struct eth_hdr *)malloc(sizeof(struct eth_hdr));
	pdu->ethhdr->ethertype = htons(ETH_P_HIP);
	
	pdu->hiphdr = (struct hip_hdr *)malloc(sizeof(struct hip_hdr));
	pdu->hiphdr->dst = HIP_DST_ADDR;
	pdu->hiphdr->src = HIP_DST_ADDR;
	pdu->hiphdr->len = (1u<<8) - 1;
	pdu->hiphdr->version = HIP_VERSION;
	pdu->hiphdr->type = HIP_TYPE_HI;

	pdu->rtphdr = (struct rtp_hdr *)malloc(sizeof(struct rtp_hdr));
	pdu->rtphdr->version = RTP_VERSION;
	pdu->rtphdr->type = RTP_TYPE_DATA;
	pdu->rtphdr->seq_num = 0;
	pdu->rtphdr->payload_len = 0;

	pdu->sdu = NULL;

	return pdu;
}

void fill_pdu(struct pdu *pdu,
	      uint8_t *src_mac_addr,
	      uint8_t *dst_mac_addr,
	      uint8_t src_hip_addr,
	      uint8_t dst_hip_addr,
	      uint8_t rtp_type,
	      uint32_t seq_num,
	      const uint8_t *payload,
	      size_t payload_len)
{
	size_t sdu_len = 0;
	
	/* Fill Ethernet header */
	memcpy(pdu->ethhdr->dst_mac, dst_mac_addr, 6);
	memcpy(pdu->ethhdr->src_mac, src_mac_addr, 6);
	pdu->ethhdr->ethertype = htons(ETH_P_HIP);
	
	/* Fill HIP header */
	pdu->hiphdr->dst = dst_hip_addr;
	pdu->hiphdr->src = src_hip_addr;
	pdu->hiphdr->version = HIP_VERSION;
	pdu->hiphdr->type = HIP_TYPE_HI;  /* Using HI type for RTP */

	/* Calculate HIP length (RTP header + payload, divisible by 4) */
	sdu_len = RTP_HDR_LEN + payload_len;
	if (sdu_len % 4 != 0)
		sdu_len = sdu_len + (4 - (sdu_len % 4));
	pdu->hiphdr->len = sdu_len / 4;

	/* Fill RTP header */
	pdu->rtphdr->version = RTP_VERSION;
	pdu->rtphdr->type = rtp_type;
	pdu->rtphdr->seq_num = htonl(seq_num);
	pdu->rtphdr->payload_len = htons(payload_len);

	/* Fill payload */
	if (payload_len > 0 && payload != NULL) {
		pdu->sdu = (uint8_t *)calloc(1, payload_len);
		memcpy(pdu->sdu, payload, payload_len);
	} else {
		pdu->sdu = NULL;
	}
}

size_t serialize_pdu(struct pdu *pdu, uint8_t *snd_buf)
{
	size_t snd_len = 0;

	/* Copy Ethernet header */
	memcpy(snd_buf + snd_len, pdu->ethhdr, ETH_HDR_LEN);
	snd_len += ETH_HDR_LEN;

	/* Pack and copy HIP header */
	uint32_t hiphdr = 0;
	hiphdr |= (uint32_t) pdu->hiphdr->dst << 24;
	hiphdr |= (uint32_t) pdu->hiphdr->src << 16;
	hiphdr |= (uint32_t) (pdu->hiphdr->len & 0xff) << 8;
	hiphdr |= (uint32_t) (pdu->hiphdr->version & 0xf) << 4;
	hiphdr |= (uint32_t) (pdu->hiphdr->type & 0xf);
	hiphdr = htonl(hiphdr);
	memcpy(snd_buf + snd_len, &hiphdr, HIP_HDR_LEN);
	snd_len += HIP_HDR_LEN;

	/* Pack and copy RTP header */
	uint64_t rtphdr = 0;
	rtphdr |= (uint64_t) (pdu->rtphdr->version & 0xf) << 44;
	rtphdr |= (uint64_t) (pdu->rtphdr->type & 0xf) << 40;
	rtphdr |= (uint64_t) ntohl(pdu->rtphdr->seq_num) << 16;
	rtphdr |= (uint64_t) ntohs(pdu->rtphdr->payload_len);
	
	/* Convert to network byte order (6 bytes for RTP header) */
	uint8_t rtp_bytes[8];
	rtp_bytes[0] = (pdu->rtphdr->version << 4) | (pdu->rtphdr->type & 0xf);
	uint32_t seq = ntohl(pdu->rtphdr->seq_num);
	rtp_bytes[1] = (seq >> 16) & 0xff;
	rtp_bytes[2] = (seq >> 8) & 0xff;
	rtp_bytes[3] = seq & 0xff;
	uint16_t plen = ntohs(pdu->rtphdr->payload_len);
	rtp_bytes[4] = (plen >> 8) & 0xff;
	rtp_bytes[5] = plen & 0xff;
	
	memcpy(snd_buf + snd_len, rtp_bytes, RTP_HDR_LEN);
	snd_len += RTP_HDR_LEN;

	/* Attach payload */
	uint16_t payload_len = ntohs(pdu->rtphdr->payload_len);
	if (payload_len > 0 && pdu->sdu != NULL) {
		memcpy(snd_buf + snd_len, pdu->sdu, payload_len);
		snd_len += payload_len;
	}

	/* Pad to make total HIP SDU length divisible by 4 */
	size_t expected_len = ETH_HDR_LEN + HIP_HDR_LEN + (pdu->hiphdr->len * 4);
	while (snd_len < expected_len) {
		snd_buf[snd_len++] = 0;
	}

	return snd_len;
}

size_t deserialize_pdu(struct pdu *pdu, uint8_t *rcv_buf)
{
	size_t rcv_len = 0;

	/* Unpack Ethernet header */
	pdu->ethhdr = (struct eth_hdr *)malloc(ETH_HDR_LEN);
	memcpy(pdu->ethhdr, rcv_buf + rcv_len, ETH_HDR_LEN);
	rcv_len += ETH_HDR_LEN;

	/* Unpack HIP header */
	pdu->hiphdr = (struct hip_hdr *)malloc(HIP_HDR_LEN);
	uint32_t *tmp = (uint32_t *) (rcv_buf + rcv_len);
	uint32_t header = ntohl(*tmp);
	pdu->hiphdr->dst = (uint8_t) (header >> 24);
	pdu->hiphdr->src = (uint8_t) (header >> 16);
	pdu->hiphdr->len = (size_t) ((header >> 8) & 0xff);
	pdu->hiphdr->version = (uint8_t) ((header >> 4) & 0xf);
	pdu->hiphdr->type = (uint8_t) (header & 0xf);
	rcv_len += HIP_HDR_LEN;

	/* Unpack RTP header */
	pdu->rtphdr = (struct rtp_hdr *)malloc(RTP_HDR_LEN);
	uint8_t *rtp_bytes = rcv_buf + rcv_len;
	pdu->rtphdr->version = (rtp_bytes[0] >> 4) & 0xf;
	pdu->rtphdr->type = rtp_bytes[0] & 0xf;
	pdu->rtphdr->seq_num = htonl((rtp_bytes[1] << 16) | (rtp_bytes[2] << 8) | rtp_bytes[3]);
	pdu->rtphdr->payload_len = htons((rtp_bytes[4] << 8) | rtp_bytes[5]);
	rcv_len += RTP_HDR_LEN;

	/* Unpack payload */
	uint16_t payload_len = ntohs(pdu->rtphdr->payload_len);
	if (payload_len > 0) {
		pdu->sdu = (uint8_t *)calloc(1, payload_len);
		memcpy(pdu->sdu, rcv_buf + rcv_len, payload_len);
	} else {
		pdu->sdu = NULL;
	}
	rcv_len += payload_len;

	/* Skip any padding */
	size_t expected_len = ETH_HDR_LEN + HIP_HDR_LEN + (pdu->hiphdr->len * 4);
	rcv_len = expected_len;

	return rcv_len;
}

void print_pdu_content(struct pdu *pdu)
{
	printf("====================================================\n");
	printf("\t Source MAC address: ");
	print_mac_addr(pdu->ethhdr->src_mac, 6);
	printf("\t Destination MAC address: ");
	print_mac_addr(pdu->ethhdr->dst_mac, 6);
	printf("\t Ethertype: 0x%04x\n", ntohs(pdu->ethhdr->ethertype));

	printf("\t Source HIP address: %u\n", pdu->hiphdr->src);
	printf("\t Destination HIP address: %u\n", pdu->hiphdr->dst);
	printf("\t HIP SDU length: %u\n", pdu->hiphdr->len * 4);
	printf("\t HIP protocol version: %u\n", pdu->hiphdr->version);
	printf("\t HIP PDU type: 0x%02x\n", pdu->hiphdr->type);

	printf("\t RTP version: %u\n", pdu->rtphdr->version);
	printf("\t RTP type: 0x%02x\n", pdu->rtphdr->type);
	printf("\t RTP sequence: %u\n", ntohl(pdu->rtphdr->seq_num));
	printf("\t RTP payload length: %u\n", ntohs(pdu->rtphdr->payload_len));

	if (pdu->sdu && ntohs(pdu->rtphdr->payload_len) > 0) {
		printf("\t Payload (first 32 bytes): ");
		size_t print_len = ntohs(pdu->rtphdr->payload_len);
		if (print_len > 32) print_len = 32;
		for (size_t i = 0; i < print_len; i++) {
			printf("%02x ", pdu->sdu[i]);
		}
		printf("\n");
	}
	printf("====================================================\n");
}

void destroy_pdu(struct pdu *pdu)
{
	free(pdu->ethhdr);
	free(pdu->hiphdr);
	free(pdu->rtphdr);
	if (pdu->sdu)
		free(pdu->sdu);
	free(pdu);
}
