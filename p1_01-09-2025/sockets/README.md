# UNIX and RAW scokets #

In order to start with coding the MIP daemon and Ping applications for
the mandatory assignment, it is essential to understand the following
basic concepts:

* UNIX sockets
* epoll()
* RAW sockets

## UNIX sockets ##

The following [example](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p1_01-09-2025/sockets/unix_sockets) about UNIX sockets contains the implementation
of a client/server chat application that uses these sockets to
transmit data. UNIX sockets will serve as the interface between the
Ping applications and MIP daemons in the Oblig.

## epoll() ##

`epoll`, a powerful tool for monitoring multiple file descriptors to
determine whether a `read()` operation can be performed on any of
them. The use of `epoll` will be crucial in the Oblig and Home Exams,
enabling the daemons to asynchronously monitor both UNIX and RAW
sockets for incoming data.

Here is an example from `man epoll`:

```
#define MAX_EVENTS 10
           struct epoll_event ev, events[MAX_EVENTS];
           int listen_sock, conn_sock, nfds, epollfd;

           /* Code to set up listening socket, 'listen_sock',
              (socket(), bind(), listen()) omitted. */

           epollfd = epoll_create1(0);
           if (epollfd == -1) {
               perror("epoll_create1");
               exit(EXIT_FAILURE);
           }

           ev.events = EPOLLIN;
           ev.data.fd = listen_sock;
           if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
               perror("epoll_ctl: listen_sock");
               exit(EXIT_FAILURE);
           }

           for (;;) {
               nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
               if (nfds == -1) {
                   perror("epoll_wait");
                   exit(EXIT_FAILURE);
               }

               for (n = 0; n < nfds; ++n) {
                   if (events[n].data.fd == listen_sock) {
                       conn_sock = accept(listen_sock,
                                          (struct sockaddr *) &addr, &addrlen);
                       if (conn_sock == -1) {
                           perror("accept");
                           exit(EXIT_FAILURE);
                       }
                       ev.events = EPOLLIN | EPOLLET;
                       ev.data.fd = conn_sock;
                       if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                                   &ev) == -1) {
                           perror("epoll_ctl: conn_sock");
                           exit(EXIT_FAILURE);
                       }
                   } else {
                       do_use_fd(events[n].data.fd);
                   }
               }
           }
```

## RAW sockets ##

This
[example](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p1_01-09-2025/sockets/raw_sockets)
provides the source code for sender and receiver applications that
send and receive data via RAW sockets. The code includes
implementations of some utility functions such as `print_mac_addr()`,
`get_mac_from_interface()`, and `send/recv_raw_packets()` that you can
use in your own implementation of the MIP daemon.
