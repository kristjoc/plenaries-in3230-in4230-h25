#ifndef _RTP_H_
#define _RTP_H_

#include <stdint.h>
#include <stdio.h>

/* Forward declarations */
struct ifs_data;
struct pdu;

#define RTP_HDR_LEN     sizeof(struct rtp_hdr)
#define RTP_VERSION     1
#define WINDOW_SIZE     8
#define MAX_PAYLOAD     1024

/* Packet types */
#define RTP_TYPE_DATA   0x1
#define RTP_TYPE_ACK    0x2

struct rtp_hdr {
	uint8_t  version : 4;
	uint8_t  type : 4;
	uint32_t seq_num;
	uint16_t payload_len;
} __attribute__((packed));

/* Packet in retransmission queue */
struct rtp_packet {
	uint8_t  data[MAX_PAYLOAD];
	uint16_t len;
	uint32_t seq_num;
};

/* Sender state */
struct sender_state {
	FILE     *fp;
	uint32_t oldest_unacked_seq;  /* Oldest unacknowledged sequence number */
	uint32_t next_seq;            /* Next sequence number to send */
	struct rtp_packet retx_queue[WINDOW_SIZE];  /* Retransmission queue */
	int      queue_size;
};

/* Receiver state */
struct receiver_state {
	FILE     *fp;
	uint32_t expected_seq;
};

/* Function declarations */
int sender_init(struct sender_state *state, const char *filename);
int receiver_init(struct receiver_state *state, const char *filename);

int send_next_packet(int sock, struct ifs_data *ifd, struct sender_state *state,
                     uint8_t *dst_mac, uint8_t dst_hip);
void handle_ack(struct sender_state *state, uint32_t ack_num);
int go_back_n(int sock, struct ifs_data *ifd, struct sender_state *state,
              uint8_t *dst_mac, uint8_t dst_hip);

void send_ack(int sock, struct ifs_data *ifd, uint32_t seq_num,
              uint8_t *dst_mac, uint8_t dst_hip);
void handle_data(int sock, struct ifs_data *ifd, struct receiver_state *state,
                 struct pdu *pdu);

#endif /* _RTP_H_ */
