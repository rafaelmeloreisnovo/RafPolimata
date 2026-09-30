# Governance anchor: CLOSURE_L2.
import importlib.util
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "evidence_garden.py"
spec = importlib.util.spec_from_file_location("evidence_garden", SCRIPT)
eg = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(eg)


class EvidenceGardenTests(unittest.TestCase):
    def test_demo_experiment_runs_and_preserves_claim_boundary(self):
        config_path = ROOT / "Benchmark/evidence_garden/demo_experiment.json"
        cfg, config_sha = eg.load_config(config_path)
        with tempfile.TemporaryDirectory(dir=ROOT) as tmp:
            out = pathlib.Path(tmp) / "receipt.json"
            receipt = eg.run_experiment(cfg, config_sha, out)
            self.assertEqual(receipt["run_state"], "PASS")
            self.assertFalse(receipt["claim_allowed"])
            self.assertEqual(receipt["stations"]["S1_correctness"]["cross_variant_stdout_equivalence"]["state"], "PASS")
            self.assertEqual(receipt["stations"]["S3_performance"]["state"], "PASS")
            self.assertEqual(receipt["stations"]["S3_performance"]["statistics"]["reference"]["execution_success_fraction"], 1.0)
            self.assertIn("alternate", receipt["stations"]["S3_performance"]["comparisons_to_baseline"])
            self.assertEqual(receipt["stations"]["S6_reproduction"]["state"], "PENDING")
            self.assertTrue(out.is_file())

    def test_compare_requires_two_receipts(self):
        summary = eg.compare_receipts([])
        self.assertEqual(summary["state"], eg.TOKEN_VAZIO)

    def test_same_receipt_twice_is_bounded_reproduction_pass(self):
        config_path = ROOT / "Benchmark/evidence_garden/demo_experiment.json"
        cfg, config_sha = eg.load_config(config_path)
        with tempfile.TemporaryDirectory(dir=ROOT) as tmp:
            receipt = eg.run_experiment(cfg, config_sha, pathlib.Path(tmp) / "receipt.json")
            summary = eg.compare_receipts([receipt, json.loads(json.dumps(receipt))])
            self.assertEqual(summary["state"], "PASS")
            self.assertFalse(summary["claim_allowed"])
            self.assertEqual(summary["performance"]["state"], "OBSERVED_UNPROMOTED")

    def test_path_escape_rejected(self):
        with self.assertRaises(ValueError):
            eg.safe_repo_path("../outside")

    def test_distribution_free_interval_is_explicitly_unavailable_when_too_small(self):
        interval = eg.median_ci_nonparametric([1,2,3,4,5], 0.95)
        self.assertEqual(interval["state"], eg.TOKEN_VAZIO)


if __name__ == "__main__":
    unittest.main()
