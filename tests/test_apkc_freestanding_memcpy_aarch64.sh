#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
for cmd in clang readelf nm; do command -v "$cmd" >/dev/null 2>&1 || exit 127; done
command -v ld.lld >/dev/null 2>&1 || exit 127
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
cat >"$TMP/probe.c" <<'C'
#include "mem.h"
typedef struct { u64 x[32]; } Blob;
static Blob src, dst;
void _start(void) {
    dst = src; /* force aggregate-copy lowering */
    os_exit((i32)(dst.x[0] & 0xffu));
}
C
clang -ffreestanding -fno-builtin -nostdlib -nostdinc -I Apkc \
  --target=aarch64-linux-gnu -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none \
  "$TMP/probe.c" -o "$TMP/probe.elf"
readelf -h "$TMP/probe.elf" >"$TMP/readelf.txt"
grep -Eq 'Class:[[:space:]]+ELF64' "$TMP/readelf.txt"
grep -Eq 'Machine:[[:space:]]+AArch64' "$TMP/readelf.txt"
if nm -u "$TMP/probe.elf" | grep -Eq '(^|[[:space:]])memcpy$'; then
  echo 'FAIL unresolved memcpy' >&2; exit 1
fi
# Negative control: remove the exported shim and require the same aggregate copy to fail link.
python3 - "$ROOT/Apkc/mem.h" "$TMP/mem.h" <<'PY'
import sys
s=open(sys.argv[1]).read()
a=s.index('static __attribute__((noinline)) void *_apkc_memcpy_impl')
b=s.index('/* branchless byte-level equality', a)
open(sys.argv[2],'w').write(s[:a]+s[b:])
PY
cp Apkc/sys.h "$TMP/sys.h"
if clang -ffreestanding -fno-builtin -nostdlib -nostdinc -I "$TMP" \
  --target=aarch64-linux-gnu -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none \
  "$TMP/probe.c" -o "$TMP/negative.elf" 2>"$TMP/negative.err"; then
  echo 'FAIL negative control unexpectedly linked' >&2; exit 1
fi
grep -q 'undefined symbol: memcpy' "$TMP/negative.err"
echo 'PASS aarch64_freestanding_memcpy_link'
echo 'PASS negative_control_requires_memcpy_shim'
echo 'claim_allowed=false'
