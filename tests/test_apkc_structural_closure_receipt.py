from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile

from tests.test_validate_apkc_formats import make_dex, make_elf32, make_elf64

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/apkc_structural_closure_receipt.py"
COMMIT = "a" * 40
DIGEST = "b" * 64


def source_proof(commit: str = COMMIT) -> dict:
    return {
        "schema": "raf.apkc.source-to-binary-proof.v2",
        "commit": commit,
        "aarch64": {
            "build": "PASS",
            "identity": "PASS",
            "reproducibility": "PASS",
            "sha256": DIGEST,
        },
        "arm32": {
            "build": "PASS",
            "identity": "PASS",
            "reproducibility": "PASS",
            "sha256": DIGEST,
        },
        "apk_payload_runtime": "TOKEN_VAZIO",
        "state": "PASS",
        "claim_allowed": False,
    }


class ApkCStructuralClosureReceiptTest(unittest.TestCase):
    def make_inputs(self, root: Path, *, both_abis: bool = True, commit: str = COMMIT):
        apk = root / "candidate.apk"
        with zipfile.ZipFile(apk, "w", compression=zipfile.ZIP_STORED) as archive:
            archive.writestr("classes.dex", make_dex())
            archive.writestr("lib/arm64-v8a/libmain.so", make_elf64())
            if both_abis:
                archive.writestr("lib/armeabi-v7a/libmain.so", make_elf32())

        proof = root / "source-proof.json"
        proof.write_text(json.dumps(source_proof(commit)), encoding="utf-8")
        gate = root / "first-part.json"
        gate.write_text(
            json.dumps({"schema": "fixture.first-part", "state": "PASS", "claim_allowed": False}),
            encoding="utf-8",
        )
        preflight = root / "preflight.json"
        preflight.write_text(
            json.dumps({"schema": "fixture.preflight", "state": "PASS", "claim_allowed": False}),
            encoding="utf-8",
        )
        return apk, proof, gate, preflight

    def run_receipt(self, root: Path, apk: Path, proof: Path, gate: Path, preflight: Path):
        output = root / "receipt.json"
        completed = subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--apk",
                str(apk),
                "--source-proof",
                str(proof),
                "--first-part-gate",
                str(gate),
                "--runtime-preflight",
                str(preflight),
                "--source-commit",
                COMMIT,
                "--output",
                str(output),
            ],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        return completed, json.loads(output.read_text(encoding="utf-8"))

    def test_dual_abi_structural_receipt_passes_scoped_only(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = self.make_inputs(root)
            completed, report = self.run_receipt(root, *inputs)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(report["state"], "PASS_STRUCTURAL")
        self.assertTrue(report["structural_claim_allowed"])
        self.assertFalse(report["claim_allowed"])
        self.assertEqual(report["runtime_boundary"]["apk_installed"], "TOKEN_VAZIO")

    def test_missing_apk_is_incomplete(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            apk, proof, gate, preflight = self.make_inputs(root)
            apk.unlink()
            completed, report = self.run_receipt(root, apk, proof, gate, preflight)
        self.assertEqual(completed.returncode, 1)
        self.assertEqual(report["state"], "INCOMPLETE")
        self.assertFalse(report["structural_claim_allowed"])

    def test_source_commit_mismatch_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = self.make_inputs(root, commit="c" * 40)
            completed, report = self.run_receipt(root, *inputs)
        self.assertEqual(completed.returncode, 1)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("commit mismatch" in error for error in report["errors"]))

    def test_missing_arm32_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs = self.make_inputs(root, both_abis=False)
            completed, report = self.run_receipt(root, *inputs)
        self.assertEqual(completed.returncode, 1)
        self.assertEqual(report["state"], "FAIL")
        self.assertFalse(report["checks"]["both_abis"])


if __name__ == "__main__":
    unittest.main()
