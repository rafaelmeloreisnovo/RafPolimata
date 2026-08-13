#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BRIDGE="$ROOT/scripts/capture_android_proof_chain_hardened.sh"
GATE="$ROOT/scripts/apkc_sign_install_gate.sh"
TMP="${TMPDIR:-/tmp}/apkc_bridge_test_$$"
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

cat > "$TMP/base-fail.sh" <<'BASE'
#!/usr/bin/env bash
exit 7
BASE

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

# Positive required-sign path: bridge must expose only the signed artifact.
OUT="$TMP/pass"
PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 DO_INSTALL=0 \
  OUT_DIR="$OUT" APK_NAME=x.apk APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$GATE" \
  bash "$BRIDGE"
grep -Fxq "$OUT/x-signed.apk" "$OUT/06_sign_gate/install-target.txt"
grep -Fxq 'status=PASS' "$OUT/06_sign_gate/signing-status.txt"
grep -q $'path_canonicality_gate\tPASS' "$OUT/status.tsv"
grep -q $'sign_install_gate\tPASS' "$OUT/status.tsv"
grep -q $'android_install_hardened\tTOKEN_VAZIO' "$OUT/status.tsv"

# Required signing failure: no raw fallback may cross the bridge.
OUT="$TMP/verify-fail"
expect_rc 84 env PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test FAKE_APKSIGNER_MODE=verify_fail \
  DO_SIGN=1 DO_INSTALL=0 OUT_DIR="$OUT" APK_NAME=x.apk \
  APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$GATE" bash "$BRIDGE"
grep -Fxq 'TOKEN_VAZIO' "$OUT/06_sign_gate/install-target.txt"

# Base capture failure blocks the trust transition before signing.
OUT="$TMP/base-fail"
expect_rc 89 env PATH="$TMP/bin:$PATH" DO_SIGN=1 DO_INSTALL=0 OUT_DIR="$OUT" APK_NAME=x.apk \
  APKC_BASE_CAPTURE="$TMP/base-fail.sh" APKC_SIGN_GATE="$GATE" bash "$BRIDGE"
grep -q '^TOKEN_VAZIO:' "$OUT/06_sign_gate/install-target.txt"

# A malicious/buggy gate returning success with an external path is rejected.
cat > "$TMP/gate-forged.sh" <<'GATE'
#!/usr/bin/env bash
set -eu
out=$3
mkdir -p "$out"
printf '/tmp/foreign.apk\n' > "$out/install-target.txt"
printf 'status=PASS\nclaim_allowed=false\n' > "$out/signing-status.txt"
exit 0
GATE
OUT="$TMP/forged"
expect_rc 92 env DO_SIGN=auto DO_INSTALL=0 OUT_DIR="$OUT" APK_NAME=x.apk \
  APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$TMP/gate-forged.sh" bash "$BRIDGE"
grep -Fxq 'TOKEN_VAZIO' "$OUT/06_sign_gate/install-target.txt"

# Exact expected-target matching alone is insufficient when OUT_DIR itself
# carries a textual alias. The independent PathGate must reject before install.
mkdir -p "$TMP/dot-alias"
OUT="$TMP/dot-alias/./run"
expect_rc 101 env PATH="$TMP/bin:$PATH" APKSIGNER_KEYSTORE=test DO_SIGN=1 DO_INSTALL=0 \
  OUT_DIR="$OUT" APK_NAME=x.apk APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$GATE" \
  bash "$BRIDGE"
CANON_OUT="$TMP/dot-alias/run"
grep -Fxq 'TOKEN_VAZIO' "$CANON_OUT/06_sign_gate/install-target.txt"
grep -q $'path_canonicality_gate\tFAIL' "$CANON_OUT/status.tsv"
[ ! -e "$CANON_OUT/08_install_hardened.txt" ]

# Explicit compatibility mode remains allowed but carries no signing claim.
OUT="$TMP/skip"
DO_SIGN=0 DO_INSTALL=0 OUT_DIR="$OUT" APK_NAME=x.apk \
  APKC_BASE_CAPTURE="$TMP/base-ok.sh" APKC_SIGN_GATE="$GATE" bash "$BRIDGE"
grep -Fxq "$OUT/x.apk" "$OUT/06_sign_gate/install-target.txt"
grep -Fxq 'status=SKIPPED_EXPLICIT' "$OUT/06_sign_gate/signing-status.txt"
grep -Fxq 'claim_allowed=false' "$OUT/06_sign_gate/signing-status.txt"
grep -q $'path_canonicality_gate\tPASS' "$OUT/status.tsv"

printf '%s\n' 'PASS: hardened bridge signed-target + path-canonicality + fail-closed + forged-target + compatibility cases'
