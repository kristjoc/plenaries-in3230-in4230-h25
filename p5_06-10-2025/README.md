# Plenary Session 5 - 06.10.2025 #

During this session, we discussed Home Exam 1 in detail and covered
the new concepts it introduces. In Home Exam 1, we build on the
network stack we've been developing throughout the course by
implementing a routing daemon. This daemon will provide **forwarding**
and **routing** services to the MIP daemon.

Using a similar topology as in Home Exam 1, we demonstrated step by step how a
routing daemon can:

- discover neighbors using broadcast HELLO messages
- build a routing table and share its routes by unicasting UPDATE
  messages to neighbors
- detect link failures in the network

The routing daemon should run a simple Distance Vector Routing (DVR)
protocol, with Poisoned Reverse loop protection (more about this in
the next session). An example of DVR protocol is RIPv1, which due to
its limitations is superseeded by RIPv2.

Read more about RIP (Routing Information Protocol)
[here](https://en.wikipedia.org/wiki/Routing_Information_Protocol).


## Finite State Machine for Distance Vector Routing: Neighbor Discovery and Timeouts ##

A Routing daemon that runs a Routing Protocol can be modeled as a
Finite State Machine. The FSM can change from one state to another in
response to some inputs; the change from one state to another is
called a transition. Examples of transitions in our Home Exam 1 are
when a HELLO message is sent to all neighbors or when a neighbor is
down and the Routing table needs to be updated. In this example
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p5_06-10-2025/dvr-fsm),
we implement a simple FSM-based protocol where each node periodically
broadcasts a "HELLO" message over Ethernet every 5 seconds, using a
single raw socket and sending from all detected network interfaces.
The protocol maintains a list of neighboring nodes by tracking their
MAC addresses and the last time a HELLO message was received from
each. If no HELLO is received from a neighbor for two consecutive
intervals (10 seconds), that neighbor is marked as disconnected.

### Usage Instructions ###

** Compile the project**

```bash
cd dvr-fsm && make all
```

**Start the topology with:**

```bash
sudo -E mn --mac --custom plenary-topo.py --topo p5 --link tc

# Launch xterms for each host
xterm A B C
```

This creates three hosts (`A`, `B`, `C`) interconnected with
point-to-point links, each with 10ms delay and no loss by default.

**Running the Protocol**

2. **Start the protocol on each xterm:**
   ```bash
   sudo ./fsm
   ```

Each host will broadcast "HELLO" messages and maintain a neighbor
table, printing state changes and information about seen neighbors.

### Testing Under Network Impairments

You may use Mininet's link parameters to introduce delay and loss. For
example, to simulate a 33% packet loss and 2s delay on a link (RTT
~4s):

```bash
# In Mininet CLI:
link A B loss 33 delay 2000ms
```
This will help you observe how the protocol detects dead neighbors
when "HELLO" packets cannot make it.


## libQueue ##

One requirement for the MIP and Routing daemons is that they MUST operate
asynchronously, which means that they must not block the execution flow while
waiting for response packets. In order to achieve this, one may combine several
methods like non-blocking sockets, epoll() or queues.
[Here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p5_06-10-2025/libqueue)
you may find a complete implementation of a queue library that can be used in
your Home Exams. When the MIP daemon receives a MIP packet that needs to be
forwarded, it can store the packet in the queue, initiate a route lookup using
the forwarding engine, and then retrieve that packet from the queue. At the same
time, MIP daemon MUST be able to process incoming and send outgoing MIP packets.
