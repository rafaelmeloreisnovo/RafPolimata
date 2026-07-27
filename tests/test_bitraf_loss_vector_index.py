#!/usr/bin/env python3
import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "bitraf_loss_vector_index.py"
FIXTURE = ROOT / "tests" / "fixtures" / "bitraf_loss_observations.v1.jsonl"
QUERY = ROOT / "tests" / "fixtures" / "bitraf_loss_query.v1.json"

spec = importlib.util.spec_from_file_location("bitraf_loss_vector_index", SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class BitrafLossVectorTests(unittest.TestCase):
    def rows(self):
        return [
            json.loads(line)
            for line in FIXTURE.read_text(encoding="utf-8").splitlines()
            if line.strip()
        ]

    def test_classification_distinguishes_zero_erasure_and_void(self):
        rows = self.rows()
        classes = {
            row["observation_id"]: module.normalize_observation(row)["class"]
            for row in rows
        }
        self.assertEqual(classes["obs-001"], "MATCH")
        self.assertEqual(classes["obs-003"], "FLIP_0_TO_1")
        self.assertEqual(classes["obs-004"], "FLIP_1_TO_0")
        self.assertEqual(classes["obs-005"], "ERASURE")
        self.assertEqual(classes["obs-006"], "TOKEN_VAZIO_OBSERVED")
        self.assertEqual(classes["obs-007"], "TOKEN_VAZIO_EXPECTED")

    def test_index_is_deterministic(self):
        first = module.build_index(self.rows())
        second = module.build_index(reversed(self.rows()))
        self.assertEqual(first["index_sha3_256"], second["index_sha3_256"])
        self.assertEqual(first["vector_dim"], 32)
        self.assertEqual(
            [r["observation"]["observation_id"] for r in first["records"]],
            sorted(
                r["observation"]["observation_id"]
                for r in first["records"]
            ),
        )

    def test_audit_rates(self):
        index = module.build_index(self.rows())
        audit = module.audit_index(index)
        self.assertEqual(audit["record_count"], 8)
        self.assertAlmostEqual(audit["bit_flip_rate_on_comparable"], 0.4)
        self.assertAlmostEqual(audit["erasure_rate_on_all_records"], 0.125)
        self.assertIn("physical_cause", audit["TOKEN_VAZIO"])

    def test_query_is_explicitly_heuristic(self):
        index = module.build_index(self.rows())
        query = json.loads(QUERY.read_text(encoding="utf-8"))
        result = module.query_index(index, query, top_k=3)
        self.assertEqual(result["recovery_status"], "HEURISTIC_ONLY_NOT_ECC")
        self.assertFalse(result["claim_allowed"])
        self.assertEqual(len(result["neighbours"]), 3)


if __name__ == "__main__":
    unittest.main()
