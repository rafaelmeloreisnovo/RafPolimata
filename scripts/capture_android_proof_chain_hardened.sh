#!/usr/bin/env bash
set -u

# Hardened compatibility entrypoint for ApkC Android proof capture.
# Phase 1 delegates build/evidence capture with signing+install disabled.
# Phase 2 selects an install artifact only through apkc_sign_install_gate.sh.
# Phase 2.5 requires canonical path evidence before release to install.
# Phase 3 installs/launches only that exact selected artifact.
# Phase 4 freezes and verifies the final Android custody receipt.
# This wrapper does not promote runtime claims; claim_allowed remains false.

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit 2

OUT_DIR="${OUT_DIR:-proofs/run-arm64-full-chain/out}"
APK_NAME="${APK_NAME:-hello-both.apk}"
PKG="${PKG:-com.rafael.teste}"
LIB="${LIB:-hello}"
DO_INSTALL="${DO_INSTALL:-1}"
REQUESTED_DO_SIGN="${DO_SIGN:-auto}"
BASE_CAPTURE="${APKC_BASE_CAPTURE:-$ROOT/scripts/capture_android_proof_chain.sh}"
SIGN_GATE="${APKC_SIGN_GATE:-$ROOT/scripts/apkc_sign_install_gate.sh}"
PATH_GATE="${APKC_PATH_GATE:-$ROOT/scripts/apkc_path_canonicality_gate.sh}"
FINAL_RECEIPT="${APKC_ANDROID_FINAL_RECEIPT:-$ROOT/scripts/apkc_android_final_receipt.sh}"

APK_RAW="$OUT_DIR/$APK_NAME"
APK_SIGNED="$OUT_DIR/${APK_NAME%.apk}-signed.apk"
SIGN_DIR="$OUT_DIR/06_sign_gate"
TARGET_FILE="$SIGN_DIR/install-target.txt"
STATUS="$OUT_DIR/status.tsv"

mkdir -p "$OUT_DIR" "$SIGN_DIR"

append_status() {
  printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" >> "$STATUS"
}

write_token_vazio() {
  local file="$1" msg="$2"
  {
    printf 'TOKEN_VAZIO: %s\n' "$msg"
    printf 'claim_allowed=false\n'
  } > "$file"
}

run_capture() {
  local out="$1"; shift
  {
    printf '# command:'
    printf ' %q' "$@"
    printf '\n# ---- stdout/stderr ----\n'
    "$@"
  } > "$out" 2>&1
}

case "$REQUESTED_DO_SIGN" in
  0|1|auto) ;;
  *)
    write_token_vazio "$TARGET_FILE" 'invalid DO_SIGN policy'
    exit 81
    ;;
esac

[ -f "$BASE_CAPTURE" ] || {
  write_token_vazio "$TARGET_FILE" 'base capture missing'
  exit 87
}
[ -f "$SIGN_GATE" ] || {
  write_token_vazio "$TARGET_FILE" 'sign gate missing'
  exit 88
}
[ -f "$PATH_GATE" ] || {
  write_token_vazio "$TARGET_FILE" 'path canonicality gate missing'
  exit 100
}
[ -f "$FINAL_RECEIPT" ] || {
  write_token_vazio "$TARGET_FILE" 'final Android receipt gate missing'
  exit 98
}
command -v bash >/dev/null 2>&1 || {
  write_token_vazio "$TARGET_FILE" 'bash unavailable'
  exit 97
}

# Build/evidence phase only. Force legacy capture not to sign or install, so its
# historical raw fallback cannot cross the hardened trust boundary. Invoke via
# bash so repository mode bits cannot silently block an otherwise valid gate.
if ! DO_SIGN=0 DO_INSTALL=0 OUT_DIR="$OUT_DIR" APK_NAME="$APK_NAME" PKG="$PKG" LIB="$LIB" \
     bash "$BASE_CAPTURE"; then
  append_status "hardened_bridge" "FAIL" "capture_android_proof_chain_hardened.sh" "base capture returned non-zero"
  write_token_vazio "$TARGET_FILE" 'base capture failed; signing/install blocked'
  exit 89
fi

if [ ! -s "$APK_RAW" ]; then
  append_status "hardened_bridge" "TOKEN_VAZIO" "06_sign_gate/install-target.txt" "raw APK missing after base capture"
  write_token_vazio "$TARGET_FILE" 'raw APK missing after base capture'
  exit 90
fi

rm -rf "$SIGN_DIR"
mkdir -p "$SIGN_DIR"
set +e
DO_SIGN="$REQUESTED_DO_SIGN" bash "$SIGN_GATE" "$APK_RAW" "$APK_SIGNED" "$SIGN_DIR"
gate_rc=$?
set -e
if [ "$gate_rc" -ne 0 ]; then
  append_status "sign_install_gate" "FAIL" "06_sign_gate/signing-status.txt" "gate rc=$gate_rc; install target withheld"
  exit "$gate_rc"
fi

[ -f "$TARGET_FILE" ] || {
  append_status "sign_install_gate" "FAIL" "06_sign_gate/install-target.txt" "gate returned 0 without target file"
  exit 91
}
IFS= read -r APK_TO_INSTALL < "$TARGET_FILE" || APK_TO_INSTALL=''

# Constrain the selected target to an artifact owned by this run. A forged,
# stale, relative, or arbitrary external path cannot be installed.
case "$REQUESTED_DO_SIGN:$APK_TO_INSTALL" in
  0:"$APK_RAW") ;;
  1:"$APK_SIGNED") ;;
  auto:"$APK_RAW"|auto:"$APK_SIGNED") ;;
  *)
    printf 'TOKEN_VAZIO\n' > "$TARGET_FILE"
    append_status "sign_install_gate" "FAIL" "06_sign_gate/install-target.txt" "unexpected install target for policy; withheld"
    exit 92
    ;;
esac

[ -s "$APK_TO_INSTALL" ] || {
  printf 'TOKEN_VAZIO\n' > "$TARGET_FILE"
  append_status "sign_install_gate" "FAIL" "06_sign_gate/install-target.txt" "selected artifact missing/empty"
  exit 93
}

# Defense-in-depth: exact target selection is not sufficient if OUT_DIR itself
# contains a textual alias such as /a/./b. Require the independent canonicality
# policy before any artifact can cross into the installation phase.
set +e
bash "$PATH_GATE" "$OUT_DIR" > "$OUT_DIR/07_path_canonicality_gate.txt" 2>&1
path_gate_rc=$?
set -e
if [ "$path_gate_rc" -ne 0 ]; then
  printf 'TOKEN_VAZIO\n' > "$TARGET_FILE"
  append_status "path_canonicality_gate" "FAIL" "07_path_canonicality_gate.txt" "gate rc=$path_gate_rc; install target withheld"
  exit 101
fi
append_status "path_canonicality_gate" "PASS" "07_path_canonicality_gate.txt" "canonical path policy satisfied; claim_allowed=false"
append_status "sign_install_gate" "PASS" "06_sign_gate" "policy satisfied; claim_allowed=false"

if [ "$DO_INSTALL" != 1 ]; then
  write_token_vazio "$OUT_DIR/08_install_hardened.txt" 'DO_INSTALL=0; validated target preserved but install not attempted'
  append_status "android_install_hardened" "TOKEN_VAZIO" "08_install_hardened.txt" "install disabled explicitly"
  exit 0
fi

if command -v adb >/dev/null 2>&1; then
  if run_capture "$OUT_DIR/08_install_hardened.txt" adb install -r "$APK_TO_INSTALL"; then
    append_status "android_install_hardened" "PASS" "08_install_hardened.txt" "adb install exit=0"
  else
    append_status "android_install_hardened" "FAIL" "08_install_hardened.txt" "adb install non-zero"
    write_token_vazio "$OUT_DIR/09_launch_hardened.txt" 'install failed; launch blocked'
    exit 94
  fi
  if run_capture "$OUT_DIR/09_launch_hardened.txt" adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1; then
    append_status "android_launch_hardened" "PASS" "09_launch_hardened.txt" "launch command exit=0; runtime health not claimed"
  else
    append_status "android_launch_hardened" "FAIL" "09_launch_hardened.txt" "launch command non-zero"
    exit 95
  fi
  run_capture "$OUT_DIR/10_logcat_hardened.txt" sh -c "adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true" || true
  append_status "android_runtime_hardened" "AUDIT" "10_logcat_hardened.txt" "captured; semantic PASS remains TOKEN_VAZIO"
elif command -v pm >/dev/null 2>&1; then
  if run_capture "$OUT_DIR/08_install_hardened.txt" pm install -r "$APK_TO_INSTALL"; then
    append_status "android_install_hardened" "PASS" "08_install_hardened.txt" "pm install exit=0"
  else
    append_status "android_install_hardened" "FAIL" "08_install_hardened.txt" "pm install non-zero"
    write_token_vazio "$OUT_DIR/09_launch_hardened.txt" 'install failed; launch blocked'
    exit 94
  fi
  if run_capture "$OUT_DIR/09_launch_hardened.txt" monkey -p "$PKG" -c android.intent.category.LAUNCHER 1; then
    append_status "android_launch_hardened" "PASS" "09_launch_hardened.txt" "launch command exit=0; runtime health not claimed"
  else
    append_status "android_launch_hardened" "FAIL" "09_launch_hardened.txt" "launch command non-zero"
    exit 95
  fi
  run_capture "$OUT_DIR/10_logcat_hardened.txt" sh -c "logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true" || true
  append_status "android_runtime_hardened" "AUDIT" "10_logcat_hardened.txt" "captured; semantic PASS remains TOKEN_VAZIO"
else
  write_token_vazio "$OUT_DIR/08_install_hardened.txt" 'adb/pm unavailable'
  write_token_vazio "$OUT_DIR/09_launch_hardened.txt" 'install unavailable; launch blocked'
  write_token_vazio "$OUT_DIR/10_logcat_hardened.txt" 'runtime capture unavailable'
  append_status "android_install_hardened" "TOKEN_VAZIO" "08_install_hardened.txt" "no adb or pm"
  exit 96
fi

# Freeze all mutable evidence before final hashing. No covered artifact or
# status.tsv may be changed after this call. The finalizer independently
# revalidates its SHA-256 manifest and remains claim_allowed=false.
printf '%s\n' 'claim_allowed=false' > "$OUT_DIR/hardened-chain-claim.txt"
if ! bash "$FINAL_RECEIPT" "$OUT_DIR"; then
  printf '%s\n' '[ApkC hardened chain FAIL: final Android receipt rejected]' >&2
  exit 99
fi

printf '%s\n' '[ApkC hardened chain complete: custody receipt PASS; runtime semantic claim remains TOKEN_VAZIO]'
