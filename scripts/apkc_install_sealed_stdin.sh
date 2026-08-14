#!/usr/bin/env bash
set -u
APK=${1:-}; EXPECTED=${2:-}; OUT_DIR=${3:-}; MODE=${4:-adb-shell-pm}
[ -n "$APK" ] && [ -n "$EXPECTED" ] && [ -n "$OUT_DIR" ] || { echo "usage: $0 APK EXPECTED_SHA256 OUT_DIR [adb-shell-pm|pm-local]" >&2; exit 191; }
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
SRC=${APKC_STDIN_HELPER_SRC:-$ROOT/scripts/apkc_install_sealed_stdin.c}
FINALIZER=${APKC_STDIN_FINALIZER:-$ROOT/scripts/apkc_finalize_stdin_install_receipt.sh}
CC_BIN=${CC:-cc}
mkdir -p "$OUT_DIR" || exit 192
STATUS="$OUT_DIR/sealed-stdin-launcher.status"
write_status(){ printf 'launcher_status=%s\ntransport_request=%s\nclaim_allowed=false\nreason=%s\n' "$1" "$MODE" "$2" >"$STATUS"; }
[ -f "$SRC" ] || { write_status FAIL HELPER_SOURCE_MISSING; exit 193; }
[ -f "$FINALIZER" ] || { write_status FAIL FINALIZER_MISSING; exit 194; }
command -v "$CC_BIN" >/dev/null 2>&1 || { write_status FAIL COMPILER_MISSING; exit 195; }
BIN=$(mktemp "${TMPDIR:-/tmp}/apkc-stdin-helper.XXXXXX") || { write_status FAIL TMP_CREATE_FAILED; exit 196; }
trap 'rm -f "$BIN"' EXIT
"$CC_BIN" -O2 -Wall -Wextra -Werror "$SRC" -o "$BIN" || { write_status FAIL COMPILE_FAILED; exit 197; }
write_status RUNNING STARTED
set +e
case "$MODE" in
  adb-shell-pm)
    ADB=${APKC_ADB_BIN:-adb}
    command -v "$ADB" >/dev/null 2>&1 || [ -x "$ADB" ] || { set -e; write_status FAIL ADB_MISSING; exit 198; }
    "$BIN" "$APK" "$EXPECTED" "$OUT_DIR" -- "$ADB" shell pm install -S @APK_SIZE@ @APK_STDIN@
    helper_rc=$?
    ;;
  pm-local)
    PM=${APKC_PM_BIN:-pm}
    command -v "$PM" >/dev/null 2>&1 || [ -x "$PM" ] || { set -e; write_status FAIL PM_MISSING; exit 199; }
    "$BIN" "$APK" "$EXPECTED" "$OUT_DIR" -- "$PM" install -S @APK_SIZE@ @APK_STDIN@
    helper_rc=$?
    ;;
  *) set -e; write_status FAIL UNKNOWN_MODE; exit 200;;
esac
bash "$FINALIZER" "$OUT_DIR"
final_rc=$?
set -e
if [ "$final_rc" -ne 0 ]; then write_status FAIL FINALIZER_REJECTED; exit "$final_rc"; fi
if [ "$helper_rc" -ne 0 ]; then write_status FAIL HELPER_NONZERO; exit "$helper_rc"; fi
write_status PASS SEALED_STDIN_INSTALL_PASS
exit 0
