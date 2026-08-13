#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/scripts/apkc_sign_install_gate.sh"
TMP="${TMPDIR:-/tmp}/apkc_sign_gate_test_$$"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP/bin"
printf 'raw-apk\n' > "$TMP/raw.apk"

cat > "$TMP/bin/apksigner" <<'FAKE'
#!/usr/bin/env bash
mode=${FAKE_APKSIGNER_MODE:-pass}
if [ "$1" = sign ]; then
  [ "$mode" = sign_fail ] && exit 9
  out=''
  while [ $# -gt 0 ]; do
    if [ "$1" = --out ]; then shift; out=$1; fi
    shift || true
  done
  [ -n "$out" ] || exit 8
  printf 'signed-apk\n' > "$out"
  exit 0
fi
if [ "$1" = verify ]; then
  [ "$mode" = verify_fail ] && exit 10
  apk=''
  for arg in "$@"; do apk=$arg; done
  if [ "$mode" = mutate_verify ]; then
    printf 'mutated-during-verify\n' >> "$apk"
  fi
  printf '%s\n' 'Signer #1 certificate SHA-256 digest: TEST_ONLY'
  exit 0
fi
exit 11
FAKE
chmod +x "$TMP/bin/apksigner"

expect_rc() {
  expected=$1
  shift
  set +e
  "$@" >/dev/null 2>&1
  got=$?
  set -e
  if [ "$got" -ne "$expected" ]; then
    printf 'FAIL: expected rc=%s got=%s: %s\n' "$expected" "$got" "$*" >&2
    exit 1
  fi
}

is_sha256_file() {
  [ -f "$1" ] || return 1
  [ "$(wc -l < "$1" | tr -d ' ')" = 1 ] || return 1
  digest=$(cat "$1")
  case "$digest" in ''|*[!0-9a-f]*) return 1 ;; esac
  [ "${#digest}" -eq 64 ]
}

# Required signing + keystore: signed and verified target is selected and the
# exact digest that survived verification is frozen by the SignGate.
PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 \
  "$GATE" "$TMP/raw.apk" "$TMP/signed.apk" "$TMP/pass"
grep -Fxq 'status=PASS' "$TMP/pass/signing-status.txt"
grep -Fxq "$TMP/signed.apk" "$TMP/pass/install-target.txt"
grep -q 'certificate SHA-256 digest' "$TMP/pass/apksigner-verify.txt"
is_sha256_file "$TMP/pass/verified-apk.sha256"
[ "$(cat "$TMP/pass/verified-apk.sha256")" = "$(sha256sum "$TMP/signed.apk" | awk '{print $1}')" ]

# Required signing may never fall back to raw if keystore is absent.
expect_rc 85 env PATH="$TMP/bin:$PATH" DO_SIGN=1 \
  "$GATE" "$TMP/raw.apk" "$TMP/no-key.apk" "$TMP/no-key"
grep -Fxq 'TOKEN_VAZIO' "$TMP/no-key/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/no-key/verified-apk.sha256"

# Failed sign cannot expose raw or stale signed target.
expect_rc 83 env PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 FAKE_APKSIGNER_MODE=sign_fail \
  "$GATE" "$TMP/raw.apk" "$TMP/sign-fail.apk" "$TMP/sign-fail"
grep -Fxq 'TOKEN_VAZIO' "$TMP/sign-fail/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/sign-fail/verified-apk.sha256"
[ ! -e "$TMP/sign-fail.apk" ]

# Failed verify cannot expose signed or raw target.
expect_rc 84 env PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 FAKE_APKSIGNER_MODE=verify_fail \
  "$GATE" "$TMP/raw.apk" "$TMP/verify-fail.apk" "$TMP/verify-fail"
grep -Fxq 'TOKEN_VAZIO' "$TMP/verify-fail/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/verify-fail/verified-apk.sha256"
[ ! -e "$TMP/verify-fail.apk" ]

# Even an apksigner process that returns success cannot mutate the artifact
# during verification without being detected by hash-before/hash-after.
expect_rc 87 env PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 FAKE_APKSIGNER_MODE=mutate_verify \
  "$GATE" "$TMP/raw.apk" "$TMP/verify-drift.apk" "$TMP/verify-drift"
grep -Fxq 'TOKEN_VAZIO' "$TMP/verify-drift/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/verify-drift/verified-apk.sha256"
[ ! -e "$TMP/verify-drift.apk" ]
grep -Fq 'changed during apksigner verification' "$TMP/verify-drift/signing-status.txt"

# Auto without keystore accepts only a raw APK that verifies and binds digest.
PATH="$TMP/bin:$PATH" DO_SIGN=auto "$GATE" "$TMP/raw.apk" "$TMP/auto.apk" "$TMP/auto-pass"
grep -Fxq "$TMP/raw.apk" "$TMP/auto-pass/install-target.txt"
is_sha256_file "$TMP/auto-pass/verified-apk.sha256"
[ "$(cat "$TMP/auto-pass/verified-apk.sha256")" = "$(sha256sum "$TMP/raw.apk" | awk '{print $1}')" ]
expect_rc 86 env PATH="$TMP/bin:$PATH" DO_SIGN=auto FAKE_APKSIGNER_MODE=verify_fail \
  "$GATE" "$TMP/raw.apk" "$TMP/auto-fail.apk" "$TMP/auto-fail"
grep -Fxq 'TOKEN_VAZIO' "$TMP/auto-fail/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/auto-fail/verified-apk.sha256"

# Explicit bypass preserves compatibility but cannot claim signing/digest.
DO_SIGN=0 "$GATE" "$TMP/raw.apk" "$TMP/skip.apk" "$TMP/skip"
grep -Fxq "$TMP/raw.apk" "$TMP/skip/install-target.txt"
grep -Fxq 'TOKEN_VAZIO' "$TMP/skip/verified-apk.sha256"
grep -Fxq 'status=SKIPPED_EXPLICIT' "$TMP/skip/signing-status.txt"
grep -Fxq 'verified_digest=TOKEN_VAZIO' "$TMP/skip/signing-status.txt"
grep -Fxq 'claim_allowed=false' "$TMP/skip/signing-status.txt"

printf '%s\n' 'PASS: sign/install gate binds apksigner-verified digest and rejects verification-time drift; compatibility bypass remains unclaimed'
