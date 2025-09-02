# RAW sockets

In this example, we will implement a sender and receiver applications
that use RAW sockets to communicate to each other in a chat session.

`topo_p2p.py` is the python script that generates the mininet topology.

`epoll` will be used to monitor the activity of stdin and raw socket descriptor

## Usage

- Compile all programmes with `make all` in the current directory
- Create the mininet topology using `sudo -E mn --mac --custom topo_p2p.py --topo mytopo`
- In the mininet console, access node h1 and h2 using `xterm h1 h2`
  (You should have used -Y argument in the ssh command: `ssh -Y debian@ip_address_of_your_VM`
- From the xterm consoles, run `./receiver` at node h2 and `./sender` at node h1

  Practise and implement new features in the applications.

