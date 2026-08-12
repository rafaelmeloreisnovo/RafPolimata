#!/data/data/com.termux/files/usr/bin/sh
set -eu

# Independent verifier for an archived ApkC/RAFAELIA Termux ARM32 run.
# It never upgrades runtime claims; it only verifies custody/integrity artifacts.

RUN_DIR=${1:-}
if [ -z "$RUN_DIR" ] || [ ! -d "$RUN_DIR" ]; then
  printf '%s\n' 'usage: apkc_verify_termux_receipt.sh <run-dir>' >&2
  exit 64
fi

RECEIPT="$RUN_DIR/receipt.sha256"
STATUS="$RUN_DIR/finalization-status.txt"
EXIT_REC="$RUN_DIR/run-exit.txt"

command -v sha256sum >/dev/null 2>&1 || {
  printf '%s\n' 'FAIL: sha256sum unavailable' >&2
  exit 69
}

for f in "$RECEIPT" "$STATUS" "$EXIT_REC"; do
  [ -s "$f" ] || {
    printf 'FAIL: required custody artifact missing/empty: %s\n' "$f" >&2
    exit 65
  }
done

grep -Fxq 'receipt_status=PASS' "$STATUS" || {
  printf '%s\n' 'FAIL: finalization-status does not assert receipt_status=PASS' >&2
  exit 66
}

gate_line=$(grep '^gate_exit_code=' "$EXIT_REC" || true)
[ -n "$gate_line" ] || {
  printf '%s\n' 'FAIL: run-exit.txt lacks gate_exit_code' >&2
  exit 67
}

# Receipt entries are relative to the run directory; verify from there.
(
  cd "$RUN_DIR"
  sha256sum -c receipt.sha256
) || {
  printf '%s\n' 'FAIL: receipt hash verification failed' >&2
  exit 68
}

# The status file must preserve the same original gate result as run-exit.txt.
status_gate=$(grep '^gate_exit_code=' "$STATUS" || true)
[ "$status_gate" = "$gate_line" ] || {
  printf 'FAIL: gate exit mismatch: run=%s status=%s\n' "$gate_line" "${status_gate:-TOKEN_VAZIO}" >&2
  exit 70
}

printf 'PASS: custody receipt verified; %s; claim_allowed remains false\n' "$gate_line"
