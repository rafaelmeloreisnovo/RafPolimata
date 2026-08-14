#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SEALED_GATE="$ROOT/scripts/apkc_install_sealed_memfd_gate.sh"
FD_WRAPPER="$ROOT/scripts/capture_android_proof_chain_fd_hardened.sh"
OUT_DIR="${OUT_DIR:-proofs/run-arm64-full-chain/out}"

fail() {
  local rc=$1 msg=$2
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'sealed_bridge_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
    printf 'reason=%s\n' "$msg"
  } > "$OUT_DIR/sealed-bridge.status" 2>/dev/null || true
  exit "$rc"
}

[ -f "$SEALED_GATE" ] || fail 141 'sealed memfd gate missing'
[ -f "$FD_WRAPPER" ] || fail 142 'existing fd-hardened wrapper missing'

{
  printf 'sealed_bridge_status=READY\n'
  printf 'claim_allowed=false\n'
  printf 'same_inode_inplace_protection=MITIGATION_PATH_SELECTED\n'
  printf 'android_memfd_compatibility=TOKEN_VAZIO\n'
  printf 'adb_proc_self_fd_compatibility=TOKEN_VAZIO\n'
  printf 'pm_proc_self_fd_compatibility=TOKEN_VAZIO\n'
} > "$OUT_DIR/sealed-bridge.status"

export APKC_FD_HANDOFF_GATE="$SEALED_GATE"
export OUT_DIR
exec bash "$FD_WRAPPER"
