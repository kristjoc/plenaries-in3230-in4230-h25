#!/usr/bin/env python

""" mininet script to test IN3230/IN4230-H25 Oblig assignment"""

from mininet.topo import Topo

from mininet.cli import CLI
# from mininet.term import makeTerm
from mininet.term import tunnelX11
import os
import signal
import time

# Usage example:
#
# 1. First, run the following command to start mininet with this script:
# sudo mn --mac --custom oblig-mn-script-v3.py --topo oblig --link tc
#
# 2. Second, inside the mininet console run 'init_oblig'
#
# 3. Third, inside the mininet console run 'EOF' to gracefully kill
# the mininet console

class Oblig(Topo):
    "Simple topology for Oblig."

    def __init__(self):
        "Set up our custom topo."

        # Initialize topology
        Topo.__init__(self)

        # Create 3 hosts, A, B, and C..
        A = self.addHost('A')
        B = self.addHost('B')
        C = self.addHost('C')

        # Create p2p links between A - B and B - C.
        self.addLink(A, B, bw=10, delay='10ms', loss=0.0, use_tbf=False)
        self.addLink(B, C, bw=10, delay='10ms', loss=0.0, use_tbf=False)


terms = []


def openTerm(self, node, title, geometry, cmd="bash"):
    "Open xterm window."

    display, tunnel = tunnelX11(node)

    return node.popen(["xterm",
                       "-hold",
                       "-title", "'%s'" % title,
                       "-geometry", geometry,
                       "-display", display,
                       "-e", cmd])


def init_oblig(self, line):
    "init is an example command to extend the Mininet CLI"

    net = self.mn
    A = net.get('A')
    B = net.get('B')
    C = net.get('C')

    # Start MIP daemons
    terms.append(openTerm(self,
                          node=A,
                          title="Host A",
                          geometry="80x20+0+0",
                          cmd="./mip_daemon -d usockA 10"))
    time.sleep(1)

    terms.append(openTerm(self,
                          node=B,
                          title="Host B",
                          geometry="80x20+550+0",
                          cmd="./mip_daemon -d usockB 20"))
    time.sleep(1)

    terms.append(openTerm(self,
                          node=C,
                          title="Host C",
                          geometry="80x20+1100+0",
                          cmd="./mip_daemon -d usockC 30"))
    time.sleep(1)

    # Run ping_server on Host B
    terms.append(openTerm(self,
                          node=B,
                          title="Server [B]",
                          geometry="80x20+550+300",
                          cmd="./ping_server usockB"))

    time.sleep(1)

    # Run ping_clients on Hosts A and C
    # Since this is the first try, the RTT should be ~40ms
    terms.append(openTerm(self,
                          node=A,
                          title="Client [A]",
                          geometry="80x20+0+300",
                          cmd="./ping_client usockA \"Hello IN3230\" 20"))
    time.sleep(3)

    terms.append(openTerm(self,
                          node=C,
                          title="Client [C]",
                          geometry="80x20+0+600",
                          cmd="./ping_client usockC \"Hello IN4230\" 20"))
    time.sleep(3)

    # This MUST output 'ping timeout' since A is not able to reach C.
    terms.append(openTerm(self,
                          node=A,
                          title="Client [A]",
                          geometry="80x20+0+300",
                          cmd="./ping_client usockA \"Hello IN4230\" 30"))
    time.sleep(3)

    # This time the RTT should be smaller, ~20ms, since MIP-ARP cache
    # is being utilized.
    terms.append(openTerm(self,
                          node=A,
                          title="Client [A]",
                          geometry="80x20+0+300",
                          cmd="./ping_client usockA \"Hello again IN4230\" 20"))


# Mininet Callbacks
# Inside mininet console run 'init_oblig'

CLI.do_init_oblig = init_oblig


# Inside mininet console run 'EOF' to gracefully kill the mininet console
orig_EOF = CLI.do_EOF


# Kill mininet console
def do_EOF(self, line):
    for t in terms:
        os.kill(t.pid, signal.SIGKILL)
    return orig_EOF(self, line)


CLI.do_EOF = do_EOF


# Topologies
topos = {
    'oblig': (lambda: Oblig()),
}
