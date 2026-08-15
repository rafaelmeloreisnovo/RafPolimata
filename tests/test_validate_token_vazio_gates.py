#!/usr/bin/env python3
"""Regression tests for the TOKEN_VAZIO gate.

Governance anchor: CLOSURE_L1.  These tests intentionally exercise explicit
gap markers; examples are not claims and must never be promoted to PASS.
"""
from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from sys import path as sys_path

sys_path.insert(0, str(Path(__file__).parent.parent / "tools"))

from validate_token_vazio_gates import TokenVazioFinding, TokenVazioValidator

TOKEN = "TOKEN" + "_VAZIO"


class TestTokenVazioValidation(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.repo_root = Path(self.temp_dir.name)
        closure_dir = self.repo_root / "docs" / "closures"
        closure_dir.mkdir(parents=True)
        (closure_dir / "CLOSURE_L1.md").write_text(
            "# L1: provenance/reproducibility\nStatus: governed\n",
            encoding="utf-8",
        )
        (closure_dir / "CLOSURE_L2.md").write_text(
            "# L2: runtime evidence\nStatus: governed\n",
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def _git(self, *args: str) -> str:
        result = subprocess.run(
            ["git", "-C", str(self.repo_root), *args],
            text=True,
            capture_output=True,
            check=True,
        )
        return result.stdout.strip()

    def _commit_baseline(self) -> str:
        self._git("init")
        self._git("config", "user.email", "custody-test@example.invalid")
        self._git("config", "user.name", "Custody Test")
        self._git("add", ".")
        self._git("commit", "-m", "baseline")
        return self._git("rev-parse", "HEAD")

    def test_valid_token_with_inline_closure_is_warning(self) -> None:
        test_file = self.repo_root / "valid.md"
        test_file.write_text(f"gap={TOKEN}; see CLOSURE_L1\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        self.assertEqual(len(validator.findings), 1)
        self.assertTrue(validator.findings[0].has_closure)
        self.assertEqual(validator.findings[0].severity, "WARNING")

    def test_token_without_closure_is_error(self) -> None:
        test_file = self.repo_root / "invalid.md"
        test_file.write_text(f"gap={TOKEN}\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        self.assertEqual(len(validator.findings), 1)
        self.assertEqual(validator.findings[0].severity, "ERROR")
        self.assertTrue(validator.should_halt_ci())

    def test_missing_closure_reference_is_error(self) -> None:
        test_file = self.repo_root / "missing.md"
        test_file.write_text(f"gap={TOKEN}; see CLOSURE_L99\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        self.assertEqual(validator.findings[0].severity, "ERROR")

    def test_allowed_l9_reference_remains_warning(self) -> None:
        test_file = self.repo_root / "l9.md"
        test_file.write_text(f"gap={TOKEN}; see CLOSURE_L9\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        self.assertEqual(validator.findings[0].severity, "WARNING")

    def test_repository_scan_does_not_duplicate_findings(self) -> None:
        (self.repo_root / "one.md").write_text(f"{TOKEN} CLOSURE_L1\n", encoding="utf-8")
        (self.repo_root / "two.py").write_text(f"state='{TOKEN}'\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        errors, warnings = validator.scan_repository()
        self.assertEqual((errors, warnings), (1, 1))
        self.assertEqual(len(validator.findings), 2)
        identities = {(f.file_path, f.line_number) for f in validator.findings}
        self.assertEqual(len(identities), 2)

    def test_generated_and_git_content_are_skipped(self) -> None:
        generated = self.repo_root / "docs" / "generated"
        generated.mkdir(parents=True)
        (generated / "auto.md").write_text(TOKEN + "\n", encoding="utf-8")
        git_dir = self.repo_root / ".git"
        git_dir.mkdir()
        (git_dir / "config").write_text(TOKEN + "\n", encoding="utf-8")
        (self.repo_root / "regular.md").write_text(TOKEN + "\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_repository()
        self.assertEqual([(f.file_path, f.line_number) for f in validator.findings], [("regular.md", 1)])

    def test_case_insensitive_detection(self) -> None:
        test_file = self.repo_root / "case.md"
        test_file.write_text("token_vazio\nToKeN_VaZiO\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        self.assertEqual(len(validator.findings), 2)

    def test_report_is_deterministic_except_timestamp(self) -> None:
        test_file = self.repo_root / "report.md"
        test_file.write_text(TOKEN + "\n", encoding="utf-8")
        validator = TokenVazioValidator(self.repo_root)
        validator.scan_file(test_file)
        report1 = validator.report()
        report2 = validator.report()
        self.assertEqual(report1["schema"], "rafaelia.token_vazio_validator.v1")
        self.assertEqual(report1["report_hash"], report2["report_hash"])
        json.dumps(report1)

    def test_finding_data_structure(self) -> None:
        finding = TokenVazioFinding(
            file_path="test.md",
            line_number=42,
            context="context",
            has_closure=True,
            closure_file="CLOSURE_L1",
            severity="WARNING",
        )
        finding_dict = finding.to_dict()
        self.assertEqual(finding_dict["line_number"], 42)
        self.assertEqual(finding_dict["closure_file"], "CLOSURE_L1")

    def test_changed_scope_ignores_legacy_unlinked_debt(self) -> None:
        (self.repo_root / "legacy.py").write_text(f"legacy='{TOKEN}'\n", encoding="utf-8")
        (self.repo_root / "changed.md").write_text("baseline\n", encoding="utf-8")
        base = self._commit_baseline()
        (self.repo_root / "changed.md").write_text("baseline\nordinary change\n", encoding="utf-8")

        validator = TokenVazioValidator(self.repo_root)
        scope = validator.changed_lines_since(base)
        validator.scan_repository(scope)
        self.assertEqual(validator.findings, [])
        self.assertNotIn("legacy.py", scope)

    def test_changed_scope_rejects_new_unlinked_gap(self) -> None:
        (self.repo_root / "new-gap.md").write_text("baseline\n", encoding="utf-8")
        base = self._commit_baseline()
        (self.repo_root / "new-gap.md").write_text(f"baseline\nnew={TOKEN}\n", encoding="utf-8")

        validator = TokenVazioValidator(self.repo_root)
        scope = validator.changed_lines_since(base)
        validator.scope = "changed_lines"
        validator.changed_since = base
        errors, warnings = validator.scan_repository(scope)
        self.assertEqual((errors, warnings), (1, 0))
        self.assertEqual(validator.findings[0].file_path, "new-gap.md")

    def test_file_level_l1_governs_changed_gap_line(self) -> None:
        test_file = self.repo_root / "governed.md"
        test_file.write_text("# Governance: CLOSURE_L1\nbaseline\n", encoding="utf-8")
        base = self._commit_baseline()
        test_file.write_text(f"# Governance: CLOSURE_L1\nbaseline\nstate={TOKEN}\n", encoding="utf-8")

        validator = TokenVazioValidator(self.repo_root)
        scope = validator.changed_lines_since(base)
        errors, warnings = validator.scan_repository(scope)
        self.assertEqual((errors, warnings), (0, 1))
        self.assertEqual(validator.findings[0].closure_file, "CLOSURE_L1")

    def test_changed_scope_reports_scope_and_base(self) -> None:
        test_file = self.repo_root / "governed.txt"
        test_file.write_text("CLOSURE_L1\n", encoding="utf-8")
        base = self._commit_baseline()
        test_file.write_text(f"CLOSURE_L1\n{TOKEN}\n", encoding="utf-8")

        validator = TokenVazioValidator(self.repo_root)
        validator.scope = "changed_lines"
        validator.changed_since = base
        validator.scan_repository(validator.changed_lines_since(base))
        report = validator.report()
        self.assertEqual(report["summary"]["scope"], "changed_lines")
        self.assertEqual(report["summary"]["changed_since"], base)
        self.assertEqual(report["status"], "PASS")

    def test_invalid_diff_base_fails_closed(self) -> None:
        self._commit_baseline()
        validator = TokenVazioValidator(self.repo_root)
        with self.assertRaises(RuntimeError):
            validator.changed_lines_since("definitely-not-a-git-ref")

    def test_closure_map_uses_realistic_l_ids(self) -> None:
        validator = TokenVazioValidator(self.repo_root)
        self.assertIn("L1", validator.closure_map)
        self.assertIn("L2", validator.closure_map)
        self.assertNotIn("L0", validator.closure_map)


if __name__ == "__main__":
    unittest.main()
