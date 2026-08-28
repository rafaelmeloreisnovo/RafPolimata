CC ?= clang
CXX ?= clang++
CFLAGS := -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror
CXXFLAGS := -O2 -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-stack-protector -Wall -Wextra -Werror

.PHONY: c-check cpp-check armv7-syntax aarch64-syntax x64-syntax rust-check zig-check d-check selftest clean

c-check:
	$(CC) $(CFLAGS) -Iinclude -c c/raf_hash_fabric.c -o /tmp/rhf-c.o

cpp-check:
	$(CXX) $(CXXFLAGS) -c cpp/raf_hash_fabric.cpp -o /tmp/rhf-cpp.o

aarch64-syntax:
	$(CC) --target=aarch64-linux-android24 -c asm/aarch64/rhf_mix32.S -o /tmp/rhf-a64.o

armv7-syntax:
	$(CC) --target=armv7a-linux-androideabi24 -c asm/armv7/rhf_mix32.S -o /tmp/rhf-a32.o

x64-syntax:
	$(CC) --target=x86_64-linux-android24 -c asm/x86_64/rhf_mix32.S -o /tmp/rhf-x64.o

rust-check:
	rustc --crate-type lib --emit=obj -C panic=abort -O rust/lib.rs -o /tmp/rhf-rust.o

zig-check:
	zig build-obj -O ReleaseSmall zig/raf_hash_fabric.zig -femit-bin=/tmp/rhf-zig.o

d-check:
	ldc2 -betterC -O3 -c d/raf_hash_fabric.d -of=/tmp/rhf-d.o

selftest:
	$(CC) -O2 -Iinclude c/raf_hash_fabric.c tests/selftest.c -o /tmp/rhf-selftest
	/tmp/rhf-selftest

clean:
	rm -f /tmp/rhf-c.o /tmp/rhf-cpp.o /tmp/rhf-a64.o /tmp/rhf-a32.o /tmp/rhf-x64.o /tmp/rhf-rust.o /tmp/rhf-zig.o /tmp/rhf-d.o /tmp/rhf-selftest
