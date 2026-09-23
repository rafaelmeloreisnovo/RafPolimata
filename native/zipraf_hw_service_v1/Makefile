CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2

.PHONY: all test freestanding bbs clean
all: test

test: /tmp/zipraf_hw_selftest
	/tmp/zipraf_hw_selftest

/tmp/zipraf_hw_selftest: c/zipraf_hw_service.c tests/selftest.c include/zipraf_hw_service.h fs/zipraf_cap_fs.h
	$(CC) $(CFLAGS) -Iinclude c/zipraf_hw_service.c tests/selftest.c -o $@

freestanding:
	$(CC) -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -Iinclude -c c/zipraf_hw_service.c -o /tmp/zipraf_hw_service.o
	$(CC) -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -c tests/selftest.c -o /tmp/zipraf_hw_selftest.o

bbs: /tmp/zipraf_hw_bbs

/tmp/zipraf_hw_bbs: c/zipraf_hw_service.c bbs/zipraf_hw_bbs.c include/zipraf_hw_service.h
	$(CC) $(CFLAGS) -Iinclude c/zipraf_hw_service.c bbs/zipraf_hw_bbs.c -o $@

clean:
	rm -f /tmp/zipraf_hw_selftest /tmp/zipraf_hw_service.o /tmp/zipraf_hw_selftest.o /tmp/zipraf_hw_bbs
