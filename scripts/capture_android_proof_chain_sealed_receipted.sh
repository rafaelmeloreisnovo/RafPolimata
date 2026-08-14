#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SEALED_CAPTURE=${APKC_SEALED_CAPTURE:-$ROOT/scripts/capture_android_proof_chain_sealed_hardened.sh}
FINALIZER=${APKC_INSTALL_RECEIPT_FINALIZER:-$ROOT/scripts/apkc_finalize_install_receipt.sh}
OUT_DIR=${OUT_DIR:-proofs/run-arm64-full-chain/out}

fail() {
  local rc=$1 reason=$2
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'receipted_bridge_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'reason=%s\n' "$reason"
  } > "$OUT_DIR/receipted-bridge.status" 2>/dev/null || true
  exit "$rc"
}

[ -x "$SEALED_CAPTURE" ] || [ -f "$SEALED_CAPTURE" ] || fail 161 'sealed capture missing'
[ -x "$FINALIZER" ] || [ -f "$FINALIZER" ] || fail 162 'install receipt finalizer missing'

mkdir -p "$OUT_DIR" || fail 163 'cannot create output directory'
{
  printf 'receipted_bridge_status=RUNNING\n'
  printf 'claim_allowed=false\n'
  printf 'authority=sealed_snapshot_plus_installer_exit\n'
} > "$OUT_DIR/receipted-bridge.status" || fail 164 'cannot write bridge status'

set +e
OUT_DIR="$OUT_DIR" bash "$SEALED_CAPTURE"
capture_rc=$?
bash "$FINALIZER" "$OUT_DIR"
final_rc=$?
set -e

if [ "$final_rc" -ne 0 ]; then
  {
    printf 'receipted_bridge_status=FAIL\n'
    printf 'capture_exit=%s\n' "$capture_rc"
    printf 'finalizer_exit=%s\n' "$final_rc"
    printf 'claim_allowed=false\n'
    printf 'reason=COMBINED_INSTALL_RECEIPT_REJECTED\n'
  } > "$OUT_DIR/receipted-bridge.status"
  exit "$final_rc"
fi

if [ "$capture_rc" -ne 0 ]; then
  {
    printf 'receipted_bridge_status=FAIL\n'
    printf 'capture_exit=%s\n' "$capture_rc"
    printf 'finalizer_exit=0\n'
    printf 'claim_allowed=false\n'
    printf 'reason=CAPTURE_NONZERO_DESPITE_FINAL_RECEIPT\n'
  } > "$OUT_DIR/receipted-bridge.status"
  exit "$capture_rc"
fi

{
  printf 'receipted_bridge_status=PASS\n'
  printf 'capture_exit=0\n'
  printf 'finalizer_exit=0\n'
  printf 'claim_allowed=false\n'
  printf 'reason=SNAPSHOT_AND_INSTALLER_PASS\n'
} > "$OUT_DIR/receipted-bridge.status"
exit 0
