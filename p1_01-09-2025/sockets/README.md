# 2nd Plenary session: 14.09.2023 #

The second plenary session focused on the basic concepts one needs to know in
order to start coding for the MIP daemon and Ping applications of Home Exam 1.
These concepts include:

* UNIX sockets
* epoll()
* RAW sockets

Before diving into these concepts, we began our session by revisiting the
tutorial on launching and accessing a virtual machine in the NREC cloud.
Following this, we discussed MIP, a minimal network layer protocol that we will
use to build our simple network stack, as part of the course assignments. For
more dettails about MIP, read its specifications
[here](https://www.uio.no/studier/emner/matnat/ifi/IN3230/h23/hjemmeeksamen-1/ispec-mip-2023.txt).


## UNIX sockets ##

We first discussed UNIX sockets and reviewed the implementation of a
client/server chat application that uses these sockets to transmit data. UNIX
sockets will be used as the interface between the Ping applications and MIP
daemons in Home Exam 1. Check the code example
[here](https://codeberg.org/kr1stj0n/plenaries-in3230-in4230-h23/src/branch/main/p2_14-09-2023/unix_sockets).

## epoll() ##

Next, we talked about `epoll`, a powerful tool for monitoring multiple file
descriptors to determine whether a `read()` operation can be performed on any of
them. The use of `epoll` will be crucial in the Home Exams, enabling the
daemons to asynchronously monitor both UNIX and RAW sockets for incoming data.

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

Unfortunately, we didn't have enough time to look at RAW sockets. Nonetheless,
you can find the code and materials related to them
[here](https://codeberg.org/kr1stj0n/plenaries-in3230-in4230-h23/src/branch/main/p2_14-09-2023/raw_sockets)

This example provides the source code of a sender and receiver application that
send/receive data via RAW sockets. The code contains implementations of some
utility functions such as `print_mac_addr()`, `get_mac_from_interface()`, and
`send/recv_raw_packets()` that you can use in your own implementation of the MIP
daemon.

## Next plenary ##

If today's plenary might have been a bit confusing for some of you due to the
new concepts, fear not nor be discouraged.

In our next session, we will pick up where we left off with the RAW socket
example that we didn't have a chance to complete today. Additionally, we will
code a simple implementation of MIP-ARP using RAW sockets that helps three nodes
(A --- B --- C) to get to know each other. MIP-ARP is a crucial part of the MIP
daemon in Home Exam 1. Meanwhile, consider to check
<https://beej.us/guide/bgnet/>, which is a good resource about socket
programming in C.


Have a nice weekend and see you next Thursday!
