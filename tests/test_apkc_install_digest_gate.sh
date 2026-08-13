#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/scripts/apkc_install_digest_gate.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/apkc-digest-gate.XXXXXX")" || exit 1
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0
ok() { printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad() { printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }

[ -f "$GATE" ] || { printf 'FAIL gate_missing\n'; exit 1; }
bash -n "$GATE" || { printf 'FAIL gate_syntax\n'; exit 1; }

APK="$TMP/app.apk"
STATE="$TMP/state"
printf 'apk-version-A\n' > "$APK"

if bash "$GATE" freeze "$APK" "$STATE" >/dev/null 2>&1 && \
   bash "$GATE" verify "$APK" "$STATE" >/dev/null 2>&1 && \
   grep -qx 'binding_status=PASS' "$STATE/install-apk.sha256.verify" && \
   grep -qx 'claim_allowed=false' "$STATE/install-apk.sha256.verify"; then
  ok positive_same_bytes
else
  bad positive_same_bytes
fi

# Re-freeze known bytes, then change the APK before the install-boundary verify.
rm -rf "$STATE"
printf 'apk-version-B\n' > "$APK"
if bash "$GATE" freeze "$APK" "$STATE" >/dev/null 2>&1; then
  printf 'apk-version-C-tampered\n' > "$APK"
  set +e
  bash "$GATE" verify "$APK" "$STATE" >/dev/null 2>&1
  rc=$?
  set -e
  if [ "$rc" -eq 120 ] && \
     grep -qx 'binding_status=FAIL' "$STATE/install-apk.sha256.verify" && \
     grep -qx 'claim_allowed=false' "$STATE/install-apk.sha256.verify"; then
    ok byte_swap_rejected
  else
    bad byte_swap_rejected
  fi
else
  bad byte_swap_rejected
fi

# No freeze record must never be treated as equivalent evidence.
rm -rf "$STATE"
mkdir -p "$STATE"
set +e
bash "$GATE" verify "$APK" "$STATE" >/dev/null 2>&1
rc=$?
set -e
if [ "$rc" -eq 117 ]; then
  ok missing_freeze_rejected
else
  bad missing_freeze_rejected
fi

# A syntactically malformed/re-authored freeze record is fail-closed.
cat > "$STATE/install-apk.sha256.freeze" <<'EOF'
format=APKC_INSTALL_DIGEST_BINDING_V1
claim_allowed=true
selected_apk_sha256=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
EOF
set +e
bash "$GATE" verify "$APK" "$STATE" >/dev/null 2>&1
rc=$?
set -e
if [ "$rc" -eq 118 ]; then
  ok malformed_freeze_rejected
else
  bad malformed_freeze_rejected
fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
