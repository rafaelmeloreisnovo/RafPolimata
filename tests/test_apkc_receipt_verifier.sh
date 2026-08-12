#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
VERIFY="$ROOT/scripts/apkc_verify_termux_receipt.sh"
TMP=${TMPDIR:-/tmp}/apkc_receipt_test_$$
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP"

make_run() {
  d=$1
  mkdir -p "$d"
  printf 'receipt_status=PASS\ngate_exit_code=0\n' > "$d/finalization-status.txt"
  printf 'gate_exit_code=0\n' > "$d/run-exit.txt"
  printf 'apk-bytes\n' > "$d/hello.apk"
  printf 'summary\n' > "$d/validation-summary.md"
  (
    cd "$d"
    find . -maxdepth 1 -type f ! -name 'receipt.sha256' ! -name 'receipt-verify.txt' -print \
      | LC_ALL=C sort \
      | while IFS= read -r f; do sha256sum "$f"; done
  ) > "$d/receipt.sha256"
}

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

# Positive control: intact producer-compatible fixture.
make_run "$TMP/ok"
"$VERIFY" "$TMP/ok" >/dev/null

# Omission attack: drop a payload line while keeping the file present.
cp -R "$TMP/ok" "$TMP/omit"
grep -v 'hello.apk$' "$TMP/omit/receipt.sha256" > "$TMP/omit/receipt.new"
mv "$TMP/omit/receipt.new" "$TMP/omit/receipt.sha256"
expect_rc 76 "$VERIFY" "$TMP/omit"

# Duplicate-manifest attack: repeat an otherwise valid path entry.
cp -R "$TMP/ok" "$TMP/dup"
grep 'hello.apk$' "$TMP/dup/receipt.sha256" > "$TMP/dup/extra.line"
cat "$TMP/dup/extra.line" >> "$TMP/dup/receipt.sha256"
rm -f "$TMP/dup/extra.line"
expect_rc 75 "$VERIFY" "$TMP/dup"

# Content tamper remains rejected by SHA-256 verification.
cp -R "$TMP/ok" "$TMP/tamper"
printf 'tampered\n' > "$TMP/tamper/hello.apk"
expect_rc 68 "$VERIFY" "$TMP/tamper"

# Extra regular file not covered by the receipt is also an omission mismatch.
cp -R "$TMP/ok" "$TMP/extra"
printf 'unexpected\n' > "$TMP/extra/unlisted.txt"
expect_rc 76 "$VERIFY" "$TMP/extra"

printf '%s\n' 'PASS: ApkC receipt verifier positive + omission + duplicate + tamper + extra-file cases'
