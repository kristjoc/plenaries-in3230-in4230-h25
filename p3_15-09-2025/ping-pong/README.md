## "PING - PONG"

In this example, we're going to send PING/PONG messages between two
neighbors via RAW sockets.

`topo_p2p.py` is the python script that generates the mininet topology.
The topology is the most basic one, a point-to-point topology.

    [NodeA]ifAB------------ifBA[NodeB]

NodeA and NodeB will perform a simple handshake of ping pong messages.
epoll() will be used to monitor the activity of the RAW socket descriptors.

While NodeB will be listening to a RAW socket, NodeA will send a PING message
encapsulated in a simple hello packet which has its own hello header. NodeB will
receive the packet and reply back with a PONG message to nodeA.

Usage:

- Compile all programmes with `make all` in the current directory
- Create the mininet topology using `sudo mn --custom topo_p2p.py --topo mytopo`
- In the mininet console, access node A and B using `xterm A B`
  (You should have used -Y argument in the SSH command:
  `ssh -Y debian@ip_address_of_your_VM`
- From the xterm consoles, run `./nodeB` at node B and `./nodeA` at node A.

  Practise and implement new features in the applications.

  All the best! :-)
