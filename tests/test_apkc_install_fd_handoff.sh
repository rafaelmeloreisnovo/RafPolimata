#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/scripts/apkc_install_fd_handoff.sh"
TMP="${TMPDIR:-/tmp}/apkc-fd-handoff-test.$$"
PASS=0
FAIL=0
mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

ok() { PASS=$((PASS+1)); printf 'PASS %s\n' "$1"; }
bad() { FAIL=$((FAIL+1)); printf 'FAIL %s\n' "$1"; }

[ -f "$GATE" ] || { printf 'missing gate\n' >&2; exit 1; }

printf 'ORIGINAL_APK_BYTES\n' > "$TMP/app.apk"
ORIG_SHA=$(sha256sum "$TMP/app.apk" | awk '{print $1}')

cat > "$TMP/fake-installer.sh" <<'EOF'
#!/usr/bin/env bash
set -u
FD_PATH=$1
ORIGINAL_PATH=$2
CAPTURE=$3
# Simulate a pathname swap only after the helper has already opened+hashed FD.
printf 'REPLACEMENT_BYTES\n' > "$ORIGINAL_PATH.new"
mv -f "$ORIGINAL_PATH.new" "$ORIGINAL_PATH"
cat "$FD_PATH" > "$CAPTURE"
EOF
chmod +x "$TMP/fake-installer.sh"

OUT="$TMP/out-positive"
mkdir -p "$OUT"
if bash "$GATE" "$TMP/app.apk" "$ORIG_SHA" "$OUT" -- "$TMP/fake-installer.sh" '@APK_FD@' "$TMP/app.apk" "$TMP/consumed.bin"; then
  if [ "$(sha256sum "$TMP/consumed.bin" | awk '{print $1}')" = "$ORIG_SHA" ] && \
     [ "$(sha256sum "$TMP/app.apk" | awk '{print $1}')" != "$ORIG_SHA" ]; then
    ok pathname_swap_reads_original_fd
  else
    bad pathname_swap_reads_original_fd
  fi
else
  bad pathname_swap_reads_original_fd
fi

# Digest mismatch must block the installer entirely.
printf 'SECOND_ORIGINAL\n' > "$TMP/app2.apk"
WRONG_SHA=$(printf 'WRONG\n' | sha256sum | awk '{print $1}')
MARKER="$TMP/installer-called"
cat > "$TMP/marker-installer.sh" <<EOF
#!/usr/bin/env bash
touch "$MARKER"
exit 0
EOF
chmod +x "$TMP/marker-installer.sh"
OUT2="$TMP/out-mismatch"
mkdir -p "$OUT2"
set +e
bash "$GATE" "$TMP/app2.apk" "$WRONG_SHA" "$OUT2" -- "$TMP/marker-installer.sh" '@APK_FD@'
RC=$?
set -e
if [ "$RC" -eq 114 ] && [ ! -e "$MARKER" ]; then
  ok digest_mismatch_blocks_installer
else
  bad digest_mismatch_blocks_installer
fi

# Placeholder ambiguity must fail closed.
OUT3="$TMP/out-placeholder"
mkdir -p "$OUT3"
REAL_SHA=$(sha256sum "$TMP/app2.apk" | awk '{print $1}')
set +e
bash "$GATE" "$TMP/app2.apk" "$REAL_SHA" "$OUT3" -- /bin/true
RC=$?
set -e
if [ "$RC" -eq 115 ]; then ok missing_placeholder_rejected; else bad missing_placeholder_rejected; fi

# Preserve the explicit limitation: no claim is allowed for same-inode mutation.
if grep -q '^same_inode_inplace_protection=TOKEN_VAZIO$' "$OUT/fd-handoff.status" && \
   grep -q '^claim_allowed=false$' "$OUT/fd-handoff.status"; then
  ok limitation_and_claim_gate_preserved
else
  bad limitation_and_claim_gate_preserved
fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
