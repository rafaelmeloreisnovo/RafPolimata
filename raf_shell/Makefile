# RAF Polimata — Pipeline Orchestration Shell
# Freestanding style: no libc calls, direct x86_64 syscalls, zero malloc.
# All modules in separate headers, composed in one translation unit.

CC      = gcc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -Wpedantic \
           -fno-builtin -fno-stack-protector \
           -Wno-unused-function -Wno-unused-parameter \
           -Wno-attributes \
           -I.
TARGET  = ../raf_orchestrator

.PHONY: all clean check

all: $(TARGET)

$(TARGET): raf_shell.c sys_host.h str_util.h tty.h logbuf.h \
           dirbrowse.h sig_audit.h pipe_stages.h
	$(CC) $(CFLAGS) raf_shell.c -o $(TARGET)
	@echo "[phi] raf_shell built: $$(wc -c < $(TARGET)) bytes"

# Syntax check only (no link) — fast CI gate
check:
	$(CC) $(CFLAGS) -fsyntax-only raf_shell.c

clean:
	rm -f $(TARGET) raf_bench.log *.sig
