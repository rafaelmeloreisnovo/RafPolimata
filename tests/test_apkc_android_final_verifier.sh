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

rehash_file_canonically() {
  d="$1"
  rel="$2"
  manifest="$d/11_android_receipt/receipt.sha256"
  new_hash="$(cd "$d" && sha256sum "$rel" | awk '{print $1}')" || return 1
  : > "$manifest.tmp"
  while IFS= read -r line; do
    path="${line#*  }"
    if [ "$path" = "$rel" ]; then
      printf '%s  %s\n' "$new_hash" "$path" >> "$manifest.tmp"
    else
      printf '%s\n' "$line" >> "$manifest.tmp"
    fi
  done < "$manifest"
  mv "$manifest.tmp" "$manifest"
}

rehash_state_canonically() {
  rehash_file_canonically "$1" '11_android_receipt/android-final-state.tsv'
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

# Semantic state tamper with a recomputed, still-canonical SHA-256 manifest must
# be rejected by schema semantics rather than by formatting noise.
SEM="$TMP/semantic"
cp -a "$BASE" "$SEM"
sed -i 's/^claim_allowed=false$/claim_allowed=true/' "$SEM/11_android_receipt/android-final-state.tsv"
rehash_state_canonically "$SEM" || exit 3
expect_fail semantic_rehash_rejected bash "$VERIFIER" "$SEM"

# Target digest claim tamper with recomputed canonical manifest must still fail
# coherence against the actual selected APK bytes.
TD="$TMP/target-digest"
cp -a "$BASE" "$TD"
sed -i 's/^install_target_sha256=.*/install_target_sha256=0000000000000000000000000000000000000000000000000000000000000000/' "$TD/11_android_receipt/android-final-state.tsv"
rehash_state_canonically "$TD" || exit 3
expect_fail target_digest_rehash_rejected bash "$VERIFIER" "$TD"

# A forged legacy absolute selector must not survive simply because the attacker
# recomputed its manifest hash. It must identify the same canonical target suffix.
LEGACY_BAD="$TMP/legacy-absolute-mismatch"
cp -a "$BASE" "$LEGACY_BAD"
printf '%s\n' '/historical/other/location/not-the-selected.apk' > "$LEGACY_BAD/06_sign_gate/install-target.txt"
rehash_file_canonically "$LEGACY_BAD" '06_sign_gate/install-target.txt' || exit 3
expect_fail legacy_absolute_mismatch_rehashed_rejected bash "$VERIFIER" "$LEGACY_BAD"

# A genuine legacy absolute selector that ends in the frozen relative identity is
# accepted after relocation; old OUT_DIR existence is deliberately not required.
LEGACY_OK="$TMP/legacy-absolute-valid"
cp -a "$BASE" "$LEGACY_OK"
printf '%s\n' '/old/device/run/07_signed/app.apk' > "$LEGACY_OK/06_sign_gate/install-target.txt"
rehash_file_canonically "$LEGACY_OK" '06_sign_gate/install-target.txt' || exit 3
expect_pass legacy_absolute_suffix_accepted bash "$VERIFIER" "$LEGACY_OK"

# An optional producer-eligible evidence file added after finalization must make
# exact coverage fail instead of silently remaining outside custody.
EXTRA="$TMP/extra-eligible"
cp -a "$BASE" "$EXTRA"
printf '%s\n' 'signing_status=PASS' > "$EXTRA/06_sign_gate/signing-status.txt"
expect_fail eligible_file_omission_rejected bash "$VERIFIER" "$EXTRA"

printf 'RESULT pass=%d fail=%d claim_allowed=false runtime_semantic_pass=TOKEN_VAZIO\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
