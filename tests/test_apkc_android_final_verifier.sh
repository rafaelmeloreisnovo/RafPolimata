#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd -P)"
PRODUCER="$ROOT/scripts/apkc_android_final_receipt.sh"
VERIFIER="$ROOT/scripts/apkc_verify_android_final_receipt.sh"
TMP="$(mktemp -d)" || exit 1
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
PASS=0
FAIL=0

ok() { PASS=$((PASS+1)); printf 'PASS %s\n' "$1"; }
bad() { FAIL=$((FAIL+1)); printf 'FAIL %s\n' "$1" >&2; }
expect_pass() { name="$1"; shift; if "$@" >/dev/null 2>&1; then ok "$name"; else bad "$name"; fi; }
expect_fail() { name="$1"; shift; if "$@" >/dev/null 2>&1; then bad "$name"; else ok "$name"; fi; }

make_fixture() {
  d="$1"
  mkdir -p "$d/06_sign_gate" "$d/07_signed"
  printf '%s\n' '07_signed/app.apk' > "$d/06_sign_gate/install-target.txt"
  printf '%s\n' 'SIGNED_APK_BYTES' > "$d/07_signed/app.apk"
  printf '%s\n' 'install_rc=0' > "$d/08_install_hardened.txt"
  printf '%s\n' 'launch_rc=0' > "$d/09_launch_hardened.txt"
  printf '%s\n' 'logcat_capture=present_not_semantic_pass' > "$d/10_logcat_hardened.txt"
  printf '%s\n' 'claim_allowed=false' 'runtime_semantic_pass=TOKEN_VAZIO' > "$d/status.tsv"
  bash "$PRODUCER" "$d" >/dev/null 2>&1
}

BASE="$TMP/base"
make_fixture "$BASE" || { printf '%s\n' 'fixture producer failed' >&2; exit 2; }
expect_pass positive bash "$VERIFIER" "$BASE"

# Relocation must preserve independent verification.
RELOC="$TMP/relocated"
cp -a "$BASE" "$RELOC"
expect_pass relocated bash "$VERIFIER" "$RELOC"

# Byte tamper must be rejected.
TAMPER="$TMP/tamper"
cp -a "$BASE" "$TAMPER"
printf '%s\n' 'tampered' >> "$TAMPER/09_launch_hardened.txt"
expect_fail tamper_rejected bash "$VERIFIER" "$TAMPER"

# Omission attack: remaining hashes are still valid, but exact coverage is not.
OMIT="$TMP/omit"
cp -a "$BASE" "$OMIT"
grep -v '09_launch_hardened.txt$' "$OMIT/11_android_receipt/receipt.sha256" > "$OMIT/11_android_receipt/r.tmp"
mv "$OMIT/11_android_receipt/r.tmp" "$OMIT/11_android_receipt/receipt.sha256"
expect_fail omitted_path_rejected bash "$VERIFIER" "$OMIT"

# Duplicate path with valid digest must be rejected before hash validation.
DUP="$TMP/duplicate"
cp -a "$BASE" "$DUP"
head -n1 "$DUP/11_android_receipt/receipt.sha256" >> "$DUP/11_android_receipt/receipt.sha256"
expect_fail duplicate_path_rejected bash "$VERIFIER" "$DUP"

# Semantic state tamper with recomputed valid SHA-256 must still be rejected.
SEM="$TMP/semantic"
cp -a "$BASE" "$SEM"
sed -i 's/^claim_allowed=false$/claim_allowed=true/' "$SEM/11_android_receipt/android-final-state.tsv"
(
  cd "$SEM" || exit 1
  state_hash="$(sha256sum '11_android_receipt/android-final-state.tsv' | awk '{print $1}')"
  awk -v h="$state_hash" '$2=="11_android_receipt/android-final-state.tsv" {$1=h} {print}' \
    '11_android_receipt/receipt.sha256' > '11_android_receipt/r.tmp' &&
  mv '11_android_receipt/r.tmp' '11_android_receipt/receipt.sha256'
)
expect_fail semantic_rehash_rejected bash "$VERIFIER" "$SEM"

# Target digest claim tamper with recomputed manifest must still fail coherence.
TD="$TMP/target-digest"
cp -a "$BASE" "$TD"
sed -i 's/^install_target_sha256=.*/install_target_sha256=0000000000000000000000000000000000000000000000000000000000000000/' "$TD/11_android_receipt/android-final-state.tsv"
(
  cd "$TD" || exit 1
  state_hash="$(sha256sum '11_android_receipt/android-final-state.tsv' | awk '{print $1}')"
  awk -v h="$state_hash" '$2=="11_android_receipt/android-final-state.tsv" {$1=h} {print}' \
    '11_android_receipt/receipt.sha256' > '11_android_receipt/r.tmp' &&
  mv '11_android_receipt/r.tmp' '11_android_receipt/receipt.sha256'
)
expect_fail target_digest_rehash_rejected bash "$VERIFIER" "$TD"

# An optional producer-eligible evidence file added after finalization must make
# exact coverage fail instead of silently remaining outside custody.
EXTRA="$TMP/extra-eligible"
cp -a "$BASE" "$EXTRA"
printf '%s\n' 'signing_status=PASS' > "$EXTRA/06_sign_gate/signing-status.txt"
expect_fail eligible_file_omission_rejected bash "$VERIFIER" "$EXTRA"

printf 'RESULT pass=%d fail=%d claim_allowed=false runtime_semantic_pass=TOKEN_VAZIO\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
