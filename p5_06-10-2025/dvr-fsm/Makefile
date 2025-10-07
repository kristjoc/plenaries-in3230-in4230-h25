CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

TARGET = fsm
SRCS = fsm.c
OBJS = $(SRCS:.c=.o)
HEADERS = fsm.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
