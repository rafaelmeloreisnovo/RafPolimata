import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "scripts" / "compile_android_runtime_evidence.py"
SPEC = importlib.util.spec_from_file_location("compile_android_runtime_evidence", PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC and SPEC.loader
SPEC.loader.exec_module(MODULE)


class AndroidRuntimeEvidenceTest(unittest.TestCase):
    def fixture(self):
        return json.loads(
            (ROOT / "tests/fixtures/android-runtime-receipt.dispatched.json").read_text()
        )

    def test_dispatch_is_only_partial(self):
        evidence = MODULE.compile_evidence(self.fixture())
        self.assertEqual("PARTIAL", evidence["evidence_state"])
        self.assertFalse(evidence["claim_allowed"])

    def test_exit_zero_without_boot_is_tested_not_verified(self):
        receipt = self.fixture()
        receipt["execution_exit_code"] = 0
        evidence = MODULE.compile_evidence(receipt)
        self.assertEqual("TESTED", evidence["evidence_state"])

    def test_boot_hash_promotes_only_to_verified_limited(self):
        receipt = self.fixture()
        receipt["execution_exit_code"] = 0
        receipt["guest_boot_artifact_sha256"] = "a" * 64
        evidence = MODULE.compile_evidence(receipt)
        self.assertEqual("VERIFIED_LIMITED", evidence["evidence_state"])

    def test_private_path_exposure_is_blocked(self):
        receipt = self.fixture()
        receipt["private_paths_exposed"] = True
        with self.assertRaises(MODULE.EvidenceError):
            MODULE.compile_evidence(receipt)

    def test_missing_output_hash_is_blocked(self):
        receipt = self.fixture()
        receipt.pop("output_sha256")
        with self.assertRaises(MODULE.EvidenceError):
            MODULE.compile_evidence(receipt)


if __name__ == "__main__":
    unittest.main()
