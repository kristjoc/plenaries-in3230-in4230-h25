from mininet.topo import Topo

# Usage example:
# sudo mn --mac --custom plenary-topo.py --topo p5 --link tc


class PlenaryTopo( Topo ):
    "Simple topology for Plenaries."

    def __init__( self ):
        "Set up our custom topo."

        # Initialize topology
        Topo.__init__( self )

        # Add hosts
        A = self.addHost('A')
        B = self.addHost('B')
        C = self.addHost('C')

        # Add links
        self.addLink(A, B, bw=10, delay='10ms', loss=0.0, use_tbf=False)
        self.addLink(B, C, bw=10, delay='10ms', loss=0.0, use_tbf=False)
        self.addLink(A, C, bw=10, delay='10ms', loss=0.0, use_tbf=False)


topos = { 'p5': ( lambda: PlenaryTopo() ) }
