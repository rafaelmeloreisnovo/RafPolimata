#!/usr/bin/env bash
set -Eeuo pipefail

usage() {
  echo 'usage: apkc_strict_native_build.sh <language> <arm64|arm32|x86_64|rv64> <output.so> <source>' >&2
  exit 64
}

[[ $# -eq 4 ]] || usage
LANGUAGE="${1,,}"
ARCH="$2"
OUTPUT="$3"
SOURCE="$4"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLANG_BIN="${CLANG:-clang}"
PYTHON_BIN="${PYTHON:-python3}"
READELF_BIN="${READELF:-llvm-readelf}"
command -v "$CLANG_BIN" >/dev/null 2>&1 || { echo "missing compiler: $CLANG_BIN" >&2; exit 69; }
command -v "$PYTHON_BIN" >/dev/null 2>&1 || { echo "missing python: $PYTHON_BIN" >&2; exit 69; }
command -v "$READELF_BIN" >/dev/null 2>&1 || READELF_BIN="readelf"
[[ -s "$SOURCE" ]] || { echo "source missing or empty: $SOURCE" >&2; exit 66; }

case "$ARCH" in
  arm64) TARGET="aarch64-linux-android21" ;;
  arm32) TARGET="armv7a-linux-androideabi21" ;;
  x86_64) TARGET="x86_64-linux-android21" ;;
  rv64) TARGET="riscv64-linux-android35" ;;
  *) echo "unsupported architecture: $ARCH" >&2; exit 65 ;;
esac

TMP_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/rafaelia-native.XXXXXX")"
trap 'rm -rf "$TMP_ROOT"' EXIT
LOWERED="$TMP_ROOT/lowered.c"
REWRITTEN="$TMP_ROOT/rewritten.c"
LOWER_MANIFEST="$TMP_ROOT/lower.json"
REWRITE_MANIFEST="$TMP_ROOT/rewrite.json"

case "$LANGUAGE" in
  c)
    "$PYTHON_BIN" "$ROOT/scripts/raf_c_rewrite.py" "$SOURCE" "$REWRITTEN" --manifest "$REWRITE_MANIFEST"
    ;;
  cpp)
    "$PYTHON_BIN" "$ROOT/scripts/raf_c_rewrite.py" "$SOURCE" "$REWRITTEN" --manifest "$REWRITE_MANIFEST"
    ;;
  asm)
    "$CLANG_BIN" --target="$TARGET" -c -x assembler "$SOURCE" -o "$TMP_ROOT/input.o"
    "$CLANG_BIN" --target="$TARGET" -fuse-ld=lld -nostdlib -shared \
      -Wl,--no-undefined -Wl,--gc-sections -Wl,--build-id=none \
      -Wl,-soname,"$(basename "$OUTPUT")" "$TMP_ROOT/input.o" -o "$OUTPUT"
    ;;
  rs|kt|java|py|sh|pl|js|php|jsx|go|rb|swift|groovy|clj)
    "$PYTHON_BIN" "$ROOT/scripts/raf_kernel_lower.py" \
      --language "$LANGUAGE" --source "$SOURCE" --output "$LOWERED" --manifest "$LOWER_MANIFEST"
    "$PYTHON_BIN" "$ROOT/scripts/raf_c_rewrite.py" "$LOWERED" "$REWRITTEN" --manifest "$REWRITE_MANIFEST"
    ;;
  *) echo "language has no strict native route: $LANGUAGE" >&2; exit 65 ;;
esac

if [[ "$LANGUAGE" != asm ]]; then
  CFLAGS=(
    --target="$TARGET" -std=c11 -Os -fPIC -ffreestanding -fno-builtin
    -fno-stack-protector -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables
    -fno-optimize-sibling-calls -fvisibility=hidden -ffunction-sections -fdata-sections
    -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic -nostdinc -I "$ROOT/Apkc"
  )
  if [[ "$LANGUAGE" == cpp ]]; then
    CFLAGS=(
      --target="$TARGET" -x c++ -std=c++17 -Os -fPIC -ffreestanding -fno-builtin
      -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit
      -fno-stack-protector -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables
      -fno-optimize-sibling-calls -fvisibility=hidden -ffunction-sections -fdata-sections
      -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic -nostdinc -I "$ROOT/Apkc"
    )
  fi
  "$CLANG_BIN" "${CFLAGS[@]}" -c "$REWRITTEN" -o "$TMP_ROOT/input.o"
  "$CLANG_BIN" --target="$TARGET" -fuse-ld=lld -nostdlib -nostartfiles -nodefaultlibs \
    -shared -Wl,--no-undefined -Wl,--gc-sections -Wl,--build-id=none \
    -Wl,-soname,"$(basename "$OUTPUT")" "$TMP_ROOT/input.o" -o "$OUTPUT"
fi

[[ -s "$OUTPUT" ]] || { echo "output missing: $OUTPUT" >&2; exit 74; }
if "$READELF_BIN" -Ws "$OUTPUT" | awk '$7 == "UND" && $8 != "" { print; bad=1 } END { exit bad ? 1 : 0 }'; then
  :
else
  echo 'undefined symbol gate: FAIL' >&2
  exit 1
fi
if "$READELF_BIN" -l "$OUTPUT" | grep -q 'INTERP'; then
  echo 'PT_INTERP gate: FAIL' >&2
  exit 1
fi
printf 'apkc_strict_native_build: PASS lang=%s arch=%s output=%s\n' "$LANGUAGE" "$ARCH" "$OUTPUT"
