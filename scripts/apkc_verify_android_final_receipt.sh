#!/usr/bin/env bash
set -u

# Independent, fail-closed verifier for ApkC Android final custody receipts.
# Verifies canonical state, exact manifest coverage, confinement and SHA-256.
# It validates custody/structure only; it never promotes runtime semantics.

OUT_DIR="${1:-}"
[ -n "$OUT_DIR" ] || { printf '%s\n' 'usage: apkc_verify_android_final_receipt.sh OUT_DIR' >&2; exit 64; }
[ -d "$OUT_DIR" ] || { printf '%s\n' 'OUT_DIR missing' >&2; exit 65; }
command -v sha256sum >/dev/null 2>&1 || { printf '%s\n' 'sha256sum missing' >&2; exit 66; }
command -v sort >/dev/null 2>&1 || { printf '%s\n' 'sort missing' >&2; exit 67; }
command -v cmp >/dev/null 2>&1 || { printf '%s\n' 'cmp missing' >&2; exit 67; }

OUT_DIR="$(cd "$OUT_DIR" 2>/dev/null && pwd -P)" || { printf '%s\n' 'cannot canonicalize OUT_DIR' >&2; exit 65; }
RECEIPT_DIR="$OUT_DIR/11_android_receipt"
STATE="$RECEIPT_DIR/android-final-state.tsv"
MANIFEST="$RECEIPT_DIR/receipt.sha256"
TARGET_FILE="$OUT_DIR/06_sign_gate/install-target.txt"

[ -f "$STATE" ] || { printf '%s\n' 'android final state missing' >&2; exit 68; }
[ -f "$MANIFEST" ] || { printf '%s\n' 'receipt manifest missing' >&2; exit 68; }
[ -f "$TARGET_FILE" ] || { printf '%s\n' 'install target evidence missing' >&2; exit 68; }

# Canonical state schema: exactly six ordered lines, no duplicates or extensions.
[ "$(wc -l < "$STATE" | tr -d ' ')" = 6 ] || { printf '%s\n' 'non-canonical state line count' >&2; exit 69; }
mapfile -t S < "$STATE" || { printf '%s\n' 'cannot read state' >&2; exit 69; }
[ "${S[0]:-}" = 'receipt_status=PASS' ] || { printf '%s\n' 'receipt status is not PASS' >&2; exit 69; }
[ "${S[1]:-}" = 'claim_allowed=false' ] || { printf '%s\n' 'claim gate mismatch' >&2; exit 69; }
[ "${S[2]:-}" = 'runtime_semantic_pass=TOKEN_VAZIO' ] || { printf '%s\n' 'runtime semantic boundary mismatch' >&2; exit 69; }
case "${S[3]:-}" in install_target_rel=*) TARGET_REL="${S[3]#install_target_rel=}" ;; *) printf '%s\n' 'install_target_rel missing/out of order' >&2; exit 69 ;; esac
case "${S[4]:-}" in install_target_sha256=*) TARGET_SHA="${S[4]#install_target_sha256=}" ;; *) printf '%s\n' 'install_target_sha256 missing/out of order' >&2; exit 69 ;; esac
[ "${S[5]:-}" = 'manifest_path_mode=OUT_DIR_RELATIVE_V1' ] || { printf '%s\n' 'manifest path mode mismatch' >&2; exit 69; }

case "$TARGET_REL" in ''|/*|../*|*/../*|*/..|..|.|./*|*/./*|*/.|*//*|*\\*) printf '%s\n' 'non-canonical install target path' >&2; exit 70 ;; esac
case "$TARGET_REL" in *$'\n'*|*$'\r'*|*$'\t'*) printf '%s\n' 'install target contains control character' >&2; exit 70 ;; esac
case "$TARGET_SHA" in *[!0-9a-f]*|'') printf '%s\n' 'non-canonical target digest' >&2; exit 70 ;; esac
[ "${#TARGET_SHA}" = 64 ] || { printf '%s\n' 'target digest length mismatch' >&2; exit 70; }
[ -s "$OUT_DIR/$TARGET_REL" ] || { printf '%s\n' 'selected APK missing or empty' >&2; exit 71; }
OBSERVED_TARGET_SHA="$(sha256sum "$OUT_DIR/$TARGET_REL" | awk '{print $1}')" || exit 71
[ "$OBSERVED_TARGET_SHA" = "$TARGET_SHA" ] || { printf '%s\n' 'selected APK digest mismatch' >&2; exit 71; }

# install-target.txt is historical selection evidence. Relative form must equal the
# canonical identity. Legacy absolute form is allowed for relocation compatibility,
# but must end in the exact canonical relative identity. This binds the historical
# selector to the frozen target without requiring the old OUT_DIR to still exist.
[ "$(wc -l < "$TARGET_FILE" | tr -d ' ')" = 1 ] || { printf '%s\n' 'install target evidence must contain exactly one line' >&2; exit 72; }
IFS= read -r TARGET_RAW < "$TARGET_FILE" || exit 72
[ -n "$TARGET_RAW" ] && [ "$TARGET_RAW" != 'TOKEN_VAZIO' ] || { printf '%s\n' 'invalid install target evidence' >&2; exit 72; }
case "$TARGET_RAW" in *$'\n'*|*$'\r'*|*$'\t'*) printf '%s\n' 'install target evidence contains control character' >&2; exit 72 ;; esac
case "$TARGET_RAW" in
  /*)
    case "$TARGET_RAW" in
      *'/../'*|*/..|*/./*|*/.|*//*|*\\*) printf '%s\n' 'legacy absolute install target is non-canonical' >&2; exit 72 ;;
    esac
    case "$TARGET_RAW" in
      *"/$TARGET_REL") : ;;
      *) printf '%s\n' 'legacy absolute install target disagrees with final state' >&2; exit 72 ;;
    esac
    ;;
  *)
    [ "$TARGET_RAW" = "$TARGET_REL" ] || { printf '%s\n' 'relative install target disagrees with final state' >&2; exit 72; }
    ;;
esac

# Reconstruct the producer's exact eligible set independently.
EXPECTED="$(mktemp)" || exit 73
ACTUAL="$(mktemp)" || { rm -f "$EXPECTED"; exit 73; }
trap 'rm -f "$EXPECTED" "$ACTUAL"' EXIT HUP INT TERM
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
} | LC_ALL=C sort -u > "$EXPECTED"

: > "$ACTUAL"
PREV=''
while IFS= read -r line; do
  digest="${line%%  *}"
  rel="${line#*  }"
  [ "$rel" != "$line" ] || { printf '%s\n' 'malformed manifest line' >&2; exit 74; }
  case "$digest" in *[!0-9a-f]*|'') printf '%s\n' 'non-canonical manifest digest' >&2; exit 74 ;; esac
  [ "${#digest}" = 64 ] || { printf '%s\n' 'manifest digest length mismatch' >&2; exit 74; }
  case "$rel" in ''|/*|../*|*/../*|*/..|..|.|./*|*/./*|*/.|*//*|*\\*) printf '%s\n' 'non-canonical manifest path' >&2; exit 74 ;; esac
  [ "$rel" != '11_android_receipt/receipt.sha256' ] || { printf '%s\n' 'manifest self-reference forbidden' >&2; exit 74; }
  [ "$rel" != '11_android_receipt/receipt-verify.txt' ] || { printf '%s\n' 'mutable verifier output forbidden' >&2; exit 74; }
  if [ -n "$PREV" ]; then
    [ "$(printf '%s\n%s\n' "$PREV" "$rel" | LC_ALL=C sort | head -n1)" = "$PREV" ] || { printf '%s\n' 'manifest paths not canonically ordered' >&2; exit 75; }
    [ "$PREV" != "$rel" ] || { printf '%s\n' 'duplicate manifest path' >&2; exit 75; }
  fi
  PREV="$rel"
  printf '%s\n' "$rel" >> "$ACTUAL"
done < "$MANIFEST"

cmp -s "$EXPECTED" "$ACTUAL" || { printf '%s\n' 'manifest exact coverage mismatch' >&2; exit 76; }

# Cryptographic integrity is last: semantic/canonical failures cannot hide behind
# a recomputed but structurally invalid manifest.
if ! (cd "$OUT_DIR" && sha256sum -c '11_android_receipt/receipt.sha256') >/dev/null 2>&1; then
  printf '%s\n' 'receipt hash verification failed' >&2
  exit 77
fi

printf '%s\n' 'receipt_status=PASS'
printf '%s\n' 'claim_allowed=false'
printf '%s\n' 'runtime_semantic_pass=TOKEN_VAZIO'
printf '%s\n' 'independent_verifier=PASS'
exit 0
