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

rehash_run() {
  d=$1
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

# Metadata ambiguity attack: append contradictory status while rehashing all bytes.
cp -R "$TMP/ok" "$TMP/status-conflict"
printf 'receipt_status=FAIL\n' >> "$TMP/status-conflict/finalization-status.txt"
rehash_run "$TMP/status-conflict"
expect_rc 77 "$VERIFY" "$TMP/status-conflict"

# Duplicate gate metadata must be rejected even if the receipt is recomputed.
cp -R "$TMP/ok" "$TMP/run-exit-duplicate"
printf 'gate_exit_code=0\n' >> "$TMP/run-exit-duplicate/run-exit.txt"
rehash_run "$TMP/run-exit-duplicate"
expect_rc 78 "$VERIFY" "$TMP/run-exit-duplicate"

# Exit codes are canonical POSIX process status values (0..255).
cp -R "$TMP/ok" "$TMP/gate-out-of-range"
printf 'receipt_status=PASS\ngate_exit_code=999\n' > "$TMP/gate-out-of-range/finalization-status.txt"
printf 'gate_exit_code=999\n' > "$TMP/gate-out-of-range/run-exit.txt"
rehash_run "$TMP/gate-out-of-range"
expect_rc 77 "$VERIFY" "$TMP/gate-out-of-range"

# Canonical representation attack: valid hashes in a different line order must
# be rejected even though sha256sum -c would otherwise accept every line.
cp -R "$TMP/ok" "$TMP/reordered"
sed -n '2,$p' "$TMP/reordered/receipt.sha256" > "$TMP/reordered/receipt.new"
sed -n '1p' "$TMP/reordered/receipt.sha256" >> "$TMP/reordered/receipt.new"
mv "$TMP/reordered/receipt.new" "$TMP/reordered/receipt.sha256"
expect_rc 79 "$VERIFY" "$TMP/reordered"

# Producer sha256sum emits lowercase hex. Uppercase is cryptographically
# equivalent but non-canonical and therefore rejected by the custody protocol.
cp -R "$TMP/ok" "$TMP/uppercase"
tr 'abcdef' 'ABCDEF' < "$TMP/uppercase/receipt.sha256" > "$TMP/uppercase/receipt.new"
mv "$TMP/uppercase/receipt.new" "$TMP/uppercase/receipt.sha256"
expect_rc 71 "$VERIFY" "$TMP/uppercase"

printf '%s\n' 'PASS: ApkC verifier positive + omission + duplicate + tamper + extra-file + metadata + canonical-order/hex cases'
