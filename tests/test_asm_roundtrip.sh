#!/usr/bin/env sh
# Roundtrip smoke test for the APKc internal assembler.
#
# Honest boundary:
# - Python encoder golden tests always run;
# - ARM hosts additionally compile/run the C encoder reference test;
# - the APKc executable is built only from the canonical runtime-hardened source;
# - non-ARM hosts keep the APK/C roundtrip as TOKEN_VAZIO.

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
for cmd in "$CC" python3 sha256sum; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "FAIL: required command not found: $cmd" >&2
    exit 1
  fi
done

if [ ! -f Apkc/apkc.c ]; then
  echo "FAIL: Apkc/apkc.c missing" >&2
  exit 1
fi

if [ ! -f Apkc/hello.s.txt ]; then
  echo "FAIL: Apkc/hello.s.txt missing" >&2
  exit 1
fi

PATCHER=scripts/patch_apkc_runtime_source_c_escape.py
if [ ! -f "$PATCHER" ]; then
  echo "FAIL: canonical runtime hardening patcher missing: $PATCHER" >&2
  exit 1
fi

TMPDIR_ROOT=${TMPDIR:-/tmp}
WORK="$TMPDIR_ROOT/rafpolimata_asm_roundtrip_$$"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT HUP INT TERM

ENC_C="$WORK/test_arm64_encoders"
HARDENED_SRC="$WORK/apkc.runtime-hardened.c"
HARDENING_MANIFEST="$WORK/apkc-runtime-source-hardening.json"
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

python3 "$PATCHER" --input Apkc/apkc.c --output "$HARDENED_SRC" --write-manifest "$HARDENING_MANIFEST"
python3 "$PATCHER" --input Apkc/apkc.c --output "$HARDENED_SRC" --write-manifest "$HARDENING_MANIFEST" --check
RAW_SHA=$(sha256sum Apkc/apkc.c | awk '{print $1}')
HARDENED_SHA=$(sha256sum "$HARDENED_SRC" | awk '{print $1}')
printf 'SOURCE raw_sha256=%s hardened_sha256=%s manifest=%s\n' "$RAW_SHA" "$HARDENED_SHA" "$HARDENING_MANIFEST"

echo "BUILD: $CC -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start $HARDENED_SRC -o $APKC"
"$CC" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start "$HARDENED_SRC" -o "$APKC"

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

echo "PASS: ARM64 hardened-source assembler roundtrip generated $APK"
echo "LOG: $LOG"
