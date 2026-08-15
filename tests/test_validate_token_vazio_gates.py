#!/usr/bin/env python3
"""
Tests for TOKEN_VAZIO validator (Hotfix H1)
"""

import json
import tempfile
import unittest
from pathlib import Path
from sys import path as sys_path

# Add tools to path
sys_path.insert(0, str(Path(__file__).parent.parent / "tools"))

from validate_token_vazio_gates import TokenVazioValidator, TokenVazioFinding


class TestTokenVazioValidation(unittest.TestCase):
    """Test TOKEN_VAZIO detection and closure linkage."""

    def setUp(self):
        """Create temporary repository structure."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.repo_root = Path(self.temp_dir.name)

        # Create closures directory
        (self.repo_root / "docs" / "closures").mkdir(parents=True)

        # Create some closure files
        (self.repo_root / "docs" / "closures" / "CLOSURE_L0.md").write_text(
            "# L0: Source Provenance\nStatus: PASS\n"
        )
        (self.repo_root / "docs" / "closures" / "CLOSURE_L1.md").write_text(
            "# L1: Reproducibility\nStatus: PASS\n"
        )

    def tearDown(self):
        """Clean up."""
        self.temp_dir.cleanup()

    def test_valid_token_vazio_with_closure(self):
        """TOKEN_VAZIO linked to existing closure should PASS."""
        test_file = self.repo_root / "test_valid.md"
        test_file.write_text(
            "# Document\n"
            "Some claim with TOKEN_VAZIO reference: see CLOSURE_L0 for proof.\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find TOKEN_VAZIO
        self.assertTrue(len(validator.findings) > 0)
        # Should mark as WARNING (has closure)
        self.assertTrue(validator.findings[0].has_closure)

    def test_token_vazio_without_closure_reference(self):
        """TOKEN_VAZIO without closure reference should ERROR."""
        test_file = self.repo_root / "test_invalid.md"
        test_file.write_text(
            "# Document\n"
            "Some incomplete claim: TOKEN_VAZIO\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find TOKEN_VAZIO
        self.assertTrue(len(validator.findings) > 0)
        # Should mark as ERROR (no closure)
        self.assertFalse(validator.findings[0].has_closure)
        self.assertEqual(validator.findings[0].severity, "ERROR")

    def test_token_vazio_with_missing_closure_file(self):
        """TOKEN_VAZIO referencing missing closure should ERROR."""
        test_file = self.repo_root / "test_missing_closure.md"
        test_file.write_text(
            "# Document\n"
            "Claim with TOKEN_VAZIO and missing closure: see CLOSURE_L99 for details.\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find TOKEN_VAZIO reference
        self.assertTrue(len(validator.findings) > 0)
        # Closure doesn't exist and not in ALLOWED_MISSING
        self.assertEqual(validator.findings[0].severity, "ERROR")

    def test_allowed_missing_closure_l9(self):
        """L9 (42 attractors) is allowed to remain TOKEN_VAZIO."""
        test_file = self.repo_root / "test_l9.md"
        test_file.write_text(
            "# Document\n"
            "42 attractors TOKEN_VAZIO convergence: see CLOSURE_L9 for falsification.\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find TOKEN_VAZIO reference
        self.assertTrue(len(validator.findings) > 0)
        # L9 is in ALLOWED_MISSING
        self.assertTrue(validator.findings[0].has_closure)
        self.assertEqual(validator.findings[0].severity, "WARNING")

    def test_repository_scan(self):
        """Full repository scan should find all TOKEN_VAZIO."""
        (self.repo_root / "docs").mkdir(exist_ok=True)
        (self.repo_root / "src").mkdir(exist_ok=True)

        # File 1: valid closure reference
        (self.repo_root / "docs" / "file1.md").write_text(
            "Document with TOKEN_VAZIO in CLOSURE_L0\n"
        )

        # File 2: invalid (no closure)
        (self.repo_root / "src" / "file2.py").write_text(
            'status = "TOKEN_VAZIO"  # unlinked\n'
        )

        # File 3: should skip
        (self.repo_root / "docs" / "generated").mkdir(parents=True, exist_ok=True)
        (self.repo_root / "docs" / "generated" / "auto.md").write_text(
            "TOKEN_VAZIO auto-generated\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        error_count, warning_count = validator.scan_repository()

        # Should find TOKEN_VAZIO in files 1 and 2, skip generated
        self.assertEqual(error_count, 1)  # file2 (unlinked)
        self.assertGreater(warning_count, 0)  # file1 (linked)

    def test_report_generation(self):
        """Report should be valid JSON with required fields."""
        test_file = self.repo_root / "test.md"
        test_file.write_text("TOKEN_VAZIO\n")

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        report = validator.report()

        # Verify report structure
        self.assertIn("schema", report)
        self.assertEqual(report["schema"], "rafaelia.token_vazio_validator.v1")
        self.assertIn("timestamp", report)
        self.assertIn("findings", report)
        self.assertIn("summary", report)
        self.assertIn("status", report)
        self.assertIn("report_hash", report)

        # Verify hash is consistent
        hash1 = report["report_hash"]
        report2 = validator.report()
        hash2 = report2["report_hash"]
        self.assertEqual(hash1, hash2)

    def test_should_halt_ci_on_errors(self):
        """CI should halt if errors present."""
        test_file = self.repo_root / "test.md"
        test_file.write_text("TOKEN_VAZIO without closure\n")

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should halt if errors
        if validator.findings[0].severity == "ERROR":
            self.assertTrue(validator.should_halt_ci())

    def test_multiple_token_vazio_in_file(self):
        """Multiple TOKEN_VAZIO in same file should all be detected."""
        test_file = self.repo_root / "test_multiple.md"
        test_file.write_text(
            "Line 1: TOKEN_VAZIO without closure\n"
            "Line 3: See CLOSURE_L0 for TOKEN_VAZIO proof\n"
            "Line 5: Another TOKEN_VAZIO standalone\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find 3 TOKEN_VAZIO references
        self.assertEqual(len(validator.findings), 3)

    def test_case_insensitive_detection(self):
        """TOKEN_VAZIO detection should be case-insensitive."""
        test_file = self.repo_root / "test_case.md"
        test_file.write_text(
            "With TOKEN_VAZIO uppercase\n"
            "And token_vazio lowercase\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)

        # Should find both
        self.assertEqual(len(validator.findings), 2)

    def test_skips_binary_and_generated(self):
        """Should skip binary files and generated content."""
        (self.repo_root / "docs" / "generated").mkdir(parents=True)
        (self.repo_root / ".git").mkdir(parents=True)

        # Write to generated dir
        (self.repo_root / "docs" / "generated" / "auto.md").write_text(
            "TOKEN_VAZIO in generated\n"
        )

        # Write to .git dir
        (self.repo_root / ".git" / "config").write_text(
            "TOKEN_VAZIO in git config\n"
        )

        # Regular file for comparison
        (self.repo_root / "regular.md").write_text(
            "TOKEN_VAZIO in regular file\n"
        )

        validator = TokenVazioValidator(self.repo_root)
        error_count, warning_count = validator.scan_repository()

        # Should only find in regular file
        findings_in_regular = [
            f for f in validator.findings
            if "regular.md" in f.file_path
        ]
        self.assertTrue(len(findings_in_regular) > 0)

        findings_in_generated = [
            f for f in validator.findings
            if "generated" in f.file_path
        ]
        self.assertEqual(len(findings_in_generated), 0)

    def test_finding_data_structure(self):
        """TokenVazioFinding should have all required fields."""
        finding = TokenVazioFinding(
            file_path="test.md",
            line_number=42,
            context="Some context",
            has_closure=True,
            closure_file="CLOSURE_L0",
            severity="WARNING"
        )

        finding_dict = finding.to_dict()
        self.assertIn("file_path", finding_dict)
        self.assertIn("line_number", finding_dict)
        self.assertIn("context", finding_dict)
        self.assertIn("has_closure", finding_dict)
        self.assertIn("closure_file", finding_dict)
        self.assertIn("severity", finding_dict)


class TestTokenVazioIntegration(unittest.TestCase):
    """Integration tests with document governance."""

    def setUp(self):
        """Create temporary directory with closure structure."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.repo_root = Path(self.temp_dir.name)
        (self.repo_root / "docs" / "closures").mkdir(parents=True)

    def tearDown(self):
        """Clean up."""
        self.temp_dir.cleanup()

    def test_closure_map_building(self):
        """Validator should correctly map existing closures."""
        # Create multiple closure files
        for gap_id in ["L0", "L1", "L2", "G1"]:
            (self.repo_root / "docs" / "closures" / f"CLOSURE_{gap_id}.md").write_text(
                f"# {gap_id}\n"
            )

        validator = TokenVazioValidator(self.repo_root)
        self.assertIn("L0", validator.closure_map)
        self.assertIn("L1", validator.closure_map)
        self.assertIn("L2", validator.closure_map)
        self.assertIn("G1", validator.closure_map)

    def test_report_serialization(self):
        """Report should be JSON-serializable."""
        test_file = self.repo_root / "test.md"
        test_file.write_text("TOKEN_VAZIO\n")

        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        report = validator.report()

        # Should be able to serialize to JSON
        json_str = json.dumps(report, indent=2)
        self.assertIsInstance(json_str, str)
        self.assertGreater(len(json_str), 0)

        # Should be able to deserialize back
        report_restored = json.loads(json_str)
        self.assertEqual(report["report_hash"], report_restored["report_hash"])


if __name__ == "__main__":
    unittest.main()
