#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
import warnings
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "audit_zip_artifact", ROOT / "scripts/audit_zip_artifact.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)


class ZipArtifactAuditTests(unittest.TestCase):
    def test_valid_zip_is_hashed_without_extraction(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "valid.zip"
            with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as archive:
                archive.writestr("docs/readme.txt", "hello governance")
            report = MOD.audit_zip(path)
            self.assertEqual(report["state"], "PASS")
            self.assertEqual(report["summary"]["entry_count"], 1)
            self.assertEqual(len(report["entries"][0]["sha256"]), 64)
            self.assertFalse((Path(td) / "docs").exists())

    def test_path_traversal_is_blocking_and_not_hashed(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "traversal.zip"
            with zipfile.ZipFile(path, "w") as archive:
                archive.writestr("../outside.txt", "blocked")
            report = MOD.audit_zip(path)
            self.assertEqual(report["state"], "FAIL")
            self.assertIn("path_traversal", report["entries"][0]["blocking_flags"])
            self.assertIsNone(report["entries"][0]["sha256"])

    def test_duplicate_names_are_blocking(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "duplicate.zip"
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", UserWarning)
                with zipfile.ZipFile(path, "w") as archive:
                    archive.writestr("same.txt", "one")
                    archive.writestr("same.txt", "two")
            report = MOD.audit_zip(path)
            self.assertEqual(report["state"], "FAIL")
            self.assertEqual(report["summary"]["duplicate_names"], ["same.txt"])

    def test_executable_suffix_requires_review(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "binary.zip"
            with zipfile.ZipFile(path, "w") as archive:
                archive.writestr("bin/libmain.so", b"not an elf")
            report = MOD.audit_zip(path)
            self.assertEqual(report["state"], "REVIEW_REQUIRED")
            self.assertIn(
                "executable_or_sensitive_suffix",
                report["entries"][0]["review_flags"],
            )

    def test_large_declared_entry_is_not_opened(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "large.zip"
            with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as archive:
                archive.writestr("large.bin", b"x" * 4096)
            report = MOD.audit_zip(path, max_entry_bytes=1024)
            self.assertEqual(report["state"], "FAIL")
            self.assertIn("entry_too_large", report["entries"][0]["blocking_flags"])
            self.assertIsNone(report["entries"][0]["sha256"])


if __name__ == "__main__":
    unittest.main()
