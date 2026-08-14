#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
WRAP="$ROOT/scripts/apkc_provider_to_physical_codebound.sh"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/apkc-codebound-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
PASS=0; FAIL=0
ok(){ printf 'PASS %s\n' "$1"; PASS=$((PASS+1)); }
bad(){ printf 'FAIL %s\n' "$1"; FAIL=$((FAIL+1)); }
mkroot(){
  local r=$1; mkdir -p "$r/scripts"
  for f in apkc_provider_to_physical_receipted.sh apkc_verify_provider_c04_transfer.sh apkc_termux_sealed_stdin_gate_receipted.sh apkc_termux_sealed_stdin_gate.sh apkc_install_sealed_stdin.sh apkc_install_sealed_stdin.c apkc_finalize_stdin_install_receipt.sh apkc_probe_memfd_runtime.c; do printf 'fixture:%s\n' "$f" > "$r/scripts/$f"; done
  cat > "$r/scripts/apkc_seal_receipt_tree.sh" <<'S'
#!/usr/bin/env bash
set -euo pipefail
root=$1; name=$2
( cd "$root"; find . -type f ! -name "$name" -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done ) > "$root/$name"
S
  chmod +x "$r/scripts/apkc_seal_receipt_tree.sh"
}
ART="$TMP/a.zip"; printf 'artifact\n' > "$ART"
R="$TMP/root"; mkroot "$R"
CHAIN="$TMP/chain-ok.sh"; cat > "$CHAIN" <<'S'
#!/usr/bin/env bash
set -euo pipefail
mkdir -p "$2/nested"; printf 'evidence=ok\n' > "$2/nested/status.txt"; printf 'claim_allowed=false\n' > "$2/claim.status"
S
chmod +x "$CHAIN"
OUT="$TMP/out"
if APKC_CODEBOUND_ROOT="$R" APKC_CODEBOUND_CHAIN="$CHAIN" APKC_CODEBOUND_SEALER="$R/scripts/apkc_seal_receipt_tree.sh" bash "$WRAP" "$ART" "$OUT" probe-only >/dev/null 2>&1 \
  && grep -q '^codebound_status=PASS$' "$OUT/codebound.status" \
  && [ "$(wc -l < "$OUT/gate-code.sha256")" -eq 9 ] \
  && (cd "$OUT" && sha256sum -c codebound-tree.sha256 >/dev/null); then ok positive_codebound_chain; else bad positive_codebound_chain; fi

printf tamper >> "$R/scripts/apkc_termux_sealed_stdin_gate.sh"
if ! (cd "$R" && sha256sum -c "$OUT/gate-code.sha256" >/dev/null 2>&1); then ok gate_code_tamper_detected; else bad gate_code_tamper_detected; fi

printf tamper >> "$OUT/execution/nested/status.txt"
if ! (cd "$OUT" && sha256sum -c codebound-tree.sha256 >/dev/null 2>&1); then ok nested_evidence_tamper_detected; else bad nested_evidence_tamper_detected; fi

R2="$TMP/root2"; mkroot "$R2"; rm "$R2/scripts/apkc_install_sealed_stdin.c"; set +e
APKC_CODEBOUND_ROOT="$R2" APKC_CODEBOUND_CHAIN="$CHAIN" APKC_CODEBOUND_SEALER="$R2/scripts/apkc_seal_receipt_tree.sh" bash "$WRAP" "$ART" "$TMP/out2" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 227 ]; then ok missing_dependency_fail_closed; else bad missing_dependency_fail_closed; fi

R3="$TMP/root3"; mkroot "$R3"; CHAINF="$TMP/chain-fail.sh"; cat > "$CHAINF" <<'S'
#!/usr/bin/env bash
mkdir -p "$2/nested"; printf 'negative\n' > "$2/nested/failure.txt"; exit 23
S
chmod +x "$CHAINF"; set +e
APKC_CODEBOUND_ROOT="$R3" APKC_CODEBOUND_CHAIN="$CHAINF" APKC_CODEBOUND_SEALER="$R3/scripts/apkc_seal_receipt_tree.sh" bash "$WRAP" "$ART" "$TMP/out3" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 23 ] && grep -q '^codebound_status=FAIL$' "$TMP/out3/codebound.status" && (cd "$TMP/out3" && sha256sum -c codebound-tree.sha256 >/dev/null); then ok negative_exit_and_evidence_preserved; else bad negative_exit_and_evidence_preserved; fi

R4="$TMP/root4"; mkroot "$R4"; mkdir -p "$TMP/out4"; printf stale > "$TMP/out4/x"; set +e
APKC_CODEBOUND_ROOT="$R4" APKC_CODEBOUND_CHAIN="$CHAIN" APKC_CODEBOUND_SEALER="$R4/scripts/apkc_seal_receipt_tree.sh" bash "$WRAP" "$ART" "$TMP/out4" probe-only >/dev/null 2>&1
rc=$?; set -e
if [ "$rc" -eq 223 ]; then ok dirty_output_rejected; else bad dirty_output_rejected; fi

if grep -Rqs 'claim_allowed=true' "$TMP/out3"; then bad claim_gate_preserved; else ok claim_gate_preserved; fi
printf 'RESULT pass=%s fail=%s claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
