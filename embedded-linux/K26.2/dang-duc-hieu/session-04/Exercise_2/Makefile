# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2

TARGETS = manager searcher

# Default Target (Build all binaries)
all: $(TARGETS)

# Compile Manager
manager: manager.c
	$(CC) $(CFLAGS) -o manager manager.c

# Compile Searcher
searcher: searcher.c
	$(CC) $(CFLAGS) -o searcher searcher.c

# Run the manager program
run: all
	./manager

# Clean up binaries and log/data files
clean:
	rm -f $(TARGETS)

.PHONY: all run clean