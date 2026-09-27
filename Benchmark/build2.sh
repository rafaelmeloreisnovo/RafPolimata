#!/usr/bin/env bash
# build2.sh — RAFAELIA no-libc Linux/Android userspace build
# Canonical Termux-oriented Benchmark builder. This is raw-syscall userspace,
# not physical bare-metal and not the OS-neutral freestanding/ L0 target.
set -euo pipefail

echo "=== RAFAELIA no-libc userspace build ==="

ARCH="$(uname -m)"
CFLAGS="-O2 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -fno-builtin \
        -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables \
        -fno-ident -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-plt \
        -Wall -Wextra -Wno-unused-function -I."
LDFLAGS="-Wl,--gc-sections -Wl,--build-id=none"

# ARM32 native Termux: exactly one profile. Keep EABI softfp/NEON explicit.
case "$ARCH" in
  armv7l|armv8l)
    echo "[ARM32] Termux native build (EABI softfp + NEON; clock_gettime timer)"
    gcc $CFLAGS $LDFLAGS \
        -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fno-pic \
        -static -e _start -o raf_enterprise_a32 raf_main.c
    echo "[ARM32] OK -> raf_enterprise_a32"
    file raf_enterprise_a32
    ;;
esac

# ARM64 native Linux/Android userspace.
if [[ "$ARCH" == "aarch64" ]]; then
    echo "[ARM64] gcc -march=armv8.2-a+crc+crypto ..."
    gcc $CFLAGS $LDFLAGS \
        -march=armv8.2-a+crc+crypto \
        -static -e _start -o raf_enterprise_a64 raf_main.c
    echo "[ARM64] OK -> raf_enterprise_a64"
    size raf_enterprise_a64
fi

# x86-64 native Linux userspace.
if [[ "$ARCH" == "x86_64" ]]; then
    echo "[x86-64] gcc -march=native ..."
    gcc $CFLAGS $LDFLAGS \
        -march=native -static -e _start -o raf_enterprise_x64 raf_main.c
    echo "[x86-64] OK -> raf_enterprise_x64"
    size raf_enterprise_x64
fi

# Optional GNU hard-float cross artifact; never overwrites the native softfp file.
if command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1; then
    echo "[ARM32] cross-compile (GNU hard-float profile) ..."
    arm-linux-gnueabihf-gcc $CFLAGS $LDFLAGS \
        -march=armv7-a+fp -mfpu=neon-vfpv4 -mfloat-abi=hard \
        -static -e _start -o raf_enterprise_a32_hf raf_main.c
    echo "[ARM32] OK -> raf_enterprise_a32_hf"
fi

echo "=== Build complete ==="
for b in raf_enterprise_*; do
    [[ -f "$b" ]] && strip --strip-all "$b" && echo "stripped: $(du -h "$b" | cut -f1) $b"
done
