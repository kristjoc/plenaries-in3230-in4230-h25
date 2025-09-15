# Plenary Session 3 - 15.09.2025 #

Today's plenary session focused on two different approaches for
sending packets with multiple headers using RAW sockets.

	- using a `struct msghdr`
	- serializing the `struct pdu` into a byte array


Check the code for the
[first](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p3_15-09-2025/ping-pong)
and
[second](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p3_15-09-2025/hip)
methods, and feel free to adapt them for the Oblig and Home Exams.  


**NOTE:** The examples we code during plenaries are intentionally
simplified and may contain hardcoded elements. For your assignments,
make sure to write clean C code with proper functions and clear comments.  

See you next session on Monday, 22.09.2025, at 14:15 in OJD Seminarrom
C (3437). In addition to coding essential components for the Oblig, we
will also implement a Wireshark dissector for our HiP protocol.

