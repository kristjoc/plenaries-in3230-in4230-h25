# Plenary Session 2 - 08.09.2025 #

This plenary session focused entirely on RAW sockets and the MIP-ARP
protocol. Using a simple three-node topology (A-B-C), we covered the
main functions a host needs to call to:

	- send a simple broadcast ARP request via a RAW socket
	- send an ARP reply in response to a BROADCAST ARP request
	- walk through all the interfaces of a host and store their MAC addreses
	- etc.

Enough talk - let's get our hands on the C code!  

Check the code
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p2_08-09-2025/greet-your-neighbor)
and feel free to adapt it for the Oblig and Home Exams.  

Instead of using `struct msghdr` to send data via a RAW socket, an
alternative approach involves copying a `struct` into a byte array
(serialization), sending it via a RAW socket, and then deserializing
the data at the receiving end. One can chose to use
serialization/deserialization or encapsulating the information into
`struct msghdr` in order to send/receive MIP packets via RAW sockets.
Both options are valid and will be discussed in detail in the next
plenary session.  

**NOTE:** The examples we code during plenaries are intentionally
simplified and may contain hardcoded elements. For your assignments,
make sure to write clean C code with proper functions and clear comments.  

See you next session on Monday, 15.09.2025, at 14:15 in OJD Seminarrom
C (3437). We'll code HiP - Hi Protocol, which enables mininet hosts to
perform handshakes and exchange serialized data packets via RAW
sockets.
