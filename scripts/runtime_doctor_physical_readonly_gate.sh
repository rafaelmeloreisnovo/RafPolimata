#!/bin/sh
set -eu

# RAFAELIA Runtime Doctor Ω — physical read-only evidence gate.
# Scope: observation/readiness only. No install, attach, injection, patch,
# privilege escalation, package mutation, APK build or VM boot is performed.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
WORKSPACE=${RUNTIME_DOCTOR_WORKSPACE:-"$(dirname "$ROOT")"}
OUT_DIR=${RUNTIME_DOCTOR_PHYSICAL_OUT:-"$ROOT/artifacts/runtime-doctor/physical/$(date -u '+%Y%m%dT%H%M%SZ')"}

usage() {
  cat <<'EOF'
usage: sh scripts/runtime_doctor_physical_readonly_gate.sh [--workspace DIR] [--out-dir DIR]

Read-only gate. It never installs Frida, attaches to a process, injects code,
patches runtime state, escalates privilege, builds APKs or boots VMs.
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --workspace)
      [ "$#" -ge 2 ] || { echo 'missing value for --workspace' >&2; exit 64; }
      WORKSPACE=$2; shift 2 ;;
    --out-dir)
      [ "$#" -ge 2 ] || { echo 'missing value for --out-dir' >&2; exit 64; }
      OUT_DIR=$2; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown argument: $1" >&2; usage >&2; exit 64 ;;
  esac
done

need() {
  command -v "$1" >/dev/null 2>&1 || {
    echo "physical-readonly: missing required command: $1" >&2
    exit 127
  }
}
need python3
need sha256sum

mkdir -p "$OUT_DIR"

COMMIT=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf 'TOKEN_VAZIO_GIT_COMMIT')
PYTHON_VERSION=$(python3 -c 'import platform; print(platform.python_version())')
UNAME_S=$(uname -s 2>/dev/null || printf TOKEN_VAZIO)
UNAME_M=$(uname -m 2>/dev/null || printf TOKEN_VAZIO)
ANDROID_RELEASE=TOKEN_VAZIO
ANDROID_SDK=TOKEN_VAZIO
ANDROID_ABI=TOKEN_VAZIO
if command -v getprop >/dev/null 2>&1; then
  ANDROID_RELEASE=$(getprop ro.build.version.release 2>/dev/null || true)
  ANDROID_SDK=$(getprop ro.build.version.sdk 2>/dev/null || true)
  ANDROID_ABI=$(getprop ro.product.cpu.abi 2>/dev/null || true)
  [ -n "$ANDROID_RELEASE" ] || ANDROID_RELEASE=TOKEN_VAZIO
  [ -n "$ANDROID_SDK" ] || ANDROID_SDK=TOKEN_VAZIO
  [ -n "$ANDROID_ABI" ] || ANDROID_ABI=TOKEN_VAZIO
fi

ANDROID_ENV_PRESENT=false
[ -n "${ANDROID_ROOT:-}" ] && ANDROID_ENV_PRESENT=true
TERMUX_PREFIX_PRESENT=false
case "${PREFIX:-}" in
  */files/usr|*/files/usr/) TERMUX_PREFIX_PRESENT=true ;;
esac

cat > "$OUT_DIR/environment.json" <<EOF
{
  "schema": "raf.runtime-doctor-physical-environment.v1",
  "repository_commit": "$COMMIT",
  "python": "$PYTHON_VERSION",
  "uname_system": "$UNAME_S",
  "uname_machine": "$UNAME_M",
  "android_environment_present": $ANDROID_ENV_PRESENT,
  "termux_prefix_shape_observed": $TERMUX_PREFIX_PRESENT,
  "android_release": "$ANDROID_RELEASE",
  "android_sdk": "$ANDROID_SDK",
  "android_abi": "$ANDROID_ABI",
  "raw_prefix_stored": false,
  "raw_device_serial_stored": false,
  "claim_allowed": false
}
EOF

set +e
python3 "$ROOT/scripts/frida_runtime_probe.py" --json \
  > "$OUT_DIR/frida-readiness.json" \
  2> "$OUT_DIR/frida-readiness.stderr.txt"
FRIDA_RC=$?
set -e
printf 'exit_code=%s\n' "$FRIDA_RC" > "$OUT_DIR/frida-readiness.exit.txt"

set +e
python3 "$ROOT/scripts/runtime_doctor_agent.py" \
  --workspace "$WORKSPACE" \
  --repo "RafPolimata=$ROOT" \
  --symptom termux \
  --symptom frida \
  --symptom runtime \
  --execute-probes \
  --json-out "$OUT_DIR/runtime-doctor.json" \
  --markdown-out "$OUT_DIR/runtime-doctor.md" \
  > "$OUT_DIR/runtime-doctor.stdout.txt" \
  2> "$OUT_DIR/runtime-doctor.stderr.txt"
DOCTOR_RC=$?
set -e
printf 'exit_code=%s\n' "$DOCTOR_RC" > "$OUT_DIR/runtime-doctor.exit.txt"

python3 - "$OUT_DIR" <<'PY'
from __future__ import annotations

import json
import sys
from pathlib import Path

out = Path(sys.argv[1])

def load(name: str):
    path = out / name
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return None
    return value if isinstance(value, dict) else None

def read_exit(name: str):
    try:
        text = (out / name).read_text(encoding="utf-8").strip()
        return int(text.split("=", 1)[1])
    except Exception:
        return None

env = load("environment.json") or {}
frida = load("frida-readiness.json") or {}
doctor = load("runtime-doctor.json") or {}
frida_rc = read_exit("frida-readiness.exit.txt")
doctor_rc = read_exit("runtime-doctor.exit.txt")

android = env.get("android_environment_present") is True
termux = env.get("termux_prefix_shape_observed") is True
physical_termux_observed = android and termux
frida_state = str(frida.get("state", "TOKEN_VAZIO_FRIDA_REPORT_MISSING"))
doctor_summary = doctor.get("summary") if isinstance(doctor.get("summary"), dict) else {}
doctor_state = str(doctor_summary.get("state", "TOKEN_VAZIO_RUNTIME_DOCTOR_REPORT_MISSING"))

if not physical_termux_observed:
    state = "TOKEN_VAZIO_NOT_PHYSICAL_TERMUX"
elif frida_rc not in (0, None) or doctor_rc not in (0, None):
    state = "REVIEW_REQUIRED_PROBE_EXIT"
elif frida_state == "TOOLS_AND_DEVICE_ENUMERATION_OBSERVED":
    state = "PHYSICAL_READINESS_OBSERVED_LIMITED"
else:
    state = "PHYSICAL_TERMUX_OBSERVED_FRIDA_GAP"

open_tokens = []
if not physical_termux_observed:
    open_tokens.append("TOKEN_VAZIO_PHYSICAL_TERMUX_RUNTIME")
if frida_state != "TOOLS_AND_DEVICE_ENUMERATION_OBSERVED":
    open_tokens.append("TOKEN_VAZIO_FRIDA_PHYSICAL_DEVICE_RECEIPT")
open_tokens.extend([
    "TOKEN_VAZIO_FRIDA_ATTACH_RECEIPT",
    "TOKEN_VAZIO_FRIDA_HOOK_RECEIPT",
    "TOKEN_VAZIO_FRIDA_BACKGROUND_PERSISTENCE_RECEIPT",
    "TOKEN_VAZIO_FRIDA_EPHEMERAL_PATCH_ROLLBACK_RECEIPT",
])

report = {
    "schema": "raf.runtime-doctor-physical-readonly-receipt.v1",
    "state": state,
    "mode": "PHYSICAL_READONLY",
    "repository_commit": env.get("repository_commit", "TOKEN_VAZIO_GIT_COMMIT"),
    "environment": {
        "android_observed": android,
        "termux_prefix_shape_observed": termux,
        "physical_termux_observed": physical_termux_observed,
        "android_release": env.get("android_release", "TOKEN_VAZIO"),
        "android_sdk": env.get("android_sdk", "TOKEN_VAZIO"),
        "android_abi": env.get("android_abi", "TOKEN_VAZIO"),
        "uname_machine": env.get("uname_machine", "TOKEN_VAZIO"),
    },
    "runtime_doctor": {
        "exit_code": doctor_rc,
        "state": doctor_state,
        "selected_skills": doctor_summary.get("selected_skills", "TOKEN_VAZIO"),
        "open_gaps": doctor_summary.get("open_gaps", "TOKEN_VAZIO"),
    },
    "frida": {
        "exit_code": frida_rc,
        "state": frida_state,
        "version": frida.get("frida_version", "TOKEN_VAZIO"),
        "route": frida.get("route", "TOKEN_VAZIO"),
        "device_enumeration_observed": (
            frida.get("capabilities", {}).get("device_enumeration_observed", False)
            if isinstance(frida.get("capabilities"), dict) else False
        ),
    },
    "forbidden_automatic_actions": {
        "install": False,
        "attach": False,
        "injection": False,
        "hook": False,
        "patch": False,
        "privilege_escalation": False,
        "apk_build": False,
        "vm_boot": False,
    },
    "open_tokens": sorted(set(open_tokens)),
    "claim_allowed": False,
    "F_ok": "Read-only environment, Runtime Doctor and Frida readiness evidence were captured without automatic mutation.",
    "F_gap": "Physical/dynamic capabilities remain open unless the corresponding direct receipt is observed in this exact run.",
    "F_next": "If physical Termux is observed, close only readiness gaps supported by this receipt; keep attach/hook/patch/runtime-build routes separate and explicit.",
}
(out / "physical-readonly-summary.json").write_text(
    json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
    encoding="utf-8",
)
PY

(
  cd "$OUT_DIR"
  find . -maxdepth 1 -type f \
    ! -name 'receipt.sha256' \
    ! -name 'receipt-verify.txt' \
    -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$OUT_DIR/receipt.sha256"
(
  cd "$OUT_DIR"
  sha256sum -c receipt.sha256 > receipt-verify.txt 2>&1
)

echo "physical-readonly: receipt=$OUT_DIR"
echo "physical-readonly: no install/attach/injection/patch/build/boot executed"
