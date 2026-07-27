#!/usr/bin/env bash
set -Eeuo pipefail

export LC_ALL=C
export LANG=C
export TZ=UTC
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-0}"
umask 077

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

command -v "$CLANG_BIN" >/dev/null 2>&1 || { echo "missing compiler: $CLANG_BIN" >&2; exit 69; }
command -v "$PYTHON_BIN" >/dev/null 2>&1 || { echo "missing python: $PYTHON_BIN" >&2; exit 69; }
[[ -s "$SOURCE" ]] || { echo "source missing or empty: $SOURCE" >&2; exit 66; }

case "$ARCH" in
  arm64)
    TARGET="aarch64-linux-android21"
    EXPECT_MACHINE="AArch64"
    ARCH_CFLAGS=(-march=armv8-a+simd)
    ;;
  arm32)
    TARGET="armv7a-linux-androideabi21"
    EXPECT_MACHINE="ARM"
    ARCH_CFLAGS=(-march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv4)
    ;;
  x86_64)
    TARGET="x86_64-linux-android21"
    EXPECT_MACHINE="Advanced Micro Devices X86-64"
    ARCH_CFLAGS=(-march=x86-64)
    ;;
  rv64)
    TARGET="riscv64-linux-android35"
    EXPECT_MACHINE="RISC-V"
    ARCH_CFLAGS=(-march=rv64gc -mabi=lp64d)
    ;;
  *) echo "unsupported architecture: $ARCH" >&2; exit 65 ;;
esac

OUTPUT_DIR="$(dirname "$OUTPUT")"
mkdir -p "$OUTPUT_DIR"
RECEIPT="$OUTPUT.receipt.json"
COMMIT_TMP="$OUTPUT.tmp.$$"
RECEIPT_TMP="$RECEIPT.tmp.$$"
TMP_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/rafaelia-native.XXXXXX")"
COMMITTED=0
cleanup() {
  rm -rf "$TMP_ROOT"
  rm -f "$COMMIT_TMP" "$RECEIPT_TMP"
  if [[ "$COMMITTED" -ne 1 ]]; then
    rm -f "$OUTPUT" "$RECEIPT"
  fi
}
trap cleanup EXIT HUP INT TERM

# The invocation owns this output base. Remove prior pairs immediately so a
# failed new attempt cannot be mistaken for a successful current build.
rm -f "$OUTPUT" "$RECEIPT"

LOWERED="$TMP_ROOT/lowered.c"
REWRITTEN="$TMP_ROOT/rewritten.c"
LOWER_MANIFEST="$TMP_ROOT/lower.json"
REWRITE_MANIFEST="$TMP_ROOT/rewrite.json"
OBJECT="$TMP_ROOT/input.o"
SEALED="$TMP_ROOT/sealed.so"

case "$LANGUAGE" in
  c|cpp)
    "$PYTHON_BIN" "$ROOT/scripts/raf_c_rewrite.py" \
      "$SOURCE" "$REWRITTEN" --manifest "$REWRITE_MANIFEST"
    ;;
  asm)
    "$CLANG_BIN" --target="$TARGET" "${ARCH_CFLAGS[@]}" \
      -c -x assembler "$SOURCE" -o "$OBJECT"
    ;;
  rs|kt|java|py|sh|pl|js|php|jsx|go|rb|swift|groovy|clj)
    "$PYTHON_BIN" "$ROOT/scripts/raf_kernel_lower.py" \
      --language "$LANGUAGE" --source "$SOURCE" --output "$LOWERED" \
      --manifest "$LOWER_MANIFEST"
    "$PYTHON_BIN" "$ROOT/scripts/raf_c_rewrite.py" \
      "$LOWERED" "$REWRITTEN" --manifest "$REWRITE_MANIFEST"
    ;;
  *) echo "language has no strict native route: $LANGUAGE" >&2; exit 65 ;;
esac

if [[ "$LANGUAGE" != asm ]]; then
  CFLAGS=(
    --target="$TARGET" "${ARCH_CFLAGS[@]}" -Os -fPIC -ffreestanding -fno-builtin
    -fno-stack-protector -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables
    -fno-optimize-sibling-calls -fvisibility=hidden -ffunction-sections -fdata-sections
    -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic -nostdinc -I "$ROOT/Apkc"
  )
  if [[ "$LANGUAGE" == cpp ]]; then
    CFLAGS+=(
      -x c++ -std=c++17 -fno-exceptions -fno-rtti -fno-threadsafe-statics
      -fno-use-cxa-atexit
    )
  else
    CFLAGS+=(-std=c11)
  fi
  "$CLANG_BIN" "${CFLAGS[@]}" -c "$REWRITTEN" -o "$OBJECT"
fi

SONAME="$(basename "$OUTPUT")"
"$CLANG_BIN" --target="$TARGET" "${ARCH_CFLAGS[@]}" -fuse-ld=lld \
  -nostdlib -nostartfiles -nodefaultlibs -shared \
  -Wl,--no-undefined -Wl,-z,defs -Wl,-Bsymbolic -Wl,--gc-sections \
  -Wl,--build-id=none -Wl,-z,noexecstack -Wl,-z,text -Wl,--fatal-warnings \
  -Wl,--hash-style=sysv -Wl,-soname,"$SONAME" "$OBJECT" -o "$SEALED"

[[ -s "$SEALED" ]] || { echo "output missing: $SEALED" >&2; exit 74; }
RAF_EXPECT_MACHINE="$EXPECT_MACHINE" \
  bash "$ROOT/scripts/audit_strict_elf.sh" --profile android-so "$SEALED"

sha256_file() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  else
    shasum -a 256 "$1" | awk '{print $1}'
  fi
}

SOURCE_SHA="$(sha256_file "$SOURCE")"
OUTPUT_SHA="$(sha256_file "$SEALED")"
COMPILER_VERSION="$($CLANG_BIN --version | head -n 1)"

cp "$SEALED" "$COMMIT_TMP"
chmod 0644 "$COMMIT_TMP"

"$PYTHON_BIN" - "$RECEIPT_TMP" "$LANGUAGE" "$ARCH" "$TARGET" "$SOURCE" \
  "$SOURCE_SHA" "$OUTPUT" "$OUTPUT_SHA" "$COMPILER_VERSION" \
  "$LOWER_MANIFEST" "$REWRITE_MANIFEST" <<'PY'
import json
from pathlib import Path
import sys

(
    receipt_path, language, arch, target, source, source_sha, output, output_sha,
    compiler_version, lower_manifest_path, rewrite_manifest_path,
) = sys.argv[1:]

def load_optional(path: str):
    candidate = Path(path)
    return json.loads(candidate.read_text(encoding="utf-8")) if candidate.is_file() else None

receipt = {
    "schema": "rafaelia.apkc.strict-native-receipt.v2",
    "status": "STRICT_ELF_PASS",
    "claim_allowed": True,
    "claim_scope": "sealed_self_contained_android_shared_object",
    "language": language,
    "architecture": arch,
    "target": target,
    "source": source,
    "source_sha256": source_sha,
    "output": output,
    "output_sha256": output_sha,
    "compiler": compiler_version,
    "build_plane_dependencies": ["python3", "clang", "lld", "readelf", "nm"],
    "runtime_external_dependencies": [],
    "lowering_manifest": load_optional(lower_manifest_path),
    "rewrite_manifest": load_optional(rewrite_manifest_path),
    "gates": {
        "source_nonempty": "PASS",
        "lowering_or_rewrite": "PASS",
        "no_undefined_symbols": "PASS",
        "no_pt_interp": "PASS",
        "no_dt_needed": "PASS",
        "no_rwx_load": "PASS",
        "no_executable_stack": "PASS",
        "no_build_id": "PASS",
        "machine_identity": "PASS"
    },
    "not_claimed": [
        "apk_signature",
        "android_installation",
        "android_runtime_launch",
        "device_driver_execution",
        "full_source_language_semantics"
    ]
}
Path(receipt_path).write_text(
    json.dumps(receipt, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
    encoding="utf-8"
)
PY
chmod 0644 "$RECEIPT_TMP"

# Promote the pair. If the second rename fails, the EXIT trap removes the first.
mv -f "$COMMIT_TMP" "$OUTPUT"
mv -f "$RECEIPT_TMP" "$RECEIPT"
COMMITTED=1

printf 'apkc_strict_native_build: PASS lang=%s arch=%s output=%s sha256=%s\n' \
  "$LANGUAGE" "$ARCH" "$OUTPUT" "$OUTPUT_SHA"
