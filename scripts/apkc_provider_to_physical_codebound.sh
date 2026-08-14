#!/usr/bin/env bash
set -euo pipefail
ARTIFACT=${1:-}
OUT_DIR=${2:-}
MODE=${3:-probe-only}
[ -n "$ARTIFACT" ] && [ -n "$OUT_DIR" ] || {
  echo "usage: $0 ARTIFACT_ZIP OUT_DIR [extract-only|probe-only|adb-shell-pm|pm-local]" >&2
  exit 221
}
case "$MODE" in extract-only|probe-only|adb-shell-pm|pm-local) ;; *) echo "codebound_status=FAIL reason=UNKNOWN_MODE claim_allowed=false" >&2; exit 222;; esac
ROOT=${APKC_CODEBOUND_ROOT:-${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}}
CHAIN=${APKC_CODEBOUND_CHAIN:-$ROOT/scripts/apkc_provider_to_physical_receipted.sh}
SEAL=${APKC_CODEBOUND_SEALER:-$ROOT/scripts/apkc_seal_receipt_tree.sh}

if [ -e "$OUT_DIR" ] && [ -n "$(find "$OUT_DIR" -mindepth 1 -print -quit 2>/dev/null)" ]; then
  echo "codebound_status=FAIL reason=OUT_DIR_NOT_EMPTY claim_allowed=false" >&2
  exit 223
fi
mkdir -p "$OUT_DIR"
STATUS="$OUT_DIR/codebound.status"
MANIFEST="$OUT_DIR/gate-code.sha256"
VERIFY="$OUT_DIR/gate-code-verify.txt"
EXEC_DIR="$OUT_DIR/execution"

command -v sha256sum >/dev/null 2>&1 || { printf 'codebound_status=FAIL\nreason=SHA256SUM_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 224; }
[ -f "$CHAIN" ] || { printf 'codebound_status=FAIL\nreason=CHAIN_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 225; }
[ -f "$SEAL" ] || { printf 'codebound_status=FAIL\nreason=SEALER_MISSING\nclaim_allowed=false\n' > "$STATUS"; exit 226; }

DEPS=(
  scripts/apkc_provider_to_physical_receipted.sh
  scripts/apkc_verify_provider_c04_transfer.sh
  scripts/apkc_termux_sealed_stdin_gate_receipted.sh
  scripts/apkc_termux_sealed_stdin_gate.sh
  scripts/apkc_install_sealed_stdin.sh
  scripts/apkc_install_sealed_stdin.c
  scripts/apkc_finalize_stdin_install_receipt.sh
  scripts/apkc_probe_memfd_runtime.c
  scripts/apkc_seal_receipt_tree.sh
)
: > "$MANIFEST"
for rel in "${DEPS[@]}"; do
  path="$ROOT/$rel"
  [ -f "$path" ] && [ ! -L "$path" ] || {
    printf 'codebound_status=FAIL\nreason=GATE_DEPENDENCY_MISSING_OR_SYMLINK\ndependency=%s\nclaim_allowed=false\n' "$rel" > "$STATUS"
    exit 227
  }
  digest=$(sha256sum "$path" | awk '{print $1}')
  printf '%s  %s\n' "$digest" "$rel" >> "$MANIFEST"
done

(
  cd "$ROOT"
  sha256sum -c "$MANIFEST" > "$VERIFY" 2>&1
) || { printf 'codebound_status=FAIL\nreason=GATE_CODE_VERIFY_FAILED\nclaim_allowed=false\n' > "$STATUS"; exit 228; }

printf 'codebound_status=IN_PROGRESS\nmode=%s\ndependency_count=%s\nclaim_allowed=false\n' "$MODE" "${#DEPS[@]}" > "$STATUS"
set +e
bash "$CHAIN" "$ARTIFACT" "$EXEC_DIR" "$MODE"
chain_rc=$?
set -e

if [ "$chain_rc" -eq 0 ]; then
  printf 'codebound_status=PASS\nmode=%s\ndependency_count=%s\nchain_exit=0\nclaim_allowed=false\n' "$MODE" "${#DEPS[@]}" > "$STATUS"
else
  printf 'codebound_status=FAIL\nmode=%s\ndependency_count=%s\nchain_exit=%s\nreason=CHAIN_REJECTED\nclaim_allowed=false\n' "$MODE" "${#DEPS[@]}" "$chain_rc" > "$STATUS"
fi

# Seal code manifest + verifier + complete child execution tree as one evidence root.
bash "$SEAL" "$OUT_DIR" codebound-tree.sha256 || exit 229
(cd "$OUT_DIR" && sha256sum -c codebound-tree.sha256 >/dev/null) || exit 230

[ "$chain_rc" -eq 0 ] || exit "$chain_rc"
exit 0
