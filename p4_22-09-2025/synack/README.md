# Wireshark Plugin Example - HiP Protocol

In this example, we're going to test the Wireshark plugin of a simple
greeting protocol called HiP - Hi Protocol.

## "HiP"

`topo_p2p.py` is the python script that generates the following mininet
topology:

    [NodeA]ifAB------------ifBA[NodeB]

NodeA will send a broadcast HiP packet of type HIP\_TYPE\_SYN via the RAW socket
and NodeB will reply via a unicast HiP packet of type HIP\_TYPE\_SYNACK.

Usage:

- Compile all programmes with `make all` in the current directory.
- Create the mininet topology using `sudo mn --custom topo_p2p.py --topo mytopo`
- In the mininet console, launch the xterms of node A and B using `xterm A B`  
  (You should have used -Y argument in the SSH command:
  `ssh -Y debian@ip_address_of_your_VM`
- Launch another `xterm A` from the mininet console, run `wireshark` and capture
  traffic on `A-eth0` interface
- From the xterm consoles, `cd` to `bin` directory and run `./hip s` at node B and
  `./hip c hello` at node A
