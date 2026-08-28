CC ?= clang
CXX ?= clang++
CFLAGS := -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror
CXXFLAGS := -O2 -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-stack-protector -Wall -Wextra -Werror

.PHONY: source-contract-check c-check cpp-check armv7-syntax aarch64-syntax x64-syntax rust-check zig-check d-check selftest clean

source-contract-check:
	@command -v git >/dev/null 2>&1 || { echo "SOURCE_CONTRACT_FAIL: git unavailable"; exit 1; }
	@r=$$(sed -n 's/^#define RHF_SOURCE_README_GIT_BLOB  "\([0-9a-f]*\)"/\1/p' include/raf_hash_fabric.h); \
	 l=$$(sed -n 's/^#define RHF_SOURCE_LICENSE_GIT_BLOB "\([0-9a-f]*\)"/\1/p' include/raf_hash_fabric.h); \
	 m=$$(sed -n 's/^#define RHF_SOURCE_SCOPE_GIT_BLOB   "\([0-9a-f]*\)"/\1/p' include/raf_hash_fabric.h); \
	 ar=$$(git hash-object README.md); \
	 al=$$(git hash-object LICENSE.md); \
	 am=$$(git hash-object LICENSE_SCOPE_V1.json); \
	 test -n "$$r" -a -n "$$l" -a -n "$$m" || { echo "SOURCE_CONTRACT_FAIL: anchor missing"; exit 1; }; \
	 test "$$ar" = "$$r" || { echo "SOURCE_CONTRACT_FAIL: README.md bit identity changed expected=$$r actual=$$ar"; exit 1; }; \
	 test "$$al" = "$$l" || { echo "SOURCE_CONTRACT_FAIL: LICENSE.md bit identity changed expected=$$l actual=$$al"; exit 1; }; \
	 test "$$am" = "$$m" || { echo "SOURCE_CONTRACT_FAIL: LICENSE_SCOPE_V1.json bit identity changed expected=$$m actual=$$am"; exit 1; }; \
	 echo "SOURCE_CONTRACT_PASS README=$$ar LICENSE=$$al SCOPE=$$am"

c-check: source-contract-check
	$(CC) $(CFLAGS) -Iinclude -c c/raf_hash_fabric.c -o /tmp/rhf-c.o

cpp-check: source-contract-check
	$(CXX) $(CXXFLAGS) -c cpp/raf_hash_fabric.cpp -o /tmp/rhf-cpp.o

aarch64-syntax: source-contract-check
	$(CC) --target=aarch64-linux-android24 -c asm/aarch64/rhf_mix32.S -o /tmp/rhf-a64.o

armv7-syntax: source-contract-check
	$(CC) --target=armv7a-linux-androideabi24 -c asm/armv7/rhf_mix32.S -o /tmp/rhf-a32.o

x64-syntax: source-contract-check
	$(CC) --target=x86_64-linux-android24 -c asm/x86_64/rhf_mix32.S -o /tmp/rhf-x64.o

rust-check: source-contract-check
	rustc --crate-type lib --emit=obj -C panic=abort -O rust/lib.rs -o /tmp/rhf-rust.o

zig-check: source-contract-check
	zig build-obj -O ReleaseSmall zig/raf_hash_fabric.zig -femit-bin=/tmp/rhf-zig.o

d-check: source-contract-check
	ldc2 -betterC -O3 -c d/raf_hash_fabric.d -of=/tmp/rhf-d.o

selftest: source-contract-check
	$(CC) -O2 -Iinclude c/raf_hash_fabric.c tests/selftest.c -o /tmp/rhf-selftest
	/tmp/rhf-selftest

clean:
	rm -f /tmp/rhf-c.o /tmp/rhf-cpp.o /tmp/rhf-a64.o /tmp/rhf-a32.o /tmp/rhf-x64.o /tmp/rhf-rust.o /tmp/rhf-zig.o /tmp/rhf-d.o /tmp/rhf-selftest
