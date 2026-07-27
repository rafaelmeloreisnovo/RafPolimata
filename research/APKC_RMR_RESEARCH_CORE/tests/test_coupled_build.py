"""Adversarial tests for the coupled build contract.

Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
Contract-Role: COUPLING_ADVERSARIAL_TESTS
License-Role: RESEARCH_NONCOMMERCIAL_ONLY
Normative comment: a one-bit mutation in any sealed artifact must fail.
"""
from __future__ import annotations

import importlib.util
import shutil
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "coupled_build", ROOT / "tools" / "coupled_build.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class CoupledBuildTests(unittest.TestCase):
    def test_current_tree_is_sealed(self) -> None:
        receipt = MODULE.verify(ROOT)
        self.assertEqual(receipt["state"], "PASS_COUPLED", receipt["errors"])

    def test_document_bit_mutation_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            copied = Path(tmp) / "core"
            shutil.copytree(ROOT, copied)
            target = copied / "licenses" / "COMMERCIAL_LICENSING_POLICY.md"
            data = bytearray(target.read_bytes())
            data[0] ^= 1
            target.write_bytes(bytes(data))
            receipt = MODULE.verify(copied)
            self.assertEqual(receipt["state"], "FAIL")
            self.assertTrue(
                any(
                    "COMMERCIAL_LICENSING_POLICY.md" in item
                    for item in receipt["errors"]
                )
            )

    def test_normative_comment_mutation_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            copied = Path(tmp) / "core"
            shutil.copytree(ROOT, copied)
            target = copied / "core" / "apkc_rmr_research_core.c"
            text = target.read_text(encoding="utf-8")
            target.write_text(
                text.replace("Normative comment:", "Descriptive comment:", 1),
                encoding="utf-8",
            )
            receipt = MODULE.verify(copied)
            self.assertEqual(receipt["state"], "FAIL")


if __name__ == "__main__":
    unittest.main()
