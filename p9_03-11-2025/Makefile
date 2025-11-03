CC = gcc
CFLAGS = -I include -Wall

src = $(wildcard src/*.c)
obj = $(patsubst src/%.c, build/%.o, $(src))
headers = $(wildcard include/*.h)

.PHONY: all build install clean

# Main target
rtp: $(obj)
	$(CC) $(CFLAGS) $(obj) -o rtp

# Compile object files
build/%.o: src/%.c $(headers) | build
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
build:
	mkdir -p build

# All target
all: build rtp install

# Install target
install:
	mkdir -p bin
	mv rtp bin/

# Clean target
clean:
	rm -rf build/ bin/ rtp
