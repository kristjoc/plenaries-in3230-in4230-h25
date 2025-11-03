#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils.h"
#include "ether.h"
#include "hip.h"
#include "rtp.h"
#include "pdu.h"

/* Initialize sender */
int sender_init(struct sender_state *state, const char *filename)
{
	memset(state, 0, sizeof(*state));

	state->fp = fopen(filename, "rb");
	if (!state->fp) {
		perror("fopen");
		return -1;
	}

	state->oldest_unacked_seq = 0;
	state->next_seq = 0;
	state->queue_size = 0;

	return 0;
}

/* Initialize receiver */
int receiver_init(struct receiver_state *state, const char *filename)
{
	memset(state, 0, sizeof(*state));

	state->fp = fopen(filename, "wb");
	if (!state->fp) {
		perror("fopen");
		return -1;
	}

	state->expected_seq = 0;

	return 0;
}

/* Send next packet from file */
int send_next_packet(int sock, struct ifs_data *ifd, struct sender_state *state,
                     uint8_t *dst_mac, uint8_t dst_hip)
{
	/* Check if window is full */
	if (state->queue_size >= WINDOW_SIZE)
		return 0;

	/* Read chunk from file */
	struct rtp_packet *pkt = &state->retx_queue[state->queue_size];
	size_t bytes = fread(pkt->data, 1, MAX_PAYLOAD, state->fp);

	if (bytes == 0)
		return 0; /* No more data */

	pkt->len = bytes;
	pkt->seq_num = state->next_seq;

	/* Create and fill PDU */
	struct pdu *pdu = alloc_pdu();
	fill_pdu(pdu, ifd->addr[0].sll_addr, dst_mac, ifd->local_hip_addr, dst_hip,
		 RTP_TYPE_DATA, pkt->seq_num, pkt->data, pkt->len);

	/* Serialize and send */
	uint8_t buf[MAX_BUF_SIZE];
	size_t len = serialize_pdu(pdu, buf);

	ssize_t sent = sendto(sock, buf, len, 0,
                              (struct sockaddr *)&ifd->addr[0], sizeof(struct sockaddr_ll));

	destroy_pdu(pdu);

	if (sent < 0) {
		perror("sendto");
		return -1;
	}

	printf("[SEND] seq=%u, len=%u bytes\n", pkt->seq_num, pkt->len);

	state->next_seq++;
	state->queue_size++;

	return 1;
}

/* Handle received ACK */
void handle_ack(struct sender_state *state, uint32_t ack_num)
{
	printf("[ACK] received for seq=%u (oldest_unacked=%u)\n", ack_num, state->oldest_unacked_seq);

	/* Check if ACK is in valid range */
	if (ack_num >= state->oldest_unacked_seq && ack_num < state->next_seq) {
		/* Number of packets acknowledged */
		uint32_t num_acked = ack_num - state->oldest_unacked_seq + 1;

		/* Remove ACKed packets from retransmission queue */
		if (num_acked <= state->queue_size) {
			/* Shift remaining packets in queue */
			for (int i = 0; i < state->queue_size - num_acked; i++) {
				state->retx_queue[i] = state->retx_queue[i + num_acked];
			}
			state->queue_size -= num_acked;

			/* Slide window forward */
			state->oldest_unacked_seq = ack_num + 1;
		}
	}
}

/* Go-Back-N: retransmit all packets in queue */
int go_back_n(int sock, struct ifs_data *ifd, struct sender_state *state,
              uint8_t *dst_mac, uint8_t dst_hip)
{
	printf("[GO-BACK-N] Retransmitting %d packets from seq=%u\n",
               state->queue_size, state->oldest_unacked_seq);

	/* Retransmit all packets in the retransmission queue */
	for (int i = 0; i < state->queue_size; i++) {
		struct rtp_packet *pkt = &state->retx_queue[i];

		/* Create and fill PDU */
		struct pdu *pdu = alloc_pdu();
		fill_pdu(pdu, ifd->addr[0].sll_addr, dst_mac, ifd->local_hip_addr, dst_hip,
			 RTP_TYPE_DATA, pkt->seq_num, pkt->data, pkt->len);

		/* Serialize and send */
		uint8_t buf[MAX_BUF_SIZE];
		size_t len = serialize_pdu(pdu, buf);

		sendto(sock, buf, len, 0,
		       (struct sockaddr *)&ifd->addr[0], sizeof(struct sockaddr_ll));

		destroy_pdu(pdu);

		printf("[RETX] seq=%u\n", pkt->seq_num);
	}

	return 0;
}

/* Send ACK packet */
void send_ack(int sock, struct ifs_data *ifd, uint32_t seq_num,
              uint8_t *dst_mac, uint8_t dst_hip)
{
	/* Create ACK PDU */
	struct pdu *pdu = alloc_pdu();
	fill_pdu(pdu, ifd->addr[0].sll_addr, dst_mac, ifd->local_hip_addr, dst_hip,
		 RTP_TYPE_ACK, seq_num, NULL, 0);

	/* Serialize and send */
	uint8_t buf[MAX_BUF_SIZE];
	size_t len = serialize_pdu(pdu, buf);

	sendto(sock, buf, len, 0,
               (struct sockaddr *)&ifd->addr[0], sizeof(struct sockaddr_ll));

	destroy_pdu(pdu);

	printf("[ACK] sent for seq=%u\n", seq_num);
}

/* Handle received data packet */
void handle_data(int sock, struct ifs_data *ifd, struct receiver_state *state,
                 struct pdu *pdu)
{
	uint32_t seq = ntohl(pdu->rtphdr->seq_num);
	uint16_t len = ntohs(pdu->rtphdr->payload_len);

	printf("[RECV] seq=%u (expected=%u), len=%u\n", seq, state->expected_seq, len);

	if (seq == state->expected_seq) {
		/* In-order packet - accept it */

		/* Write payload to file */
		if (len > 0) {
			fwrite(pdu->sdu, 1, len, state->fp);
			fflush(state->fp);
		}

		/* Send ACK */
		send_ack(sock, ifd, seq, pdu->ethhdr->src_mac, pdu->hiphdr->src);

		/* Advance expected sequence */
		state->expected_seq++;

	} else {
		/* Out-of-order packet - discard and send duplicate ACK */
		printf("[OUT-OF-ORDER] expected=%u, got=%u\n", state->expected_seq, seq);

		if (state->expected_seq > 0) {
			/* Send ACK for last correctly received packet */
			send_ack(sock, ifd, state->expected_seq - 1,
				 pdu->ethhdr->src_mac, pdu->hiphdr->src);
		}
	}
}
