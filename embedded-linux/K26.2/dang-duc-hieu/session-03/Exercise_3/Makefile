CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L

TARGETS = writer_flock writer_fcntl

all: $(TARGETS)

writer_flock: writer_flock.c
	$(CC) $(CFLAGS) -o $@ $<

writer_fcntl: writer_fcntl.c
	$(CC) $(CFLAGS) -o $@ $<

test_flock: writer_flock
	@echo "=== Testing flock concurrency (10 parallel writers) ==="
	@rm -f system.log
	@for i in $$(seq 1 10); do \
		./writer_flock "flock message $$i" & \
	done; wait
	@echo "--- system.log content ---"
	@cat system.log
	@echo "--- Total lines written: $$(wc -l < system.log) / 10 expected ---"

test_fcntl: writer_fcntl
	@echo "=== Testing fcntl concurrency (10 parallel writers) ==="
	@rm -f system.log
	@for i in $$(seq 1 10); do \
		./writer_fcntl "fcntl message $$i" & \
	done; wait
	@echo "--- system.log content ---"
	@cat system.log
	@echo "--- Total lines written: $$(wc -l < system.log) / 10 expected ---"

clean:
	rm -f $(TARGETS) system.log

.PHONY: all test_flock test_fcntl clean