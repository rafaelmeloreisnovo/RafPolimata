#!/usr/bin/env bash
set -euo pipefail
ARTIFACT=${1:-}
OUT_DIR=${2:-}
MODE=${3:-probe-only}
[ -n "$ARTIFACT" ] && [ -n "$OUT_DIR" ] || {
  echo "usage: $0 ARTIFACT_ZIP OUT_DIR [extract-only|probe-only|adb-shell-pm|pm-local]" >&2
  exit 291
}
case "$MODE" in extract-only|probe-only|adb-shell-pm|pm-local) ;; *) echo "provider_physical_chain_status=FAIL reason=UNKNOWN_MODE claim_allowed=false" >&2; exit 292;; esac
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
TRANSFER=${APKC_PROVIDER_TRANSFER_GATE:-$ROOT/scripts/apkc_verify_provider_c04_transfer.sh}
PHYSICAL_RECEIPTED=${APKC_PHYSICAL_RECEIPTED_GATE:-$ROOT/scripts/apkc_termux_sealed_stdin_gate_receipted.sh}
SEAL=${APKC_RECEIPT_TREE_SEALER:-$ROOT/scripts/apkc_seal_receipt_tree.sh}

if [ -e "$OUT_DIR" ] && find "$OUT_DIR" -mindepth 1 -print -quit 2>/dev/null | grep -q .; then
  echo "provider_physical_chain_status=FAIL reason=OUT_DIR_NOT_EMPTY claim_allowed=false" >&2
  exit 293
fi
mkdir -p "$OUT_DIR"
STATUS="$OUT_DIR/provider-physical-chain.status"
[ -f "$TRANSFER" ] || { printf 'provider_physical_chain_status=FAIL\nreason=TRANSFER_GATE_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 294; }
[ -f "$SEAL" ] || { printf 'provider_physical_chain_status=FAIL\nreason=RECEIPT_TREE_SEALER_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 295; }
if [ "$MODE" != extract-only ] && [ ! -f "$PHYSICAL_RECEIPTED" ]; then
  printf 'provider_physical_chain_status=FAIL\nreason=PHYSICAL_RECEIPTED_GATE_MISSING\nclaim_allowed=false\n' > "$STATUS"
  exit 296
fi

printf 'provider_physical_chain_status=IN_PROGRESS\nmode=%s\nclaim_allowed=false\n' "$MODE" > "$STATUS"
set +e
APKC_TERMUX_GATE="$PHYSICAL_RECEIPTED" bash "$TRANSFER" "$ARTIFACT" "$OUT_DIR" "$MODE"
transfer_rc=$?
set -e

if [ "$transfer_rc" -eq 0 ]; then
  printf 'provider_physical_chain_status=PASS\nmode=%s\ntransfer_exit=0\nrecursive_binding=provider_to_physical_tree\nclaim_allowed=false\n' "$MODE" > "$STATUS"
else
  printf 'provider_physical_chain_status=FAIL\nmode=%s\ntransfer_exit=%s\nreason=TRANSFER_OR_PHYSICAL_GATE_REJECTED\nclaim_allowed=false\n' "$MODE" "$transfer_rc" > "$STATUS"
fi

# Seal the whole evidence root after the final status is written. This binds
# top-level provider/artifact evidence and every nested physical receipt tree.
bash "$SEAL" "$OUT_DIR" provider-physical-tree.sha256 || exit 297
(cd "$OUT_DIR" && sha256sum -c provider-physical-tree.sha256 >/dev/null) || exit 298

[ "$transfer_rc" -eq 0 ] || exit "$transfer_rc"
exit 0
