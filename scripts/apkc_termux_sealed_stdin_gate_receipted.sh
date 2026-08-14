#!/usr/bin/env bash
set -euo pipefail
APK=${1:-}; EXPECTED=${2:-}; OUT_DIR=${3:-}; MODE=${4:-probe-only}
[ -n "$APK" ] && [ -n "$EXPECTED" ] && [ -n "$OUT_DIR" ] || { echo "usage: $0 APK EXPECTED_SHA256 OUT_DIR [probe-only|adb-shell-pm|pm-local]" >&2; exit 281; }
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
GATE=${APKC_PHYSICAL_GATE:-$ROOT/scripts/apkc_termux_sealed_stdin_gate.sh}
SEAL=${APKC_RECEIPT_TREE_SEALER:-$ROOT/scripts/apkc_seal_receipt_tree.sh}
mkdir -p "$OUT_DIR"
STATUS="$OUT_DIR/recursive-custody.status"
printf 'recursive_custody_status=IN_PROGRESS\nmode=%s\nclaim_allowed=false\n' "$MODE" > "$STATUS"
[ -f "$GATE" ] || { printf 'recursive_custody_status=FAIL\nreason=PHYSICAL_GATE_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 282; }
[ -f "$SEAL" ] || { printf 'recursive_custody_status=FAIL\nreason=RECEIPT_TREE_SEALER_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 283; }
set +e
bash "$GATE" "$APK" "$EXPECTED" "$OUT_DIR" "$MODE"
rc=$?
set -e
printf 'physical_gate_exit=%s\n' "$rc" >> "$STATUS"
if [ "$rc" -ne 0 ]; then
  printf 'recursive_custody_status=FAIL\nreason=PHYSICAL_GATE_REJECTED\nclaim_allowed=false\n' >> "$STATUS"
  exit "$rc"
fi
# Replace IN_PROGRESS record before sealing so the final status itself is bound.
printf 'recursive_custody_status=PASS\nmode=%s\nphysical_gate_exit=0\nreason=PHYSICAL_GATE_PASS_TREE_SEALED\nclaim_allowed=false\n' "$MODE" > "$STATUS"
bash "$SEAL" "$OUT_DIR" receipt-tree.sha256
# Verify once more independently after the helper returns.
(cd "$OUT_DIR" && sha256sum -c receipt-tree.sha256 >/dev/null) || exit 284
exit 0
