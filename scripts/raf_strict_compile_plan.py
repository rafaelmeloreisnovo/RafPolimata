#!/usr/bin/env python3
"""Emit a deterministic M063 compilation plan without executing a compiler."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import tempfile

SCHEMA = "rafaelia.strict.compile.plan.v1"
ROOT = Path(__file__).resolve().parent.parent
MATRIX = ROOT / "ci" / "contracts" / "rafaelia_language_completion_v1.tsv"

ARCH_FLAGS = {
    "arm32": ["-march=armv7-a", "-mfloat-abi=softfp", "-mfpu=neon-vfpv4"],
    "arm64": ["-march=armv8-a+simd"],
    "x86_64": ["-march=x86-64"],
    "rv64": ["-march=rv64gc", "-mabi=lp64d"],
}

C_COMMON = [
    "-std=c11",
    "-Os",
    "-ffreestanding",
    "-fno-builtin",
    "-fno-stack-protector",
    "-fno-ident",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
    "-fno-optimize-sibling-calls",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wshadow",
]

CPP_EXTRA = [
    "-std=c++17",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-threadsafe-statics",
    "-fno-use-cxa-atexit",
]

LINK_COMMON = [
    "-nostdlib",
    "-nostartfiles",
    "-nodefaultlibs",
    "-Wl,--gc-sections",
    "-Wl,--strip-all",
    "-Wl,--build-id=none",
]

RUST_FLAGS = [
    "--crate-type=lib",
    "--emit=obj",
    "-Cpanic=abort",
    "-Copt-level=z",
    "-Ccodegen-units=1",
    "-Cstrip=symbols",
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
    compiler: str | None = None
    compile_flags: list[str] = []
    link_flags: list[str] = []
    status = final_class

    if language == "c":
        compiler = "clang"
        compile_flags = C_COMMON + ARCH_FLAGS[arch]
        link_flags = LINK_COMMON
        status = "STRICT_OBJECT_CANDIDATE"
    elif language == "cpp":
        compiler = "clang++"
        compile_flags = [flag for flag in C_COMMON if flag != "-std=c11"] + CPP_EXTRA + ARCH_FLAGS[arch]
        link_flags = LINK_COMMON
        status = "STRICT_OBJECT_CONDITIONAL"
    elif language == "rs":
        compiler = "rustc"
        compile_flags = RUST_FLAGS.copy()
        status = "NO_STD_OBJECT_CONDITIONAL"
    elif language == "asm":
        compiler = "ApkC_internal_encoder"
        status = "DIRECT_ISA_PLAN"
    elif route == "LOWER_TO_RAF_IR":
        status = "LOWERING_REQUIRED"
    elif route == "DEVICE_KERNEL":
        status = "DEVICE_KERNEL_ONLY"
    elif route == "MODEL_TO_INTERNAL_KERNELS":
        status = "DATA_ONLY_LOWERING_REQUIRED"

    return {
        "schema": SCHEMA,
        "language": language,
        "architecture": arch,
        "source": source,
        "output": output,
        "route": route,
        "final_class": final_class,
        "status": status,
        "compiler": compiler,
        "compile_flags": compile_flags,
        "link_flags": link_flags,
        "required_source_contract": {
            "fixed_width_types": True,
            "caller_owned_or_static_storage": True,
            "no_heap": True,
            "no_gc": True,
            "no_tailcall": True,
            "no_shadow": True,
            "bit_exact_vectors": True,
        },
        "required_final_gates": [
            "zero_external_runtime",
            "zero_undefined_symbols",
            "zero_unapproved_relocations",
            "section_allowlist",
            "reproducible_hash",
            "cycle_measurement_on_target",
        ],
        "execute_plan": False,
        "claim_allowed": False,
    }


def selftest() -> int:
    with tempfile.TemporaryDirectory(prefix="rafaelia-plan-") as tmp:
        target = str(Path(tmp) / "out.o")
        c_plan = compile_plan("c", "arm32", "kernel.c", target)
        assert c_plan["status"] == "STRICT_OBJECT_CANDIDATE"
        assert "-mfpu=neon-vfpv4" in c_plan["compile_flags"]
        assert "-fno-optimize-sibling-calls" in c_plan["compile_flags"]

        py_plan = compile_plan("py", "arm64", "kernel.py", target)
        assert py_plan["status"] == "LOWERING_REQUIRED"
        assert py_plan["compiler"] is None

        gpu_plan = compile_plan("glsl", "arm64", "kernel.comp", target)
        assert gpu_plan["status"] == "DEVICE_KERNEL_ONLY"

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
