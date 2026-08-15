#!/usr/bin/env bash
# L5 bounded FFI proof: native C ABI <-> CPython ctypes.
# Exit 0=observed interop PASS; 1=falsified; 2=TOKEN_VAZIO toolchain.
set -u -o pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
mkdir -p docs/proofs
TS="$(date -u +%Y%m%dT%H%M%SZ)"
RECEIPT="docs/proofs/L5_FFI_C_PYTHON_${TS}.json"
TD="$(mktemp -d)"
trap 'rm -rf "$TD"' EXIT

CC="${CC:-}"
if [ -z "$CC" ]; then
  if command -v clang >/dev/null 2>&1; then CC=clang
  elif command -v gcc >/dev/null 2>&1; then CC=gcc
  elif command -v cc >/dev/null 2>&1; then CC=cc
  else
    echo 'L5 TOKEN_VAZIO_TOOLCHAIN missing=C_compiler'
    exit 2
  fi
fi
if ! command -v python3 >/dev/null 2>&1; then
  echo 'L5 TOKEN_VAZIO_TOOLCHAIN missing=python3'
  exit 2
fi

SRC="$TD/raf_ffi_contract.c"
LIB="$TD/libraf_ffi_contract.so"
cat >"$SRC" <<'C'
#include <stdint.h>
#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#define RAF_EXPORT __attribute__((visibility("default")))
#else
#define RAF_EXPORT
#endif

RAF_EXPORT uint32_t raf_ffi_abi_version(void) { return 0x00010000u; }
RAF_EXPORT uint32_t raf_ffi_add_u32(uint32_t a, uint32_t b) { return a + b; }
RAF_EXPORT uint64_t raf_ffi_xor64(uint64_t a, uint64_t b) { return a ^ b; }
RAF_EXPORT int32_t raf_ffi_transform(const uint8_t *in, size_t n, uint8_t *out, size_t cap) {
    if (!in || !out || cap < n) return -1;
    for (size_t i = 0; i < n; ++i) out[i] = (uint8_t)(in[i] ^ 0xA5u);
    return (int32_t)n;
}
C

if ! "$CC" -std=c11 -O2 -Wall -Wextra -Werror -fPIC -shared "$SRC" -o "$LIB"; then
  echo 'L5 FAIL native_shared_library_compile' >&2
  exit 1
fi
[ -s "$LIB" ] || { echo 'L5 FAIL shared_library_empty' >&2; exit 1; }

# Symbol visibility is part of the ABI proof when an inspection tool exists.
if command -v nm >/dev/null 2>&1; then
  for sym in raf_ffi_abi_version raf_ffi_add_u32 raf_ffi_xor64 raf_ffi_transform; do
    nm -D "$LIB" 2>/dev/null | grep -q "[[:space:]]$sym$" || { echo "L5 FAIL exported_symbol_missing=$sym" >&2; exit 1; }
  done
elif command -v readelf >/dev/null 2>&1; then
  for sym in raf_ffi_abi_version raf_ffi_add_u32 raf_ffi_xor64 raf_ffi_transform; do
    readelf -Ws "$LIB" 2>/dev/null | grep -q "[[:space:]]$sym$" || { echo "L5 FAIL exported_symbol_missing=$sym" >&2; exit 1; }
  done
fi

LIB_ABS="$(cd "$(dirname "$LIB")" && pwd)/$(basename "$LIB")"
PY_OUT="$TD/python-result.json"
python3 - "$LIB_ABS" "$PY_OUT" <<'PY'
import ctypes as C
import json, sys
from pathlib import Path
lib=C.CDLL(sys.argv[1])
lib.raf_ffi_abi_version.argtypes=[]
lib.raf_ffi_abi_version.restype=C.c_uint32
lib.raf_ffi_add_u32.argtypes=[C.c_uint32,C.c_uint32]
lib.raf_ffi_add_u32.restype=C.c_uint32
lib.raf_ffi_xor64.argtypes=[C.c_uint64,C.c_uint64]
lib.raf_ffi_xor64.restype=C.c_uint64
lib.raf_ffi_transform.argtypes=[C.POINTER(C.c_uint8),C.c_size_t,C.POINTER(C.c_uint8),C.c_size_t]
lib.raf_ffi_transform.restype=C.c_int32

assert lib.raf_ffi_abi_version() == 0x00010000
assert lib.raf_ffi_add_u32(20,22) == 42
assert lib.raf_ffi_add_u32(0xFFFFFFFF,1) == 0
assert lib.raf_ffi_xor64(0xAAAAAAAAAAAAAAAA,0x5555555555555555) == 0xFFFFFFFFFFFFFFFF
src=bytes([0,1,42,127,128,255])
In=(C.c_uint8*len(src))(*src)
Out=C.c_uint8*len(src)
out=Out()
rc=lib.raf_ffi_transform(In,len(src),out,len(src))
assert rc == len(src)
expected=bytes(b^0xA5 for b in src)
assert bytes(out) == expected
# Bounds/error contract: insufficient output capacity must fail and not be promoted.
small=(C.c_uint8*2)()
assert lib.raf_ffi_transform(In,len(src),small,2) == -1
Path(sys.argv[2]).write_text(json.dumps({
  'abi_version':'1.0',
  'add_20_22':42,
  'u32_wrap':0,
  'xor64':'ffffffffffffffff',
  'transform_hex':expected.hex(),
  'bounds_rejection':'PASS',
  'ctypes_load':'PASS'
},sort_keys=True),encoding='utf-8')
PY
rc=$?
if [ $rc -ne 0 ]; then
  echo 'L5 FAIL python_ctypes_interop' >&2
  exit 1
fi

SHA="$(sha256sum "$LIB" | awk '{print $1}')"
CC_VERSION="$($CC --version 2>/dev/null | head -1 | sed 's/"/\\"/g')"
PY_VERSION="$(python3 --version 2>&1 | sed 's/"/\\"/g')"
RESULT="$(cat "$PY_OUT")"
cat >"$RECEIPT" <<JSON
{
  "schema":"rafpolimata.l5.ffi.c-python.v1",
  "timestamp_utc":"$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "git_head":"$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO_GIT)",
  "scope":"native C ABI <-> CPython ctypes",
  "compiler":"$CC_VERSION",
  "python":"$PY_VERSION",
  "shared_object_sha256":"$SHA",
  "observations":$RESULT,
  "state":"PASS_C_PYTHON_FFI",
  "twelve_language_coverage":"TOKEN_VAZIO_OUT_OF_SCOPE",
  "claim_allowed":false
}
JSON

echo "L5 PASS_C_PYTHON_FFI sha256=$SHA receipt=$RECEIPT"
echo 'L5 scope is bounded: this proves one real C<->Python FFI path, not 12-language interoperability.'
exit 0
