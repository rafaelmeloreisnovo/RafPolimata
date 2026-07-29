from __future__ import annotations

import importlib.util
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "seven_constructions_lint", ROOT / "scripts" / "seven_constructions_lint.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class SevenConstructionsLintTests(unittest.TestCase):
    def setUp(self) -> None:
        self.contract = ROOT / "data" / "formal" / "seven-formal-constructions-2026-07-28.json"

    def test_contract_has_exactly_seven_ordered_constructions(self) -> None:
        data = MODULE.load_contract(self.contract)
        self.assertEqual([r["id"] for r in data["constructions"]], [f"S7F-{i:02d}" for i in range(1, 8)])

    def test_claim_gate_is_closed(self) -> None:
        data = json.loads(self.contract.read_text(encoding="utf-8"))
        self.assertIs(data["claim_allowed"], False)

    def test_poincare_origin_distance_is_finite_inside_ball(self) -> None:
        for radius in (0.0, 0.25, 0.5, 0.9, 0.999999):
            self.assertTrue(math.isfinite(MODULE.poincare_distance_to_origin(radius)))

    def test_poincare_distance_diverges_toward_boundary(self) -> None:
        self.assertLess(MODULE.poincare_distance_to_origin(0.9), MODULE.poincare_distance_to_origin(0.999999))

    def test_reciprocal_loglog_is_negative_on_unit_ball_samples(self) -> None:
        for radius in (0.1, 0.5, 0.99):
            self.assertLess(MODULE.reciprocal_loglog(radius), 0.0)

    def test_reciprocal_loglog_singularity_is_detected(self) -> None:
        with self.assertRaises(ZeroDivisionError):
            MODULE.reciprocal_loglog(MODULE.SINGULAR_RADIUS)

    def test_blenddigs_gap_is_not_promoted(self) -> None:
        data = MODULE.load_contract(self.contract)
        record = next(r for r in data["constructions"] if r["id"] == "S7F-05")
        self.assertEqual(record["state"], "AUTHORIAL_NOTATION_UNDERDEFINED")
        self.assertIn("oplus", record["gaps"])
        self.assertIn("otimes", record["gaps"])

    def test_action_coupling_gap_is_not_promoted(self) -> None:
        data = MODULE.load_contract(self.contract)
        record = next(r for r in data["constructions"] if r["id"] == "S7F-07")
        self.assertEqual(record["state"], "ACTION_INCOMPLETE_REQUIRES_VARIATIONAL_COUPLING")
        self.assertIn("attention_functional", record["gaps"])

    def test_full_audit_passes_without_claim_promotion(self) -> None:
        receipt = MODULE.audit(self.contract)
        self.assertEqual(receipt["state"], "LOCAL_PASS")
        self.assertIs(receipt["claim_allowed"], False)
        self.assertTrue(all(receipt["checks"].values()))


if __name__ == "__main__":
    unittest.main()
