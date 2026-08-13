#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BRIDGE="$ROOT/scripts/capture_android_proof_chain_hardened.sh"
REAL_DIGEST_GATE="$ROOT/scripts/apkc_install_digest_gate.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/apkc-verify-freeze-bridge.XXXXXX")" || exit 1
trap 'rm -rf "$TMP"' EXIT

OUT="$TMP/out"
MOCKBIN="$TMP/bin"
mkdir -p "$OUT" "$MOCKBIN"

BASE="$TMP/base.sh"
SIGN="$TMP/sign.sh"
PATHG="$TMP/path.sh"
FINAL="$TMP/final.sh"
ADB_LOG="$TMP/adb-called.txt"

cat > "$BASE" <<'EOF'
#!/usr/bin/env bash
set -u
mkdir -p "$OUT_DIR"
printf 'apk-before-sign-verify\n' > "$OUT_DIR/$APK_NAME"
printf 'fixture\tPASS\tbase\tclaim_allowed=false\n' > "$OUT_DIR/status.tsv"
EOF

# Malicious/buggy SignGate fixture: records the digest of bytes that allegedly
# passed apksigner verification, then mutates the selected artifact before
# returning success. The bridge must compare this digest to its independent
# freeze and fail closed before adb/pm can run.
cat > "$SIGN" <<'EOF'
#!/usr/bin/env bash
set -u
raw="$1"; signed="$2"; out="$3"
mkdir -p "$out"
printf 'signed-bytes-that-were-verified\n' > "$signed"
digest="$(sha256sum "$signed" | awk '{print $1}')"
printf '%s\n' "$digest" > "$out/verified-apk.sha256"
printf '%s\n' "$signed" > "$out/install-target.txt"
printf 'status=PASS\nartifact=signed\nverification=fixture\nverified_digest_file=verified-apk.sha256\nclaim_allowed=false\n' > "$out/signing-status.txt"
printf 'mutated-after-verification\n' >> "$signed"
EOF

cat > "$PATHG" <<'EOF'
#!/usr/bin/env bash
exit 0
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
chmod +x "$BASE" "$SIGN" "$PATHG" "$FINAL" "$MOCKBIN/adb"

set +e
PATH="$MOCKBIN:$PATH" \
OUT_DIR="$OUT" APK_NAME="app.apk" PKG="com.test.apkc" LIB="hello" \
DO_SIGN=1 DO_INSTALL=1 \
APKC_BASE_CAPTURE="$BASE" \
APKC_SIGN_GATE="$SIGN" \
APKC_PATH_GATE="$PATHG" \
APKC_INSTALL_DIGEST_GATE="$REAL_DIGEST_GATE" \
APKC_ANDROID_FINAL_RECEIPT="$FINAL" \
bash "$BRIDGE" > "$TMP/bridge.out" 2>&1
rc=$?
set -e

pass=0
fail=0
ok() { printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad() { printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }

if [ "$rc" -eq 105 ]; then ok verify_to_freeze_mismatch_rejected; else bad verify_to_freeze_mismatch_rejected; fi
if [ ! -e "$ADB_LOG" ]; then ok adb_not_called; else bad adb_not_called; fi
if grep -qx 'TOKEN_VAZIO' "$OUT/06_sign_gate/install-target.txt"; then ok install_target_withheld; else bad install_target_withheld; fi
if grep -q '^verification_digest_binding[[:space:]]FAIL[[:space:]]' "$OUT/status.tsv"; then ok failure_recorded; else bad failure_recorded; fi
if grep -qx 'binding_status=FAIL' "$OUT/07_install_digest_gate/verified-to-freeze.binding"; then ok mismatch_evidence_preserved; else bad mismatch_evidence_preserved; fi
if grep -qx 'claim_allowed=false' "$OUT/07_install_digest_gate/verified-to-freeze.binding"; then ok claim_gate_preserved; else bad claim_gate_preserved; fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
