# Plenary Session 4 - 22.09.2025 #

We began the session by reviewing a Mininet script that I had
previously shared on Mattermost for testing the mandatory assignment.
The script automates the initialization of MIP daemons and
applications with the appropriate argument sequencing, allowing for
the evaluation of specific scenarios.

	- A pings B
	- C pings B
	- A pings C (expecting a timeout)
	- A pings B again (the reported Round-Trip Time should be shorter due to ARP cache utilization)

You may find the script
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p4_22-09-2025/extras/oblig-mn-script-v2.py).
In addition,
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p4_22-09-2025/extras/oblig-grading-guidelines.org)
is a summary of the implementation requirements for the mandatory
assignment.

The last part of the session was focused on implementing a wireshark
dissector for our HiP protocol, following the tutorial in
[this](https://mika-s.github.io/wireshark/lua/dissector/2017/11/04/creating-a-wireshark-dissector-in-lua-1.html)
webpage.

Here is the final code of the plugin written in Lua. Place the
`hip.lua` file in `~/.local/lib/wireshark/plugins/` and run the
Mininet project in the `synack` directory. Wireshark will be able to
capture the HIP\_SYN and the HIP\_SYNACK packets exchanged between the
client and server and print the correct packet type. See the
screenshots for the expected subtree in the
[wireshark-plugin](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p4_22-09-2025/wireshark-plugin)
directory.

You can modify the Lua code of the plugin to capture MIP packets,
which will help you debug your implementation of the mandatory
assignment.

See you in the next session on Monday, 29.09.2025, at 14:15 in OJD
Seminarrom C (3437).
