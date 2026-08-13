#!/usr/bin/env bash
set -u

# Build an append-only Android evidence receipt after hardened install/launch capture.
# The manifest is canonical and relocatable: every covered pathname is relative to
# OUT_DIR and verification is anchored by cd into OUT_DIR. This proves custody of
# observed bytes only; it does not promote runtime semantic health or global claims.

OUT_DIR="${1:-}"
[ -n "$OUT_DIR" ] || { printf '%s\n' 'usage: apkc_android_final_receipt.sh OUT_DIR' >&2; exit 64; }
[ -d "$OUT_DIR" ] || { printf '%s\n' 'OUT_DIR missing' >&2; exit 65; }
command -v sha256sum >/dev/null 2>&1 || { printf '%s\n' 'sha256sum missing' >&2; exit 66; }
command -v sort >/dev/null 2>&1 || { printf '%s\n' 'sort missing' >&2; exit 67; }

OUT_DIR="$(cd "$OUT_DIR" 2>/dev/null && pwd -P)" || { printf '%s\n' 'cannot canonicalize OUT_DIR' >&2; exit 65; }
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
IFS= read -r TARGET_RAW < "$TARGET_FILE" || fail 70 'cannot read install target'
[ -n "$TARGET_RAW" ] || fail 71 'empty install target'
[ "$TARGET_RAW" != TOKEN_VAZIO ] || fail 72 'install target is TOKEN_VAZIO'

# Resolve legacy absolute targets and future relative targets, then independently
# enforce confinement. The receipt records only the relative identity so relocation
# of the complete OUT_DIR does not change the canonical evidence representation.
case "$TARGET_RAW" in
  /*) TARGET="$TARGET_RAW" ;;
  *) TARGET="$OUT_DIR/$TARGET_RAW" ;;
esac
case "$TARGET" in
  "$OUT_DIR"/*) ;;
  *) fail 73 'install target escapes OUT_DIR' ;;
esac
TARGET_REL="${TARGET#"$OUT_DIR"/}"
case "$TARGET_REL" in
  ''|/*|../*|*/../*|*/..|..|.|./*|*/./*|*/.) fail 74 'install target contains non-canonical traversal or dot segment' ;;
esac
case "$TARGET_REL" in
  *$'\n'*|*$'\r'*|*$'\t'*) fail 74 'install target contains forbidden control character' ;;
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
  printf 'install_target_rel=%s\n' "$TARGET_REL"
  printf 'install_target_sha256=%s\n' "$TARGET_SHA"
  printf 'manifest_path_mode=OUT_DIR_RELATIVE_V1\n'
} > "$STATE"

# Build an exact, deterministic set of relative paths. receipt-verify.txt is
# deliberately outside the covered set because verification output is mutable.
FILES="$(mktemp)" || fail 79 'mktemp failed'
trap 'rm -f "$FILES"' EXIT HUP INT TERM
{
  printf '%s\n' '11_android_receipt/android-final-state.tsv'
  printf '%s\n' '06_sign_gate/install-target.txt'
  printf '%s\n' "$TARGET_REL"
  printf '%s\n' '08_install_hardened.txt'
  printf '%s\n' '09_launch_hardened.txt'
  printf '%s\n' '10_logcat_hardened.txt'
  printf '%s\n' 'status.tsv'
  [ -s "$OUT_DIR/06_sign_gate/signing-status.txt" ] && printf '%s\n' '06_sign_gate/signing-status.txt'
  [ -s "$OUT_DIR/06_sign_gate/apksigner-verify.txt" ] && printf '%s\n' '06_sign_gate/apksigner-verify.txt'
} | LC_ALL=C sort -u > "$FILES"

: > "$MANIFEST"
while IFS= read -r rel; do
  case "$rel" in
    ''|/*|../*|*/../*|*/..|..|.|./*|*/./*|*/.) fail 80 "non-canonical receipt path: $rel" ;;
  esac
  [ -f "$OUT_DIR/$rel" ] || fail 80 "receipt input disappeared: $rel"
  (cd "$OUT_DIR" && sha256sum "$rel") >> "$MANIFEST" || fail 81 "hash failed: $rel"
done < "$FILES"

# Verify from the custody root, not from the creator's original working directory.
if (cd "$OUT_DIR" && sha256sum -c '11_android_receipt/receipt.sha256') > "$VERIFY" 2>&1; then
  printf '%s\n' 'receipt_status=PASS' >> "$VERIFY"
  printf '%s\n' 'claim_allowed=false' >> "$VERIFY"
  printf '%s\n' 'runtime_semantic_pass=TOKEN_VAZIO' >> "$VERIFY"
  printf '%s\n' 'manifest_path_mode=OUT_DIR_RELATIVE_V1' >> "$VERIFY"
  exit 0
fi
printf '%s\n' 'receipt_status=FAIL' >> "$VERIFY"
printf '%s\n' 'claim_allowed=false' >> "$VERIFY"
exit 82
