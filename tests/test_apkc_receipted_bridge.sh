#!/usr/bin/env bash
set -u
BRIDGE=${1:-scripts/capture_android_proof_chain_sealed_receipted.sh}
FINALIZER=${2:-scripts/apkc_finalize_install_receipt.sh}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
SHA=$(printf 'a%.0s' {1..64})
pass=0 fail=0

make_capture() {
  local path=$1 exit_code=$2 installer_exit=$3 mode=${4:-good}
  cat > "$path" <<EOT
#!/usr/bin/env bash
set -u
mkdir -p "\$OUT_DIR"
cat > "\$OUT_DIR/sealed-memfd.status" <<EOS
sealed_snapshot_status=READY
claim_allowed=false
expected_sha256=$SHA
sealed_snapshot_sha256=$SHA
seals=0xf
EOS
EOT
  if [ "$mode" != missing_installer ]; then
    printf 'printf '\''installer_exit=%s\\nclaim_allowed=false\\n'\'' > "$OUT_DIR/installer-exit.status"\n' "$installer_exit" >> "$path"
  fi
  printf 'exit %s\n' "$exit_code" >> "$path"
  chmod +x "$path"
}

case_run() {
  local name=$1 expected=$2 capture=$3 out=$4
  mkdir -p "$out"
  set +e
  OUT_DIR="$out" APKC_SEALED_CAPTURE="$capture" APKC_INSTALL_RECEIPT_FINALIZER="$FINALIZER" bash "$BRIDGE" >/dev/null 2>&1
  rc=$?
  set -e
  ok=0
  if [ "$expected" = zero ]; then [ "$rc" -eq 0 ] && ok=1; else [ "$rc" -ne 0 ] && ok=1; fi
  if [ "$ok" -eq 1 ] && grep -q '^claim_allowed=false$' "$out/receipted-bridge.status"; then
    printf 'PASS %s rc=%s\n' "$name" "$rc"; pass=$((pass+1))
  else
    printf 'FAIL %s rc=%s\n' "$name" "$rc"; fail=$((fail+1))
  fi
}

set -e
c="$TMP/cap_ok.sh"; make_capture "$c" 0 0; case_run positive zero "$c" "$TMP/o1"; grep -q '^receipted_bridge_status=PASS$' "$TMP/o1/receipted-bridge.status"
c="$TMP/cap_instfail.sh"; make_capture "$c" 23 23; case_run installer_failure nonzero "$c" "$TMP/o2"; grep -q '^overall_status=FAIL$' "$TMP/o2/apkc-install-chain.status"
c="$TMP/cap_missing.sh"; make_capture "$c" 91 0 missing_installer; case_run missing_evidence nonzero "$c" "$TMP/o3"; grep -q '^reason=INSTALLER_EXIT_STATUS_MISSING$' "$TMP/o3/apkc-install-chain.status"

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$pass" -eq 3 ] && [ "$fail" -eq 0 ]
