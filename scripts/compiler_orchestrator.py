#!/usr/bin/env python3
"""Offline, fail-closed compiler orchestrator for RAFAELIA Android targets.

This tool never installs, launches, downloads, clones, or resolves packages.
It verifies the sealed RafPolimata compiler core before target compilation and
emits an immutable-style JSON receipt for every attempted operation.
"""

from __future__ import annotations

import argparse
import datetime as dt
import glob
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
import zipfile
from typing import Any, Iterable

SCHEMA = "raf.compiler-orchestration-receipt.v1"
FORBIDDEN_ACTION_TOKENS = {
    "install", "installdebug", "installrelease", "adb", "am", "monkey",
    "launch", "run-app", "startactivity", "connectedandroidtest",
}
ALLOWED_ABIS = {"both", "armeabi-v7a", "arm64-v8a"}


class GateError(RuntimeError):
    pass


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise GateError(f"manifest not found: {path}") from exc
    except json.JSONDecodeError as exc:
        raise GateError(f"invalid manifest JSON: {exc}") from exc
    if not isinstance(value, dict):
        raise GateError("manifest root must be an object")
    return value


def validate_manifest(manifest: dict[str, Any]) -> None:
    if manifest.get("schema") != "raf.compiler-target-registry.v1":
        raise GateError("unsupported target registry schema")
    if manifest.get("claim_allowed") is not False:
        raise GateError("registry must keep claim_allowed=false")
    policy = manifest.get("policy")
    if not isinstance(policy, dict):
        raise GateError("policy must be an object")
    if policy.get("install") != "FORBIDDEN" or policy.get("launch") != "FORBIDDEN":
        raise GateError("install and launch must remain FORBIDDEN")
    targets = manifest.get("targets")
    if not isinstance(targets, dict) or not targets:
        raise GateError("targets registry is empty")
    for name, target in targets.items():
        if not isinstance(target, dict):
            raise GateError(f"target {name}: definition must be an object")
        for field in ("repo", "level", "required_files", "required_commands",
                      "required_env_any", "build_command", "workdir",
                      "artifact_globs", "scope"):
            if field not in target:
                raise GateError(f"target {name}: missing {field}")
        command = target["build_command"]
        if not isinstance(command, list) or not command or not all(isinstance(x, str) and x for x in command):
            raise GateError(f"target {name}: build_command must be a non-empty string array")
        alternatives = target.get("required_command_any", [])
        if not isinstance(alternatives, list):
            raise GateError(f"target {name}: required_command_any must be a list")
        for group in alternatives:
            if not isinstance(group, list) or not group or not all(isinstance(x, str) and x for x in group):
                raise GateError(f"target {name}: invalid required_command_any group")
        reject_forbidden_actions(command, f"target {name}")


def reject_forbidden_actions(command: Iterable[str], label: str) -> None:
    for token in command:
        normalized = Path(token).name.lower().replace(":", "").replace("-", "")
        raw = token.lower()
        for forbidden in FORBIDDEN_ACTION_TOKENS:
            key = forbidden.replace("-", "")
            if normalized == key or normalized.endswith(key) or raw == forbidden:
                raise GateError(f"{label}: forbidden execution action: {token}")


def resolve_repo(self_root: Path, repo_spec: Any) -> Path:
    if repo_spec == "self":
        return self_root
    if not isinstance(repo_spec, dict):
        raise GateError("repo must be 'self' or an object")
    env_name = repo_spec.get("env")
    default_relative = repo_spec.get("default_relative")
    if not isinstance(env_name, str) or not isinstance(default_relative, str):
        raise GateError("external repo requires env and default_relative")
    raw = os.environ.get(env_name)
    path = Path(raw).expanduser() if raw else (self_root / default_relative)
    return path.resolve()


def expand_text(value: str, context: dict[str, str]) -> str:
    try:
        return value.format(**context)
    except KeyError as exc:
        raise GateError(f"unknown placeholder in {value!r}: {exc}") from exc


def expand_command(command: list[str], context: dict[str, str]) -> list[str]:
    result = [expand_text(token, context) for token in command]
    reject_forbidden_actions(result, "expanded command")
    return result


def git_snapshot(repo: Path) -> dict[str, Any]:
    if not (repo / ".git").exists():
        raise GateError(f"not a Git checkout: {repo}")
    commit = subprocess.run(
        ["git", "-C", str(repo), "rev-parse", "HEAD"],
        check=True, text=True, capture_output=True,
    ).stdout.strip()
    status = subprocess.run(
        ["git", "-C", str(repo), "status", "--porcelain=v1", "--untracked-files=normal"],
        check=True, text=True, capture_output=True,
    ).stdout.splitlines()
    return {"root": str(repo), "commit": commit, "dirty": bool(status), "status": status}


def command_version(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise GateError(f"required command not found: {name}")
    attempts = ([path, "--version"], [path, "-version"])
    for cmd in attempts:
        try:
            result = subprocess.run(cmd, text=True, capture_output=True, timeout=10)
        except (OSError, subprocess.TimeoutExpired):
            continue
        text = (result.stdout or result.stderr).strip().splitlines()
        if text:
            return f"{path}: {text[0][:300]}"
    return path


def command_alternatives(groups: Any) -> list[dict[str, Any]]:
    if not isinstance(groups, list):
        raise GateError("required_command_any must be a list")
    results: list[dict[str, Any]] = []
    for group in groups:
        if not isinstance(group, list) or not group or not all(isinstance(x, str) for x in group):
            raise GateError("required_command_any entries must be non-empty string arrays")
        resolved = next((name for name in group if shutil.which(name)), None)
        if resolved is None:
            raise GateError(f"one command is required: {', '.join(group)}")
        results.append({
            "any_of": group,
            "resolved": resolved,
            "version": command_version(resolved),
        })
    return results


def check_required_env(groups: Any) -> list[dict[str, Any]]:
    if not isinstance(groups, list):
        raise GateError("required_env_any must be a list")
    results: list[dict[str, Any]] = []
    for group in groups:
        if not isinstance(group, list) or not group or not all(isinstance(x, str) for x in group):
            raise GateError("required_env_any entries must be non-empty string arrays")
        found = next((name for name in group if os.environ.get(name)), None)
        results.append({"any_of": group, "resolved": found, "present": found is not None})
        if found is None:
            raise GateError(f"one environment variable is required: {', '.join(group)}")
        candidate = Path(os.environ[found]).expanduser()
        if not candidate.exists():
            raise GateError(f"{found} points to a missing path: {candidate}")
    return results


def check_required_files(repo: Path, paths: Any) -> list[dict[str, Any]]:
    if not isinstance(paths, list):
        raise GateError("required_files must be a list")
    results = []
    for relative in paths:
        if not isinstance(relative, str) or not relative:
            raise GateError("required_files entries must be strings")
        candidate = (repo / relative).resolve()
        present = candidate.is_file()
        results.append({"path": str(candidate), "present": present})
        if not present:
            raise GateError(f"required file missing: {candidate}")
    return results


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def blake3_file(path: Path) -> dict[str, str]:
    tool = shutil.which("b3sum")
    if tool is None:
        return {"state": "TOKEN_VAZIO_B3SUM_NOT_AVAILABLE"}
    result = subprocess.run([tool, str(path)], text=True, capture_output=True)
    if result.returncode != 0:
        return {"state": "TOKEN_VAZIO_B3SUM_FAILED", "stderr": result.stderr[-500:]}
    value = result.stdout.split()[0].strip().lower()
    if len(value) != 64:
        return {"state": "TOKEN_VAZIO_B3SUM_INVALID_OUTPUT"}
    return {"state": "PASS", "digest": value, "tool": tool}


def validate_android_archive(path: Path) -> dict[str, Any]:
    suffix = path.suffix.lower()
    if suffix not in {".apk", ".aab"}:
        return {"state": "NOT_APPLICABLE"}
    if not zipfile.is_zipfile(path):
        return {"state": "FAIL", "reason": "not a valid ZIP container"}
    with zipfile.ZipFile(path) as archive:
        bad = archive.testzip()
        names = archive.namelist()
    if bad is not None:
        return {"state": "FAIL", "reason": f"CRC failure at {bad}"}
    if suffix == ".apk":
        missing = [name for name in ("AndroidManifest.xml", "classes.dex") if name not in names]
        native = sorted(name for name in names if name.startswith("lib/") and name.endswith(".so"))
        if missing:
            return {"state": "FAIL", "reason": f"missing APK entries: {', '.join(missing)}"}
        if not native:
            return {"state": "FAIL", "reason": "APK contains no native library"}
        abis = sorted({name.split("/")[1] for name in native if len(name.split("/")) >= 3})
        return {
            "state": "PASS",
            "container": "APK",
            "entries": len(names),
            "native_libraries": len(native),
            "abis": abis,
        }
    manifest_entries = [name for name in names if name.endswith("/manifest/AndroidManifest.xml")]
    dex_entries = [name for name in names if name.endswith(".dex")]
    if not manifest_entries or not dex_entries:
        return {"state": "FAIL", "reason": "AAB missing manifest or DEX entries"}
    return {
        "state": "PASS",
        "container": "AAB",
        "entries": len(names),
        "manifest_entries": len(manifest_entries),
        "dex_entries": len(dex_entries),
    }


def artifact_records(patterns: Any, context: dict[str, str]) -> list[dict[str, Any]]:
    if not isinstance(patterns, list):
        raise GateError("artifact_globs must be a list")
    files: set[Path] = set()
    for pattern in patterns:
        expanded = expand_text(pattern, context)
        for raw in glob.glob(expanded, recursive=True):
            path = Path(raw)
            if path.is_file():
                files.add(path.resolve())
    records = []
    for path in sorted(files):
        records.append({
            "path": str(path),
            "size": path.stat().st_size,
            "sha256": sha256_file(path),
            "blake3": blake3_file(path),
            "android_structure": validate_android_archive(path),
        })
    return records


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=str(path.parent))
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            json.dump(value, handle, indent=2, sort_keys=True)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(tmp_name, path)
    finally:
        if os.path.exists(tmp_name):
            os.unlink(tmp_name)


def run_logged(command: list[str], cwd: Path, log_path: Path) -> dict[str, Any]:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    started = utc_now()
    with log_path.open("w", encoding="utf-8") as log:
        log.write("$ " + shlex.join(command) + "\n")
        log.flush()
        process = subprocess.run(
            command, cwd=str(cwd), text=True,
            stdout=log, stderr=subprocess.STDOUT,
        )
    return {
        "command": command,
        "cwd": str(cwd),
        "started_at": started,
        "finished_at": utc_now(),
        "exit_code": process.returncode,
        "log": str(log_path),
        "log_sha256": sha256_file(log_path),
    }


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", default="compiler/targets.v1.json")
    parser.add_argument("--target", required=True)
    parser.add_argument("--mode", choices=("plan", "preflight", "build"), default="preflight")
    parser.add_argument("--abi", choices=sorted(ALLOWED_ABIS), default="both")
    parser.add_argument("--out")
    parser.add_argument("--receipt")
    parser.add_argument("--allow-dirty", action="store_true")
    return parser


def orchestrate(args: argparse.Namespace) -> tuple[int, dict[str, Any]]:
    self_root = Path(__file__).resolve().parents[1]
    manifest_path = Path(args.manifest)
    if not manifest_path.is_absolute():
        manifest_path = self_root / manifest_path
    manifest = load_json(manifest_path)
    validate_manifest(manifest)
    target = manifest["targets"].get(args.target)
    if target is None:
        raise GateError(f"unknown target: {args.target}")

    repo = resolve_repo(self_root, target["repo"])
    out = Path(args.out).expanduser().resolve() if args.out else (
        self_root / "build" / "compiler-gate" / args.target
    ).resolve()
    context = {"self": str(self_root), "repo": str(repo), "out": str(out), "abi": args.abi}
    command = expand_command(target["build_command"], context)
    workdir = Path(expand_text(target["workdir"], context)).resolve()
    gate_commands = [
        expand_command(cmd, context)
        for cmd in manifest["compiler_gate"]["commands"]
    ]

    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "created_at": utc_now(),
        "state": "STARTED",
        "claim_allowed": False,
        "target": args.target,
        "target_level": target["level"],
        "target_scope": target["scope"],
        "mode": args.mode,
        "abi": args.abi,
        "policy": manifest["policy"],
        "manifest": {
            "path": str(manifest_path),
            "sha256": sha256_file(manifest_path),
        },
        "context": context,
        "compiler_gate_commands": gate_commands,
        "target_build_command": command,
        "install_executed": False,
        "launch_executed": False,
        "artifacts": [],
        "steps": [],
    }

    self_snapshot = git_snapshot(self_root)
    target_snapshot = self_snapshot if repo == self_root else git_snapshot(repo)
    receipt["self_repository"] = self_snapshot
    receipt["target_repository"] = target_snapshot
    if not args.allow_dirty and (self_snapshot["dirty"] or target_snapshot["dirty"]):
        raise GateError("dirty worktree detected; commit or pass --allow-dirty explicitly")

    receipt["required_files"] = check_required_files(repo, target["required_files"])
    receipt["required_environment"] = check_required_env(target["required_env_any"])
    receipt["toolchain"] = {
        name: command_version(name) for name in sorted(set(target["required_commands"]))
    }
    receipt["toolchain_alternatives"] = command_alternatives(
        target.get("required_command_any", [])
    )
    if not workdir.is_dir():
        raise GateError(f"workdir missing: {workdir}")

    if args.mode == "plan":
        receipt["state"] = "PLAN_READY"
        return 0, receipt
    if args.mode == "preflight":
        receipt["state"] = "PREFLIGHT_PASS"
        return 0, receipt

    out.mkdir(parents=True, exist_ok=True)
    for index, gate_command in enumerate(gate_commands, 1):
        step = run_logged(
            gate_command, self_root,
            out / "logs" / f"{index:02d}-compiler-gate.log",
        )
        receipt["steps"].append(step)
        if step["exit_code"] != 0:
            receipt["state"] = "COMPILER_GATE_FAILED"
            return 1, receipt

    target_step = run_logged(
        command, workdir,
        out / "logs" / f"{len(gate_commands)+1:02d}-target-build.log",
    )
    receipt["steps"].append(target_step)
    if target_step["exit_code"] != 0:
        receipt["state"] = "TARGET_BUILD_FAILED"
        return 1, receipt

    receipt["artifacts"] = artifact_records(target["artifact_globs"], context)
    if not receipt["artifacts"]:
        receipt["state"] = "TARGET_BUILD_NO_ARTIFACT"
        return 1, receipt
    invalid_android = [
        item for item in receipt["artifacts"]
        if item["android_structure"].get("state") == "FAIL"
    ]
    if invalid_android:
        receipt["state"] = "TARGET_ARTIFACT_INVALID"
        return 1, receipt
    receipt["state"] = "BUILD_PASS_ARTIFACTS_OBSERVED"
    return 0, receipt


def main() -> int:
    args = build_parser().parse_args()
    self_root = Path(__file__).resolve().parents[1]
    fallback_out = Path(args.out).expanduser().resolve() if args.out else (
        self_root / "build" / "compiler-gate" / args.target
    ).resolve()
    receipt_path = Path(args.receipt).expanduser().resolve() if args.receipt else (
        fallback_out / "compiler-receipt.json"
    )
    try:
        code, receipt = orchestrate(args)
    except (GateError, subprocess.CalledProcessError, OSError) as exc:
        receipt = {
            "schema": SCHEMA,
            "created_at": utc_now(),
            "state": "PREFLIGHT_FAILED",
            "claim_allowed": False,
            "target": args.target,
            "mode": args.mode,
            "error": str(exc),
            "install_executed": False,
            "launch_executed": False,
        }
        code = 1
    write_json_atomic(receipt_path, receipt)
    print(json.dumps(receipt, indent=2, sort_keys=True))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
