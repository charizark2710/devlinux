# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -pthread -std=c11
TARGET  = booking
SRC     = booking.c
OBJ     = $(SRC:.c=.o)

# Default target: build the executable
all: $(TARGET)

# Link object files to produce executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up build artifacts
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all run clean