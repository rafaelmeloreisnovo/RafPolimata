#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
WRAP="$ROOT/scripts/apkc_provider_to_physical_receipted.sh"
SEAL="$ROOT/scripts/apkc_seal_receipt_tree.sh"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/apkc-provider-physical-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
PASS=0; FAIL=0
ok(){ printf 'PASS %s\n' "$1"; PASS=$((PASS+1)); }
bad(){ printf 'FAIL %s\n' "$1"; FAIL=$((FAIL+1)); }

ART="$TMP/provider.zip"; printf 'artifact-fixture\n' > "$ART"
TRANSFER_OK="$TMP/transfer-ok.sh"
cat > "$TRANSFER_OK" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
artifact=$1; out=$2; mode=$3
mkdir -p "$out/physical/install-chain"
printf 'provider=bound\nclaim_allowed=false\n' > "$out/provider.status"
printf 'nested=original\nclaim_allowed=false\n' > "$out/physical/install-chain/installer.status"
printf 'transfer_status=PASS\nmode=%s\nclaim_allowed=false\n' "$mode" > "$out/provider-c04-transfer.status"
EOF
TRANSFER_FAIL="$TMP/transfer-fail.sh"
cat > "$TRANSFER_FAIL" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
artifact=$1; out=$2; mode=$3
mkdir -p "$out/physical/install-chain"
printf 'negative=evidence\nclaim_allowed=false\n' > "$out/physical/install-chain/failure.status"
exit 23
EOF
chmod +x "$TRANSFER_OK" "$TRANSFER_FAIL"
PHYS="$TMP/physical-receipted.sh"; printf '#!/usr/bin/env bash\nexit 0\n' > "$PHYS"; chmod +x "$PHYS"

OUT1="$TMP/out1"
if APKC_PROVIDER_TRANSFER_GATE="$TRANSFER_OK" APKC_PHYSICAL_RECEIPTED_GATE="$PHYS" APKC_RECEIPT_TREE_SEALER="$SEAL" bash "$WRAP" "$ART" "$OUT1" probe-only >/dev/null 2>&1 \
  && grep -q '^provider_physical_chain_status=PASS$' "$OUT1/provider-physical-chain.status" \
  && grep -q './physical/install-chain/installer.status' "$OUT1/provider-physical-tree.sha256" \
  && (cd "$OUT1" && sha256sum -c provider-physical-tree.sha256 >/dev/null); then ok positive_recursive_binding; else bad positive_recursive_binding; fi

printf 'nested=tampered\nclaim_allowed=false\n' > "$OUT1/physical/install-chain/installer.status"
if ! (cd "$OUT1" && sha256sum -c provider-physical-tree.sha256 >/dev/null 2>&1); then ok nested_tree_substitution_detected; else bad nested_tree_substitution_detected; fi

OUT2="$TMP/out2"; set +e
APKC_PROVIDER_TRANSFER_GATE="$TRANSFER_FAIL" APKC_PHYSICAL_RECEIPTED_GATE="$PHYS" APKC_RECEIPT_TREE_SEALER="$SEAL" bash "$WRAP" "$ART" "$OUT2" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 23 ] && [ -f "$OUT2/provider-physical-tree.sha256" ] \
  && grep -q '^provider_physical_chain_status=FAIL$' "$OUT2/provider-physical-chain.status" \
  && (cd "$OUT2" && sha256sum -c provider-physical-tree.sha256 >/dev/null); then ok negative_evidence_sealed_and_exit_preserved; else bad negative_evidence_sealed_and_exit_preserved; fi

OUT3="$TMP/out3"; mkdir -p "$OUT3"; printf stale > "$OUT3/stale.txt"; set +e
APKC_PROVIDER_TRANSFER_GATE="$TRANSFER_OK" APKC_PHYSICAL_RECEIPTED_GATE="$PHYS" APKC_RECEIPT_TREE_SEALER="$SEAL" bash "$WRAP" "$ART" "$OUT3" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 233 ]; then ok dirty_output_rejected; else bad dirty_output_rejected; fi

OUT4="$TMP/out4"; set +e
APKC_PROVIDER_TRANSFER_GATE="$TMP/missing-transfer.sh" APKC_PHYSICAL_RECEIPTED_GATE="$PHYS" APKC_RECEIPT_TREE_SEALER="$SEAL" bash "$WRAP" "$ART" "$OUT4" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 234 ] && grep -q 'claim_allowed=false' "$OUT4/provider-physical-chain.status"; then ok missing_transfer_fail_closed; else bad missing_transfer_fail_closed; fi

OUT5="$TMP/out5"; set +e
APKC_PROVIDER_TRANSFER_GATE="$TRANSFER_OK" APKC_PHYSICAL_RECEIPTED_GATE="$PHYS" APKC_RECEIPT_TREE_SEALER="$SEAL" bash "$WRAP" "$ART" "$OUT5" invalid-mode >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 232 ]; then ok unknown_mode_rejected; else bad unknown_mode_rejected; fi

if grep -Rqs 'claim_allowed=true' "$OUT2"; then bad claim_gate_preserved; else ok claim_gate_preserved; fi
printf 'RESULT pass=%s fail=%s claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
