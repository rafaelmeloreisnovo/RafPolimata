#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BRIDGE="$ROOT/scripts/capture_android_proof_chain_hardened.sh"
SIGN_GATE="$ROOT/scripts/apkc_sign_install_gate.sh"
FINAL_RECEIPT="$ROOT/scripts/apkc_android_final_receipt.sh"
TMP="${TMPDIR:-/tmp}/apkc_android_final_bridge_$$"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP/bin"

cat > "$TMP/base-ok.sh" <<'BASE'
#!/usr/bin/env bash
set -eu
mkdir -p "$OUT_DIR"
printf 'legacy\tAUDIT\tfixture\tbase capture fixture\n' > "$OUT_DIR/status.tsv"
printf 'raw-apk\n' > "$OUT_DIR/$APK_NAME"
exit 0
BASE

cat > "$TMP/bin/apksigner" <<'FAKE'
#!/usr/bin/env bash
if [ "$1" = sign ]; then
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
  printf '%s\n' 'Signer #1 certificate SHA-256 digest: TEST_ONLY'
  exit 0
fi
exit 11
FAKE
chmod +x "$TMP/bin/apksigner"

cat > "$TMP/bin/adb" <<'FAKE'
#!/usr/bin/env bash
case "${1:-}" in
  install)
    printf '%s\n' 'Success'
    exit 0
    ;;
  shell)
    printf '%s\n' 'Events injected: 1'
    exit 0
    ;;
  logcat)
    printf '%s\n' 'NativeActivity fixture log; no runtime semantic claim'
    exit 0
    ;;
esac
exit 12
FAKE
chmod +x "$TMP/bin/adb"

# Positive integrated custody path. All Android commands are fixtures only.
OUT="$TMP/pass"
PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 DO_INSTALL=1 \
  OUT_DIR="$OUT" APK_NAME=x.apk PKG=com.example.fixture LIB=fixture \
  APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$SIGN_GATE" \
  APKC_ANDROID_FINAL_RECEIPT="$FINAL_RECEIPT" bash "$BRIDGE"

grep -Fxq 'receipt_status=PASS' "$OUT/11_android_receipt/android-final-state.tsv"
grep -Fxq 'claim_allowed=false' "$OUT/11_android_receipt/android-final-state.tsv"
grep -Fxq 'runtime_semantic_pass=TOKEN_VAZIO' "$OUT/11_android_receipt/android-final-state.tsv"
sha256sum -c "$OUT/11_android_receipt/receipt.sha256" >/dev/null

# Post-finalization mutation must invalidate the receipt.
printf 'tamper-after-finalization\n' >> "$OUT/status.tsv"
if sha256sum -c "$OUT/11_android_receipt/receipt.sha256" >/dev/null 2>&1; then
  printf '%s\n' 'FAIL: post-finalization status mutation was not detected' >&2
  exit 1
fi

# Missing finalizer must fail before physical-chain completion can be claimed.
OUT="$TMP/missing-finalizer"
set +e
PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 DO_INSTALL=1 \
  OUT_DIR="$OUT" APK_NAME=x.apk APKC_BASE_CAPTURE="$TMP/base-ok.sh" \
  APKC_SIGN_GATE="$SIGN_GATE" APKC_ANDROID_FINAL_RECEIPT="$TMP/does-not-exist.sh" \
  bash "$BRIDGE" >/dev/null 2>&1
rc=$?
set -e
[ "$rc" -eq 98 ] || { printf 'FAIL: expected missing-finalizer rc=98 got=%s\n' "$rc" >&2; exit 1; }
grep -q '^TOKEN_VAZIO:' "$OUT/06_sign_gate/install-target.txt"

printf '%s\n' 'PASS: integrated final receipt + tamper detection + missing-finalizer fail-closed; claim_allowed=false'
