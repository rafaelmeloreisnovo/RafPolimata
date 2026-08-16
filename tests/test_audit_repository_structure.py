#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "audit_repository_structure", ROOT / "scripts/audit_repository_structure.py"
)
assert SPEC and SPEC.loader
AUDIT = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = AUDIT
SPEC.loader.exec_module(AUDIT)


class RepositoryStructureAuditTests(unittest.TestCase):
    def make_repo(self) -> tuple[tempfile.TemporaryDirectory[str], Path, dict]:
        td = tempfile.TemporaryDirectory()
        root = Path(td.name)
        for name in (".github", "docs", "scripts", "configs", "tests"):
            (root / name).mkdir()
        (root / "README.md").write_text("# Root\n[Guide](docs/guide.md)\n", encoding="utf-8")
        (root / "docs/guide.md").write_text("# Guide\n", encoding="utf-8")
        policy = {
            "schema": "raf.document-governance-policy.v1",
            "root_policy": {
                "allowed_files": ["README.md"],
                "allowed_prefixes": ["RAF_"],
            },
            "areas": [
                {"prefixes": ["docs/"]},
                {"prefixes": ["scripts/"]},
                {"prefixes": ["configs/"]},
                {"prefixes": ["tests/"]},
                {"prefixes": [".github/workflows/"]},
            ],
        }
        return td, root, policy

    def test_root_policy_detects_unexpected_file(self) -> None:
        td, root, policy = self.make_repo()
        try:
            (root / "mystery.txt").write_text("x", encoding="utf-8")
            self.assertEqual(AUDIT.root_loose_files(root, policy), ["mystery.txt"])
        finally:
            td.cleanup()

    def test_existing_markdown_target_is_not_broken(self) -> None:
        td, root, _ = self.make_repo()
        try:
            self.assertEqual(AUDIT.broken_markdown_links(root), [])
            (root / "docs/guide.md").write_text(
                "# Guide\n[Missing](missing.md)\n", encoding="utf-8"
            )
            self.assertEqual(
                AUDIT.broken_markdown_links(root),
                ["docs/guide.md -> missing.md"],
            )
        finally:
            td.cleanup()

    def test_code_fence_markdown_like_syntax_is_not_a_link(self) -> None:
        td, root, _ = self.make_repo()
        try:
            (root / "docs/guide.md").write_text(
                "# Guide\n```text\nidentity[Int](42)\nidentity[String](\"hi\")\n```\n",
                encoding="utf-8",
            )
            self.assertEqual(AUDIT.broken_markdown_links(root), [])
        finally:
            td.cleanup()

    def test_real_link_after_code_fence_is_still_validated(self) -> None:
        td, root, _ = self.make_repo()
        try:
            (root / "docs/guide.md").write_text(
                "```text\nidentity[Int](42)\n```\n[Missing](missing.md)\n",
                encoding="utf-8",
            )
            self.assertEqual(
                AUDIT.broken_markdown_links(root),
                ["docs/guide.md -> missing.md"],
            )
        finally:
            td.cleanup()

    def test_report_separates_blocker_from_review(self) -> None:
        td, root, policy = self.make_repo()
        try:
            (root / "mystery.txt").write_text("x", encoding="utf-8")
            report, code = AUDIT.build_report(root, policy, 5)
            self.assertEqual(code, 0)
            self.assertEqual(report["state"], "REVIEW_REQUIRED")
            self.assertEqual(report["reviews"]["root_loose_files"], ["mystery.txt"])

            (root / "docs/guide.md").write_text(
                "# Guide\n[Missing](missing.md)\n", encoding="utf-8"
            )
            report, code = AUDIT.build_report(root, policy, 5)
            self.assertEqual(code, 1)
            self.assertEqual(report["state"], "FAIL")
        finally:
            td.cleanup()


if __name__ == "__main__":
    unittest.main()
