#!/usr/bin/env bash
set -uo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT_ARG="${1:-artifacts/runtime-truth}"
case "$OUT_ARG" in
  /*) OUT="$OUT_ARG" ;;
  *) OUT="$ROOT/$OUT_ARG" ;;
esac
IMPLEMENTATION_BASE_COMMIT="${RAFPOLIMATA_IMPLEMENTATION_BASE_COMMIT:-b230f79b4519f398d6a0f32c2235527966f14a36}"

mkdir -p "$OUT"
OUT="$(CDPATH= cd -- "$OUT" && pwd)"

RUNTIME_STDOUT="$OUT/runtime-truth.stdout.log"
RUNTIME_STDERR="$OUT/runtime-truth.stderr.log"
DOCTOR_STDOUT="$OUT/build-doctor.stdout.log"
DOCTOR_STDERR="$OUT/build-doctor.stderr.log"
DOCTOR_JSON="$OUT/build-doctor.json"
DOCTOR_MD="$OUT/build-doctor.md"
TOOLCHAIN_JSON="$OUT/toolchain_manifest.json"
RECEIPT_JSON="$OUT/runtime_truth_receipt.json"

STARTED_AT="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
SOURCE_COMMIT="$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf '%s' 'TOKEN_VAZIO_GIT_COMMIT')"
SOURCE_BRANCH="$(git -C "$ROOT" symbolic-ref --quiet --short HEAD 2>/dev/null || printf '%s' 'DETACHED_OR_TOKEN_VAZIO')"
SOURCE_REPOSITORY="$(git -C "$ROOT" config --get remote.origin.url 2>/dev/null || printf '%s' 'rafaelmeloreisnovo/RafPolimata')"

: >"$RUNTIME_STDOUT"
: >"$RUNTIME_STDERR"
: >"$DOCTOR_STDOUT"
: >"$DOCTOR_STDERR"
rm -f "$DOCTOR_JSON" "$DOCTOR_MD" "$TOOLCHAIN_JSON" "$RECEIPT_JSON"

set +e
(
  cd "$ROOT"
  bash scripts/validate_runtime_truth_local.sh
) > >(tee "$RUNTIME_STDOUT") 2> >(tee "$RUNTIME_STDERR" >&2)
RUNTIME_EXIT=$?

(
  cd "$ROOT"
  python3 scripts/ecosystem_build_doctor.py \
    --repo rafpolimata=. \
    --json-out "$DOCTOR_JSON" \
    --markdown-out "$DOCTOR_MD" \
    --fail-on critical
) > >(tee "$DOCTOR_STDOUT") 2> >(tee "$DOCTOR_STDERR" >&2)
DOCTOR_EXIT=$?
set -e

FINISHED_AT="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"

export ROOT OUT IMPLEMENTATION_BASE_COMMIT STARTED_AT FINISHED_AT
export SOURCE_COMMIT SOURCE_BRANCH SOURCE_REPOSITORY
export RUNTIME_EXIT DOCTOR_EXIT
export RUNTIME_STDOUT RUNTIME_STDERR DOCTOR_STDOUT DOCTOR_STDERR
export DOCTOR_JSON DOCTOR_MD TOOLCHAIN_JSON RECEIPT_JSON

python3 - <<'PY'
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
from typing import Any


def first_line(command: list[str]) -> str:
    try:
        completed = subprocess.run(
            command,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
    except OSError:
        return "TOKEN_VAZIO_COMMAND_ABSENT"
    output = completed.stdout.strip().splitlines()
    if not output:
        return f"TOKEN_VAZIO_NO_VERSION_OUTPUT_EXIT_{completed.returncode}"
    return output[0]


def sha256_file(path: Path) -> str:
    if not path.is_file():
        return "TOKEN_VAZIO_ARTIFACT_MISSING"
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def blake3_file(path: Path) -> str:
    if not path.is_file():
        return "TOKEN_VAZIO_ARTIFACT_MISSING"
    b3sum = shutil.which("b3sum")
    if b3sum is None:
        return "TOKEN_VAZIO_B3SUM_ABSENT"
    completed = subprocess.run(
        [b3sum, str(path)],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if completed.returncode != 0 or not completed.stdout.strip():
        return f"TOKEN_VAZIO_B3SUM_EXIT_{completed.returncode}"
    return completed.stdout.split()[0]


def artifact(path: Path, root: Path) -> dict[str, Any]:
    try:
        display_path = str(path.relative_to(root))
    except ValueError:
        display_path = str(path)
    return {
        "path": display_path,
        "exists": path.is_file(),
        "size_bytes": path.stat().st_size if path.is_file() else None,
        "sha256": sha256_file(path),
        "blake3": blake3_file(path),
    }


root = Path(os.environ["ROOT"]).resolve()
out = Path(os.environ["OUT"]).resolve()
toolchain_path = Path(os.environ["TOOLCHAIN_JSON"])
receipt_path = Path(os.environ["RECEIPT_JSON"])

runtime_exit = int(os.environ["RUNTIME_EXIT"])
doctor_exit = int(os.environ["DOCTOR_EXIT"])

source_scripts = [
    root / "scripts/validate_runtime_truth_local.sh",
    root / "scripts/ecosystem_build_doctor.py",
    root / "scripts/run_runtime_truth_receipt.sh",
    root / "ECOSYSTEM_RUNTIME_STATE.json",
]

toolchain = {
    "schema": "raf.runtime-truth-toolchain-manifest.v1",
    "source_repository": os.environ["SOURCE_REPOSITORY"],
    "source_commit": os.environ["SOURCE_COMMIT"],
    "implementation_base_commit": os.environ["IMPLEMENTATION_BASE_COMMIT"],
    "source_branch": os.environ["SOURCE_BRANCH"],
    "generated_at": os.environ["FINISHED_AT"],
    "os": platform.platform(),
    "uname": " ".join(platform.uname()),
    "machine": platform.machine(),
    "python": first_line(["python3", "--version"]),
    "cc": first_line(["cc", "--version"]),
    "clang": first_line(["clang", "--version"]),
    "make": first_line(["make", "--version"]),
    "git": first_line(["git", "--version"]),
    "b3sum": first_line(["b3sum", "--version"]),
    "source_files": [artifact(path, root) for path in source_scripts],
    "claim_allowed": False,
}
toolchain_path.write_text(json.dumps(toolchain, indent=2, sort_keys=True) + "\n")

required_paths = [
    Path(os.environ["RUNTIME_STDOUT"]),
    Path(os.environ["RUNTIME_STDERR"]),
    Path(os.environ["DOCTOR_STDOUT"]),
    Path(os.environ["DOCTOR_STDERR"]),
    Path(os.environ["DOCTOR_JSON"]),
    Path(os.environ["DOCTOR_MD"]),
    toolchain_path,
]

missing = [str(path) for path in required_paths if not path.is_file()]
if runtime_exit != 0 or doctor_exit != 0:
    state = "FAIL"
elif missing:
    state = "INCOMPLETE"
else:
    state = "PASS"

receipt = {
    "schema": "raf.runtime-truth-receipt.v1",
    "cycle_id": "C02",
    "state": state,
    "claim_allowed": False,
    "source_repository": os.environ["SOURCE_REPOSITORY"],
    "source_commit": os.environ["SOURCE_COMMIT"],
    "implementation_base_commit": os.environ["IMPLEMENTATION_BASE_COMMIT"],
    "source_branch": os.environ["SOURCE_BRANCH"],
    "started_at": os.environ["STARTED_AT"],
    "finished_at": os.environ["FINISHED_AT"],
    "command": "bash scripts/validate_runtime_truth_local.sh",
    "runtime_truth_exit_code": runtime_exit,
    "build_doctor_exit_code": doctor_exit,
    "missing_required_artifacts": missing,
    "claim_boundary": {
        "repository_local_truth": (
            "VERIFIED_BY_EXECUTION" if state == "PASS" else "NOT_PROMOTED"
        ),
        "static_analysis": (
            "VERIFIED_BY_EXECUTION" if doctor_exit == 0 else "NOT_PROMOTED"
        ),
        "host_build_execution": (
            "VERIFIED_BY_EXECUTION" if runtime_exit == 0 else "NOT_PROMOTED"
        ),
        "android_build": "TOKEN_VAZIO",
        "apk_install": "TOKEN_VAZIO",
        "physical_runtime": "TOKEN_VAZIO",
        "performance_claim": "FORBIDDEN_OUT_OF_SCOPE",
    },
    "artifacts": [artifact(path, root) for path in required_paths],
    "falsifiers": [
        "runtime_truth_exit_code_nonzero",
        "build_doctor_exit_code_nonzero",
        "source_commit_missing_or_mismatched",
        "required_artifact_missing",
        "required_sha256_missing",
        "host_pass_promoted_to_android_runtime",
    ],
}
receipt_path.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
print(json.dumps({"state": state, "receipt": str(receipt_path)}, sort_keys=True))
PY

if [ "$RUNTIME_EXIT" -ne 0 ] || [ "$DOCTOR_EXIT" -ne 0 ]; then
  printf 'runtime-truth-receipt: FAIL runtime=%s doctor=%s receipt=%s\n' \
    "$RUNTIME_EXIT" "$DOCTOR_EXIT" "$RECEIPT_JSON" >&2
  exit 1
fi

printf 'runtime-truth-receipt: PASS receipt=%s\n' "$RECEIPT_JSON"
