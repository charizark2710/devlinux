# Compiler and Flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -std=c99 -D_GNU_SOURCE
LDFLAGS :=

# Target Executables
SERVER  := server
CLIENT  := client

# Source Files
SERVER_SRC := server.c
CLIENT_SRC := client.c

# Object Files
SERVER_OBJ := $(SERVER_SRC:.c=.o)
CLIENT_OBJ := $(CLIENT_SRC:.c=.o)

# Socket cleanup file
SOCKET_PATH := /tmp/monitor.sock

# Default rule: build both server and client
.PHONY: all
all: $(SERVER) $(CLIENT)

# Build Server Executable
$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build Client Executable
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Implicit rule for compiling C source files into Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts and stale socket files
.PHONY: clean
clean:
	rm -f $(SERVER) $(CLIENT) $(SERVER_OBJ) $(CLIENT_OBJ) $(SOCKET_PATH)

# Rebuild everything from scratch
.PHONY: re
re: clean all