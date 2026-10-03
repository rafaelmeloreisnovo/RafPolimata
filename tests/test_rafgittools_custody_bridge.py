#!/usr/bin/env python3
# CLOSURE_L9 governs explicit TOKEN_VAZIO markers in this bridge test fixture.
from pathlib import Path
import copy
import importlib.util
import json

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "validate_rafgittools_custody_bridge.py"
spec = importlib.util.spec_from_file_location("validator", SCRIPT)
validator = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(validator)
contract = json.loads((ROOT / "configs" / "rafgittools-custody-consumer.v1.json").read_text())

envelope = {
    "schemaVersion": "rafgittools.rafpolimata-custody-bridge.v1",
    "bridgeId": "TEST-CLOSURE_L9-001",
    "producer": "rafaelmeloreisnovo/RafGitTools",
    "consumer": "rafaelmeloreisnovo/RafPolimata",
    "sourceRef": "github:RafGitTools@0123456789012345678901234567890123456789",
    "artifactRef": "TOKEN_VAZIO",
    "executionRef": "NOT_RUN",
    "evidenceRefs": [],
    "state": "IMPLEMENTED_UNTESTED",
    "claimAllowed": False,
    "predecessorReceipt": "TOKEN_VAZIO",
    "supersedesReceipt": "TOKEN_VAZIO",
    "capabilityLabels": ["Pat_actions", "Pat_envir"],
    "observedAt": "2026-10-02T00:00:00Z"
}


def expect_error(candidate, fragment: str) -> None:
    errors = validator.validate(candidate, contract)
    assert any(fragment in error for error in errors), (fragment, errors)


assert validator.validate(envelope, contract) == []

bad = copy.deepcopy(envelope)
bad["claimAllowed"] = True
expect_error(bad, "claimAllowed must remain false")

bad = copy.deepcopy(envelope)
bad["schemaVersion"] = "rafgittools.rafpolimata-custody-bridge.v0"
expect_error(bad, "bridge schema mismatch")

bad = copy.deepcopy(envelope)
bad["producer"] = "example/other-producer"
expect_error(bad, "producer authority mismatch")

bad = copy.deepcopy(envelope)
bad["consumer"] = "example/other-consumer"
expect_error(bad, "consumer authority mismatch")

bad = copy.deepcopy(envelope)
bad["state"] = "SUCCESS"
expect_error(bad, "state not accepted")

bad = copy.deepcopy(envelope)
del bad["predecessorReceipt"]
expect_error(bad, "missing fields")

bad = copy.deepcopy(envelope)
bad["unexpected"] = "shadow-field"
expect_error(bad, "unexpected fields")

for field in ("bridgeId", "observedAt", "sourceRef", "artifactRef", "executionRef", "predecessorReceipt", "supersedesReceipt"):
    bad = copy.deepcopy(envelope)
    bad[field] = ""
    expect_error(bad, field)

bad = copy.deepcopy(envelope)
bad["evidenceRefs"] = [""]
expect_error(bad, "evidenceRefs must be a string array")

bad = copy.deepcopy(envelope)
bad["evidenceRefs"] = "TOKEN_VAZIO"
expect_error(bad, "evidenceRefs must be a string array")

bad = copy.deepcopy(envelope)
bad["capabilityLabels"] = ["Pat_actions", "Pat_actions"]
expect_error(bad, "capabilityLabels must be unique")

bad = copy.deepcopy(envelope)
bad["capabilityLabels"] = ["Github_pat_example"]
expect_error(bad, "secret-looking credential material is forbidden")

bad = copy.deepcopy(envelope)
bad["capabilityLabels"] = ["lowercase_invalid"]
expect_error(bad, "invalid capability label")

expect_error([], "envelope must be an object")

print("PASS rafgittools-custody-consumer-v1-falsifier-matrix")
