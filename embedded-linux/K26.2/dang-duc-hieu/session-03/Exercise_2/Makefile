# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
TARGET  = main
SRC     = main.c
DATA    = products.bin

# Default Target (Build the executable)
all: $(TARGET)

# Compile C source code into executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run program using append redirection (creates products.bin if missing)
run: $(TARGET)
	./$(TARGET)

# Clean up binary files and executables
clean:
	rm -f $(TARGET) $(DATA)

.PHONY: all run view clean