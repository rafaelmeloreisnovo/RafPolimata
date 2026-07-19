CC ?= clang
CFLAGS := -O2 -std=c11 -Wall -Wextra -Werror -pedantic
CORE_FLAGS := $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector

.PHONY: all test test-segment cli audit arm32 arm64 clean

all: test test-segment cli audit

test:
	$(CC) $(CFLAGS) raf_convscan.c test_convscan.c -o test_convscan
	./test_convscan

test-segment:
	$(CC) $(CFLAGS) raf_segment_v1.c test_segment_v1.c -o test_segment_v1
	./test_segment_v1

cli:
	$(CC) $(CFLAGS) raf_convscan.c convscan_cli.c -o convscan

audit:
	$(CC) $(CORE_FLAGS) -c raf_convscan.c -o raf_convscan.o
	@test -z "$$(nm -u raf_convscan.o)"
	@! strings raf_convscan.o | grep -E 'malloc|calloc|realloc|free|memcpy|memset|printf|fopen|fread|fwrite'
	$(CC) $(CORE_FLAGS) -c raf_segment_v1.c -o raf_segment_v1.o
	@test -z "$$(nm -u raf_segment_v1.o)"
	@! strings raf_segment_v1.o | grep -E 'malloc|calloc|realloc|free|memcpy|memset|printf|fopen|fread|fwrite'

arm32:
	clang --target=armv7-none-eabi $(CORE_FLAGS) -march=armv7-a -mfloat-abi=soft -c raf_convscan.c -o raf_convscan-arm32.o
	clang --target=armv7-none-eabi $(CORE_FLAGS) -march=armv7-a -mfloat-abi=soft -c raf_segment_v1.c -o raf_segment_v1-arm32.o
	@test -z "$$(llvm-nm -u raf_convscan-arm32.o)"
	@test -z "$$(llvm-nm -u raf_segment_v1-arm32.o)"

arm64:
	clang --target=aarch64-none-elf $(CORE_FLAGS) -march=armv8-a -c raf_convscan.c -o raf_convscan-arm64.o
	clang --target=aarch64-none-elf $(CORE_FLAGS) -march=armv8-a -c raf_segment_v1.c -o raf_segment_v1-arm64.o
	@test -z "$$(llvm-nm -u raf_convscan-arm64.o)"
	@test -z "$$(llvm-nm -u raf_segment_v1-arm64.o)"

clean:
	rm -f test_convscan test_segment_v1 convscan *.o
