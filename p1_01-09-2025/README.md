# Plenary Session 1 - 01.09.2025 #


During this first Plenary session we discussed about the following
topics:

* Course prerequisites
* NREC cloud
* Minimal Interconnection Protocol (MIP)
* UNIX and RAW sockets

## Course Prerequisites ##

In the first part of the session, we discussed some of the
prerequisites for the course, which can be summarized as follows:

* An introductory course on Operating Systems and Networks
* Basic C programming

The prerequisites mentioned above are not mandatory. However, if a
student lacks these skills, it is highly recommended to attend the
plenaries/group sessions where we will cover the necessary concepts
required to complete the assignments.

We then proceeded with an interactive question-and-answer session to
enable students to effectively self-assess their skills. You can
review the results
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/blob/main/p1_01-09-2025/menti-in3230-in4230-h25.pdf).

## NREC Cloud ##

Next, we focused on NREC, a cloud infrastructure platform delivered in
collaboration between the universities of Oslo and Bergen. We provided
a step-by-step demonstration of how to launch a virtual machine in the
cloud and access it via SSH. You can follow along with the detailed
tutorial provided
[here](https://www.uio.no/studier/emner/matnat/ifi/IN3230/h25/obligatorisk-oppgave/running-your-vm-on-nrec.html).

Unfortunately, there were some students who encountered issues while
trying to access the VM instance. After further investigation, it was
found that the virtual machines could not renew their IPv6 IP leases
after expiration. A quick fix to set the IPv6 address statically was
provided in the Mattermost Plenum channel. Meanwhile, NREC has been
informed, and a case is currently ongoing.

## Minimal Interconnection Protocol (MIP) ##

The final part of the session focused on introducing MIP, a minimal
network layer protocol that we will implement to construct our simple
network stack for the course assignments. Through an illustrative
example, we explored all the steps involved in the ping-pong process,
including the application request via the UNIX interface, the MIP-ARP
request and reply, as well as the subsequent ping and pong packets.

## UNIX and RAW Sockets

In the last ten minutes of the session, we provided a very brief
introduction to UNIX and raw sockets through two examples. Although we
did not have time to compile and run these examples, the code, which
can be found
[here](https://github.com/kristjoc/plenaries-in3230-in4230-h25/tree/main/p1_01-09-2025/sockets)
was shared with the students for independent study. We plan to
complete these examples and delve into the details in the next plenary
session.

## Next plenary ##

If today's plenary might have been a bit confusing for some of you due to the
new concepts, fear not nor be discouraged.

In our next session, we will continue from where we left off with the
UNIX and raw socket examples that we didn't have a chance to complete
today. Additionally, we will code a simple implementation of MIP-ARP
using RAW sockets that helps three nodes (A --- B --- C) to get to
know each other. MIP-ARP is a crucial part of the MIP daemon in Oblig.
Meanwhile, consider to check <https://beej.us/guide/bgnet/>, which is
a good resource about socket programming in C.

Thanks, everyone! Looking forward to seeing you in the next session on
Monday, 08.09.2025, at 14:15 in OJD Seminarrom C (3437).
