#!/usr/bin/env python3
"""Validate RafCI authorial-client boundaries using Python stdlib only.

This verifier validates the control contract. It does not replace, copy, certify,
or relicense Android SDK/NDK, Gradle, AGP, CMake, R8, JDK, Kotlin, or provider
implementations, and it does not prove runtime execution.

Gap routing is explicit: structural/operational unknowns bind to
CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY and runtime/device unknowns bind to
CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE. Closure linkage never promotes a gap.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
SPEC_PATH = ROOT / "rafci" / "authorial_clients.v1.json"

REQUIRED_CLIENTS = {
    "c",
    "java",
    "kotlin",
    "python",
    "gradle",
    "android_gradle_plugin",
    "android_sdk",
    "android_ndk",
    "cmake",
    "jni",
    "r8",
    "assemble",
}

REQUIRED_INVARIANTS = {
    "SOURCE!=ARTIFACT!=EXECUTION!=EVIDENCE!=CLAIM",
    "TOKEN_VAZIO!=0",
    "IMPLEMENTED_UNTESTED!=PASS",
    "FACTORY_TOOL!=RUNTIME_DEPENDENCY",
    "PROVIDER_CONTROL!=UPSTREAM_AUTHORSHIP",
    "AUTHORIAL_CLIENT!=REIMPLEMENTED_PROVIDER",
    "LICENSE_NOTICE_PRESERVED_ON_EXISTING_OR_IMPORTED_MATERIAL",
}

REQUIRED_CLOSURES = {
    "CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY",
    "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
}

FACTORY_CLIENTS = {
    "gradle",
    "android_gradle_plugin",
    "android_sdk",
    "android_ndk",
    "cmake",
    "r8",
    "assemble",
}


def load_spec(path: Path = SPEC_PATH) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def canonical_digest(data: dict[str, Any]) -> str:
    payload = json.dumps(data, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def validate(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []

    if data.get("schema") != "rafaelia.rafci.authorial-clients/v1":
        errors.append("schema")
    if data.get("authority") != "authority.rafpolimata.rafci":
        errors.append("authority")
    if data.get("claim_allowed") is not False:
        errors.append("claim_allowed")

    closure_routes = set(data.get("closure_routes", []))
    if not REQUIRED_CLOSURES.issubset(closure_routes):
        errors.append("closure_routes")

    invariants = set(data.get("invariants", []))
    missing_invariants = sorted(REQUIRED_INVARIANTS - invariants)
    if missing_invariants:
        errors.append("missing_invariants:" + ",".join(missing_invariants))

    licensing = data.get("licensing_policy", {})
    if licensing.get("blanket_relicense_allowed") is not False:
        errors.append("blanket_relicense_allowed")
    if licensing.get("existing_path_notice_is_authoritative") is not True:
        errors.append("existing_path_notice_is_authoritative")
    if licensing.get("third_party_notice_removal_allowed") is not False:
        errors.append("third_party_notice_removal_allowed")
    if licensing.get("fork_control_implies_authorship") is not False:
        errors.append("fork_control_implies_authorship")
    clean_room = set(licensing.get("clean_room_replacement_requires", []))
    required_clean_room = {
        "behavioral_or_format_specification",
        "independent_implementation_record",
        "source_provenance",
        "tests_or_falsifiers",
        "file_level_license_decision",
    }
    if not required_clean_room.issubset(clean_room):
        errors.append("clean_room_requirements")

    clients = data.get("clients")
    if not isinstance(clients, list):
        return errors + ["clients"]

    ids: list[str] = []
    for client in clients:
        cid = client.get("id")
        if not isinstance(cid, str) or not cid:
            errors.append("client_id")
            continue
        ids.append(cid)
        for field in (
            "surface",
            "client_role",
            "provider_role",
            "runtime_dependency",
            "freestanding_eligible",
            "replacement_strategy",
            "claim_ceiling",
        ):
            if field not in client:
                errors.append(f"{cid}:missing:{field}")

        if cid in FACTORY_CLIENTS and client.get("client_role") == "REIMPLEMENTED_PROVIDER":
            errors.append(f"{cid}:provider_rebranding")
        if cid in FACTORY_CLIENTS and client.get("claim_ceiling") == "FREESTANDING_STRUCTURAL":
            errors.append(f"{cid}:factory_promoted_to_freestanding")
        if cid == "r8" and "OFF_BY_DEFAULT" not in str(client.get("replacement_strategy")):
            errors.append("r8:not_off_by_default")
        if cid == "jni" and client.get("freestanding_eligible") is not False:
            errors.append("jni:freestanding")
        if cid == "python" and client.get("runtime_dependency") is not False:
            errors.append("python:runtime_dependency")
        if cid == "c" and client.get("freestanding_eligible") is not True:
            errors.append("c:freestanding")

    if len(ids) != len(set(ids)):
        errors.append("duplicate_client_ids")

    missing_clients = sorted(REQUIRED_CLIENTS - set(ids))
    extra_required_gap = sorted(set(ids) & {"sdk", "ndk"})
    if missing_clients:
        errors.append("missing_clients:" + ",".join(missing_clients))
    if extra_required_gap:
        errors.append("ambiguous_alias_ids:" + ",".join(extra_required_gap))

    policy = data.get("consumer_policy", {})
    if policy.get("third_party_runtime_dependency_default") != "DENY":
        errors.append("consumer_policy:third_party_runtime")
    if policy.get("r8_default") != "OFF_UNTIL_SEPARATE_SEMANTIC_GATE":
        errors.append("consumer_policy:r8")
    if policy.get("jni_default") != "ONE_NARROW_CANONICAL_EDGE":
        errors.append("consumer_policy:jni")

    for consumer in data.get("known_consumers", []):
        if consumer.get("state") != "POINTER_ONLY_NO_BODY_COPY":
            errors.append("consumer_body_copy")
        if "RETAINS_IMPLEMENTATION_AND_TEST_AUTHORITY" not in str(consumer.get("authority")):
            errors.append("consumer_authority_transfer")

    return errors


def run_selftest(spec: dict[str, Any]) -> None:
    assert validate(spec) == [], validate(spec)

    mutated = copy.deepcopy(spec)
    mutated["claim_allowed"] = True
    assert "claim_allowed" in validate(mutated)

    mutated = copy.deepcopy(spec)
    mutated["closure_routes"] = []
    assert "closure_routes" in validate(mutated)

    mutated = copy.deepcopy(spec)
    mutated["licensing_policy"]["third_party_notice_removal_allowed"] = True
    assert "third_party_notice_removal_allowed" in validate(mutated)

    mutated = copy.deepcopy(spec)
    mutated["clients"] = [c for c in mutated["clients"] if c["id"] != "android_ndk"]
    assert any(e.startswith("missing_clients:") for e in validate(mutated))

    mutated = copy.deepcopy(spec)
    for client in mutated["clients"]:
        if client["id"] == "r8":
            client["replacement_strategy"] = "ALWAYS_ON"
    assert "r8:not_off_by_default" in validate(mutated)

    mutated = copy.deepcopy(spec)
    for client in mutated["clients"]:
        if client["id"] == "jni":
            client["freestanding_eligible"] = True
    assert "jni:freestanding" in validate(mutated)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--spec", type=Path, default=SPEC_PATH)
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()

    spec = load_spec(args.spec)
    errors = validate(spec)
    if errors:
        print(json.dumps({"state": "FAIL", "errors": errors}, indent=2, sort_keys=True))
        return 1

    if args.selftest:
        run_selftest(spec)

    receipt = {
        "schema": "rafaelia.rafci.authorial-clients-receipt/v1",
        "state": "PASS_CONTRACT_ONLY",
        "spec_sha256": canonical_digest(spec),
        "client_count": len(spec["clients"]),
        "client_ids": sorted(c["id"] for c in spec["clients"]),
        "closure_routes": sorted(spec["closure_routes"]),
        "stdlib_only_validator": True,
        "provider_body_copied": False,
        "provider_relicensed": False,
        "runtime_execution": "TOKEN_VAZIO",
        "physical_execution": "TOKEN_VAZIO",
        "claim_allowed": False,
    }

    if args.receipt:
        args.receipt.parent.mkdir(parents=True, exist_ok=True)
        args.receipt.write_text(
            json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
    print(json.dumps(receipt, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
