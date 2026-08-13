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
expect_fail() { name="$1"; shift; if "$@" >/dev/null 2>&1; then bad "$name"; else ok "$name"; fi; }

fixture() {
  d="$1"
  mkdir -p "$d/06_sign_gate" "$d/07_signed"
  printf '%s\n' '07_signed/app.apk' > "$d/06_sign_gate/install-target.txt"
  printf '%s\n' 'SIGNED_APK_BYTES' > "$d/07_signed/app.apk"
  printf '%s\n' 'install_rc=0' > "$d/08_install_hardened.txt"
  printf '%s\n' 'launch_rc=0' > "$d/09_launch_hardened.txt"
  printf '%s\n' 'logcat_capture=present_not_semantic_pass' > "$d/10_logcat_hardened.txt"
  printf '%s\n' 'claim_allowed=false' 'runtime_semantic_pass=TOKEN_VAZIO' > "$d/status.tsv"
}

make_base() {
  d="$1"
  fixture "$d"
  bash "$PRODUCER" "$d" >/dev/null 2>&1 || return 1
}

rehash_rel() {
  d="$1"; rel="$2"
  manifest="$d/11_android_receipt/receipt.sha256"
  hash="$(cd "$d" && sha256sum "$rel" | awk '{print $1}')" || return 1
  awk -v h="$hash" -v p="$rel" '{ if ($2==p) print h "  " p; else print }' "$manifest" > "$manifest.tmp" || return 1
  mv "$manifest.tmp" "$manifest"
}

# Producer must fail closed before emitting a PASS receipt for // alias.
P1="$TMP/producer-double-slash"
fixture "$P1"
printf '%s\n' '07_signed//app.apk' > "$P1/06_sign_gate/install-target.txt"
expect_fail producer_double_slash_rejected bash "$PRODUCER" "$P1"

# Producer must fail closed for backslash identity as well.
P2="$TMP/producer-backslash"
fixture "$P2"
printf '%s\n' '07_signed\app.apk' > "$P2/06_sign_gate/install-target.txt"
expect_fail producer_backslash_rejected bash "$PRODUCER" "$P2"

BASE="$TMP/base"
make_base "$BASE" || { printf '%s\n' 'base producer failed' >&2; exit 2; }

# Verifier: rewrite frozen identity to //, recompute affected hashes, keep the
# manifest otherwise structurally valid; canonicality must still reject.
V1="$TMP/verifier-double-slash"
cp -a "$BASE" "$V1"
printf '%s\n' '07_signed//app.apk' > "$V1/06_sign_gate/install-target.txt"
sed -i 's#^install_target_rel=.*#install_target_rel=07_signed//app.apk#' "$V1/11_android_receipt/android-final-state.tsv"
rehash_rel "$V1" '06_sign_gate/install-target.txt' || exit 3
rehash_rel "$V1" '11_android_receipt/android-final-state.tsv' || exit 3
manifest="$V1/11_android_receipt/receipt.sha256"
awk '{ p=$2; if (p=="07_signed/app.apk") p="07_signed//app.apk"; print $1 "  " p }' "$manifest" | LC_ALL=C sort -k2,2 > "$manifest.tmp"
mv "$manifest.tmp" "$manifest"
expect_fail verifier_double_slash_rehashed_rejected bash "$VERIFIER" "$V1"

# Verifier: backslash alias with selector/state hashes recomputed must still fail.
V2="$TMP/verifier-backslash"
cp -a "$BASE" "$V2"
printf '%s\n' '07_signed\app.apk' > "$V2/06_sign_gate/install-target.txt"
sed -i 's#^install_target_rel=.*#install_target_rel=07_signed\\app.apk#' "$V2/11_android_receipt/android-final-state.tsv"
rehash_rel "$V2" '06_sign_gate/install-target.txt' || exit 3
rehash_rel "$V2" '11_android_receipt/android-final-state.tsv' || exit 3
expect_fail verifier_backslash_rehashed_rejected bash "$VERIFIER" "$V2"

printf 'RESULT pass=%d fail=%d claim_allowed=false runtime_semantic_pass=TOKEN_VAZIO\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
