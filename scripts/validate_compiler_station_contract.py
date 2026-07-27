#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONTRACT = ROOT / "ci" / "contracts" / "apkc_compiler_station_v2.json"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"compiler-station contract: FAIL — {message}")


def load_contract() -> dict[str, object]:
    data = json.loads(CONTRACT.read_text(encoding="utf-8"))
    require(data.get("schema") == "rafaelia.apkc.compiler-station.v2", "wrong schema")
    require(data.get("claim_allowed") is False, "repository contract must remain CI-pending")
    require(data.get("claim_promotion_gate") == "ALL_BLOCKING_TESTS_PASS", "missing promotion gate")
    return data


def unique_nonempty(items: object, name: str) -> list[str]:
    require(isinstance(items, list) and items, f"{name} must be a nonempty list")
    require(all(isinstance(item, str) and item for item in items), f"{name} has invalid entry")
    values = list(items)
    require(len(values) == len(set(values)), f"{name} contains duplicates")
    return values


def main() -> int:
    data = load_contract()

    routes = data.get("routes")
    require(isinstance(routes, list) and len(routes) == 4, "exactly four canonical routes required")
    route_ids = [route.get("id") for route in routes if isinstance(route, dict)]
    require(route_ids == ["ROOT_U32_IR", "STRICT_C_REWRITE", "HOSTED_RAF_KERNEL", "DIRECT_ASSEMBLY"],
            "route registry/order mismatch")

    hosted = next(route for route in routes if route["id"] == "HOSTED_RAF_KERNEL")
    hosted_languages = unique_nonempty(hosted.get("input_languages"), "hosted languages")
    require(len(hosted_languages) == 14, f"expected 14 hosted annotation routes, found {len(hosted_languages)}")

    emulated_headers = unique_nonempty(data.get("emulated_headers"), "emulated_headers")
    emulated_functions = unique_nonempty(data.get("emulated_functions"), "emulated_functions")
    require("memmove" in emulated_functions and "strtoul" in emulated_functions,
            "critical emulation surfaces absent")
    require("ctype.h" not in emulated_headers, "unsupported ctype.h cannot be advertised")

    gates = unique_nonempty(data.get("blocking_gates"), "blocking_gates")
    for gate in (
        "manifest_signature_recomputation",
        "manifest_tamper_rejection",
        "rollback_removes_stale_artifacts",
        "no_dt_needed",
        "c_and_cpp_export_names_unmangled",
    ):
        require(gate in gates, f"missing blocking gate: {gate}")

    not_claimed = unique_nonempty(data.get("not_claimed"), "not_claimed")
    require("general_purpose_C_or_CPP_compiler" in not_claimed, "general compiler boundary missing")
    require("APK_packaging_signature_installation_or_launch" in not_claimed, "APK runtime boundary missing")

    canonical_files = unique_nonempty(data.get("canonical_files"), "canonical_files")
    for relative in canonical_files:
        require((ROOT / relative).is_file(), f"canonical file missing: {relative}")

    rewriter = (ROOT / "scripts" / "raf_c_rewrite.py").read_text(encoding="utf-8")
    lowerer = (ROOT / "scripts" / "raf_kernel_lower.py").read_text(encoding="utf-8")
    frontend = (ROOT / "raf_frontend.c").read_text(encoding="utf-8")
    libc_emu = (ROOT / "Apkc" / "raf_libc_emu.h").read_text(encoding="utf-8")
    elf_audit = (ROOT / "scripts" / "audit_strict_elf.sh").read_text(encoding="utf-8")
    builder = (ROOT / "scripts" / "apkc_strict_native_build.sh").read_text(encoding="utf-8")

    for header in emulated_headers:
        require(f'"{header}"' in rewriter, f"rewriter does not register header {header}")
    for function in emulated_functions:
        if function == "raf_write":
            require(re.search(r"\braf_write\s*\(", libc_emu) is not None, "raf_write implementation absent")
        else:
            require(re.search(rf"\b{re.escape(function)}\s*\(", libc_emu) is not None,
                    f"libc emulation absent: {function}")

    require('SCHEMA = "rafaelia.c.rewrite.v2"' in rewriter, "rewriter schema mismatch")
    require('SCHEMA = "rafaelia.kernel.lower.v2"' in lowerer, "lowering schema mismatch")
    require("claim_allowed\": False" in rewriter, "rewriter claim gate must be false")
    require("claim_allowed\": False" in lowerer, "lowerer claim gate must be false")
    require("#define RAF_OPS_SCHEMA 4u" in frontend, "ops schema 4 not active")
    require("transaction_state" in frontend and "ir_value" in frontend, "transaction receipt fields absent")
    require("extern \"C\"" in libc_emu, "C++ export de-mangling absent")
    require("--profile exec|android-so" in elf_audit, "ELF audit profiles absent")
    require("STRICT_ELF_PASS" in builder and "runtime_external_dependencies" in builder,
            "strict native receipt boundary absent")

    artifacts = data.get("artifacts")
    require(isinstance(artifacts, list) and len(artifacts) == 6, "artifact registry must contain six types")
    suffixes = [artifact.get("suffix") for artifact in artifacts if isinstance(artifact, dict)]
    require(suffixes == [".s", ".hex", ".bin", ".ops", ".so", ".so.receipt.json"],
            "artifact suffix registry mismatch")
    require(all(artifact.get("transactional") is True for artifact in artifacts),
            "all canonical artifacts must be transactional")

    print(
        "compiler-station contract: PASS — "
        f"routes={len(routes)} hosted={len(hosted_languages)} "
        f"functions={len(emulated_functions)} gates={len(gates)} files={len(canonical_files)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
