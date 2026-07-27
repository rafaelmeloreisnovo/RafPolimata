#!/usr/bin/env python3
"""Emit a deterministic M063 compilation plan; `make compile` executes it."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import tempfile

SCHEMA = "rafaelia.strict.compile.plan.v2"
ROOT = Path(__file__).resolve().parent.parent
MATRIX = ROOT / "ci" / "contracts" / "rafaelia_language_completion_v1.tsv"
EXECUTOR = "scripts/apkc_strict_native_build.sh"

ARCH_FLAGS = {
    "arm32": ["-march=armv7-a", "-mfloat-abi=softfp", "-mfpu=neon-vfpv4"],
    "arm64": ["-march=armv8-a+simd"],
    "x86_64": ["-march=x86-64"],
    "rv64": ["-march=rv64gc", "-mabi=lp64d"],
}

C_COMMON = [
    "-std=c11", "-Os", "-ffreestanding", "-fno-builtin",
    "-fno-stack-protector", "-fno-ident", "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables", "-fno-optimize-sibling-calls",
    "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections",
    "-Wall", "-Wextra", "-Werror", "-Wshadow",
]

CPP_EXTRA = [
    "-std=c++17", "-fno-exceptions", "-fno-rtti",
    "-fno-threadsafe-statics", "-fno-use-cxa-atexit",
]

LINK_COMMON = [
    "-nostdlib", "-nostartfiles", "-nodefaultlibs", "-Wl,--no-undefined",
    "-Wl,--gc-sections", "-Wl,--strip-all", "-Wl,--build-id=none",
]


def load_matrix(path: Path = MATRIX) -> dict[str, dict[str, str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise ValueError("empty language matrix")
    header = lines[0].split("\t")
    rows: dict[str, dict[str, str]] = {}
    for line in lines[1:]:
        if not line:
            continue
        values = line.split("\t")
        if len(values) != len(header):
            raise ValueError(f"invalid TSV row: {line}")
        row = dict(zip(header, values, strict=True))
        language = row["language"]
        if language in rows:
            raise ValueError(f"duplicate language: {language}")
        rows[language] = row
    if len(rows) != 23:
        raise ValueError(f"expected 23 languages, found {len(rows)}")
    return rows


def compile_plan(language: str, arch: str, source: str, output: str) -> dict[str, object]:
    rows = load_matrix()
    language = language.lower()
    if language not in rows:
        raise ValueError(f"unknown language: {language}")
    if arch not in ARCH_FLAGS:
        raise ValueError(f"unknown arch: {arch}")

    row = rows[language]
    route = row["strict_route"]
    final_class = row["final_class"]
    compile_flags: list[str] = []
    link_flags: list[str] = []
    status = final_class
    executor: str | None = None
    source_contract = "native_subset"

    if language == "c":
        executor = EXECUTOR
        compile_flags = C_COMMON + ARCH_FLAGS[arch]
        link_flags = LINK_COMMON
        status = "STRICT_NATIVE_EXECUTABLE"
    elif language == "cpp":
        executor = EXECUTOR
        compile_flags = [flag for flag in C_COMMON if flag != "-std=c11"] + CPP_EXTRA + ARCH_FLAGS[arch]
        link_flags = LINK_COMMON
        status = "STRICT_NATIVE_CONDITIONAL"
    elif language == "asm":
        executor = EXECUTOR
        status = "DIRECT_ISA_EXECUTABLE"
    elif route == "LOWER_TO_RAF_IR":
        executor = EXECUTOR
        status = "PORTABLE_KERNEL_EXECUTABLE"
        source_contract = "exactly_one_RAF_KERNEL_annotation"
    elif language == "rs":
        executor = EXECUTOR
        status = "PORTABLE_KERNEL_EXECUTABLE"
        source_contract = "RAF_KERNEL_or_future_no_std_direct_route"
    elif route == "DEVICE_KERNEL":
        status = "DEVICE_KERNEL_ONLY"
        source_contract = "device_toolchain_and_runtime_gate"
    elif route == "MODEL_TO_INTERNAL_KERNELS":
        status = "DATA_ONLY_LOWERING_REQUIRED"
        source_contract = "model_operator_inventory_required"

    command = [executor, language, arch, output, source] if executor else []
    return {
        "schema": SCHEMA,
        "language": language,
        "architecture": arch,
        "source": source,
        "output": output,
        "route": route,
        "final_class": final_class,
        "status": status,
        "executor": executor,
        "execution_command": command,
        "compile_flags": compile_flags,
        "link_flags": link_flags,
        "source_contract": source_contract,
        "required_final_gates": [
            "source_dependent_output",
            "zero_external_runtime",
            "zero_undefined_symbols",
            "zero_unapproved_relocations",
            "no_pt_interp",
            "reproducible_hash",
            "architecture_identity",
        ],
        "execute_plan": False,
        "execute_with": "make compile RAF_LANG=... RAF_ARCH=... SRC=... OUT=...",
        "claim_allowed": False,
    }


def selftest() -> int:
    with tempfile.TemporaryDirectory(prefix="rafaelia-plan-") as tmp:
        target = str(Path(tmp) / "out.so")
        c_plan = compile_plan("c", "arm32", "kernel.c", target)
        assert c_plan["status"] == "STRICT_NATIVE_EXECUTABLE"
        assert c_plan["executor"] == EXECUTOR
        assert "-mfpu=neon-vfpv4" in c_plan["compile_flags"]
        assert "-fno-optimize-sibling-calls" in c_plan["compile_flags"]

        py_plan = compile_plan("py", "arm64", "kernel.py", target)
        assert py_plan["status"] == "PORTABLE_KERNEL_EXECUTABLE"
        assert py_plan["source_contract"] == "exactly_one_RAF_KERNEL_annotation"

        gpu_plan = compile_plan("glsl", "arm64", "kernel.comp", target)
        assert gpu_plan["status"] == "DEVICE_KERNEL_ONLY"
        assert gpu_plan["executor"] is None

        model_plan = compile_plan("tflite", "arm64", "model.tflite", target)
        assert model_plan["status"] == "DATA_ONLY_LOWERING_REQUIRED"

        assert len(load_matrix()) == 23
    print("strict compile plan selftest: PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--language")
    parser.add_argument("--arch", choices=sorted(ARCH_FLAGS))
    parser.add_argument("--source")
    parser.add_argument("--output")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()

    if args.selftest:
        return selftest()
    if not all((args.language, args.arch, args.source, args.output)):
        parser.error("--language, --arch, --source and --output are required")

    plan = compile_plan(args.language, args.arch, args.source, args.output)
    print(json.dumps(plan, ensure_ascii=False, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
