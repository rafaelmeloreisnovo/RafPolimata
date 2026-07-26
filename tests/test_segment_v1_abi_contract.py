from __future__ import annotations

import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/validate_segment_v1_abi_contract.py"
INPUTS = (
    "runtime/conversation_indexer/SEGMENT_V1_ABI_CONTRACT.json",
    "runtime/conversation_indexer/raf_segment_v1.h",
    "runtime/conversation_indexer/raf_segment_v1.c",
    "docs/copilot/TASK_02_CONVERSATION_SEGMENTS_V1.md",
)


class SegmentV1AbiContractTest(unittest.TestCase):
    def run_validator(self, repo: Path):
        output = repo / "artifacts/segment-v1-abi/report.json"
        completed = subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--repo",
                str(repo),
                "--output",
                str(output),
            ],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        return completed, json.loads(output.read_text(encoding="utf-8"))

    def make_fixture(self, destination: Path):
        for relative in INPUTS:
            source = ROOT / relative
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)

    def test_current_frozen_abi_reconciles(self):
        completed, report = self.run_validator(ROOT)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(report["state"], "PASS_WITH_RECONCILED_SPEC_CONFLICTS")
        self.assertEqual(report["extractor_gate"]["segment_v1_wire_layout"], "FROZEN")
        self.assertFalse(report["claim_allowed"])

    def test_role_renumbering_is_detected(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_fixture(repo)
            header = repo / "runtime/conversation_indexer/raf_segment_v1.h"
            header.write_text(
                header.read_text(encoding="utf-8").replace(
                    "#define RAF_SEGMENT_ROLE_USER 1u",
                    "#define RAF_SEGMENT_ROLE_USER 9u",
                ),
                encoding="utf-8",
            )
            completed, report = self.run_validator(repo)
        self.assertEqual(completed.returncode, 1)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(any("role mismatch USER" in item for item in report["errors"]))

    def test_record_offset_drift_is_detected(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            self.make_fixture(repo)
            codec = repo / "runtime/conversation_indexer/raf_segment_v1.c"
            codec.write_text(
                codec.read_text(encoding="utf-8").replace(
                    "put_u64le(out + 48u, record->title_offset);",
                    "put_u64le(out + 56u, record->title_offset);",
                    1,
                ),
                encoding="utf-8",
            )
            completed, report = self.run_validator(repo)
        self.assertEqual(completed.returncode, 1)
        self.assertEqual(report["state"], "FAIL")
        self.assertTrue(
            any("conversation.title_offset" in item for item in report["errors"])
        )


if __name__ == "__main__":
    unittest.main()
