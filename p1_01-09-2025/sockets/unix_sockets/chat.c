/*
 * File: chat.c
 */

#include <stdio.h>	/* standard input/output library functions */
#include <stdlib.h>	/* standard library definitions (macros) */
#include <unistd.h>	/* standard symbolic constants and types */
#include <string.h>	/* string operations (strncpy, memset..) */

#include <sys/epoll.h>	/* epoll */
#include <sys/socket.h>	/* sockets operations */
#include <sys/un.h>	/* definitions for UNIX domain sockets */

#include "chat.h"

/**
 * Prepare (create, bind, listen) the server listening socket
 *
 * Returns the file descriptor of the server socket.
 *
 * In C, you declare a function as static when you want to limit its scope to
 * the current source file. When a function is declared as static, it can only
 * be called from within the same source file where it is defined.
 */
static int prepare_server_sock(void)
{
	struct sockaddr_un addr;
	int sd = -1, rc = -1;

	/* 1. Create local UNIX socket of type SOCK_SEQPACKET. */

	/* Why SOCK_SEQPACKET?
	 * SOCK_SEQPACKET is a socket type that provides a reliable,
	 * connection-oriented, and sequenced data transmission mechanism.
	 *
	 * Reliability: SOCK_SEQPACKET sockets offer a reliable data
	 * transmission mechanism. It ensures that data sent from one end will
	 * be received in the same order by the other end.
	 *
	 * Connection-Oriented: SOCK_SEQPACKET sockets are connection-oriented.
	 * This means they establish a connection between two endpoints
	 * (typically through a three-way handshake) before data transmission
	 * begins. This connection setup helps in ensuring that data is sent and
	 * received reliably.
	 *
	 * Sequenced Data: In some scenarios, it's important that data is
	 * received in the order it was sent. For example, if you are
	 * transmitting parts of a file, you want them to be reassembled in the
	 * correct order. SOCK_SEQPACKET ensures this order.
	 *
	 * Guaranteed Message Boundaries: SOCK_SEQPACKET also maintains message
	 * boundaries. It means that if you send distinct messages, they will be
	 * received as distinct messages. This can simplify the message parsing
	 * on the receiving end.
	 */

	sd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (sd	== -1) {
		perror("socket()");
		return rc;
	}

	/*
	 * For portability clear the whole structure, since some
	 * implementations have additional (nonstandard) fields in
	 * the structure.
	 */

	memset(&addr, 0, sizeof(struct sockaddr_un));

	/* 2. Bind socket to socket name. */

	addr.sun_family = AF_UNIX;

	/* Why `-1`??? Check character arrays in C!
	 * 's' 'e' 'r' 'v' 'e' 'r' '.' 's' 'o' 'c' 'k' 'e' 't' '\0'
	 * sizeof() counts the null-terminated character ('\0') as well.
	 */
	strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

	/* Unlink the socket so that we can reuse the program.
	 * This is bad hack! Better solution with a lock file,
	 * or signal handling.
	 * Check https://gavv.github.io/articles/unix-socket-reuse
	 */
	unlink(SOCKET_NAME);

	rc = bind(sd, (const struct sockaddr *)&addr, sizeof(addr));
	if (rc == -1) {
		perror("bind");
		close(sd);
		return rc;
	}

	/*
	 * 3. Prepare for accepting incomming connections.
	 * The backlog size is set to MAX_CONNS.
	 * So while one request is being processed other requests
	 * can be waiting.
	 */

	rc = listen(sd, MAX_CONNS);
	if (rc == -1) {
		perror("listen()");
		close(sd);
		return rc;
	}

	return sd;
}

static int add_to_epoll_table(int efd, struct epoll_event *ev, int fd)
{
	int rc = 0;

	/* EPOLLIN event: The associated file is available for read(2)
	 * operations. Check man epoll in Linux for more useful events.
	 */
	ev->events = EPOLLIN;
	ev->data.fd = fd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, ev) == -1) {
		perror("epoll_ctl");
		rc = -1;
	}

	return rc;
}

static void handle_client(int fd)
{
	char buf[256];
	int rc;

	/* The memset() function fills the first 'sizeof(buf)' bytes
	 * of the memory area pointed to by 'buf' with the constant byte 0.
	 */
	memset(buf, 0, sizeof(buf));

	/* read() attempts to read up to 'sizeof(buf)' bytes from file
	 * descriptor fd into the buffer starting at buf.
	 */
	rc = read(fd, buf, sizeof(buf));
	if (rc <= 0) {
		close(fd);
		printf("<%d> left the chat...\n", fd);
		return;
	}

	printf("<%d>: %s\n", fd, buf);
}

void server(void)
{
	struct epoll_event ev, events[MAX_EVENTS];
	int	   sd, accept_sd, epollfd, rc;

	printf("\n*** IN3230 Multiclient chat server is running! ***\n"
	       "* Waiting for users to connect... *\n\n");

	/* Call the method to create, bind and listen to the server socket */
	sd = prepare_server_sock();

	/* Create the main epoll file descriptor */
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		close(sd);
		exit(EXIT_FAILURE);
	}

	/* Add the main listening socket to epoll table */
	rc = add_to_epoll_table(epollfd, &ev, sd);
	if (rc == -1) {
		close(sd);
		exit(EXIT_FAILURE);
	}

	while (1) {
		rc = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (rc == -1) {
			perror("epoll_wait");
			close(sd);
			exit(EXIT_FAILURE);
		} else if (events->data.fd == sd) {
			/* someone is trying to connect() to the main listening
			 * socket.
			 */
			accept_sd = accept(sd, NULL, NULL);
			if (accept_sd == -1) {
				perror("accept");
				continue;
			}

			printf("<%d> joined the chat...\n", accept_sd);

			/* Add the new socket to epoll table */
			rc = add_to_epoll_table(epollfd, &ev, accept_sd);
			if (rc == -1) {
				close(sd);
				exit(EXIT_FAILURE);
			}
		} else {
			/* existing user is trying to send data */
			handle_client(events->data.fd);
		}
	}

	close(sd);

	/* Unlink the socket. */
	unlink(SOCKET_NAME);

	exit(EXIT_SUCCESS);
}

void client(void)
{
	struct sockaddr_un addr;
	int	   sd, rc;
	char   buf[256];

	sd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (sd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

	rc = connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if ( rc < 0) {
		perror("connect");
		close(sd);
		exit(EXIT_FAILURE);
	}

	printf("\n*** WELCOME TO IN3230/4230 CHAT ***\n"
	       "* Please, Be Kind & Polite! *\n");

	do {
		memset(buf, 0, sizeof(buf));

		fgets(buf, sizeof(buf), stdin);

		rc = write(sd, buf, strlen(buf));
		if (rc < 0) {
			perror("write");
			close(sd);
			exit(EXIT_FAILURE);
		}
	} while (1);
}
