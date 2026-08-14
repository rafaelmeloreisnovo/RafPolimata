#!/usr/bin/env bash
# Deterministic source-transform + cross-ABI compile proof for ApkC runtime hardening.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

for cmd in python3 clang sha256sum cmp grep date mkdir; do
    command -v "$cmd" >/dev/null 2>&1 || { printf 'apkc runtime hardening proof: missing command: %s\n' "$cmd" >&2; exit 127; }
done

if command -v llvm-readelf >/dev/null 2>&1; then ELF_READER=llvm-readelf
elif command -v readelf >/dev/null 2>&1; then ELF_READER=readelf
else printf 'apkc runtime hardening proof: missing llvm-readelf/readelf\n' >&2; exit 127; fi

RUN_ID="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="Apkc/proofs/runs/$RUN_ID/runtime-hardening"
GEN="build/generated/Apkc/apkc.runtime-hardened.c"
MANIFEST="results/apkc-runtime-source-hardening.json"
STATUS="results/apkc-runtime-hardening-proof.json"
PATCHER="scripts/patch_apkc_runtime_source_c_escape.py"
mkdir -p "$RUN_DIR" "$(dirname "$GEN")" results

# The adapter preserves all canonical transform IDs/anchors but repairs the
# Python->C escape boundary for RH-008/009/013. It remains fail-closed if any
# expected replacement payload is no longer unique.
python3 "$PATCHER" --input Apkc/apkc.c --output "$GEN" --write-manifest "$MANIFEST"
python3 "$PATCHER" --input Apkc/apkc.c --output "$GEN" --write-manifest "$MANIFEST" --check

for marker in 'compiler output exceeds bounded buffer' 'source exceeds 1MiB bounded input' '_apkc_is_elf64_aarch64' '_apkc_sc5_32'; do
    grep -Fq "$marker" "$GEN" || { printf 'apkc runtime hardening proof: generated marker missing: %s\n' "$marker" >&2; exit 1; }
done
# Reject the precise malformed-C pattern observed in provider artifact 9218419408.
python3 - "$GEN" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
for msg in (
    'dex_output profile requires D8 and validated DEX output',
    'external compiler produced invalid AArch64 ET_DYN ELF',
    'source read failed', 'source exceeds 1MiB bounded input', 'source close failed'):
    if f'{msg}\n");' in s:
        raise SystemExit(f'malformed literal newline remains for: {msg}')
    if f'{msg}\\n");' not in s:
        raise SystemExit(f'escaped C newline missing for: {msg}')
print('PASS runtime C string escape contract')
PY

COMMON=(clang -ffreestanding -fno-builtin -nostdlib -nostdinc -I Apkc)
A64_1="$RUN_DIR/apkc-runtime-aarch64-1.elf"; A64_2="$RUN_DIR/apkc-runtime-aarch64-2.elf"
A32_1="$RUN_DIR/apkc-runtime-arm32-1.o"; A32_2="$RUN_DIR/apkc-runtime-arm32-2.o"
A64_HDR="$RUN_DIR/readelf-aarch64.txt"; A32_HDR="$RUN_DIR/readelf-arm32.txt"
A64_ERR="$RUN_DIR/aarch64.stderr.txt"; A32_ERR="$RUN_DIR/arm32.stderr.txt"

"${COMMON[@]}" --target=aarch64-linux-gnu -static -no-pie -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none "$GEN" -o "$A64_1" 2>"$A64_ERR"
"${COMMON[@]}" --target=aarch64-linux-gnu -static -no-pie -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none "$GEN" -o "$A64_2" 2>>"$A64_ERR"
"${COMMON[@]}" --target=arm-linux-gnueabihf -c "$GEN" -o "$A32_1" 2>"$A32_ERR"
"${COMMON[@]}" --target=arm-linux-gnueabihf -c "$GEN" -o "$A32_2" 2>>"$A32_ERR"

"$ELF_READER" -h "$A64_1" >"$A64_HDR"; "$ELF_READER" -h "$A32_1" >"$A32_HDR"
"$ELF_READER" -l "$A64_1" >"$RUN_DIR/readelf-aarch64-program.txt"
grep -Eq 'Class:[[:space:]]+ELF64' "$A64_HDR"; grep -Eq 'Machine:[[:space:]]+AArch64' "$A64_HDR"
! grep -Eq 'INTERP|Requesting program interpreter' "$RUN_DIR/readelf-aarch64-program.txt"
grep -Eq 'Class:[[:space:]]+ELF32' "$A32_HDR"; grep -Eq 'Machine:[[:space:]]+ARM' "$A32_HDR"
cmp -s "$A64_1" "$A64_2"; cmp -s "$A32_1" "$A32_2"

A64_SHA="$(sha256sum "$A64_1" | awk '{print $1}')"; A32_SHA="$(sha256sum "$A32_1" | awk '{print $1}')"; GEN_SHA="$(sha256sum "$GEN" | awk '{print $1}')"
cat >"$STATUS" <<JSON
{
  "schema":"raf.apkc.runtime-hardening-proof.v1",
  "run_id":"$RUN_ID",
  "generated_source":"$GEN",
  "generated_sha256":"$GEN_SHA",
  "aarch64":{"compile":"PASS","identity":"PASS","static_no_interp":"PASS","reproducibility":"PASS","sha256":"$A64_SHA"},
  "arm32":{"compile":"PASS","identity":"PASS","reproducibility":"PASS","sha256":"$A32_SHA"},
  "runtime_execution":"TOKEN_VAZIO",
  "state":"PASS",
  "claim_allowed":false
}
JSON
printf 'apkc runtime hardening proof: PASS aarch64=%s arm32=%s\n' "$A64_SHA" "$A32_SHA"
