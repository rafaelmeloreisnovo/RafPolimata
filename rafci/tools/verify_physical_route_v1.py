#!/usr/bin/env python3
"""Validate RafCI physical evidence routing without executing a device.

CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE

This verifier protects evidence-class boundaries. It does not convert readiness,
structural build, VM evidence, or an unexecuted acquisition producer into
physical execution evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
ROUTE = ROOT / "rafci" / "physical_route.v1.json"

EXPECTED = {
    "physical.runtime-doctor-readonly": {
        "producer": "scripts/runtime_doctor_physical_readonly_gate.sh",
        "contract": "configs/runtime-doctor-physical-readonly.v1.json",
        "class": "PHYSICAL_ENVIRONMENT_READINESS",
    },
    "physical.apkc-termux-arm32": {
        "producer": "scripts/apkc_validate_termux_arm32.sh",
        "contract": "docs/receipts/APKC_TERMUX_SEALED_STDIN_PREFLIGHT_20260814.md",
        "class": "PHYSICAL_STRUCTURAL_BUILD_ARM32",
    },
    "runtime.android-federated": {
        "producer": "scripts/compile_android_runtime_evidence.py",
        "contract": "docs/ANDROID_FEDERATED_RUNTIME_EVIDENCE_V1.md",
        "class": "FEDERATED_ANDROID_VM_RUNTIME",
    },
    "physical.rafci-same-artifact": {
        "producer": "rafci/tools/capture_physical_execution_v1.sh",
        "contract": "rafci/tools/verify_physical_execution_v1.py",
        "class": "PHYSICAL_SAME_ARTIFACT_EXECUTION_ACQUISITION",
    },
}


class RouteError(RuntimeError):
    pass


def require(value: bool, message: str) -> None:
    if not value:
        raise RouteError(message)


def load(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    require(isinstance(value, dict), f"object required: {path}")
    return value


def validate(route: dict[str, Any]) -> dict[str, Any]:
    require(route.get("schema") == "rafaelia.rafci.physical-route/v1", "schema drift")
    require(route.get("closure") == "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE", "closure drift")
    require(route.get("claim_allowed") is False, "claim_allowed must remain false")

    sources = route.get("sources")
    require(isinstance(sources, list), "sources must be a list")
    require(len(sources) == len(EXPECTED), "unexpected source count")
    ids = [item.get("id") for item in sources if isinstance(item, dict)]
    require(len(ids) == len(set(ids)), "duplicate physical route id")
    require(set(ids) == set(EXPECTED), "physical route source set drift")

    checked: list[dict[str, Any]] = []
    for item in sources:
        require(isinstance(item, dict), "source entry must be object")
        ident = item["id"]
        expected = EXPECTED[ident]
        require(item.get("producer") == expected["producer"], f"producer drift: {ident}")
        require(item.get("contract") == expected["contract"], f"contract drift: {ident}")
        require(item.get("evidence_class") == expected["class"], f"evidence class drift: {ident}")
        require(item.get("rafci_physical_execution_gate") == "TOKEN_VAZIO", f"false physical promotion: {ident}")
        require(isinstance(item.get("can_establish"), list) and item["can_establish"], f"missing positive scope: {ident}")
        require(isinstance(item.get("cannot_establish"), list) and item["cannot_establish"], f"missing ceiling: {ident}")
        for rel in (item["producer"], item["contract"]):
            path = ROOT / rel
            require(path.is_file(), f"canonical physical dependency missing: {rel}")
        checked.append({
            "id": ident,
            "evidence_class": item["evidence_class"],
            "gate": item["rafci_physical_execution_gate"],
        })

    promotion = route.get("promotion_contract")
    require(isinstance(promotion, dict), "promotion_contract missing")
    require(promotion.get("gate") == "gate.physical-execution", "promotion gate drift")
    require(promotion.get("current_state") == "TOKEN_VAZIO", "physical gate promoted without receipt")
    required = promotion.get("required_for_arm32_or_arm64")
    require(isinstance(required, list) and len(required) >= 8, "physical promotion requirements incomplete")
    joined = "\n".join(str(x).lower() for x in required)
    for needle in ("abi", "artifact sha-256", "executed", "exit/status", "unchanged", "receipt integrity", "source commit"):
        require(needle in joined, f"physical promotion requirement missing: {needle}")

    invariants = route.get("invariants")
    require(isinstance(invariants, list), "invariants missing")
    expected_invariants = {
        "PHYSICAL_READINESS != PHYSICAL_EXECUTION",
        "PHYSICAL_STRUCTURAL_BUILD != PHYSICAL_RUNTIME",
        "VM_RUNTIME != PHYSICAL_DEVICE_RUNTIME",
        "ARTIFACT_HASH != EXECUTION",
        "EXECUTION != CLAIM",
        "IMPLEMENTED_UNTESTED != PASS",
        "TOKEN_VAZIO != PASS",
    }
    require(expected_invariants.issubset(set(invariants)), "physical evidence invariants incomplete")

    canonical = json.dumps(route, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return {
        "schema": "rafaelia.rafci.physical-route-verification/v1",
        "state": "PASS_ROUTE_ONLY",
        "route_sha256": hashlib.sha256(canonical).hexdigest(),
        "checked_sources": checked,
        "physical_execution_gate": "TOKEN_VAZIO",
        "claim_allowed": False,
        "closure": "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
    }


def selftest(route: dict[str, Any]) -> None:
    # Falsifier 1: a readiness source must never become a physical PASS.
    mutated = json.loads(json.dumps(route))
    mutated["sources"][0]["rafci_physical_execution_gate"] = "PASS"
    try:
        validate(mutated)
    except RouteError:
        pass
    else:
        raise RouteError("selftest accepted readiness->physical PASS mutation")

    # Falsifier 2: even an implemented acquisition producer cannot self-promote.
    mutated = json.loads(json.dumps(route))
    for source in mutated["sources"]:
        if source["id"] == "physical.rafci-same-artifact":
            source["rafci_physical_execution_gate"] = "PASS"
    try:
        validate(mutated)
    except RouteError:
        pass
    else:
        raise RouteError("selftest accepted acquisition-implemented->physical PASS mutation")

    # Falsifier 3: removing artifact identity from promotion requirements must fail.
    mutated = json.loads(json.dumps(route))
    mutated["promotion_contract"]["required_for_arm32_or_arm64"] = [
        value for value in mutated["promotion_contract"]["required_for_arm32_or_arm64"]
        if "artifact SHA-256" not in value
    ]
    try:
        validate(mutated)
    except RouteError:
        pass
    else:
        raise RouteError("selftest accepted promotion contract without artifact identity")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()

    route = load(ROUTE)
    if args.selftest:
        selftest(route)
    receipt = validate(route)
    rendered = json.dumps(receipt, indent=2, sort_keys=True) + "\n"
    if args.receipt:
        args.receipt.parent.mkdir(parents=True, exist_ok=True)
        args.receipt.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, json.JSONDecodeError, KeyError, RouteError) as exc:
        print(f"RAFCI_PHYSICAL_ROUTE_FAIL: {exc}")
        raise SystemExit(1)
