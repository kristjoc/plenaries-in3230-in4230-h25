# Plenary session 6 - 13.10.2025 #

In this plenary session, we discussed BIRD, the BIRD Internet Routing
Daemon, which was originally developed as a school project at the
Faculty of Mathematics and Physics of Charles University in Prague.

## BIRD ##

The BIRD project aims to develop a fully functional dynamic IP routing daemon
primarily targeted on (but not limited to) Linux, FreeBSD and other UNIX-like
systems and distributed under the GNU General Public License. Read more about
BIRD [here](https://bird.network.cz/).  


Using
[this](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p6_13-10-2025/init_mn.py)
mininet script, we will build a BIRD topology as shown in
[this](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p6_13-10-2025/bird_topology.pdf)
figure.  

### Installation ###

To install `bird` in the NREC virtual machines use `sudo apt install
bird` command, which will install `BIRD version 1.6.8`. The config
files for r1, r2, and r3 are written for this version.  

### Usage ###

To run the mininet script use the following command:

```
   $ sudo -E python3 init_mn.py
```

Inside the mininet CLI, launch xterms of the nodes with `xterm A B r1
r2 r3`. Inside the xterms, one can connect to the BIRD controller
using `birdc -s bird.ctl` in the r1, r2, or r3 directories and apply
Cisco-like commands (`show routes`, `show rip/ospf neighbors`, etc.).  

Use `wireshark`, `tshark` or `tcpdump` to capture HELLO and UPDATEs packets of
the routing protocols.  

When the routing protocol has converged, you should be able to ping from A to B
`ping 162.168.3.100`.  

Last but not least, check the BIRD source code from its
[GitLab](https://gitlab.nic.cz/labs/bird) repository and get some inpiration for
your MIP routing daemon.  
