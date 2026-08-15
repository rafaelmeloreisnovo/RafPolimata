#!/usr/bin/env python3
"""
Tests for Gap Atlas builder.
"""

from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "gap_atlas_builder", ROOT / "tools/gap_atlas_builder.py"
)
assert SPEC and SPEC.loader
GAB = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = GAB
SPEC.loader.exec_module(GAB)


class GapAtlasBuilderTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def make_git_repo(self, name: str = "test_repo") -> Path:
        """Create a test git repository."""
        repo_path = self.temp_root / name
        repo_path.mkdir(parents=True, exist_ok=True)

        import subprocess
        subprocess.run(["git", "init"], cwd=repo_path, capture_output=True)
        subprocess.run(
            ["git", "config", "user.email", "test@test.com"],
            cwd=repo_path,
            capture_output=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Test User"],
            cwd=repo_path,
            capture_output=True,
        )

        # Create initial commit
        (repo_path / "README.md").write_text("# Test Repo\n")
        subprocess.run(
            ["git", "add", "-A"],
            cwd=repo_path,
            capture_output=True,
        )
        subprocess.run(
            ["git", "commit", "-m", "Initial commit"],
            cwd=repo_path,
            capture_output=True,
        )

        return repo_path

    def test_builder_initialization(self) -> None:
        """Builder should initialize with empty atlas."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        self.assertIsNotNone(builder.atlas)
        self.assertEqual(builder.atlas["schema_version"], "1.0.0")

    def test_build_atlas_structure(self) -> None:
        """Atlas should have all required gaps."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        atlas = builder.build_atlas()

        # Should have all gaps
        self.assertIn("gaps", atlas)
        self.assertEqual(len(atlas["gaps"]), len(GAB.GAPS))

        # Each gap should have required fields
        for gap_id, gap in atlas["gaps"].items():
            self.assertIn("name", gap)
            self.assertIn("description", gap)
            self.assertIn("status", gap)
            self.assertEqual(gap["status"], "PENDING")

    def test_close_l0_provenance_with_git(self) -> None:
        """L0 closure should capture git commit."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l0_provenance()

        # Should succeed if git repo is valid
        self.assertTrue(success or not repo_path.is_dir())

        # Should record closure
        self.assertTrue(len(builder.atlas["closures"]) > 0)

        closure = builder.atlas["closures"][0]
        self.assertEqual(closure["gap_id"], "L0")
        self.assertIn("source_commit", closure["evidence"])

    def test_close_l1_reproducibility_identical_hashes(self) -> None:
        """L1 closure should pass for identical build hashes."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        # Create two identical build artifacts
        build1 = self.temp_root / "build1"
        build2 = self.temp_root / "build2"
        build1.write_text("identical content")
        build2.write_text("identical content")

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l1_reproducibility([build1, build2])

        self.assertTrue(success)
        closure = builder.atlas["closures"][-1]
        self.assertEqual(closure["status"], "PASS")

    def test_close_l1_reproducibility_different_hashes(self) -> None:
        """L1 closure should fail for different build hashes."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        # Create two different build artifacts
        build1 = self.temp_root / "build1"
        build2 = self.temp_root / "build2"
        build1.write_text("content 1")
        build2.write_text("content 2")

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l1_reproducibility([build1, build2])

        self.assertFalse(success)
        closure = builder.atlas["closures"][-1]
        self.assertEqual(closure["status"], "FAIL")

    def test_close_l1_reproducibility_missing_files(self) -> None:
        """L1 closure should handle missing files gracefully."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l1_reproducibility([])

        # Should record as TOKEN_VAZIO (unknown status)
        closure = builder.atlas["closures"][-1]
        self.assertEqual(closure["status"], "TOKEN_VAZIO")

    def test_close_l3_elf_validation_valid_elf(self) -> None:
        """L3 closure should validate ARM64 ELF files."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        # Create a minimal ARM64 ELF header (64 bytes)
        elf_path = self.temp_root / "test.so"
        elf_header = bytearray(64)
        elf_header[0:4] = b"\x7fELF"  # ELF magic
        elf_header[4] = 2  # 64-bit (ELFCLASS64)
        elf_header[5] = 1  # Little-endian (ELFDATA2LSB)
        elf_header[6] = 1  # ELF version
        elf_header[18:20] = b"\xb7\x00"  # e_machine (ARM64 = 0xB7, little-endian)

        elf_path.write_bytes(elf_header)

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l3_elf_validation(elf_path)

        self.assertTrue(success)
        closure = builder.atlas["closures"][-1]
        self.assertEqual(closure["gap_id"], "L3")
        self.assertEqual(closure["status"], "PASS")
        self.assertTrue(closure["evidence"]["is_arm64"])

    def test_close_l3_elf_validation_missing_file(self) -> None:
        """L3 closure should handle missing ELF files."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        success = builder.close_l3_elf_validation(self.temp_root / "nonexistent.so")

        self.assertFalse(success)
        closure = builder.atlas["closures"][-1]
        self.assertFalse(closure["evidence"]["file_exists"])

    def test_record_evidence(self) -> None:
        """Evidence should be recorded in chain."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        evidence_data = {"key": "value"}
        builder.record_evidence("test_evidence", evidence_data)

        self.assertEqual(len(builder.atlas["evidence_chain"]), 1)
        evidence = builder.atlas["evidence_chain"][0]
        self.assertEqual(evidence["type"], "test_evidence")
        self.assertIn("hash", evidence)
        self.assertIn("timestamp", evidence)

    def test_write_atlas_json(self) -> None:
        """Atlas should be written as valid JSON."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()
        builder.close_l0_provenance()

        written_path = builder.write_atlas()

        # Should write valid JSON
        self.assertTrue(written_path.exists())
        with open(written_path) as f:
            atlas_json = json.load(f)

        self.assertIn("schema_version", atlas_json)
        self.assertIn("gaps", atlas_json)
        self.assertIn("closures", atlas_json)

    def test_generate_report(self) -> None:
        """Report should be human-readable."""
        repo_path = self.make_git_repo()
        output_path = self.temp_root / "atlas.json"

        builder = GAB.GapAtlasBuilder(repo_path, output_path)
        builder.build_atlas()

        report = builder.generate_report()

        # Should contain expected sections
        self.assertIn("# Gap Atlas Report", report)
        self.assertIn("## Gaps Status", report)
        self.assertIn("## Evidence Chain", report)

        # Should list gaps
        for gap_id in GAB.GAPS.keys():
            self.assertIn(gap_id, report)


if __name__ == "__main__":
    unittest.main()
