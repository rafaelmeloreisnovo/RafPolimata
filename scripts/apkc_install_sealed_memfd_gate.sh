#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/scripts/apkc_install_sealed_memfd.c"
APK=${1:-}
EXPECTED_SHA256=${2:-}
OUT_DIR=${3:-}

fail() {
  local rc=$1 msg=$2
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'sealed_launcher_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
    printf 'reason=%s\n' "$msg"
  } > "$OUT_DIR/sealed-launcher.status" 2>/dev/null || true
  exit "$rc"
}

[ -n "$APK" ] && [ -n "$EXPECTED_SHA256" ] && [ -n "$OUT_DIR" ] || fail 131 'missing launcher arguments'
[ -f "$SRC" ] || fail 132 'sealed memfd C source missing'
command -v sha256sum >/dev/null 2>&1 || fail 133 'sha256sum unavailable'

CC_BIN=${CC:-}
if [ -z "$CC_BIN" ]; then
  CC_BIN="$(command -v clang 2>/dev/null || command -v cc 2>/dev/null || true)"
fi
[ -n "$CC_BIN" ] || fail 134 'no C compiler available for sealed memfd gate'

BIN_DIR="${APKC_MEMFD_BIN_DIR:-$OUT_DIR/.apkc-bin}"
BIN="$BIN_DIR/apkc_install_sealed_memfd"
TMP="$BIN.tmp.$$"
mkdir -p "$BIN_DIR" || fail 135 'cannot create sealed helper bin directory'

SRC_SHA="$(sha256sum "$SRC" | awk '{print $1}')" || fail 136 'cannot hash sealed helper source'
COMPILER_VERSION="$($CC_BIN --version 2>/dev/null | head -n 1 || true)"

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror "$SRC" -o "$TMP" || {
  rm -f "$TMP"
  fail 137 'sealed memfd helper compilation failed'
}
mv -f "$TMP" "$BIN" || fail 138 'cannot install sealed memfd helper binary'
chmod 700 "$BIN" || fail 139 'cannot chmod sealed memfd helper binary'
BIN_SHA="$(sha256sum "$BIN" | awk '{print $1}')" || fail 140 'cannot hash sealed helper binary'

{
  printf 'sealed_launcher_status=READY\n'
  printf 'claim_allowed=false\n'
  printf 'source_sha256=%s\n' "$SRC_SHA"
  printf 'binary_sha256=%s\n' "$BIN_SHA"
  printf 'compiler=%s\n' "$CC_BIN"
  printf 'compiler_version=%s\n' "$COMPILER_VERSION"
  printf 'same_inode_inplace_protection=DELEGATED_TO_SEALED_MEMFD_GATE\n'
} > "$OUT_DIR/sealed-launcher.status"

exec "$BIN" "$@"
