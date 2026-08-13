#!/usr/bin/env bash
set -u

# Build an append-only Android evidence receipt after hardened install/launch capture.
# This receipt proves integrity/custody of observed artifacts only; it does not
# promote runtime semantic health or any global ApkC claim.

OUT_DIR="${1:-}"
[ -n "$OUT_DIR" ] || { printf '%s\n' 'usage: apkc_android_final_receipt.sh OUT_DIR' >&2; exit 64; }
[ -d "$OUT_DIR" ] || { printf '%s\n' 'OUT_DIR missing' >&2; exit 65; }
command -v sha256sum >/dev/null 2>&1 || { printf '%s\n' 'sha256sum missing' >&2; exit 66; }
command -v sort >/dev/null 2>&1 || { printf '%s\n' 'sort missing' >&2; exit 67; }

RECEIPT_DIR="$OUT_DIR/11_android_receipt"
TARGET_FILE="$OUT_DIR/06_sign_gate/install-target.txt"
STATE="$RECEIPT_DIR/android-final-state.tsv"
MANIFEST="$RECEIPT_DIR/receipt.sha256"
VERIFY="$RECEIPT_DIR/receipt-verify.txt"

mkdir -p "$RECEIPT_DIR"
rm -f "$STATE" "$MANIFEST" "$VERIFY"

fail() {
  rc="$1"; shift
  {
    printf 'receipt_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'runtime_semantic_pass=TOKEN_VAZIO\n'
    printf 'reason=%s\n' "$*"
  } > "$STATE"
  exit "$rc"
}

[ -f "$TARGET_FILE" ] || fail 68 'install target evidence missing'
[ "$(wc -l < "$TARGET_FILE" | tr -d ' ')" = 1 ] || fail 69 'install target must contain exactly one line'
IFS= read -r TARGET < "$TARGET_FILE" || fail 70 'cannot read install target'
[ -n "$TARGET" ] || fail 71 'empty install target'
[ "$TARGET" != TOKEN_VAZIO ] || fail 72 'install target is TOKEN_VAZIO'

# Confinement: the selected artifact must be owned by this OUT_DIR. The hardened
# bridge already enforces raw/signed policy; this receipt independently refuses
# external or parent-traversal paths.
case "$TARGET" in
  "$OUT_DIR"/*) ;;
  *) fail 73 'install target escapes OUT_DIR' ;;
esac
case "$TARGET" in
  *'/../'*|*/..|../*|..) fail 74 'install target contains parent traversal' ;;
esac
[ -s "$TARGET" ] || fail 75 'selected APK missing or empty'

# A final physical-chain receipt requires actual install and launch command
# evidence. Logcat is evidence capture, not semantic runtime PASS.
for required in \
  "$OUT_DIR/08_install_hardened.txt" \
  "$OUT_DIR/09_launch_hardened.txt" \
  "$OUT_DIR/10_logcat_hardened.txt" \
  "$OUT_DIR/status.tsv"; do
  [ -s "$required" ] || fail 76 "required evidence missing: $required"
done

TARGET_SHA="$(sha256sum "$TARGET" | awk '{print $1}')" || fail 77 'cannot hash selected APK'
case "$TARGET_SHA" in
  [0-9a-f][0-9a-f][0-9a-f][0-9a-f]*) ;;
  *) fail 78 'invalid selected APK digest' ;;
esac
[ "${#TARGET_SHA}" = 64 ] || fail 78 'invalid selected APK digest length'

{
  printf 'receipt_status=PASS\n'
  printf 'claim_allowed=false\n'
  printf 'runtime_semantic_pass=TOKEN_VAZIO\n'
  printf 'install_target=%s\n' "$TARGET"
  printf 'install_target_sha256=%s\n' "$TARGET_SHA"
} > "$STATE"

# Hash only immutable evidence files and the frozen state. receipt-verify.txt is
# deliberately outside the covered set because verification output is mutable.
FILES="$(mktemp)" || fail 79 'mktemp failed'
trap 'rm -f "$FILES"' EXIT HUP INT TERM
{
  printf '%s\n' "$STATE"
  printf '%s\n' "$TARGET_FILE"
  printf '%s\n' "$TARGET"
  printf '%s\n' "$OUT_DIR/08_install_hardened.txt"
  printf '%s\n' "$OUT_DIR/09_launch_hardened.txt"
  printf '%s\n' "$OUT_DIR/10_logcat_hardened.txt"
  printf '%s\n' "$OUT_DIR/status.tsv"
  [ -s "$OUT_DIR/06_sign_gate/signing-status.txt" ] && printf '%s\n' "$OUT_DIR/06_sign_gate/signing-status.txt"
  [ -s "$OUT_DIR/06_sign_gate/apksigner-verify.txt" ] && printf '%s\n' "$OUT_DIR/06_sign_gate/apksigner-verify.txt"
} | LC_ALL=C sort -u > "$FILES"

: > "$MANIFEST"
while IFS= read -r file; do
  [ -f "$file" ] || fail 80 "receipt input disappeared: $file"
  sha256sum "$file" >> "$MANIFEST" || fail 81 "hash failed: $file"
done < "$FILES"

if sha256sum -c "$MANIFEST" > "$VERIFY" 2>&1; then
  printf '%s\n' 'receipt_status=PASS' >> "$VERIFY"
  printf '%s\n' 'claim_allowed=false' >> "$VERIFY"
  printf '%s\n' 'runtime_semantic_pass=TOKEN_VAZIO' >> "$VERIFY"
  exit 0
fi
printf '%s\n' 'receipt_status=FAIL' >> "$VERIFY"
printf '%s\n' 'claim_allowed=false' >> "$VERIFY"
exit 82
