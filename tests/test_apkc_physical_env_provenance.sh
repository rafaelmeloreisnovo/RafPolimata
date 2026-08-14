#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SUT="$ROOT/scripts/apkc_provider_to_physical_envbound.sh"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
pass=0; total=0
ok(){ total=$((total+1)); if "$@"; then echo "PASS $1"; pass=$((pass+1)); else echo "FAIL $1"; fi; }

MOCK_CHAIN="$TMP/mock-chain.sh"
cat > "$MOCK_CHAIN" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
out=$2
mkdir -p "$out/nested"
printf 'child_status=PASS\nclaim_allowed=false\n' > "$out/child.status"
printf 'nested=true\nclaim_allowed=false\n' > "$out/nested/evidence.status"
exit "${MOCK_CHAIN_RC:-0}"
SH
chmod +x "$MOCK_CHAIN"

MOCK_SEAL="$TMP/mock-seal.sh"
cat > "$MOCK_SEAL" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
dir=$1; name=${2:-receipt.sha256}
(
 cd "$dir"
 find . -type f ! -name "$name" -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$dir/$name"
(cd "$dir" && sha256sum -c "$name" >/dev/null)
SH
chmod +x "$MOCK_SEAL"

ART="$TMP/artifact.zip"; printf 'artifact' > "$ART"
run(){ local out=$1 mode=${2:-probe-only}; shift 2 || true; APKC_ENVBOUND_ROOT="$ROOT" APKC_ENVBOUND_CHAIN="$MOCK_CHAIN" APKC_ENVBOUND_SEALER="$MOCK_SEAL" "$@" bash "$SUT" "$ART" "$out" "$mode"; }

positive(){
  local o="$TMP/o1"; run "$o" probe-only
  grep -Fxq 'environment_provenance_schema=raf.apkc.measurement-environment.v1' "$o/measurement-environment.txt" &&
  grep -Fxq 'claim_allowed=false' "$o/measurement-environment.txt" &&
  grep -Fxq 'envbound_status=PASS' "$o/envbound.status" &&
  (cd "$o" && sha256sum -c environment-bound-tree.sha256 >/dev/null)
}
ok positive positive_environment_binding

tool_manifest_tamper(){
  local o="$TMP/o2"; run "$o" probe-only
  printf 'tamper\n' >> "$o/measurement-tools.sha256"
  ! (cd "$o" && sha256sum -c environment-bound-tree.sha256 >/dev/null 2>&1)
}
ok tool_manifest_tamper tool_manifest_tamper_detected

nested_tamper(){
  local o="$TMP/o3"; run "$o" probe-only
  printf 'tamper\n' >> "$o/execution/nested/evidence.status"
  ! (cd "$o" && sha256sum -c environment-bound-tree.sha256 >/dev/null 2>&1)
}
ok nested_tamper nested_evidence_tamper_detected

missing_compiler(){
  local o="$TMP/o4" rc
  set +e; APKC_ENVBOUND_CHAIN="$MOCK_CHAIN" APKC_ENVBOUND_SEALER="$MOCK_SEAL" APKC_ENV_CC_BIN=definitely-not-a-compiler bash "$SUT" "$ART" "$o" probe-only >/dev/null 2>&1; rc=$?; set -e
  [ "$rc" -eq 209 ] && grep -Fq 'reason=COMPILER_BINARY_UNRESOLVED' "$o/envbound.status"
}
ok missing_compiler missing_compiler_fail_closed

adb_optional_probe(){
  local o="$TMP/o5"; APKC_ENV_ADB_BIN=definitely-no-adb run "$o" probe-only
  grep -Fxq 'adb_path=TOKEN_VAZIO' "$o/measurement-environment.txt"
}
ok adb_optional_probe adb_optional_in_probe_only

adb_required_install(){
  local o="$TMP/o6" rc
  set +e; APKC_ENVBOUND_CHAIN="$MOCK_CHAIN" APKC_ENVBOUND_SEALER="$MOCK_SEAL" APKC_ENV_ADB_BIN=definitely-no-adb bash "$SUT" "$ART" "$o" adb-shell-pm >/dev/null 2>&1; rc=$?; set -e
  [ "$rc" -eq 210 ] && grep -Fq 'reason=ADB_BINARY_UNRESOLVED' "$o/envbound.status"
}
ok adb_required_install adb_required_for_adb_mode

negative_chain_preserved(){
  local o="$TMP/o7" rc
  set +e; MOCK_CHAIN_RC=23 APKC_ENVBOUND_CHAIN="$MOCK_CHAIN" APKC_ENVBOUND_SEALER="$MOCK_SEAL" bash "$SUT" "$ART" "$o" probe-only >/dev/null 2>&1; rc=$?; set -e
  [ "$rc" -eq 23 ] && grep -Fxq 'chain_exit=23' "$o/envbound.status" && (cd "$o" && sha256sum -c environment-bound-tree.sha256 >/dev/null)
}
ok negative_chain_preserved negative_exit_and_evidence_preserved

claim_gate(){
  local o="$TMP/o8"; run "$o" probe-only
  ! grep -R -E '^claim_allowed=true$' "$o" >/dev/null 2>&1
}
ok claim_gate claim_gate_preserved

echo "RESULT pass=$pass total=$total claim_allowed=false"
[ "$pass" -eq "$total" ]
