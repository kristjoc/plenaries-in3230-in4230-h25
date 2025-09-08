""" Plenary IN3230/4230 - Point-to-Point topology

    A<===>B<===>C

    Usage: sudo -E mn --mac --custom topo_p2p.py --topo mytopo

"""

from mininet.topo import Topo

class MyTopo( Topo ):
    "Point to Point topology"

    def __init__( self ):
        "Create custom topo."

        # Initialize topology
        Topo.__init__( self )

        # Add hosts
        hostA = self.addHost( 'A' )
        hostB = self.addHost( 'B' )
        hostC = self.addHost( 'C' )

        # Add link
        self.addLink( hostA, hostB )
        self.addLink( hostB, hostC )

topos = { 'mytopo': ( lambda: MyTopo() ) }
