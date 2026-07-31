#!/usr/bin/env python3
"""Offline, append-only Foundation executor for RAFAELIA project checkouts.

The manifest uses a JSON document stored with a .yaml extension. JSON is a
valid YAML 1.2 subset, which keeps the contract readable by YAML tooling while
allowing this Termux runner to use only the Python standard library.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any


MANIFEST_SCHEMA = "rafaelia.foundation.manifest/v1"
RECEIPT_SCHEMA = "rafaelia.foundation.receipt/v1"
FOUNDATION_VERSION = "1.0.0"
PROJECT_ID_PATTERN = re.compile(r"^[A-Za-z][A-Za-z0-9._-]{2,127}$")
PROFILE_PATTERN = re.compile(r"^[a-z][a-z0-9-]{1,63}$")
ALLOWED_PROFILE_MODES = {"DOCUMENTATION", "COMMANDS"}
TEMPLATE_TOKENS = {"{{OUT}}", "{{SOURCE}}", "{{REPO}}"}
SHELL_EXECUTABLES = {"sh", "bash", "dash", "zsh", "fish", "busybox"}
ADAPTER_CHOICES = ("rafpolimata-compiler-gate",)


class FoundationError(RuntimeError):
    """A contract violation or execution failure."""


class FoundationGap(FoundationError):
    """A missing input that must remain an explicit TOKEN_VAZIO."""


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def sha256_text(value: str) -> str:
    """Return a stable digest without retaining potentially sensitive text."""
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def write_json(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def append_jsonl(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(data, ensure_ascii=False, sort_keys=True) + "\n")


def relative_path(value: str, field: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise FoundationError(f"{field} must be a non-empty relative path")
    normalized = value.replace("\\", "/")
    candidate = PurePosixPath(normalized)
    if candidate.is_absolute() or ".." in candidate.parts or normalized.startswith("./"):
        raise FoundationError(f"{field} must stay below repository root: {value!r}")
    return candidate.as_posix()


def safe_repo_file(repo_root: Path, relative: str, field: str) -> Path:
    normalized = relative_path(relative, field)
    target = (repo_root / normalized).resolve()
    root = repo_root.resolve()
    if target != root and root not in target.parents:
        raise FoundationError(f"{field} escapes repository root")
    return target


def captured_command(argv: list[str], cwd: Path) -> tuple[int, str] | None:
    """Run a small local observation command without invoking a shell."""
    try:
        result = subprocess.run(
            argv,
            cwd=cwd,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
            text=True,
            timeout=5,
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    return result.returncode, result.stdout.strip()


def repository_identity(repo_root: Path) -> dict[str, Any]:
    """Observe local Git identity only; no fetch, config, or mutation is used."""
    if shutil.which("git") is None:
        return {
            "state": "TOKEN_VAZIO_GIT_UNAVAILABLE",
            "next_verifiable_step": "Install or expose local git, then rerun the exact profile.",
        }

    inside = captured_command(["git", "rev-parse", "--is-inside-work-tree"], repo_root)
    if inside is None or inside[0] != 0 or inside[1] != "true":
        return {
            "state": "TOKEN_VAZIO_GIT_CHECKOUT_UNOBSERVED",
            "next_verifiable_step": "Run from a Git checkout with the source commit present.",
        }

    head = captured_command(["git", "rev-parse", "HEAD"], repo_root)
    status = captured_command(["git", "status", "--porcelain=v1", "--untracked-files=all"], repo_root)
    branch = captured_command(["git", "symbolic-ref", "--quiet", "--short", "HEAD"], repo_root)
    if head is None or head[0] != 0 or status is None or status[0] != 0:
        return {
            "state": "TOKEN_VAZIO_GIT_IDENTITY_INCOMPLETE",
            "next_verifiable_step": "Repair local Git metadata, then rerun the exact profile.",
        }

    status_lines = [line for line in status[1].splitlines() if line]
    return {
        "state": "BOUND",
        "head_sha": head[1],
        "branch": branch[1] if branch is not None and branch[0] == 0 and branch[1] else "DETACHED",
        "worktree_clean": not status_lines,
        "worktree_entry_count": len(status_lines),
        "worktree_state_sha256": sha256_text(status[1]),
    }


def read_manifest(repo_root: Path) -> tuple[dict[str, Any], Path]:
    path = repo_root / ".rafaelia" / "foundation.yaml"
    if not path.is_file():
        raise FoundationGap("TOKEN_VAZIO_MANIFEST_MISSING: run Foundation init for this checkout")
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise FoundationError(
            f"{path.relative_to(repo_root)} must be JSON-form YAML 1.2: line {exc.lineno}, column {exc.colno}"
        ) from exc
    if not isinstance(data, dict):
        raise FoundationError("foundation manifest root must be an object")
    return data, path


def assert_bool(mapping: dict[str, Any], key: str, expected: bool, location: str) -> None:
    if mapping.get(key) is not expected:
        raise FoundationError(f"{location}.{key} must be {str(expected).lower()}")


def validate_command(command: Any, location: str) -> list[str]:
    if not isinstance(command, list) or not command:
        raise FoundationError(f"{location} must be a non-empty argv list")
    if not all(isinstance(item, str) and item and "\x00" not in item and "\n" not in item for item in command):
        raise FoundationError(f"{location} contains an invalid argument")
    executable = Path(command[0]).name
    if executable in SHELL_EXECUTABLES:
        raise FoundationError(f"{location} cannot invoke a shell interpreter")
    if executable == "env" and any(Path(argument).name in SHELL_EXECUTABLES for argument in command[1:]):
        raise FoundationError(f"{location} cannot route through env to a shell interpreter")
    for argument in command:
        for token in re.findall(r"\{\{[^}]+\}\}", argument):
            if token not in TEMPLATE_TOKENS:
                raise FoundationError(f"{location} contains unsupported template token {token}")
    return command


def validate_manifest(manifest: dict[str, Any]) -> None:
    if manifest.get("schema") != MANIFEST_SCHEMA:
        raise FoundationError(f"schema must be {MANIFEST_SCHEMA}")
    if manifest.get("version") != FOUNDATION_VERSION:
        raise FoundationError(f"version must be {FOUNDATION_VERSION}")

    project = manifest.get("project")
    if not isinstance(project, dict):
        raise FoundationError("project must be an object")
    project_id = project.get("id")
    if not isinstance(project_id, str) or not PROJECT_ID_PATTERN.fullmatch(project_id):
        raise FoundationError("project.id must be a stable identifier")

    governance = manifest.get("governance")
    if not isinstance(governance, dict):
        raise FoundationError("governance must be an object")
    for key in ("claim_allowed", "network_required", "destructive_operations", "remote_ci_substituted"):
        assert_bool(governance, key, False, "governance")
    if governance.get("token_vazio_is_valid") is not True:
        raise FoundationError("governance.token_vazio_is_valid must be true")

    runtime = manifest.get("runtime")
    if not isinstance(runtime, dict):
        raise FoundationError("runtime must be an object")
    if runtime.get("class") != "ANDROID_TERMUX_LOCAL":
        raise FoundationError("runtime.class must be ANDROID_TERMUX_LOCAL")
    if runtime.get("output_directory") != "COMPILA":
        raise FoundationError("runtime.output_directory must be COMPILA")
    if runtime.get("append_only_receipts") is not True:
        raise FoundationError("runtime.append_only_receipts must be true")

    inputs = manifest.get("inputs")
    if not isinstance(inputs, dict):
        raise FoundationError("inputs must be an object")
    source = inputs.get("source")
    if source is not None:
        relative_path(source, "inputs.source")
    required_paths = inputs.get("required_paths", [])
    if not isinstance(required_paths, list) or not all(isinstance(item, str) for item in required_paths):
        raise FoundationError("inputs.required_paths must be a list of relative paths")
    for index, item in enumerate(required_paths):
        relative_path(item, f"inputs.required_paths[{index}]")

    profiles = manifest.get("profiles")
    if not isinstance(profiles, dict) or not profiles:
        raise FoundationError("profiles must be a non-empty object")
    for name, profile in profiles.items():
        if not isinstance(name, str) or not PROFILE_PATTERN.fullmatch(name):
            raise FoundationError(f"invalid profile name: {name!r}")
        if not isinstance(profile, dict):
            raise FoundationError(f"profiles.{name} must be an object")
        if profile.get("mode") not in ALLOWED_PROFILE_MODES:
            raise FoundationError(f"profiles.{name}.mode is unsupported")
        if profile.get("requires_source") not in {True, False}:
            raise FoundationError(f"profiles.{name}.requires_source must be boolean")
        profile_paths = profile.get("required_paths", [])
        if not isinstance(profile_paths, list) or not all(isinstance(item, str) for item in profile_paths):
            raise FoundationError(f"profiles.{name}.required_paths must be a list")
        for index, item in enumerate(profile_paths):
            relative_path(item, f"profiles.{name}.required_paths[{index}]")
        commands = profile.get("commands", [])
        if not isinstance(commands, list):
            raise FoundationError(f"profiles.{name}.commands must be a list")
        for index, command in enumerate(commands):
            validate_command(command, f"profiles.{name}.commands[{index}]")
        if profile["mode"] == "DOCUMENTATION" and commands:
            raise FoundationError(f"profiles.{name} documentation mode cannot execute commands")
        if profile["mode"] == "COMMANDS" and not commands:
            raise FoundationGap(f"TOKEN_VAZIO_PROFILE_COMMANDS: profiles.{name} has no commands")


def profile_for(manifest: dict[str, Any], name: str) -> dict[str, Any]:
    profiles = manifest["profiles"]
    if name not in profiles:
        available = ", ".join(sorted(profiles))
        raise FoundationGap(f"TOKEN_VAZIO_PROFILE_UNKNOWN: {name}; available: {available}")
    return profiles[name]


def substitute(argument: str, source: str | None, out_rel: str) -> str:
    values = {
        "{{OUT}}": out_rel,
        "{{SOURCE}}": source or "TOKEN_VAZIO_SOURCE",
        "{{REPO}}": ".",
    }
    result = argument
    for token, replacement in values.items():
        result = result.replace(token, replacement)
    return result


def create_run_directory(repo_root: Path) -> tuple[str, Path, str]:
    base = repo_root / "COMPILA"
    base.mkdir(parents=True, exist_ok=True)
    stem = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + f"-{os.getpid()}"
    run_id = stem
    suffix = 1
    while (base / run_id).exists():
        suffix += 1
        run_id = f"{stem}-{suffix}"
    run_dir = base / run_id
    run_dir.mkdir(parents=False, exist_ok=False)
    return run_id, run_dir, Path("COMPILA", run_id).as_posix()


def executable_identity(command: str, repo_root: Path) -> dict[str, Any]:
    """Record the resolved executable without executing version probes."""
    path: Path | None
    if "/" in command:
        local_command = command[2:] if command.startswith("./") else command
        path = safe_repo_file(repo_root, local_command, "command executable")
    else:
        located = shutil.which(command)
        path = Path(located).resolve() if located else None
    record: dict[str, Any] = {
        "requested": command,
        "available": bool(path and path.is_file() and os.access(path, os.X_OK)),
    }
    if path and path.is_file():
        record["path"] = path.as_posix()
        try:
            record["sha256"] = sha256_file(path)
            record["size_bytes"] = path.stat().st_size
        except OSError:
            record["sha256"] = "TOKEN_VAZIO_EXECUTABLE_UNREADABLE"
    return record


def environment_record(repo_root: Path, commands: list[list[str]]) -> dict[str, Any]:
    observed_tools: list[dict[str, Any]] = []
    seen: set[str] = set()
    for command in commands:
        if command and command[0] not in seen:
            seen.add(command[0])
            observed_tools.append(executable_identity(command[0], repo_root))
    return {
        "captured_at": utc_now(),
        "runtime_class": "ANDROID_TERMUX_LOCAL",
        "python": sys.version.split()[0],
        "python_executable": sys.executable,
        "implementation": platform.python_implementation(),
        "platform": platform.platform(),
        "machine": platform.machine(),
        "termux_prefix_present": bool(os.environ.get("PREFIX")),
        "termux_prefix": os.environ.get("PREFIX", "TOKEN_VAZIO_TERMUX_PREFIX"),
        "observed_tools": observed_tools,
        "claim_allowed": False,
        "network_required": False,
    }


def input_manifest(repo_root: Path, manifest_path: Path, manifest: dict[str, Any], profile: dict[str, Any]) -> list[dict[str, Any]]:
    paths = [
        ".rafaelia/foundation.yaml",
        ".rafaelia/tools/rafaelia_foundation.py",
        ".rafaelia/tools/gate_computational_v1.py",
    ]
    source = manifest["inputs"].get("source")
    if source:
        paths.append(source)
    paths.extend(manifest["inputs"].get("required_paths", []))
    paths.extend(profile.get("required_paths", []))
    seen: set[str] = set()
    records: list[dict[str, Any]] = []
    for relative in paths:
        normalized = relative_path(relative, "input path")
        if normalized in seen:
            continue
        seen.add(normalized)
        path = safe_repo_file(repo_root, normalized, "input path")
        if not path.is_file():
            raise FoundationGap(f"TOKEN_VAZIO_INPUT_MISSING: {normalized}")
        records.append({
            "path": normalized,
            "sha256": sha256_file(path),
            "size_bytes": path.stat().st_size,
        })
    if not manifest_path.is_file():
        raise FoundationError("manifest disappeared during execution")
    return records


def executable_available(command: str, repo_root: Path) -> bool:
    if "/" in command:
        local_command = command[2:] if command.startswith("./") else command
        candidate = safe_repo_file(repo_root, local_command, "command executable")
        return candidate.is_file() and os.access(candidate, os.X_OK)
    return shutil.which(command) is not None


def preflight(repo_root: Path, manifest: dict[str, Any], profile_name: str, profile: dict[str, Any], out_rel: str) -> list[list[str]]:
    source = manifest["inputs"].get("source")
    if profile["requires_source"] and not source:
        raise FoundationGap(f"TOKEN_VAZIO_SOURCE_REQUIRED: profile {profile_name} requires inputs.source")
    if source:
        source_path = safe_repo_file(repo_root, source, "inputs.source")
        if not source_path.is_file():
            raise FoundationGap(f"TOKEN_VAZIO_INPUT_MISSING: {source}")

    commands: list[list[str]] = []
    for command in profile.get("commands", []):
        argv = [substitute(argument, source, out_rel) for argument in command]
        if "{{SOURCE}}" in command and not source:
            raise FoundationGap(f"TOKEN_VAZIO_SOURCE_REQUIRED: profile {profile_name} uses {{SOURCE}}")
        if not executable_available(argv[0], repo_root):
            raise FoundationGap(f"TOKEN_VAZIO_TOOL_MISSING: {argv[0]}")
        commands.append(argv)
    return commands


def receipt_base(run_id: str, operation: str, profile_name: str | None, manifest_path: Path | None) -> dict[str, Any]:
    return {
        "schema": RECEIPT_SCHEMA,
        "foundation_version": FOUNDATION_VERSION,
        "run_id": run_id,
        "operation": operation,
        "profile": profile_name,
        "created_at": utc_now(),
        "claim_allowed": False,
        "remote_ci_substituted": False,
        "decision": "NOT_PROMOTED",
        "manifest_path": manifest_path.as_posix() if manifest_path else None,
    }


def finish_receipt(run_dir: Path, receipt: dict[str, Any]) -> Path:
    receipt_path = run_dir / "receipt.json"
    write_json(receipt_path, receipt)
    digest_path = run_dir / "receipt.sha256"
    digest_path.write_text(f"{sha256_file(receipt_path)}  receipt.json\n", encoding="utf-8")
    return receipt_path


def collect_artifacts(repo_root: Path, run_dir: Path) -> list[dict[str, Any]]:
    """Hash every regular output in a run, rejecting symlink-based escapes."""
    records: list[dict[str, Any]] = []
    excluded = {"receipt.json", "receipt.sha256"}
    for path in sorted(run_dir.rglob("*"), key=lambda item: item.as_posix()):
        if path.name in excluded:
            continue
        if path.is_symlink():
            raise FoundationError(
                f"run artifact cannot be a symlink: {path.relative_to(repo_root).as_posix()}"
            )
        if not path.is_file():
            continue
        records.append({
            "path": path.relative_to(repo_root).as_posix(),
            "sha256": sha256_file(path),
            "size_bytes": path.stat().st_size,
        })
    return records


def run_operation(repo_root: Path, operation: str, profile_name: str | None) -> int:
    repo_root = repo_root.resolve()
    run_id, run_dir, out_rel = create_run_directory(repo_root)
    manifest_path: Path | None = None
    receipt = receipt_base(run_id, operation, profile_name, manifest_path)
    command_records: list[dict[str, Any]] = []
    status = "FAIL"
    exit_code = 1
    try:
        manifest, manifest_path = read_manifest(repo_root)
        receipt["manifest_path"] = manifest_path.relative_to(repo_root).as_posix()
        validate_manifest(manifest)
        profile = profile_for(manifest, profile_name or "documentation")
        receipt["profile"] = profile_name or "documentation"
        receipt["repository_identity"] = repository_identity(repo_root)

        if operation == "plan":
            source = manifest["inputs"].get("source")
            planned = [
                [substitute(argument, source, out_rel) for argument in command]
                for command in profile.get("commands", [])
            ]
            write_json(run_dir / "environment.json", environment_record(repo_root, planned))
            write_json(run_dir / "plan.json", {
                "schema": "rafaelia.foundation.plan/v1",
                "profile": receipt["profile"],
                "mode": profile["mode"],
                "commands": planned,
                "claim_allowed": False,
                "network_required": False,
            })
            status = "PLAN_ONLY"
            exit_code = 0
            receipt["next_verifiable_step"] = "Run verify, then explicitly run the selected profile."
        else:
            inputs = input_manifest(repo_root, manifest_path, manifest, profile)
            write_json(run_dir / "input_manifest.json", {
                "schema": "rafaelia.foundation.input-manifest/v1",
                "files": inputs,
                "claim_allowed": False,
            })
            commands = preflight(repo_root, manifest, receipt["profile"], profile, out_rel)
            write_json(run_dir / "environment.json", environment_record(repo_root, commands))
            if operation == "verify":
                write_json(run_dir / "plan.json", {
                    "schema": "rafaelia.foundation.plan/v1",
                    "profile": receipt["profile"],
                    "mode": profile["mode"],
                    "commands": commands,
                    "claim_allowed": False,
                    "network_required": False,
                })
                status = "PASS_PREFLIGHT_ONLY"
                exit_code = 0
                receipt["next_verifiable_step"] = "Run the profile explicitly to produce a local execution receipt."
            elif profile["mode"] == "DOCUMENTATION":
                status = "PASS_STRUCTURE_ONLY"
                exit_code = 0
                receipt["next_verifiable_step"] = "Configure an explicit executable profile when implementation evidence is needed."
            else:
                stdout_path = run_dir / "stdout.log"
                stderr_path = run_dir / "stderr.log"
                with stdout_path.open("w", encoding="utf-8") as stdout, stderr_path.open("w", encoding="utf-8") as stderr:
                    for index, argv in enumerate(commands):
                        started_at = utc_now()
                        append_jsonl(run_dir / "commands.jsonl", {
                            "event": "COMMAND_STARTED",
                            "index": index,
                            "argv": argv,
                            "started_at": started_at,
                            "shell": False,
                        })
                        result = subprocess.run(
                            argv,
                            cwd=repo_root,
                            stdin=subprocess.DEVNULL,
                            stdout=stdout,
                            stderr=stderr,
                            check=False,
                            env={**os.environ, "RAFAELIA_FOUNDATION_OFFLINE": "1"},
                        )
                        record = {
                            "event": "COMMAND_FINISHED",
                            "index": index,
                            "argv": argv,
                            "started_at": started_at,
                            "finished_at": utc_now(),
                            "exit_code": result.returncode,
                            "shell": False,
                        }
                        append_jsonl(run_dir / "commands.jsonl", record)
                        command_records.append(record)
                        if result.returncode != 0:
                            raise FoundationError(f"command {index} failed with exit code {result.returncode}")
                status = "PASS_LOCAL_EXECUTION"
                exit_code = 0
                receipt["next_verifiable_step"] = "Review the input manifest and receipt before promoting any project-specific evidence."
    except FoundationGap as exc:
        status = "TOKEN_VAZIO"
        exit_code = 2
        receipt["gap"] = str(exc)
        receipt["next_verifiable_step"] = "Supply the named input or tool, then rerun this exact profile."
    except FoundationError as exc:
        status = "FAIL"
        exit_code = 1
        receipt["error"] = str(exc)
        receipt["next_verifiable_step"] = "Correct the contract violation, then rerun verification."
    except OSError as exc:
        status = "FAIL"
        exit_code = 1
        receipt["error"] = f"operating system error: {exc}"
        receipt["next_verifiable_step"] = "Inspect the local runtime and rerun after the environment is repaired."

    receipt["status"] = status
    receipt["exit_code"] = exit_code
    receipt["commands_executed"] = len(command_records)
    receipt["commands"] = command_records
    try:
        receipt["artifacts"] = collect_artifacts(repo_root, run_dir)
    except FoundationError as exc:
        if status.startswith("PASS") or status == "PLAN_ONLY":
            status = "FAIL"
            exit_code = 1
            receipt["status"] = status
            receipt["exit_code"] = exit_code
            receipt["error"] = str(exc)
            receipt["next_verifiable_step"] = "Remove the unsafe artifact and rerun the exact profile."
        receipt.setdefault("artifact_collection_gap", str(exc))
    receipt_path = finish_receipt(run_dir, receipt)
    print(f"STATUS={status}")
    print(f"RECEIPT={receipt_path.relative_to(repo_root).as_posix()}")
    print(f"RECEIPT_SHA256={sha256_file(receipt_path)}")
    return exit_code


def profile_definition(profile: str, source: str | None) -> dict[str, Any]:
    documentation = {
        "mode": "DOCUMENTATION",
        "requires_source": False,
        "required_paths": ["README.md"],
        "commands": [],
        "description": "Validate the declared project documentation boundary without executing build commands.",
    }
    definitions: dict[str, dict[str, Any]] = {
        "documentation": documentation,
        "python": {
            "mode": "COMMANDS",
            "requires_source": True,
            "required_paths": ["README.md"],
            "commands": [[
                "python3", "-c",
                "import py_compile; py_compile.compile('{{SOURCE}}', cfile='{{OUT}}/source.pyc', doraise=True)",
            ]],
            "description": "Compile one explicit Python source file into COMPILA with the local interpreter.",
        },
        "freestanding-object": {
            "mode": "COMMANDS",
            "requires_source": True,
            "required_paths": ["README.md"],
            "commands": [[
                "clang", "-std=c11", "-Wall", "-Wextra", "-ffreestanding",
                "-fno-builtin", "-fno-stack-protector", "-fno-pic", "-c",
                "{{SOURCE}}", "-o", "{{OUT}}/freestanding.o",
            ]],
            "description": "Compile an explicit C or assembly source into a freestanding object; it does not claim a linked ELF.",
        },
        "make": {
            "mode": "COMMANDS",
            "requires_source": False,
            "required_paths": ["Makefile"],
            "commands": [["make"]],
            "description": "Run the repository's explicit Makefile default target locally.",
        },
        "cmake": {
            "mode": "COMMANDS",
            "requires_source": False,
            "required_paths": ["CMakeLists.txt"],
            "commands": [
                ["cmake", "-S", ".", "-B", "{{OUT}}/cmake"],
                ["cmake", "--build", "{{OUT}}/cmake"],
            ],
            "description": "Configure and build an explicit local CMake project inside COMPILA.",
        },
    }
    if profile not in definitions:
        raise FoundationError(f"unknown initialization profile {profile}")
    result = {"documentation": documentation}
    if profile != "documentation":
        result[profile] = definitions[profile]
    if definitions[profile]["requires_source"] and not source:
        raise FoundationGap(f"TOKEN_VAZIO_SOURCE_REQUIRED: --source is required for {profile}")
    return result


def adapter_definition(adapter: str, project_id: str) -> tuple[dict[str, Any], list[tuple[Path, str]]]:
    """Return a versioned, explicit adapter; no repository is autodetected."""
    if adapter != "rafpolimata-compiler-gate":
        raise FoundationError(f"unknown adapter {adapter!r}")
    source = Path(__file__).resolve().parents[1] / "adapters" / "rafpolimata" / "rafpolimata_foundation_compiler_gate.py"
    if not source.is_file():
        raise FoundationError(f"adapter source is missing: {source}")
    documentation = {
        "mode": "DOCUMENTATION",
        "requires_source": False,
        "required_paths": ["README.md"],
        "commands": [],
        "description": "Validate RafPolimata's declared document boundary without executing a build.",
    }
    manifest = {
        "schema": MANIFEST_SCHEMA,
        "version": FOUNDATION_VERSION,
        "project": {
            "id": project_id,
            "kind": "rafpolimata-compiler-gate",
            "repository": "rafaelmeloreisnovo/RafPolimata",
            "adapter": "rafpolimata-compiler-gate/v1",
        },
        "governance": {
            "claim_allowed": False,
            "network_required": False,
            "destructive_operations": False,
            "remote_ci_substituted": False,
            "token_vazio_is_valid": True,
        },
        "runtime": {
            "class": "ANDROID_TERMUX_LOCAL",
            "output_directory": "COMPILA",
            "append_only_receipts": True,
        },
        "inputs": {
            "source": "raf_main.c",
            "required_paths": [
                "README.md",
                "raf_compile.h",
                "raf_main.c",
                "raf_frontend.c",
                "raf_cpu.c",
                "raf_asm_emit.c",
                "raf_precomp.c",
                "scripts/validate_runtime_truth_local.sh",
                "scripts/rafpolimata_foundation_compiler_gate.py",
            ],
        },
        "profiles": {
            "documentation": documentation,
            "compiler-local-gate": {
                "mode": "COMMANDS",
                "requires_source": True,
                "required_paths": [
                    "scripts/validate_runtime_truth_local.sh",
                    "scripts/rafpolimata_foundation_compiler_gate.py",
                ],
                "commands": [[
                    "python3",
                    "scripts/rafpolimata_foundation_compiler_gate.py",
                    "--out",
                    "{{OUT}}/test-summary.json",
                ]],
                "description": "Execute RafPolimata's tracked nine-block compiler gate and emit explicit test accounting.",
            },
        },
    }
    validate_manifest(manifest)
    return manifest, [(source, "scripts/rafpolimata_foundation_compiler_gate.py")]


def autoexec_text() -> str:
    return """#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
RUNNER="$ROOT/.rafaelia/tools/rafaelia_foundation.py"
GATE="$ROOT/.rafaelia/tools/gate_computational_v1.py"

command -v python3 >/dev/null 2>&1 || {
  printf '%s\\n' "TOKEN_VAZIO_TOOL_MISSING: python3" >&2
  exit 127
}
[ -f "$RUNNER" ] || {
  printf '%s\\n' "TOKEN_VAZIO_MANIFEST_MISSING: run Foundation init from Mapa first" >&2
  exit 2
}

if [ "${1:-}" = "gate" ]; then
  shift
  [ -f "$GATE" ] || {
    printf '%s\\n' "TOKEN_VAZIO_GATE_MISSING: run Foundation init from Mapa first" >&2
    exit 2
  }
  exec python3 "$GATE" --repo-root "$ROOT" "$@"
fi

exec python3 "$RUNNER" "$@" --repo-root "$ROOT"
"""


def project_readme_text() -> str:
    return """# RAFAELIA Foundation local checkout

This directory was generated without network access or hidden execution.

plan -> verify -> explicit run -> receipt

Use the Termux autoexec script with plan, verify, or run PROFILE. The manifest
is JSON-form YAML 1.2 so this runner has no PyYAML dependency. Each directory
under COMPILA is append-only local evidence; it does not promote a claim.
"""


def initialize(
    repo_root: Path,
    project_id: str,
    profile: str,
    source: str | None,
    adapter: str | None = None,
) -> None:
    repo_root = repo_root.resolve()
    if not repo_root.is_dir():
        raise FoundationError(f"repository root does not exist: {repo_root}")
    if not PROJECT_ID_PATTERN.fullmatch(project_id):
        raise FoundationError("--project-id must be a stable identifier")
    adapter_files: list[tuple[Path, str]] = []
    if adapter is not None:
        if profile != "documentation" or source is not None:
            raise FoundationError("--adapter is exclusive with --profile and --source")
        manifest, adapter_files = adapter_definition(adapter, project_id)
    else:
        if source is not None:
            relative_path(source, "--source")
        profiles = profile_definition(profile, source)
        manifest = {
            "schema": MANIFEST_SCHEMA,
            "version": FOUNDATION_VERSION,
            "project": {
                "id": project_id,
                "kind": profile,
                "repository": "LOCAL_CHECKOUT_UNPINNED",
            },
            "governance": {
                "claim_allowed": False,
                "network_required": False,
                "destructive_operations": False,
                "remote_ci_substituted": False,
                "token_vazio_is_valid": True,
            },
            "runtime": {
                "class": "ANDROID_TERMUX_LOCAL",
                "output_directory": "COMPILA",
                "append_only_receipts": True,
            },
            "inputs": {
                "source": source,
                "required_paths": ["README.md"],
            },
            "profiles": profiles,
        }
    targets = [
        repo_root / ".rafaelia" / "foundation.yaml",
        repo_root / ".rafaelia" / "tools" / "rafaelia_foundation.py",
        repo_root / ".rafaelia" / "tools" / "gate_computational_v1.py",
        repo_root / ".rafaelia" / "README.md",
        repo_root / "termux" / "autoexec-rafaelia.sh",
    ]
    targets.extend(repo_root / destination for _, destination in adapter_files)
    existing = [path.relative_to(repo_root).as_posix() for path in targets if path.exists()]
    if existing:
        raise FoundationError("refusing to overwrite existing Foundation paths: " + ", ".join(existing))
    validate_manifest(manifest)
    targets[1].parent.mkdir(parents=True, exist_ok=True)
    write_json(targets[0], manifest)
    shutil.copy2(Path(__file__).resolve(), targets[1])
    gate_source = Path(__file__).resolve().with_name("gate_computational_v1.py")
    if not gate_source.is_file():
        raise FoundationError(f"gate source is missing beside runner: {gate_source}")
    shutil.copy2(gate_source, targets[2])
    targets[3].write_text(project_readme_text(), encoding="utf-8")
    targets[4].parent.mkdir(parents=True, exist_ok=True)
    targets[4].write_text(autoexec_text(), encoding="utf-8")
    targets[4].chmod(targets[4].stat().st_mode | 0o111)
    for adapter_source, destination in adapter_files:
        target = repo_root / destination
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(adapter_source, target)
    compile_ignore = repo_root / "COMPILA" / ".gitignore"
    compile_ignore.parent.mkdir(parents=True, exist_ok=True)
    if not compile_ignore.exists():
        compile_ignore.write_text("# Local Foundation receipts are reviewed before explicit publication.\n*\n!.gitignore\n", encoding="utf-8")
    print(f"FOUNDATION_INITIALIZED={repo_root}")
    print(f"MANIFEST={targets[0].relative_to(repo_root).as_posix()}")
    print(f"AUTOEXEC={targets[4].relative_to(repo_root).as_posix()}")
    if adapter is not None:
        print(f"ADAPTER={adapter}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    init = subparsers.add_parser("init", help="Create non-overwriting Foundation files in a checkout.")
    init.add_argument("--repo-root", type=Path, required=True)
    init.add_argument("--project-id", required=True)
    init.add_argument("--profile", choices=("documentation", "python", "freestanding-object", "make", "cmake"), default="documentation")
    init.add_argument("--source", help="Repository-relative source file required by source-oriented profiles.")
    init.add_argument("--adapter", choices=ADAPTER_CHOICES, help="Install one explicit target adapter instead of a generic profile.")

    for command in ("plan", "verify"):
        item = subparsers.add_parser(command)
        item.add_argument("--repo-root", type=Path, default=Path("."))
        item.add_argument("--profile", default="documentation")
    run = subparsers.add_parser("run")
    run.add_argument("profile")
    run.add_argument("--repo-root", type=Path, default=Path("."))
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.command == "init":
        try:
            initialize(args.repo_root, args.project_id, args.profile, args.source, args.adapter)
        except FoundationGap as exc:
            print(str(exc), file=sys.stderr)
            return 2
        except FoundationError as exc:
            print(f"FAIL: {exc}", file=sys.stderr)
            return 1
        return 0
    return run_operation(args.repo_root, args.command, args.profile)


if __name__ == "__main__":
    raise SystemExit(main())
