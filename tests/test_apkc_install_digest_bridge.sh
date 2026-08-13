#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BRIDGE="$ROOT/scripts/capture_android_proof_chain_hardened.sh"
REAL_DIGEST_GATE="$ROOT/scripts/apkc_install_digest_gate.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/apkc-digest-bridge.XXXXXX")" || exit 1
trap 'rm -rf "$TMP"' EXIT

OUT="$TMP/out"
MOCKBIN="$TMP/bin"
mkdir -p "$OUT" "$MOCKBIN"

BASE="$TMP/base.sh"
SIGN="$TMP/sign.sh"
PATHG="$TMP/path.sh"
DIGEST_WRAP="$TMP/digest-wrap.sh"
FINAL="$TMP/final.sh"
ADB_LOG="$TMP/adb-called.txt"

cat > "$BASE" <<'EOF'
#!/usr/bin/env bash
set -u
mkdir -p "$OUT_DIR"
printf 'apk-before-freeze\n' > "$OUT_DIR/$APK_NAME"
printf 'fixture\tPASS\tbase\tclaim_allowed=false\n' > "$OUT_DIR/status.tsv"
EOF

cat > "$SIGN" <<'EOF'
#!/usr/bin/env bash
set -u
raw="$1"; signed="$2"; out="$3"
mkdir -p "$out"
printf '%s\n' "$raw" > "$out/install-target.txt"
printf 'signing_status=SKIPPED_EXPLICIT\nclaim_allowed=false\n' > "$out/signing-status.txt"
EOF

cat > "$PATHG" <<'EOF'
#!/usr/bin/env bash
exit 0
EOF

cat > "$DIGEST_WRAP" <<EOF
#!/usr/bin/env bash
set -u
mode="\$1"; apk="\$2"; state="\$3"
if [ "\$mode" = verify ]; then
  printf 'apk-swapped-after-freeze\\n' > "\$apk"
fi
exec bash "$REAL_DIGEST_GATE" "\$mode" "\$apk" "\$state"
EOF

cat > "$FINAL" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' 'UNEXPECTED_FINAL_RECEIPT_CALL' >&2
exit 125
EOF

cat > "$MOCKBIN/adb" <<EOF
#!/usr/bin/env bash
printf '%s\\n' "\$*" >> "$ADB_LOG"
exit 0
EOF
chmod +x "$BASE" "$SIGN" "$PATHG" "$DIGEST_WRAP" "$FINAL" "$MOCKBIN/adb"

set +e
PATH="$MOCKBIN:$PATH" \
OUT_DIR="$OUT" APK_NAME="app.apk" PKG="com.test.apkc" LIB="hello" \
DO_SIGN=0 DO_INSTALL=1 \
APKC_BASE_CAPTURE="$BASE" \
APKC_SIGN_GATE="$SIGN" \
APKC_PATH_GATE="$PATHG" \
APKC_INSTALL_DIGEST_GATE="$DIGEST_WRAP" \
APKC_ANDROID_FINAL_RECEIPT="$FINAL" \
bash "$BRIDGE" > "$TMP/bridge.out" 2>&1
rc=$?
set -e

pass=0
fail=0
ok() { printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad() { printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }

if [ "$rc" -eq 104 ]; then ok digest_mismatch_bridge_rejected; else bad digest_mismatch_bridge_rejected; fi
if [ ! -e "$ADB_LOG" ]; then ok adb_not_called; else bad adb_not_called; fi
if grep -qx 'TOKEN_VAZIO' "$OUT/06_sign_gate/install-target.txt"; then ok install_target_withheld; else bad install_target_withheld; fi
if grep -q '^install_digest_binding[[:space:]]FAIL[[:space:]]' "$OUT/status.tsv"; then ok failure_recorded; else bad failure_recorded; fi
if grep -qx 'binding_status=FAIL' "$OUT/07_install_digest_gate/install-apk.sha256.verify"; then ok mismatch_evidence_preserved; else bad mismatch_evidence_preserved; fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
