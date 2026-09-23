CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic

.PHONY: all selftest rust-check asm-check clean
all: selftest

selftest: build/selftest
	./build/selftest

build/selftest: c/zipraf_hw_fabric.c tests/selftest.c include/zipraf_hw_fabric.h
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude c/zipraf_hw_fabric.c tests/selftest.c -o $@

rust-check:
	mkdir -p build
	rustc --crate-type lib --edition 2021 rust/lib.rs -o build/libzipraf_hw_fabric.rlib

asm-check:
	mkdir -p build
	clang --target=armv7a-linux-gnueabihf -c asm/armv7/zipraf_xor4.S -o build/zipraf_xor4_armv7.o
	clang --target=aarch64-linux-gnu -c asm/aarch64/zipraf_xor4.S -o build/zipraf_xor4_aarch64.o
	$(CC) -c asm/x86_64/zipraf_xor4.S -o build/zipraf_xor4_x86_64.o

clean:
	rm -rf build
