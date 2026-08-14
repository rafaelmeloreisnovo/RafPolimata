#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WRAP="$ROOT/scripts/capture_android_proof_chain_fd_hardened.sh"
TMP="${TMPDIR:-/tmp}/apkc-fd-bridge-test.$$"
PASS=0
FAIL=0
mkdir -p "$TMP/realbin"
trap 'rm -rf "$TMP"' EXIT

ok(){ PASS=$((PASS+1)); printf 'PASS %s\n' "$1"; }
bad(){ FAIL=$((FAIL+1)); printf 'FAIL %s\n' "$1"; }

[ -f "$WRAP" ] || { printf 'missing wrapper\n' >&2; exit 1; }

cat > "$TMP/realbin/adb" <<'EOF'
#!/usr/bin/env bash
set -u
if [ "${1:-}" = install ]; then
  fd="${@: -1}"
  if [ "${APKC_TEST_SWAP_PATH:-0}" = 1 ]; then
    printf 'REPLACEMENT_AFTER_FD_OPEN\n' > "$APKC_TEST_ORIGINAL_PATH.new"
    mv -f "$APKC_TEST_ORIGINAL_PATH.new" "$APKC_TEST_ORIGINAL_PATH"
  fi
  cat "$fd" > "$APKC_TEST_CAPTURE"
  touch "$APKC_TEST_INSTALLER_CALLED"
  exit 0
fi
exit 0
EOF
chmod +x "$TMP/realbin/adb"

cat > "$TMP/mock-bridge.sh" <<'EOF'
#!/usr/bin/env bash
set -u
mkdir -p "$OUT_DIR/07_install_digest_gate"
apk="$OUT_DIR/test.apk"
printf 'ORIGINAL_FD_BOUND_BYTES\n' > "$apk"
sha="$(sha256sum "$apk" | awk '{print $1}')"
{
  printf 'freeze_status=PASS\n'
  printf 'claim_allowed=false\n'
  printf 'selected_apk_sha256=%s\n' "$sha"
} > "$OUT_DIR/07_install_digest_gate/install-apk.sha256.freeze"
if [ "${APKC_TEST_TAMPER_BEFORE_INSTALL:-0}" = 1 ]; then
  printf 'TAMPER_BEFORE_SHIM_OPEN\n' > "$apk"
fi
adb install -r "$apk"
EOF
chmod +x "$TMP/mock-bridge.sh"

# Positive adversarial case: pathname is replaced by the real-installer mock
# only after the FD helper opened+hashed the original APK. Consumed bytes must
# still equal the frozen original digest.
OUT="$TMP/out-positive"
mkdir -p "$OUT"
APK_ORIG="$OUT/test.apk"
CAPTURE="$TMP/consumed.bin"
CALLED="$TMP/called-positive"
export APKC_TEST_SWAP_PATH=1
export APKC_TEST_ORIGINAL_PATH="$APK_ORIG"
export APKC_TEST_CAPTURE="$CAPTURE"
export APKC_TEST_INSTALLER_CALLED="$CALLED"
set +e
PATH="$TMP/realbin:$PATH" OUT_DIR="$OUT" APKC_HARDENED_BRIDGE="$TMP/mock-bridge.sh" bash "$WRAP"
RC=$?
set -e
FROZEN="$(sed -n '3s/^selected_apk_sha256=//p' "$OUT/07_install_digest_gate/install-apk.sha256.freeze")"
if [ "$RC" -eq 0 ] && [ -f "$CALLED" ] && [ "$(sha256sum "$CAPTURE" | awk '{print $1}')" = "$FROZEN" ] && [ "$(sha256sum "$APK_ORIG" | awk '{print $1}')" != "$FROZEN" ]; then
  ok rename_swap_consumes_fd_bound_original
else
  bad rename_swap_consumes_fd_bound_original
fi

# Negative case: mutation before the shim opens the FD must mismatch the frozen
# digest and block the real installer entirely.
OUT2="$TMP/out-mismatch"
mkdir -p "$OUT2"
CAPTURE2="$TMP/consumed2.bin"
CALLED2="$TMP/called-mismatch"
export APKC_TEST_SWAP_PATH=0
export APKC_TEST_ORIGINAL_PATH="$OUT2/test.apk"
export APKC_TEST_CAPTURE="$CAPTURE2"
export APKC_TEST_INSTALLER_CALLED="$CALLED2"
set +e
PATH="$TMP/realbin:$PATH" OUT_DIR="$OUT2" APKC_HARDENED_BRIDGE="$TMP/mock-bridge.sh" APKC_TEST_TAMPER_BEFORE_INSTALL=1 bash "$WRAP"
RC=$?
set -e
if [ "$RC" -eq 114 ] && [ ! -e "$CALLED2" ]; then
  ok pre_fd_mutation_blocks_installer
else
  bad pre_fd_mutation_blocks_installer
fi

# Contract boundary must remain explicit.
if grep -q '^same_inode_inplace_protection=TOKEN_VAZIO$' "$OUT/fd-bridge.status" && \
   grep -q '^claim_allowed=false$' "$OUT/fd-bridge.status"; then
  ok same_inode_limitation_preserved
else
  bad same_inode_limitation_preserved
fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
