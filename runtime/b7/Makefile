CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
TARGET := raf_b7_selftest

.PHONY: all test clean
all: $(TARGET)

$(TARGET): raf_b7_orchestrator.c raf_b7_orchestrator.h selftest.c
	$(CC) $(CFLAGS) -I. raf_b7_orchestrator.c selftest.c -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
