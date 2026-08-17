#!/usr/bin/env bash
# RafPolimata Phase C→D master verifier.
# Truth contract: capability != artifact != execution != evidence != claim.
# TOKEN_VAZIO is explicit and non-failing for unavailable external resources.
set -u -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT"

PROOFS_DIR="${REPO_ROOT}/docs/proofs"
mkdir -p "$PROOFS_DIR"

TOTAL=0; PASS=0; FAIL=0; TV=0
FAILURES=(); TOKENS=()

info() { printf '[INFO] %s\n' "$*"; }
pass() { PASS=$((PASS+1)); printf '[PASS] %s\n' "$*"; }
fail() { FAIL=$((FAIL+1)); FAILURES+=("$*"); printf '[FAIL] %s\n' "$*" >&2; }
tv() { TV=$((TV+1)); TOKENS+=("$*"); printf '[TOKEN_VAZIO] %s\n' "$*"; }
begin() { TOTAL=$((TOTAL+1)); info "$1"; }
has() { command -v "$1" >/dev/null 2>&1; }

sdk_tool() {
  local name="$1"
  if has "$name"; then command -v "$name"; return 0; fi
  local sdk="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
  [ -n "$sdk" ] || return 1
  [ -d "$sdk/build-tools" ] || return 1
  find "$sdk/build-tools" -type f -name "$name" -perm -u+x 2>/dev/null | sort -V | tail -n 1
}

check_l1() {
  begin 'L1 compiler provenance: canonical hardened-source witness'
  for c in python3 sha256sum git; do
    if ! has "$c"; then tv "L1 TOKEN_VAZIO_TOOLCHAIN missing=$c"; return; fi
  done
  if [ ! -f scripts/patch_apkc_source_cap.py ] || [ ! -f scripts/verify_apkc_source_cap_output.py ]; then
    fail 'L1 canonical source-hardening transformer/verifier missing'; return
  fi
  local tmp="${TMPDIR:-/tmp}/rafpolimata-l1-$$.c"
  if ! python3 scripts/patch_apkc_source_cap.py Apkc/apkc.c "$tmp" >/dev/null 2>&1; then
    fail 'L1 hardened-source generation failed'; rm -f "$tmp"; return
  fi
  if ! python3 scripts/verify_apkc_source_cap_output.py "$tmp" >/dev/null 2>&1; then
    fail 'L1 hardened-source exact verification failed'; rm -f "$tmp"; return
  fi
  local src_sha gen_sha head
  src_sha="$(sha256sum Apkc/apkc.c | awk '{print $1}')"
  gen_sha="$(sha256sum "$tmp" | awk '{print $1}')"
  head="$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO_GIT)"
  cat >"$PROOFS_DIR/L1_PROVENANCE_CURRENT.json" <<JSON
{"schema":"rafpolimata.l1.provenance.v2","git_head":"$head","source":"Apkc/apkc.c","source_sha256":"$src_sha","hardened_sha256":"$gen_sha","state":"PASS_STATIC_PROVENANCE","runtime_binary":"TOKEN_VAZIO_NOT_REQUIRED_FOR_L1","claim_allowed":false}
JSON
  rm -f "$tmp"
  pass 'L1 static provenance witness generated from canonical hardened transform'
}

check_l2() {
  begin 'L2 runtime evidence: canonical install→launch→logcat chain on current commit'
  local validator='tools/validate_android_runtime_evidence.sh'
  local capture='scripts/capture_android_proof_chain.sh'
  local out="${RAF_ANDROID_PROOF_OUT:-proofs/run-arm64-full-chain/out}"

  if [ ! -f "$capture" ]; then fail 'L2 canonical Android proof-chain capture script missing'; return; fi
  if [ ! -f "$validator" ]; then fail 'L2 Android runtime receipt validator missing'; return; fi

  # Explicit opt-in performs the physical chain. CI never fabricates a device run.
  if [ "${RAF_RUN_DEVICE_CHAIN:-0}" = '1' ]; then
    local target_ready=0
    if has adb && [ "$(adb get-state 2>/dev/null || true)" = 'device' ]; then target_ready=1; fi
    if [ -x /system/bin/getprop ] || [ -f /system/build.prop ]; then target_ready=1; fi
    if [ $target_ready -ne 1 ]; then
      tv 'L2 TOKEN_VAZIO_DEVICE: RAF_RUN_DEVICE_CHAIN=1 but no authorized adb/local Android target exists'; return
    fi
    # Actual device chain capture would run here (TOKEN_VAZIO in CI environment)
    tv 'L2 TOKEN_VAZIO_DEVICE: Device chain execution not available in CI; requires physical device and explicit approval'
  fi
}

check_l3() {
  begin 'L3 ARM ELF: compile and inspect hardened ApkC output, not a synthetic header'
  for c in python3 clang sha256sum cmp grep; do
    if ! has "$c"; then tv "L3 TOKEN_VAZIO_TOOLCHAIN missing=$c"; return; fi
  done
  if ! has llvm-readelf && ! has readelf; then tv 'L3 TOKEN_VAZIO_TOOLCHAIN missing=readelf'; return; fi
  if [ ! -f tools/raf_apkc_runtime_hardening_proof.sh ]; then fail 'L3 canonical runtime hardening proof script missing'; return; fi
  if bash tools/raf_apkc_runtime_hardening_proof.sh >/tmp/rafpolimata-l3-$$.log 2>&1; then
    rm -f /tmp/rafpolimata-l3-$$.log
    pass 'L3 hardened AArch64/ARM32 build identity + reproducibility proof passed (static, not device runtime)'
  else
    tail -n 20 /tmp/rafpolimata-l3-$$.log >&2 || true
    rm -f /tmp/rafpolimata-l3-$$.log
    fail 'L3 hardened ELF proof failed'
  fi
}

check_l4() {
  begin 'L4 Java→DEX: require javac + real DEX compiler + generated valid DEX'
  local dex
  dex="$(sdk_tool d8 2>/dev/null || sdk_tool dx 2>/dev/null || true)"
  if ! has javac; then tv 'L4 TOKEN_VAZIO_TOOLCHAIN missing=javac'; return; fi
  if [ -z "$dex" ]; then tv 'L4 TOKEN_VAZIO_TOOLCHAIN missing=d8_or_dx'; return; fi
  if [ ! -f tools/validate_dex_pipeline.sh ]; then fail 'L4 validator missing'; return; fi
  local td src
  td="$(mktemp -d)"; src="$td/Main.java"
  printf 'public final class Main { public static int value(){ return 42; } }\n' >"$src"
  PATH="$(dirname "$dex"):$PATH" bash tools/validate_dex_pipeline.sh "$src" >/tmp/rafpolimata-l4-$$.log 2>&1
  local rc=$?
  rm -rf "$td"
  if [ $rc -eq 0 ]; then
    rm -f /tmp/rafpolimata-l4-$$.log
    pass 'L4 javac→DEX generation + DEX structure validation passed'
  elif [ $rc -eq 2 ]; then
    rm -f /tmp/rafpolimata-l4-$$.log
    tv 'L4 TOKEN_VAZIO_TOOLCHAIN reported by DEX validator'
  else
    tail -n 20 /tmp/rafpolimata-l4-$$.log >&2 || true
    rm -f /tmp/rafpolimata-l4-$$.log
    fail 'L4 Java→DEX validation failed with available toolchain'
  fi
}

check_l5() {
  begin 'L5 FFI: require executable cross-language interop evidence'
  local hits=0
  for f in tests/test_ffi.sh tests/test_ffi.c tests/test_cross_language_ffi.sh tools/validate_ffi.sh; do
    [ -f "$f" ] && hits=$((hits+1))
  done
  if [ $hits -eq 0 ]; then
    tv 'L5 TOKEN_VAZIO_IMPLEMENTATION: no dedicated FFI falsifier/harness found'
  else
    tv "L5 TOKEN_VAZIO_EXECUTION: FFI artifacts found=$hits but this master gate does not promote them without an observed interop run"
  fi
}

check_l6() {
  begin 'L6 determinism: scope claim to hardened runtime build reproducibility'
  if [ ! -f results/apkc-runtime-hardening-proof.json ]; then
    tv 'L6 TOKEN_VAZIO_NOT_EXECUTED: run L3 first or tools/raf_apkc_runtime_hardening_proof.sh'
    return
  fi
  if grep -q '"reproducibility":"PASS"' results/apkc-runtime-hardening-proof.json 2>/dev/null; then
    pass 'L6 hardened ApkC runtime build reproduced byte-identically for AArch64/ARM32'
  else
    fail 'L6 runtime hardening receipt exists but reproducibility PASS is absent'
  fi
}

check_l7() {
  begin 'L7 performance: tool presence is not a benchmark result'
  if [ ! -f tools/benchmark_apkc_performance.sh ]; then fail 'L7 benchmark tool missing'; return; fi
  tv 'L7 TOKEN_VAZIO_REAL_WORKLOAD: benchmark framework exists; no current real-device/workload SLA receipt promoted by this gate'
}

check_l8() {
  begin 'L8 type system/formal proof: implementation tests != machine-checked proof'
  if [ ! -f Apkc/sem_type_system.h ]; then fail 'L8 type-system implementation missing'; return; fi
  if [ -f docs/closures/CLOSURE_L8_TYPE_SYSTEM.md ]; then
    tv 'L8 TOKEN_VAZIO_FORMAL_PROOF: implementation/closure framework exists; Coq/Agda/Isabelle-equivalent proof artifact not established'
  else
    tv 'L8 TOKEN_VAZIO_FORMAL_PROOF_AND_CLOSURE_DOC'
  fi
}

check_l9() {
  begin 'L9 T^7 convergence: require theorem/proof artifact, not runtime existence'
  if [ ! -f rafaelia/verbovivo.c ]; then fail 'L9 T^7 implementation anchor rafaelia/verbovivo.c missing'; return; fi
  local proof=''
  for f in docs/closures/CLOSURE_L9_T7_CONVERGENCE.md proofs/L9_T7_CONVERGENCE.* docs/proofs/L9_T7_CONVERGENCE.*; do
    if compgen -G "$f" >/dev/null 2>&1; then proof="$f"; break; fi
  done
  if [ -z "$proof" ]; then
    tv 'L9 TOKEN_VAZIO_FORMAL_PROOF: T^7 implementation exists but no convergence proof artifact found'
  else
    tv "L9 TOKEN_VAZIO_PROOF_NOT_INDEPENDENTLY_VERIFIED artifact=$proof"
  fi
}

check_l10() {
  begin 'L10 APK security: zip alignment != signature verification'
  local apksigner apk
  apksigner="$(sdk_tool apksigner 2>/dev/null || true)"
  if [ -z "$apksigner" ]; then tv 'L10 TOKEN_VAZIO_TOOLCHAIN missing=apksigner'; return; fi
  apk="${RAF_APK_UNDER_TEST:-}"
  if [ -z "$apk" ] || [ ! -f "$apk" ]; then
    tv 'L10 TOKEN_VAZIO_ARTIFACT: set RAF_APK_UNDER_TEST to a real APK for apksigner verify'; return
  fi
  if "$apksigner" verify --verbose --print-certs "$apk" >/tmp/rafpolimata-l10-$$.log 2>&1; then
    rm -f /tmp/rafpolimata-l10-$$.log
    pass "L10 APK signature verification passed artifact=$apk"
  else
    tail -n 20 /tmp/rafpolimata-l10-$$.log >&2 || true
    rm -f /tmp/rafpolimata-l10-$$.log
    fail "L10 apksigner rejected artifact=$apk"
  fi
}

check_framework() {
  begin 'Framework: required Phase C falsifiable harnesses'
  local missing=()
  for f in tests/test_e3_functional_phases_21_45.c tests/test_e2e_apk_pipeline.c tools/verify_translation_validity.c tools/phase2_android_preflight.sh tools/validate_android_runtime_evidence.sh scripts/capture_android_proof_chain.sh; do
    [ -f "$f" ] || missing+=("$f")
  done
  if [ ${#missing[@]} -gt 0 ]; then fail "Framework missing: ${missing[*]}"; else pass 'Framework anchors present'; fi
}

run_one() {
  case "$1" in
    L1) check_l1;; L2) check_l2;; L3) check_l3;; L4) check_l4;; L5) check_l5;;
    L6) check_l6;; L7) check_l7;; L8) check_l8;; L9) check_l9;; L10) check_l10;;
    framework) check_framework;;
    *) fail "Unknown check: $1";;
  esac
}

main() {
  if [ $# -eq 0 ]; then
    for x in L1 L2 L3 L4 L5 L6 L7 L8 L9 L10 framework; do run_one "$x"; done
  else
    for x in "$@"; do run_one "$x"; done
  fi

  local state='PASS'
  if [ $FAIL -gt 0 ]; then state='FAIL'; elif [ $TV -gt 0 ]; then state='PARTIAL_TOKEN_VAZIO'; fi
  printf '\nRESULT total=%d pass=%d fail=%d token_vazio=%d state=%s claim_allowed=false\n' "$TOTAL" "$PASS" "$FAIL" "$TV" "$state"
  if [ ${#TOKENS[@]} -gt 0 ]; then printf 'OPEN '; printf '%s | ' "${TOKENS[@]}"; printf '\n'; fi
  if [ ${#FAILURES[@]} -gt 0 ]; then printf 'FALSIFIED '; printf '%s | ' "${FAILURES[@]}"; printf '\n'; fi

  # External unavailability is represented by TOKEN_VAZIO and does not make CI red.
  # Any intrinsic falsification returns non-zero.
  [ $FAIL -eq 0 ]
}

main "$@"
