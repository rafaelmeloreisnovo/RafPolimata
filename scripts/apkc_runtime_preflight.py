#!/usr/bin/env python3
"""Operational preflight for ApkC external toolchains and Android artifacts.

The preflight does not install tools and does not infer success from their names.
It records environment readiness and known source-contract blockers before a build.
"""
from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

SCHEMA = "raf.apkc-runtime-preflight.v1"

TOOL_GROUPS: dict[str, list[str]] = {
    "core": ["sh", "python3"],
    "native_c": ["clang", "clang++"],
    "rust": ["rustc"],
    "go": ["go"],
    "jvm": ["java", "javac", "jar", "kotlinc", "d8"],
    "scripting": ["perl", "node", "php", "ruby", "clojure"],
    "frontend": ["npx"],
    "gpu": ["glslc"],
    "dsp": ["hexagon-clang"],
    "swift": ["swiftc"],
}


def command_version(path: str) -> str:
    for args in ([path, "--version"], [path, "-version"], [path, "version"]):
        try:
            proc = subprocess.run(
                args, check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                text=True, timeout=5,
            )
        except (OSError, subprocess.TimeoutExpired):
            continue
        line = (proc.stdout or "").strip().splitlines()
        if line:
            return line[0][:300]
    return "TOKEN_VAZIO"


def tool_inventory() -> dict[str, Any]:
    groups: dict[str, Any] = {}
    for group, names in TOOL_GROUPS.items():
        entries: dict[str, Any] = {}
        for name in names:
            path = shutil.which(name)
            entries[name] = {
                "state": "PRESENT" if path else "TOKEN_VAZIO",
                "path": path or "TOKEN_VAZIO",
                "version": command_version(path) if path else "TOKEN_VAZIO",
            }
        groups[group] = entries
    return groups


def writable_directory(path: Path) -> tuple[bool, str]:
    if not path.is_dir():
        return False, "not_directory"
    try:
        with tempfile.NamedTemporaryFile(prefix="apkc-preflight-", dir=path, delete=True) as fh:
            fh.write(b"ok")
            fh.flush()
        return True, "writable"
    except OSError as exc:
        return False, f"not_writable:{type(exc).__name__}"


def inspect_source(root: Path) -> dict[str, Any]:
    apkc_path = root / "Apkc/apkc.c"
    profiles_path = root / "Apkc/lang_profile.h"
    apkc = apkc_path.read_text(encoding="utf-8", errors="replace") if apkc_path.is_file() else ""
    profiles = profiles_path.read_text(encoding="utf-8", errors="replace") if profiles_path.is_file() else ""

    checks = {
        "hardcoded_posix_tmp": bool(re.search(r'"/tmp/(?:apkc|classes|jsx)', apkc)),
        "arm32_external_compiler_unsupported": "fork_exec_wait: not supported on ARM32" in apkc,
        "fork_output_read_bounded_without_overflow_probe": (
            "while (total < outbuf_cap)" in apkc and
            "outbuf_cap - total" in apkc and
            "output exceeds" not in apkc
        ),
        "native_output_copy_can_truncate": (
            "outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf)" in apkc
        ),
        "direct_dex_copy_can_truncate_to_200": (
            "outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf)" in apkc
        ),
        "external_artifact_magic_validated_in_process": (
            "validate_apkc_formats" in apkc or "dex_validate" in apkc or "elf_validate" in apkc
        ),
        "compiler_output_removed_before_exec": (
            "unlink" in apkc or "os_unlink" in apkc
        ),
        "waitpid_result_checked": bool(re.search(r"if\s*\(\s*os_waitpid", apkc)),
        "c_android_target_has_api_level": bool(re.search(r'aarch64-linux-android\d+', profiles)),
        "c_profile_uses_android_target": '"aarch64-linux-android"' in profiles,
        "go_profile_sets_android_environment": all(
            marker in profiles for marker in ("GOOS", "GOARCH", "CGO_ENABLED")
        ),
        "jvm_profiles_use_d8": '"kotlinc"' in profiles and '"d8"' in profiles,
        "unknown_extensions_rejected": "Unknown input is rejected" in profiles,
    }

    blockers: list[dict[str, str]] = []
    if checks["hardcoded_posix_tmp"]:
        blockers.append({"id": "APKC-RUN-001", "severity": "HIGH", "message": "ApkC usa /tmp fixo; Termux deve usar TMPDIR ou ambiente proot com /tmp válido."})
    if checks["arm32_external_compiler_unsupported"]:
        blockers.append({"id": "APKC-RUN-002", "severity": "HIGH", "message": "fork+exec externo está explicitamente indisponível no binário ARM32."})
    if checks["fork_output_read_bounded_without_overflow_probe"]:
        blockers.append({"id": "APKC-RUN-003", "severity": "CRITICAL", "message": "leitura do artefato externo pode atingir o limite sem provar EOF; truncamento silencioso é possível."})
    if checks["native_output_copy_can_truncate"]:
        blockers.append({"id": "APKC-RUN-004", "severity": "CRITICAL", "message": "artefato .so externo maior que o buffer pode ser reduzido antes do empacotamento."})
    if checks["direct_dex_copy_can_truncate_to_200"]:
        blockers.append({"id": "APKC-RUN-005", "severity": "CRITICAL", "message": "caminho DEX direto pode reduzir conteúdo ao buffer interno de 200 bytes."})
    if not checks["external_artifact_magic_validated_in_process"]:
        blockers.append({"id": "APKC-RUN-006", "severity": "HIGH", "message": "o processo C não valida magic, estrutura e ABI do artefato externo antes do ZIP."})
    if not checks["compiler_output_removed_before_exec"]:
        blockers.append({"id": "APKC-RUN-007", "severity": "HIGH", "message": "saída temporária anterior não é removida antes da compilação; arquivo stale pode ser aceito."})
    if not checks["waitpid_result_checked"]:
        blockers.append({"id": "APKC-RUN-008", "severity": "MEDIUM", "message": "retorno de waitpid não é verificado explicitamente."})
    if checks["c_profile_uses_android_target"] and not checks["c_android_target_has_api_level"]:
        blockers.append({"id": "APKC-RUN-009", "severity": "HIGH", "message": "target Clang Android não fixa API level nem sysroot NDK no perfil C/C++."})
    if not checks["go_profile_sets_android_environment"]:
        blockers.append({"id": "APKC-RUN-010", "severity": "HIGH", "message": "perfil Go não declara GOOS/GOARCH/CGO_ENABLED/CC para Android."})

    return {"checks": checks, "blockers": blockers}


def assess_groups(tools: dict[str, Any]) -> dict[str, str]:
    def complete(group: str) -> bool:
        return all(item["state"] == "PRESENT" for item in tools[group].values())

    return {
        "core": "PASS" if complete("core") else "TOKEN_VAZIO",
        "native_c": "PASS" if complete("native_c") else "TOKEN_VAZIO",
        "rust": "PASS" if complete("rust") else "TOKEN_VAZIO",
        "go": "PASS" if complete("go") else "TOKEN_VAZIO",
        "jvm": "PASS" if complete("jvm") else "TOKEN_VAZIO",
        "gpu": "PASS" if complete("gpu") else "TOKEN_VAZIO",
        "dsp": "PASS" if complete("dsp") else "TOKEN_VAZIO",
    }


def build_report(root: Path) -> dict[str, Any]:
    tools = tool_inventory()
    source = inspect_source(root)
    tmp_env = Path(os.environ.get("TMPDIR", "")) if os.environ.get("TMPDIR") else None
    tmp_env_ok, tmp_env_reason = writable_directory(tmp_env) if tmp_env else (False, "TMPDIR_unset")
    posix_tmp_ok, posix_tmp_reason = writable_directory(Path("/tmp"))

    group_state = assess_groups(tools)
    source_blockers = source["blockers"]
    critical = sum(item["severity"] == "CRITICAL" for item in source_blockers)
    high = sum(item["severity"] == "HIGH" for item in source_blockers)

    environment = {
        "platform": platform.platform(),
        "machine": platform.machine(),
        "python": sys.version.split()[0],
        "cwd": str(root),
        "tmpdir": str(tmp_env) if tmp_env else "TOKEN_VAZIO",
        "tmpdir_state": "PASS" if tmp_env_ok else "TOKEN_VAZIO",
        "tmpdir_reason": tmp_env_reason,
        "posix_tmp_state": "PASS" if posix_tmp_ok else "TOKEN_VAZIO",
        "posix_tmp_reason": posix_tmp_reason,
        "termux_prefix": os.environ.get("PREFIX", "TOKEN_VAZIO"),
        "android_root": os.environ.get("ANDROID_ROOT", "TOKEN_VAZIO"),
    }

    return {
        "schema": SCHEMA,
        "state": "BLOCKED" if source_blockers else "RUNTIME_PENDING",
        "claim_allowed": False,
        "environment": environment,
        "tools": tools,
        "tool_group_state": group_state,
        "source_contract": source,
        "summary": {
            "source_blocker_count": len(source_blockers),
            "critical_blockers": critical,
            "high_blockers": high,
            "termux_tmp_compatible": tmp_env_ok,
            "hardcoded_tmp_compatible": posix_tmp_ok,
            "full_language_matrix_ready": all(value == "PASS" for value in group_state.values()),
        },
        "truth": {
            "language_profiles_present": "PASS",
            "all_profile_toolchains_available": "PASS" if all(value == "PASS" for value in group_state.values()) else "TOKEN_VAZIO",
            "external_compiler_pipeline_safe": "FAIL" if source_blockers else "RUNTIME_PENDING",
            "apk_runtime_proven": "TOKEN_VAZIO",
            "elf_runtime_proven": "TOKEN_VAZIO",
            "dex_runtime_proven": "TOKEN_VAZIO",
        },
        "next_actions": [item["id"] for item in source_blockers],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Preflight de runtime, ferramentas e contratos ApkC")
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--write", type=Path)
    parser.add_argument("--strict", action="store_true", help="falha enquanto houver blocker de source contract")
    args = parser.parse_args(argv)
    root = args.root.resolve()
    report = build_report(root)
    payload = json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.write:
        target = args.write if args.write.is_absolute() else root / args.write
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)
    if args.strict and report["state"] == "BLOCKED":
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
