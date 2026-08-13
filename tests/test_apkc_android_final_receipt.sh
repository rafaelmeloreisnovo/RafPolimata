#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/scripts/apkc_android_final_receipt.sh"
TMP="$(mktemp -d)" || exit 1
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

pass=0
fail=0
ok() { printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad() { printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }

fixture() {
  d="$1"
  mkdir -p "$d/06_sign_gate"
  printf 'apk-bytes\n' > "$d/hello-signed.apk"
  printf '%s\n' "$d/hello-signed.apk" > "$d/06_sign_gate/install-target.txt"
  printf 'signing_status=PASS\nclaim_allowed=false\n' > "$d/06_sign_gate/signing-status.txt"
  printf 'certificate=test-fixture-only\n' > "$d/06_sign_gate/apksigner-verify.txt"
  printf 'adb install fixture exit=0\n' > "$d/08_install_hardened.txt"
  printf 'launch fixture exit=0\n' > "$d/09_launch_hardened.txt"
  printf 'logcat fixture capture\n' > "$d/10_logcat_hardened.txt"
  printf 'android_install_hardened\tPASS\t08_install_hardened.txt\tfixture\n' > "$d/status.tsv"
}

# Positive custody fixture. This is a local structural test, not physical Android evidence.
d="$TMP/positive/out"
fixture "$d"
if bash "$GATE" "$d" && sha256sum -c "$d/11_android_receipt/receipt.sha256" >/dev/null 2>&1; then
  grep -qx 'claim_allowed=false' "$d/11_android_receipt/android-final-state.tsv" && \
  grep -qx 'runtime_semantic_pass=TOKEN_VAZIO' "$d/11_android_receipt/android-final-state.tsv" && ok positive || bad positive_metadata
else
  bad positive
fi

# Mutation after receipt must invalidate custody.
printf 'tamper\n' >> "$d/08_install_hardened.txt"
if sha256sum -c "$d/11_android_receipt/receipt.sha256" >/dev/null 2>&1; then
  bad tamper_rejected
else
  ok tamper_rejected
fi

# External install target must fail closed.
d="$TMP/external/out"
fixture "$d"
printf 'outside\n' > "$TMP/outside.apk"
printf '%s\n' "$TMP/outside.apk" > "$d/06_sign_gate/install-target.txt"
set +e
bash "$GATE" "$d" >/dev/null 2>&1
rc=$?
set -e
if [ "$rc" -eq 73 ] && grep -qx 'claim_allowed=false' "$d/11_android_receipt/android-final-state.tsv"; then ok external_target_rejected; else bad external_target_rejected; fi

# Missing launch evidence cannot produce a successful final receipt.
d="$TMP/missing/out"
fixture "$d"
rm -f "$d/09_launch_hardened.txt"
set +e
bash "$GATE" "$d" >/dev/null 2>&1
rc=$?
set -e
if [ "$rc" -eq 76 ] && grep -qx 'receipt_status=FAIL' "$d/11_android_receipt/android-final-state.tsv"; then ok missing_launch_rejected; else bad missing_launch_rejected; fi

printf 'RESULT pass=%s fail=%s claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
