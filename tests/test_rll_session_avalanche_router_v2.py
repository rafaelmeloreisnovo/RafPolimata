import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).parents[1]
SCRIPT = ROOT / "scripts/rll_session_avalanche_router.py"
CONFIG = ROOT / "configs/rll-session-avalanche-route-v2.json"
spec = importlib.util.spec_from_file_location("router", SCRIPT)
m = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(m)


class TestRLLSessionAvalancheRouterV2(unittest.TestCase):
    def setUp(self):
        self.config = json.loads(CONFIG.read_text())

    def test_config_valid(self):
        m.validate_config(self.config)

    def test_new_physics_extension_present(self):
        sources = self.config["source_artifacts"]
        self.assertIn("compression_radiation_module", sources)
        self.assertEqual(
            sources["semantic_capsule"],
            "configs/session-single-subtokenization-v2.json",
        )

    def test_required_microphysics_gates(self):
        gates = {item["id"]: item for item in self.config["gates"]}
        for gate in ("opacity_model", "particle_distribution", "process_cross_sections"):
            self.assertTrue(gates[gate]["required"])
            self.assertEqual(gates[gate]["failure_state"], "TOKEN_VAZIO")

    def test_scalar_plan_remains_finite(self):
        plan = m.compile_plan(self.config, set())
        self.assertEqual(plan["backend_count"], 1)
        self.assertEqual(plan["job_count"], 24)
        self.assertFalse(plan["claim_allowed"])

    def test_all_backends_respect_cap(self):
        plan = m.compile_plan(self.config, {"simd", "gpu", "dsp", "npu"})
        self.assertLessEqual(plan["job_count"], self.config["scale_policy"]["max_jobs"])

    def test_npu_fully_ionized_pruned(self):
        plan = m.compile_plan(self.config, {"npu"})
        self.assertFalse(any(
            job["backend"] == "npu" and job["thermal_regime"] == "fully_ionized"
            for job in plan["jobs"]
        ))

    def test_no_subparticle_promotion(self):
        semantics = self.config["relativistic_compression_semantics"]
        self.assertFalse(semantics["compression_automatically_creates_subparticles"])
        self.assertFalse(semantics["threshold_reached_equals_reaction_completed"])

    def test_no_automatic_write_or_merge(self):
        self.assertFalse(self.config["automatic_cross_repo_write"])
        self.assertFalse(self.config["automatic_merge"])


if __name__ == "__main__":
    unittest.main()
