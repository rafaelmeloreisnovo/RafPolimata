#!/usr/bin/env bash
# Phase C→D Android/device preflight.
# Capability != execution != evidence. Missing external resources become TOKEN_VAZIO.
set -u -o pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

OUT="${1:-docs/proofs/PHASE2_ANDROID_PREFLIGHT_$(date -u +%Y%m%dT%H%M%SZ).json}"
mkdir -p "$(dirname "$OUT")"

has() { command -v "$1" >/dev/null 2>&1; }
yn() { if "$@"; then printf true; else printf false; fi; }

find_sdk_tool() {
  local name="$1"
  if has "$name"; then command -v "$name"; return 0; fi
  local sdk="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
  [ -n "$sdk" ] || return 1
  [ -d "$sdk/build-tools" ] || return 1
  find "$sdk/build-tools" -type f -name "$name" -perm -u+x 2>/dev/null | sort -V | tail -n 1
}

JAVAC="$(command -v javac 2>/dev/null || true)"
DEX="$(find_sdk_tool d8 2>/dev/null || find_sdk_tool dx 2>/dev/null || true)"
ADB="$(command -v adb 2>/dev/null || true)"
APKSIGNER="$(find_sdk_tool apksigner 2>/dev/null || true)"
ZIPALIGN="$(find_sdk_tool zipalign 2>/dev/null || true)"
READELF="$(command -v llvm-readelf 2>/dev/null || command -v readelf 2>/dev/null || true)"
CLANG="$(command -v clang 2>/dev/null || true)"
PYTHON3="$(command -v python3 2>/dev/null || true)"

DEVICE_STATE="TOKEN_VAZIO_NO_ADB"
DEVICE_SERIAL=""
DEVICE_ABI=""
if [ -n "$ADB" ]; then
  DEVICE_STATE="$($ADB get-state 2>/dev/null || true)"
  [ -n "$DEVICE_STATE" ] || DEVICE_STATE="TOKEN_VAZIO_NO_DEVICE"
  if [ "$DEVICE_STATE" = "device" ]; then
    DEVICE_SERIAL="$($ADB get-serialno 2>/dev/null || true)"
    DEVICE_ABI="$($ADB shell getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r\n' || true)"
  fi
fi

LOCAL_ANDROID=false
if [ -x /system/bin/getprop ] || [ -f /system/build.prop ]; then
  LOCAL_ANDROID=true
  if [ "$DEVICE_STATE" != "device" ]; then
    DEVICE_STATE="LOCAL_ANDROID"
    DEVICE_ABI="$(getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r\n' || true)"
  fi
fi

MISSING=()
[ -n "$CLANG" ] || MISSING+=(clang)
[ -n "$PYTHON3" ] || MISSING+=(python3)
[ -n "$READELF" ] || MISSING+=(readelf)
[ -n "$JAVAC" ] || MISSING+=(javac)
[ -n "$DEX" ] || MISSING+=(d8_or_dx)

STATE="READY_HOST"
if [ ${#MISSING[@]} -gt 0 ]; then
  STATE="TOKEN_VAZIO_TOOLCHAIN"
elif [ "$DEVICE_STATE" = "device" ] || [ "$DEVICE_STATE" = "LOCAL_ANDROID" ]; then
  STATE="READY_DEVICE_EVIDENCE_RUN"
else
  STATE="TOKEN_VAZIO_DEVICE"
fi

missing_csv="$(IFS=,; echo "${MISSING[*]:-}")"
cat >"$OUT" <<JSON
{
  "schema": "rafpolimata.phase2.android-preflight.v1",
  "timestamp_utc": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "git_head": "$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO_GIT)",
  "state": "$STATE",
  "claim_allowed": false,
  "invariant": "CAPABILITY != EXECUTION != EVIDENCE",
  "toolchain": {
    "clang": $( [ -n "$CLANG" ] && echo true || echo false ),
    "python3": $( [ -n "$PYTHON3" ] && echo true || echo false ),
    "readelf": $( [ -n "$READELF" ] && echo true || echo false ),
    "javac": $( [ -n "$JAVAC" ] && echo true || echo false ),
    "dex_compiler": $( [ -n "$DEX" ] && echo true || echo false ),
    "adb": $( [ -n "$ADB" ] && echo true || echo false ),
    "apksigner": $( [ -n "$APKSIGNER" ] && echo true || echo false ),
    "zipalign": $( [ -n "$ZIPALIGN" ] && echo true || echo false ),
    "missing_required": "$missing_csv"
  },
  "device": {
    "state": "$DEVICE_STATE",
    "serial": "$DEVICE_SERIAL",
    "abi": "$DEVICE_ABI",
    "local_android": $LOCAL_ANDROID
  },
  "next_gate": "run tools/verify_all_gaps.sh; device-only claims remain TOKEN_VAZIO until install/execute/log evidence exists"
}
JSON

printf 'phase2 preflight: state=%s receipt=%s\n' "$STATE" "$OUT"
printf 'toolchain: clang=%s python3=%s readelf=%s javac=%s dex=%s adb=%s apksigner=%s\n' \
  "$([ -n "$CLANG" ] && echo yes || echo no)" \
  "$([ -n "$PYTHON3" ] && echo yes || echo no)" \
  "$([ -n "$READELF" ] && echo yes || echo no)" \
  "$([ -n "$JAVAC" ] && echo yes || echo no)" \
  "$([ -n "$DEX" ] && echo yes || echo no)" \
  "$([ -n "$ADB" ] && echo yes || echo no)" \
  "$([ -n "$APKSIGNER" ] && echo yes || echo no)"
printf 'device: state=%s serial=%s abi=%s\n' "$DEVICE_STATE" "${DEVICE_SERIAL:-TOKEN_VAZIO}" "${DEVICE_ABI:-TOKEN_VAZIO}"

# TOKEN_VAZIO is an auditable state, not a CI failure. Intrinsic falsifications are handled by verify_all_gaps.sh.
exit 0
