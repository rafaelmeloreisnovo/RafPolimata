#!/usr/bin/env bash
set -u
APK=${1:-}; EXPECTED=${2:-}; OUT_DIR=${3:-}; MODE=${4:-probe-only}
[ -n "$APK" ] && [ -n "$EXPECTED" ] && [ -n "$OUT_DIR" ] || { echo "usage: $0 APK EXPECTED_SHA256 OUT_DIR [probe-only|adb-shell-pm|pm-local]" >&2; exit 211; }
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
PROBE_SRC=${APKC_MEMFD_PROBE_SRC:-$ROOT/scripts/apkc_probe_memfd_runtime.c}
INSTALLER=${APKC_SEALED_STDIN_LAUNCHER:-$ROOT/scripts/apkc_install_sealed_stdin.sh}
CC_BIN=${CC:-cc}
ADB_BIN=${APKC_ADB_BIN:-adb}
mkdir -p "$OUT_DIR" || exit 212
STATUS="$OUT_DIR/termux-sealed-stdin-gate.status"
ENVF="$OUT_DIR/environment.txt"
PROBE_OUT="$OUT_DIR/memfd-probe.txt"
INSTALL_OUT="$OUT_DIR/install-stdout.txt"
INSTALL_ERR="$OUT_DIR/install-stderr.txt"
write_status(){ printf 'gate_status=%s\nmode=%s\nclaim_allowed=false\nreason=%s\n' "$1" "$MODE" "$2" >"$STATUS"; }
write_env(){
  machine=$(uname -m 2>/dev/null || printf TOKEN_VAZIO)
  prefix=${PREFIX:-TOKEN_VAZIO}
  if command -v getprop >/dev/null 2>&1; then env_class=ANDROID_SHELL_MEASURED; else env_class=NON_ANDROID_OR_UNPROVEN; fi
  case "$prefix" in /data/data/com.termux/*) env_class=TERMUX_ANDROID_MEASURED;; esac
  {
    printf 'environment_class=%s\n' "$env_class"
    printf 'machine=%s\n' "$machine"
    printf 'prefix=%s\n' "$prefix"
    printf 'termux_version=%s\n' "${TERMUX_VERSION:-TOKEN_VAZIO}"
    if command -v getprop >/dev/null 2>&1; then
      printf 'android_sdk=%s\n' "$(getprop ro.build.version.sdk 2>/dev/null || printf TOKEN_VAZIO)"
      printf 'android_release=%s\n' "$(getprop ro.build.version.release 2>/dev/null || printf TOKEN_VAZIO)"
      printf 'android_abi=%s\n' "$(getprop ro.product.cpu.abi 2>/dev/null || printf TOKEN_VAZIO)"
    else
      printf 'android_sdk=TOKEN_VAZIO\nandroid_release=TOKEN_VAZIO\nandroid_abi=TOKEN_VAZIO\n'
    fi
    printf 'claim_allowed=false\n'
  } >"$ENVF"
}
write_env
case "$EXPECTED" in (*[!0-9a-f]*|'') write_status FAIL EXPECTED_SHA256_MALFORMED; exit 225;; esac
[ "${#EXPECTED}" -eq 64 ] || { write_status FAIL EXPECTED_SHA256_MALFORMED; exit 225; }
[ -f "$APK" ] && [ -s "$APK" ] || { write_status FAIL APK_MISSING_OR_EMPTY; exit 213; }
command -v sha256sum >/dev/null 2>&1 || { write_status FAIL SHA256SUM_MISSING; exit 214; }
ACTUAL=$(sha256sum "$APK" | awk '{print $1}')
printf 'expected_sha256=%s\nactual_sha256=%s\ndigest_match=%s\nclaim_allowed=false\n' "$EXPECTED" "$ACTUAL" "$([ "$ACTUAL" = "$EXPECTED" ] && printf true || printf false)" > "$OUT_DIR/apk-digest.status"
[ "$ACTUAL" = "$EXPECTED" ] || { write_status FAIL APK_DIGEST_MISMATCH; exit 215; }
[ -f "$PROBE_SRC" ] || { write_status FAIL MEMFD_PROBE_SOURCE_MISSING; exit 216; }
command -v "$CC_BIN" >/dev/null 2>&1 || { write_status FAIL COMPILER_MISSING; exit 217; }
PROBE_BIN=$(mktemp "${TMPDIR:-/tmp}/apkc-memfd-probe.XXXXXX") || { write_status FAIL TMP_CREATE_FAILED; exit 218; }
trap 'rm -f "$PROBE_BIN"' EXIT
"$CC_BIN" -O2 -Wall -Wextra -Werror "$PROBE_SRC" -o "$PROBE_BIN" >"$OUT_DIR/probe-compile.txt" 2>&1 || { write_status FAIL PROBE_COMPILE_FAILED; exit 219; }
set +e
"$PROBE_BIN" >"$PROBE_OUT" 2>&1
probe_rc=$?
set -e
printf 'probe_exit=%s\nclaim_allowed=false\n' "$probe_rc" > "$OUT_DIR/memfd-probe.status"
[ "$probe_rc" -eq 0 ] || { write_status FAIL MEMFD_RUNTIME_PROBE_FAILED; exit "$probe_rc"; }
grep -Fxq 'runtime_probe=PASS' "$PROBE_OUT" || { write_status FAIL MEMFD_PROBE_DID_NOT_DECLARE_PASS; exit 220; }
grep -Fxq 'claim_allowed=false' "$PROBE_OUT" || { write_status FAIL MEMFD_PROBE_CLAIM_GATE_MISSING; exit 221; }
case "$MODE" in
  probe-only)
    printf 'install_attempted=false\ninstall_result=TOKEN_VAZIO_NOT_ATTEMPTED\nclaim_allowed=false\n' > "$OUT_DIR/install.status"
    write_status PASS MEMFD_CAPABILITY_PASS_INSTALL_NOT_ATTEMPTED
    ;;
  adb-shell-pm)
    command -v "$ADB_BIN" >/dev/null 2>&1 || [ -x "$ADB_BIN" ] || { write_status FAIL ADB_MISSING; exit 226; }
    set +e; adb_state=$("$ADB_BIN" get-state 2>"$OUT_DIR/adb-get-state.stderr"); adb_state_rc=$?; set -e
    printf 'adb_get_state_exit=%s\nadb_state=%s\nclaim_allowed=false\n' "$adb_state_rc" "$adb_state" > "$OUT_DIR/adb-device.status"
    [ "$adb_state_rc" -eq 0 ] && [ "$adb_state" = device ] || { write_status FAIL ADB_DEVICE_NOT_READY; exit 227; }
    set +e; serial=$("$ADB_BIN" get-serialno 2>/dev/null); serial_rc=$?; set -e
    if [ "$serial_rc" -eq 0 ] && [ -n "$serial" ]; then serial_hash=$(printf '%s' "$serial" | sha256sum | awk '{print $1}'); else serial_hash=TOKEN_VAZIO; fi
    {
      printf 'serial_sha256=%s\n' "$serial_hash"
      printf 'sdk=%s\n' "$("$ADB_BIN" shell getprop ro.build.version.sdk 2>/dev/null || printf TOKEN_VAZIO)"
      printf 'abi=%s\n' "$("$ADB_BIN" shell getprop ro.product.cpu.abi 2>/dev/null || printf TOKEN_VAZIO)"
      printf 'build_fingerprint=%s\n' "$("$ADB_BIN" shell getprop ro.build.fingerprint 2>/dev/null || printf TOKEN_VAZIO)"
      printf 'claim_allowed=false\n'
    } > "$OUT_DIR/adb-device-identity.status"
    [ -f "$INSTALLER" ] || { write_status FAIL SEALED_STDIN_LAUNCHER_MISSING; exit 222; }
    set +e
    APKC_ADB_BIN="$ADB_BIN" bash "$INSTALLER" "$APK" "$EXPECTED" "$OUT_DIR/install-chain" "$MODE" >"$INSTALL_OUT" 2>"$INSTALL_ERR"
    install_rc=$?
    set -e
    printf 'install_attempted=true\ninstall_exit=%s\nclaim_allowed=false\n' "$install_rc" > "$OUT_DIR/install.status"
    [ "$install_rc" -eq 0 ] || { write_status FAIL PHYSICAL_OR_TRANSPORT_INSTALL_REJECTED; exit "$install_rc"; }
    write_status PASS SEALED_STDIN_INSTALL_CHAIN_PASS
    ;;
  pm-local)
    [ -f "$INSTALLER" ] || { write_status FAIL SEALED_STDIN_LAUNCHER_MISSING; exit 222; }
    set +e
    bash "$INSTALLER" "$APK" "$EXPECTED" "$OUT_DIR/install-chain" "$MODE" >"$INSTALL_OUT" 2>"$INSTALL_ERR"
    install_rc=$?
    set -e
    printf 'install_attempted=true\ninstall_exit=%s\nclaim_allowed=false\n' "$install_rc" > "$OUT_DIR/install.status"
    [ "$install_rc" -eq 0 ] || { write_status FAIL PHYSICAL_OR_TRANSPORT_INSTALL_REJECTED; exit "$install_rc"; }
    write_status PASS SEALED_STDIN_INSTALL_CHAIN_PASS
    ;;
  *) write_status FAIL UNKNOWN_MODE; exit 223;;
esac
RECEIPT="$OUT_DIR/receipt.sha256"
(
  cd "$OUT_DIR"
  find . -maxdepth 1 -type f ! -name 'receipt.sha256' ! -name 'receipt-verify.txt' -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$RECEIPT"
if ! (cd "$OUT_DIR" && sha256sum -c receipt.sha256 > receipt-verify.txt 2>&1); then
  write_status FAIL RECEIPT_VERIFY_FAILED
  exit 224
fi
printf 'final_result=PASS\nclaim_allowed=false\n' >> "$OUT_DIR/receipt-verify.txt"
exit 0
