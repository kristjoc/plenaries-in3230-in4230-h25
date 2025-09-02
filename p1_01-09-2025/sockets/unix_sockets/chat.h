/*
 * File: chat.h
 *
 * Good practises for C header files.
 * 1. ALWAYS #include guards!
 * 2. #include only necessary libraries!
 * 3. DO NOT define global variables!
 *	  Are you sure you need them? There might be a better way.
 *	  If you still need them, make sure to employ the `extern` keyword.
 * 4. #define MACROs and declare functions prototype to be shared between
 *	  .c source files.
 *
 * Check https://valecs.gitlab.io/resources/CHeaderFileGuidelines.pdf for some
 * more nice practises.
 */

/*
 * #ifndef and #define are known as header guards.
 * Their primary purpose is to prevent header files
 * from being included multiple times.
 */

#ifndef _CHAT_H
#define _CHAT_H

/* MACROs */
#define SOCKET_NAME "server.socket"
#define MAX_CONNS 5   /* max. length of the pending connections queue */
#define MAX_EVENTS 10 /* max. number of concurrent events to check */

/*
 * We declare the signature of a function in the header file
 * and its definition in the source file.
 *
 * return_type function_name(parameter1, parameter2, ...);
 */

void server(void);
void client(void);

#endif /* _CHAT_H */
