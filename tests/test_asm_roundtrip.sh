#!/usr/bin/env sh
# Roundtrip smoke test for the APKc internal assembler.
#
# This test is intentionally honest:
# - it always runs the pure Python ARM64 encoder golden tests;
# - on ARM hosts it also compiles/runs the C encoder reference test;
# - it only executes the APK generation roundtrip on ARM hosts where apkc.c
#   can produce a native freestanding executable with its _start entry.
# - on non-ARM hosts it exits 0 after the Python encoder tests with a SKIP note,
#   because x86_64 syntax checks are not Android runtime proof.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

if command -v python3 >/dev/null 2>&1; then
  python3 tests/test_arm64_encoders.py
else
  echo "FAIL: python3 missing; cannot run tests/test_arm64_encoders.py" >&2
  exit 1
fi

ARCH=$(uname -m 2>/dev/null || echo unknown)
case "$ARCH" in
  aarch64|arm64|armv7l|armv8l|arm*)
    ;;
  *)
    echo "SKIP: apkc freestanding roundtrip needs ARM host; current arch=$ARCH"
    echo "PASS: Python encoder golden tests completed; APK/C roundtrip remains TOKEN_VAZIO on this host."
    exit 0
    ;;
esac

CC=${CC:-cc}
if ! command -v "$CC" >/dev/null 2>&1; then
  echo "FAIL: C compiler not found: $CC" >&2
  exit 1
fi

if [ ! -f Apkc/apkc.c ]; then
  echo "FAIL: Apkc/apkc.c missing" >&2
  exit 1
fi

if [ ! -f Apkc/hello.s.txt ]; then
  echo "FAIL: Apkc/hello.s.txt missing" >&2
  exit 1
fi

TMPDIR_ROOT=${TMPDIR:-/tmp}
WORK="$TMPDIR_ROOT/rafpolimata_asm_roundtrip_$$"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT HUP INT TERM

ENC_C="$WORK/test_arm64_encoders"
APKC="$WORK/apkc"
APK="$WORK/hello-arm64.apk"
LOG="$WORK/apkc-generate.txt"

if [ -f tests/test_arm64_encoders.c ]; then
  echo "BUILD: $CC -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o $ENC_C"
  "$CC" -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o "$ENC_C"
  echo "RUN: $ENC_C"
  "$ENC_C"
else
  echo "TOKEN_VAZIO: tests/test_arm64_encoders.c missing; C encoder test skipped"
fi

echo "BUILD: $CC -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start Apkc/apkc.c -o $APKC"
"$CC" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start Apkc/apkc.c -o "$APKC"

echo "RUN: $APKC Apkc/hello.s.txt -o $APK -p com.rafael.roundtrip -l RoundTrip -n hello -64"
"$APKC" Apkc/hello.s.txt -o "$APK" -p com.rafael.roundtrip -l RoundTrip -n hello -64 >"$LOG" 2>&1

if [ ! -s "$APK" ]; then
  echo "FAIL: APK was not generated or is empty" >&2
  cat "$LOG" >&2 || true
  exit 1
fi

if grep -i -E 'unknown|undef|failed|error' "$LOG" >/dev/null 2>&1; then
  echo "FAIL: assembler/generator log contains error marker" >&2
  cat "$LOG" >&2
  exit 1
fi

if command -v unzip >/dev/null 2>&1; then
  unzip -l "$APK" > "$WORK/unzip.txt"
  grep 'AndroidManifest.xml' "$WORK/unzip.txt" >/dev/null
  grep 'classes.dex' "$WORK/unzip.txt" >/dev/null
  grep 'lib/arm64-v8a/libhello.so' "$WORK/unzip.txt" >/dev/null
else
  echo "TOKEN_VAZIO: unzip missing; APK structure not inspected"
fi

if command -v readelf >/dev/null 2>&1 && command -v unzip >/dev/null 2>&1; then
  unzip -p "$APK" 'lib/arm64-v8a/libhello.so' > "$WORK/libhello-arm64.so"
  readelf -h "$WORK/libhello-arm64.so" > "$WORK/readelf-arm64.txt"
  grep 'Class:.*ELF64' "$WORK/readelf-arm64.txt" >/dev/null
  grep 'Machine:.*AArch64' "$WORK/readelf-arm64.txt" >/dev/null
else
  echo "TOKEN_VAZIO: readelf/unzip missing; ARM64 ELF header not inspected"
fi

echo "PASS: ARM64 assembler roundtrip generated $APK"
echo "LOG: $LOG"
