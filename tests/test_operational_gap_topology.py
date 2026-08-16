#!/usr/bin/env python3
from __future__ import annotations

import copy
import importlib.util
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "validate_operational_gap_topology", ROOT / "scripts/validate_operational_gap_topology.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)


class OperationalGapTopologyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.data = json.loads((ROOT / "configs/operational-gap-topology.v1.json").read_text(encoding="utf-8"))
        cls.schema = json.loads((ROOT / "schemas/operational-gap-topology.v1.schema.json").read_text(encoding="utf-8"))

    def test_canonical_topology_passes(self) -> None:
        report = MOD.validate(copy.deepcopy(self.data))
        self.assertEqual(report["state"], "PASS", report["errors"])
        self.assertGreaterEqual(report["summary"]["blocking_p0_p1"], 1)

    def test_schema_contract_keeps_claim_and_closure_fields(self) -> None:
        gap_item = self.schema["properties"]["gaps"]["items"]
        required = set(gap_item["required"])
        self.assertTrue({"claim_allowed", "provenance", "next_action", "closure"}.issubset(required))
        edge_required = set(self.schema["properties"]["edges"]["items"]["required"])
        self.assertTrue({"from", "to", "relation", "strength", "rationale"}.issubset(edge_required))

    def test_token_vazio_cannot_allow_claim(self) -> None:
        data = copy.deepcopy(self.data)
        data["gaps"][0]["state"] = "TOKEN_VAZIO"
        data["gaps"][0]["claim_allowed"] = True
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("claim_allowed=false" in item for item in report["errors"]))

    def test_unknown_edge_endpoint_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["edges"][0]["to"] = "GAP-NOT-REAL"
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("unknown to endpoint" in item for item in report["errors"]))

    def test_requires_cycle_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["edges"].append({
            "id": "EDGE-TEST-CYCLE",
            "from": "GAP-TECH-ARM64-APK-L4",
            "to": "GAP-TECH-ANDROID-RUNTIME-L2",
            "relation": "requires",
            "strength": "HARD",
            "rationale": "fixture cycle"
        })
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertIn("requires relation contains a dependency cycle", report["errors"])

    def test_owner_decision_is_fail_closed(self) -> None:
        data = copy.deepcopy(self.data)
        gap = next(gap for gap in data["gaps"] if gap["id"] == "GAP-LEGAL-LICENSE-DECISION")
        gap["owner_decision_required"] = False
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("OWNER_DECISION" in item for item in report["errors"]))

    def test_pass_requires_evidence(self) -> None:
        data = copy.deepcopy(self.data)
        gap = data["gaps"][0]
        gap["state"] = "PASS"
        gap["claim_allowed"] = False
        gap["evidence"] = []
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("PASS requires non-empty evidence" in item for item in report["errors"]))

    def test_p0_p1_requires_normative_reference(self) -> None:
        data = copy.deepcopy(self.data)
        gap = next(gap for gap in data["gaps"] if gap["urgency"] == "P0")
        gap["reference_controls"] = []
        report = MOD.validate(data)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("requires reference_controls" in item for item in report["errors"]))


if __name__ == "__main__":
    unittest.main()
