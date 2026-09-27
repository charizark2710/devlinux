# Compiler and Flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -std=c99 -D_GNU_SOURCE
LDFLAGS :=

# Target Executables
SENDER  := sender
RECEIVER  := receiver

# Source Files
SENDER_SRC := sender.c
RECEIVER_SRC := receiver.c

# Object Files
SERVER_OBJ := $(SENDER_SRC:.c=.o)
CLIENT_OBJ := $(RECEIVER_SRC:.c=.o)

# Socket cleanup file
SOCKET_PATH := /tmp/monitor.sock

# Default rule: build both sender and receiver
.PHONY: all
all: $(SENDER) $(RECEIVER)

# Build Server Executable
$(SENDER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build Client Executable
$(RECEIVER): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Implicit rule for compiling C source files into Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts and stale socket files
.PHONY: clean
clean:
	rm -f $(SENDER) $(RECEIVER) $(SERVER_OBJ) $(CLIENT_OBJ) $(SOCKET_PATH)

# Rebuild everything from scratch
.PHONY: re
re: clean all