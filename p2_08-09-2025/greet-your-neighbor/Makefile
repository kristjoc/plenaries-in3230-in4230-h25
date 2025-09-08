CC = gcc
CFLAGS  = -g -Wall

all:
	$(CC) $(CFLAGS) sender.c common.c -o sender
	$(CC) $(CFLAGS) receiver.c common.c -o receiver

clean:
	rm -f sender receiver
