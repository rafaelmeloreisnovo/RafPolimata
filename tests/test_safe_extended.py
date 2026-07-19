#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("safe_extended", ROOT / "scripts" / "safe_extended.py")
assert SPEC and SPEC.loader
SAFE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SAFE)


class SafeExtendedTests(unittest.TestCase):
    def write_workflow(self, body: str) -> Path:
        tmp = Path(tempfile.mkdtemp(prefix="safe-extended-test-"))
        path = tmp / "ci.yml"
        path.write_text(body, encoding="utf-8")
        self.addCleanup(lambda: __import__("shutil").rmtree(tmp, ignore_errors=True))
        return path

    def test_parser_ignores_on_block_and_reads_jobs(self) -> None:
        path = self.write_workflow(
            """name: CI
on:
  push:
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout
        uses: actions/checkout@v4
      - name: Compile
        run: |
          echo compile
"""
        )
        plan = SAFE.parse_workflow(path)
        self.assertEqual([job["id"] for job in plan["jobs"]], ["build"])
        self.assertEqual(plan["jobs"][0]["steps"][1]["run"], "echo compile\n")

    def test_multiline_artifact_path(self) -> None:
        path = self.write_workflow(
            """jobs:
  proof:
    steps:
      - name: Upload
        uses: actions/upload-artifact@v4
        with:
          name: proof
          path: |
            ci/reports/a.md
            results/a.json
"""
        )
        step = SAFE.parse_workflow(path)["jobs"][0]["steps"][0]
        self.assertEqual(step["with"]["path"], "ci/reports/a.md\nresults/a.json\n")

    def test_network_is_denied(self) -> None:
        denied, warnings = SAFE.inspect_script("curl https://example.invalid/x\n")
        self.assertIn("network", denied)
        self.assertEqual(warnings, [])

    def test_masked_failure_is_warning(self) -> None:
        denied, warnings = SAFE.inspect_script("false || true\n")
        self.assertEqual(denied, [])
        self.assertIn("masked_failure", warnings)

    def test_github_expression_is_denied(self) -> None:
        denied, _ = SAFE.inspect_script("echo '${{ github.sha }}'\n")
        self.assertIn("github_expression", denied)


if __name__ == "__main__":
    unittest.main()
