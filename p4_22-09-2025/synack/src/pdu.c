#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "ether.h"
#include "hip.h"
#include "pdu.h"
#include "utils.h"

struct pdu * alloc_pdu(void)
{
	struct pdu *pdu = (struct pdu *)malloc(sizeof(struct pdu));

	pdu->ethhdr = (struct eth_hdr *)malloc(sizeof(struct eth_hdr));
	pdu->ethhdr->ethertype = htons(0xFFFF);

	pdu->hiphdr = (struct hip_hdr *)malloc(sizeof(struct hip_hdr));
        pdu->hiphdr->dst = HIP_DST_ADDR;
        pdu->hiphdr->src = HIP_DST_ADDR ;
        pdu->hiphdr->len = (1u<<8) - 1;
        pdu->hiphdr->version = HIP_VERSION;
        pdu->hiphdr->type = HIP_TYPE_DATA;

	return pdu;
}

void fill_pdu(struct pdu *pdu,
	      uint8_t *src_mac_addr,
	      uint8_t *dst_mac_addr,
	      uint8_t src_hip_addr,
	      uint8_t dst_hip_addr,
	      const char *sdu,
	      uint8_t pkt_type)
{
	size_t slen = 0;

	memcpy(pdu->ethhdr->dst_mac, dst_mac_addr, 6);
	memcpy(pdu->ethhdr->src_mac, src_mac_addr, 6);
	pdu->ethhdr->ethertype = htons(ETH_P_HIP);

        pdu->hiphdr->dst = dst_hip_addr;
        pdu->hiphdr->src = src_hip_addr;
        pdu->hiphdr->version = HIP_VERSION;

	if (!pkt_type)
		pdu->hiphdr->type = HIP_TYPE_SYN;
	else if (pkt_type == 1)
		pdu->hiphdr->type = HIP_TYPE_SYNACK;
	else
		pdu->hiphdr->type = HIP_TYPE_DATA;

	slen = strlen(sdu) + 1;

	/* SDU length must be divisible by 4 */
	if (slen % 4 != 0)
		slen = slen + (4 - (slen % 4));

	/* to get the real SDU length in bytes, the len value is multiplied by 4 */
        pdu->hiphdr->len = slen / 4;

	pdu->sdu = (uint8_t *)calloc(1, slen);
	memcpy(pdu->sdu, sdu, slen);
}

size_t hip_serialize_pdu(struct pdu *pdu, uint8_t *snd_buf)
{
	size_t snd_len = 0;

	/* Copy ethernet header */
	memcpy(snd_buf + snd_len, pdu->ethhdr, sizeof(struct eth_hdr));
	snd_len += ETH_HDR_LEN;

	/* Copy HIP header */
	uint32_t hiphdr = 0;
	hiphdr |= (uint32_t) pdu->hiphdr->dst << 24;
	hiphdr |= (uint32_t) pdu->hiphdr->src << 16;
	hiphdr |= (uint32_t) (pdu->hiphdr->len & 0xff) << 8;
	hiphdr |= (uint32_t) (pdu->hiphdr->version & 0xf) << 4;
	hiphdr |= (uint32_t) (pdu->hiphdr->type & 0xf);

	/* prepare it to be sent from host to network */
	hiphdr = htonl(hiphdr);

	memcpy(snd_buf + snd_len, &hiphdr, HIP_HDR_LEN);
	snd_len += HIP_HDR_LEN;

	/* Attach SDU */
	memcpy(snd_buf + snd_len, pdu->sdu, pdu->hiphdr->len * 4);
	snd_len += pdu->hiphdr->len * 4;

	return snd_len;
}

size_t hip_deserialize_pdu(struct pdu *pdu, uint8_t *rcv_buf)
{
	/* pdu = (struct pdu *)malloc(sizeof(struct pdu)); */
	size_t rcv_len = 0;

	/* Unpack ethernet header */
	pdu->ethhdr = (struct eth_hdr *)malloc(ETH_HDR_LEN);
	memcpy(pdu->ethhdr, rcv_buf + rcv_len, ETH_HDR_LEN);
	rcv_len += ETH_HDR_LEN;

	pdu->hiphdr = (struct hip_hdr *)malloc(HIP_HDR_LEN);
	uint32_t *tmp = (uint32_t *) (rcv_buf + rcv_len);
	uint32_t header = ntohl(*tmp);
	pdu->hiphdr->dst = (uint8_t) (header >> 24);
	pdu->hiphdr->src = (uint8_t) (header >> 16);
	pdu->hiphdr->len = (size_t) (((header >> 8) & 0xff));
	pdu->hiphdr->version = (uint8_t) ((header >> 4) & 0xf);
	pdu->hiphdr->type = (uint8_t) (header & 0xf);
	rcv_len += HIP_HDR_LEN;

	pdu->sdu = (uint8_t *)calloc(1, pdu->hiphdr->len * 4);
	memcpy(pdu->sdu, rcv_buf + rcv_len, pdu->hiphdr->len * 4);
	rcv_len += pdu->hiphdr->len * 4;

	return rcv_len;
}

void print_pdu_content(struct pdu *pdu)
{
	printf("====================================================\n");
	printf("\t Source MAC address: ");
	print_mac_addr(pdu->ethhdr->src_mac, 6);
	printf("\t Destination MAC address: ");
	print_mac_addr(pdu->ethhdr->dst_mac, 6);
	printf("\t Ethertype: 0x%04x\n", pdu->ethhdr->ethertype);

	printf("\t Source HIP address: %u\n", pdu->hiphdr->src);
	printf("\t Destination HIP address: %u\n", pdu->hiphdr->dst);
	printf("\t SDU length: %d\n", pdu->hiphdr->len * 4);
	printf("\t HIP protocol version: %u\n", pdu->hiphdr->version);
	printf("\t PDU type: 0x%02x\n", pdu->hiphdr->type);

	printf("\t SDU: %s\n", pdu->sdu);
	printf("====================================================\n");
}

void destroy_pdu(struct pdu *pdu)
{
	free(pdu->ethhdr);
	free(pdu->hiphdr);
	free(pdu->sdu);
	free(pdu);
}
