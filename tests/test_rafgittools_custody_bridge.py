#!/usr/bin/env python3
# CLOSURE_L9 governs explicit TOKEN_VAZIO markers in this bridge test fixture.
from pathlib import Path
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
assert validator.validate(envelope, contract) == []

bad = dict(envelope)
bad["claimAllowed"] = True
assert "claimAllowed must remain false" in validator.validate(bad, contract)

bad_secret = dict(envelope)
bad_secret["capabilityLabels"] = ["github_pat_example"]
assert validator.validate(bad_secret, contract)

print("PASS rafgittools-custody-consumer-v1")
