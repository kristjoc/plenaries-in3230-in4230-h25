CC = gcc
CFLAGS = -I include -Wall

src = $(wildcard src/*.c)
obj = $(patsubst src/%.c, build/%.o, $(src))
headers = $(wildcard include/*.h)

# Ensure build directory exists before compiling
build:
	mkdir -p build

# Main target
hip: $(obj)
	$(CC) $(CFLAGS) $(obj) -o hip

# Compile object files
build/%.o: src/%.c ${headers} | build
	$(CC) $(CFLAGS) -c $< -o $@

# All target
all: build hip install

# Install target
install:
	mkdir -p bin
	mv hip bin/

# Clean target
clean:
	rm -rf build/ bin/
