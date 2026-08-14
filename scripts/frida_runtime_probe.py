#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

SCHEMA = "raf.frida-runtime-readiness.v1"


def run_readonly(command: list[str], timeout: int = 8) -> dict[str, Any]:
    try:
        proc = subprocess.run(
            command,
            text=True,
            capture_output=True,
            timeout=timeout,
            check=False,
            env={**os.environ, "LC_ALL": "C"},
        )
        return {
            "command": command,
            "exit_code": proc.returncode,
            "stdout": proc.stdout[-8000:],
            "stderr": proc.stderr[-4000:],
        }
    except (OSError, subprocess.TimeoutExpired) as exc:
        return {"command": command, "exit_code": None, "error": str(exc)}


def command_info(name: str) -> dict[str, Any]:
    path = shutil.which(name)
    return {"name": name, "available": path is not None, "path": path}


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def getprop(key: str) -> str | None:
    executable = shutil.which("getprop")
    if executable is None and Path("/system/bin/getprop").exists():
        executable = "/system/bin/getprop"
    if executable is None:
        return None
    result = run_readonly([executable, key], timeout=3)
    if result.get("exit_code") != 0:
        return None
    value = str(result.get("stdout", "")).strip()
    return value or None


def parse_device_lines(text: str) -> list[dict[str, str]]:
    devices: list[dict[str, str]] = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.lower().startswith("id") or set(line) <= {"-", " ", "\t"}:
            continue
        parts = line.split(None, 2)
        if len(parts) >= 2:
            item = {"id": parts[0], "type": parts[1]}
            if len(parts) == 3:
                item["name"] = parts[2]
            devices.append(item)
    return devices


def build_report(gadget_path: str | None) -> dict[str, Any]:
    commands = {name: command_info(name) for name in ("frida", "frida-ps", "frida-ls-devices", "adb")}

    uid = os.geteuid() if hasattr(os, "geteuid") else None
    root_observed = uid == 0 if uid is not None else None
    android_root = os.environ.get("ANDROID_ROOT")
    prefix = os.environ.get("PREFIX")
    android_observed = bool(android_root or getprop("ro.build.version.release"))

    probes: list[dict[str, Any]] = []
    version: str | None = None
    if commands["frida"]["available"]:
        result = run_readonly([commands["frida"]["path"], "--version"])
        probes.append(result)
        if result.get("exit_code") == 0:
            version = str(result.get("stdout", "")).strip() or None

    devices: list[dict[str, str]] = []
    if commands["frida-ls-devices"]["available"]:
        result = run_readonly([commands["frida-ls-devices"]["path"])
        probes.append(result)
        if result.get("exit_code") == 0:
            devices = parse_device_lines(str(result.get("stdout", "")))

    gadget: dict[str, Any]
    if gadget_path:
        path = Path(gadget_path).expanduser().resolve()
        if path.is_file():
            gadget = {
                "state": "PRESENT_HASHED",
                "path": str(path),
                "size": path.stat().st_size,
                "sha256": sha256_file(path),
            }
        else:
            gadget = {"state": "TOKEN_VAZIO_GADGET_PATH_NOT_FOUND", "path": str(path)}
    else:
        gadget = {"state": "TOKEN_VAZIO_GADGET_PATH_NOT_SUPPLIED"}

    tooling_present = commands["frida"]["available"] and commands["frida-ls-devices"]["available"]
    if not tooling_present:
        readiness = "TOKEN_VAZIO_FRIDA_TOOLS_NOT_FOUND"
    elif not devices:
        readiness = "TOOLS_PRESENT_DEVICE_CHANNEL_UNPROVEN"
    else:
        readiness = "TOOLS_AND_DEVICE_ENUMERATION_OBSERVED"

    if root_observed is True:
        runtime_route = "SERVER_ROUTE_CANDIDATE_ROOT_OBSERVED"
    elif root_observed is False:
        runtime_route = "GADGET_OR_DEBUGGABLE_APP_ROUTE_CANDIDATE_NONROOT"
    else:
        runtime_route = "TOKEN_VAZIO_PRIVILEGE_STATE_UNKNOWN"

    return {
        "schema": SCHEMA,
        "mode": "READ_ONLY_READINESS_PROBE",
        "state": readiness,
        "host": {
            "android_observed": android_observed,
            "android_release": getprop("ro.build.version.release"),
            "abi": getprop("ro.product.cpu.abi"),
            "uid": uid,
            "root_observed": root_observed,
            "ANDROID_ROOT": android_root,
            "PREFIX": prefix,
        },
        "commands": commands,
        "frida_version": version,
        "devices": devices,
        "gadget": gadget,
        "route": runtime_route,
        "capabilities": {
            "tooling_present": tooling_present,
            "device_enumeration_observed": bool(devices),
            "attach_tested": False,
            "injection_tested": False,
            "hook_tested": False,
            "ephemeral_patch_tested": False,
            "background_service_tested": False,
        },
        "policy": {
            "automatic_attach": False,
            "automatic_injection": False,
            "automatic_patch": False,
            "automatic_install": False,
            "automatic_privilege_escalation": False,
            "target_scope": "USER_CONTROLLED_APP_OR_EXPLICIT_TEST_TARGET_ONLY",
            "claim_allowed": False,
        },
        "F_ok": "Frida host tooling/readiness can be observed without modifying a process.",
        "F_gap": "Actual app attach, Gadget load, hooks, background persistence and dynamic correction remain unproven until target-device receipts exist.",
        "F_next": "Provide or build the developer app/Gadget artifact, hash it, run an explicit user-controlled attach/hook test, then preserve the receipt before enabling any ephemeral patch workflow.",
        "probes": probes,
    }


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="RAFAELIA Frida Runtime Doctor read-only readiness probe")
    p.add_argument("--gadget", help="optional exact Frida Gadget path to hash; no loading is performed")
    p.add_argument("--json", action="store_true", help="emit compact JSON as the final line")
    return p


def main() -> int:
    args = parser().parse_args()
    report = build_report(args.gadget)
    if args.json:
        print(json.dumps(report, ensure_ascii=False, sort_keys=True))
    else:
        print(json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
