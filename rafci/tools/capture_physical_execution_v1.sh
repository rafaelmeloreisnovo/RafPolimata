#!/data/data/com.termux/files/usr/bin/sh
set -eu

# RafCI physical same-artifact execution acquisition V1.
# CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE
# Explicit/manual only. No install, attach, hook, patch, root transition, APK build or VM boot.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
ARTIFACT=
EXPECTED_SHA256=
SCOPE=
OUT_DIR=
EXECUTE=false

usage() {
  cat <<'EOF'
usage: sh rafci/tools/capture_physical_execution_v1.sh \
  --artifact FILE --expected-sha256 HEX64 --scope arm32|arm64 \
  --out-dir DIR --execute

The --execute flag is mandatory explicit operator authorization. The target is
run with no arguments. Raw artifact paths and device serials are not stored.
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --artifact) [ "$#" -ge 2 ] || { usage >&2; exit 64; }; ARTIFACT=$2; shift 2 ;;
    --expected-sha256) [ "$#" -ge 2 ] || { usage >&2; exit 64; }; EXPECTED_SHA256=$2; shift 2 ;;
    --scope) [ "$#" -ge 2 ] || { usage >&2; exit 64; }; SCOPE=$2; shift 2 ;;
    --out-dir) [ "$#" -ge 2 ] || { usage >&2; exit 64; }; OUT_DIR=$2; shift 2 ;;
    --execute) EXECUTE=true; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown argument: $1" >&2; usage >&2; exit 64 ;;
  esac
done

[ "$EXECUTE" = true ] || { echo 'physical-execution: --execute explicit authorization is required' >&2; exit 64; }
[ -n "$ARTIFACT" ] && [ -n "$EXPECTED_SHA256" ] && [ -n "$SCOPE" ] && [ -n "$OUT_DIR" ] || { usage >&2; exit 64; }
case "$SCOPE" in arm32|arm64) ;; *) echo 'physical-execution: scope must be arm32 or arm64' >&2; exit 64 ;; esac
case "$EXPECTED_SHA256" in
  *[!0-9A-Fa-f]*|'') echo 'physical-execution: expected SHA-256 must be hexadecimal' >&2; exit 64 ;;
esac
[ "${#EXPECTED_SHA256}" -eq 64 ] || { echo 'physical-execution: expected SHA-256 must be 64 hex chars' >&2; exit 64; }
[ -f "$ARTIFACT" ] || { echo 'physical-execution: artifact is not a regular file' >&2; exit 66; }
[ -x "$ARTIFACT" ] || { echo 'physical-execution: artifact is not executable' >&2; exit 66; }

command -v python3 >/dev/null 2>&1 || { echo 'physical-execution: python3 missing' >&2; exit 127; }
command -v sha256sum >/dev/null 2>&1 || { echo 'physical-execution: sha256sum missing' >&2; exit 127; }

mkdir -p "$OUT_DIR"
STDOUT_FILE="$OUT_DIR/target.stdout.bin"
STDERR_FILE="$OUT_DIR/target.stderr.bin"
RECEIPT="$OUT_DIR/receipt.json"
MANIFEST="$OUT_DIR/receipt.sha256"
VERIFY="$OUT_DIR/receipt-verify.txt"

COMMIT=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf 'TOKEN_VAZIO_GIT_COMMIT')
UNAME_M=$(uname -m 2>/dev/null || printf 'TOKEN_VAZIO')
ANDROID_ABI=TOKEN_VAZIO
ANDROID_RELEASE=TOKEN_VAZIO
ANDROID_SDK=TOKEN_VAZIO
if command -v getprop >/dev/null 2>&1; then
  ANDROID_ABI=$(getprop ro.product.cpu.abi 2>/dev/null || true)
  ANDROID_RELEASE=$(getprop ro.build.version.release 2>/dev/null || true)
  ANDROID_SDK=$(getprop ro.build.version.sdk 2>/dev/null || true)
  [ -n "$ANDROID_ABI" ] || ANDROID_ABI=TOKEN_VAZIO
  [ -n "$ANDROID_RELEASE" ] || ANDROID_RELEASE=TOKEN_VAZIO
  [ -n "$ANDROID_SDK" ] || ANDROID_SDK=TOKEN_VAZIO
fi

ANDROID_ENV=false
[ -n "${ANDROID_ROOT:-}" ] && ANDROID_ENV=true
TERMUX_SHAPE=false
case "${PREFIX:-}" in */files/usr|*/files/usr/) TERMUX_SHAPE=true ;; esac
PHYSICAL_TERMUX=false
[ "$ANDROID_ENV" = true ] && [ "$TERMUX_SHAPE" = true ] && PHYSICAL_TERMUX=true

LINE=$(sha256sum "$ARTIFACT")
BEFORE=${LINE%% *}
EXPECTED=$(printf '%s' "$EXPECTED_SHA256" | tr 'A-F' 'a-f')
ARTIFACT_NAME=${ARTIFACT##*/}
STARTED=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
EXECUTED=false
TARGET_RC=
STATE=TOKEN_VAZIO_NOT_EXECUTED

ABI_MATCH=false
case "$SCOPE:$ANDROID_ABI:$UNAME_M" in
  arm32:armeabi-v7a:*|arm32:armeabi:*|arm32:*:armv7*|arm32:*:armv8l) ABI_MATCH=true ;;
  arm64:arm64-v8a:*|arm64:*:aarch64) ABI_MATCH=true ;;
esac

if [ "$PHYSICAL_TERMUX" != true ]; then
  STATE=TOKEN_VAZIO_NOT_PHYSICAL_TERMUX
elif [ "$ABI_MATCH" != true ]; then
  STATE=CONTRADICTION_ABI_SCOPE
elif [ "$BEFORE" != "$EXPECTED" ]; then
  STATE=CONTRADICTION_ARTIFACT_IDENTITY
else
  EXECUTED=true
  set +e
  "$ARTIFACT" >"$STDOUT_FILE" 2>"$STDERR_FILE"
  TARGET_RC=$?
  set -e
  LINE=$(sha256sum "$ARTIFACT")
  AFTER=${LINE%% *}
  if [ "$AFTER" != "$BEFORE" ]; then
    STATE=CONTRADICTION_ARTIFACT_MUTATED
  elif [ "$TARGET_RC" -ne 0 ]; then
    STATE=CONTRADICTION_TARGET_EXIT
  else
    STATE=EVIDENCE_READY_BOUNDED
  fi
fi

[ -f "$STDOUT_FILE" ] || : > "$STDOUT_FILE"
[ -f "$STDERR_FILE" ] || : > "$STDERR_FILE"
LINE=$(sha256sum "$ARTIFACT")
AFTER=${LINE%% *}
ENDED=$(date -u '+%Y-%m-%dT%H:%M:%SZ')

python3 - "$RECEIPT" "$COMMIT" "$SCOPE" "$ANDROID_ABI" "$UNAME_M" "$ANDROID_RELEASE" "$ANDROID_SDK" "$ARTIFACT_NAME" "$EXPECTED" "$BEFORE" "$AFTER" "$TARGET_RC" "$STATE" "$EXECUTED" "$PHYSICAL_TERMUX" "$ABI_MATCH" "$STARTED" "$ENDED" "$STDOUT_FILE" "$STDERR_FILE" <<'PY'
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import sys

(
    receipt_path, commit, scope, abi, uname_m, android_release, android_sdk,
    artifact_name, expected, before, after, rc_raw, state, executed_raw,
    physical_raw, abi_match_raw, started, ended, stdout_path, stderr_path,
) = sys.argv[1:]

def digest(path: str) -> str:
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def b(value: str) -> bool:
    return value == "true"

receipt = {
    "schema": "rafaelia.rafci.physical-execution-receipt/v1",
    "repository": "rafaelmeloreisnovo/RafPolimata",
    "source_commit": commit,
    "closure": "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
    "capture_state": state,
    "target_scope": scope,
    "environment": {
        "physical_termux_observed": b(physical_raw),
        "abi_scope_match": b(abi_match_raw),
        "android_abi": abi,
        "uname_machine": uname_m,
        "android_release": android_release,
        "android_sdk": android_sdk,
        "raw_device_serial_stored": False,
        "raw_termux_prefix_stored": False,
    },
    "artifact": {
        "name": artifact_name,
        "raw_path_stored": False,
        "expected_sha256": expected,
        "sha256_before": before,
        "sha256_after": after,
    },
    "execution": {
        "authorized_by_explicit_execute_flag": True,
        "executed": b(executed_raw),
        "exit_code": int(rc_raw) if rc_raw else None,
        "started_utc": started,
        "ended_utc": ended,
        "stdout_sha256": digest(stdout_path),
        "stderr_sha256": digest(stderr_path),
        "arguments": [],
    },
    "forbidden_automatic_actions": {
        "package_install": False,
        "process_attach": False,
        "hook": False,
        "runtime_patch": False,
        "privilege_escalation": False,
        "apk_build": False,
        "vm_boot": False,
    },
    "claim_allowed": False,
    "F_ok": ["same-artifact capture envelope materialized"] if state == "EVIDENCE_READY_BOUNDED" else [],
    "F_gap": [] if state == "EVIDENCE_READY_BOUNDED" else [state],
    "F_next": ["verify receipt integrity and consume with verify_physical_execution_v1.py"],
}
Path(receipt_path).write_text(json.dumps(receipt, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
PY

(
  cd "$OUT_DIR"
  find . -maxdepth 1 -type f \
    ! -name 'receipt.sha256' \
    ! -name 'receipt-verify.txt' \
    -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$MANIFEST"
(
  cd "$OUT_DIR"
  sha256sum -c receipt.sha256 > receipt-verify.txt 2>&1
)

printf 'physical-execution: state=%s scope=%s abi=%s receipt=%s\n' "$STATE" "$SCOPE" "$ANDROID_ABI" "$OUT_DIR"

[ "$STATE" = EVIDENCE_READY_BOUNDED ] || exit 1
