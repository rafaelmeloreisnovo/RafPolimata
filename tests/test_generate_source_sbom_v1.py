#!/usr/bin/env python3
"""Source SBOM V1 regression tests. Governance anchor: CLOSURE_L11."""
from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "generate_source_sbom_v1", ROOT / "scripts/generate_source_sbom_v1.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)

TOKEN = "TOKEN" + "_VAZIO"


class SourceSbomV1Tests(unittest.TestCase):
    def make_repo(self, with_license: bool = False) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        td = tempfile.TemporaryDirectory()
        root = Path(td.name)
        subprocess.run(["git", "-C", str(root), "init"], check=True, capture_output=True)
        subprocess.run(["git", "-C", str(root), "config", "user.email", "sbom-test@example.invalid"], check=True)
        subprocess.run(["git", "-C", str(root), "config", "user.name", "SBOM Test"], check=True)
        (root / "src").mkdir()
        (root / "src/main.c").write_text("/* SPDX-License-Identifier: MIT */\nint main(void){return 0;}\n", encoding="utf-8")
        (root / "requirements.txt").write_text("example==1.0\n", encoding="utf-8")
        if with_license:
            (root / "LICENSE").write_text("test license evidence\n", encoding="utf-8")
        subprocess.run(["git", "-C", str(root), "add", "."], check=True)
        subprocess.run(["git", "-C", str(root), "commit", "-m", "fixture"], check=True, capture_output=True)
        return td, root

    def test_deterministic_for_same_commit(self) -> None:
        td, root = self.make_repo()
        try:
            first = MOD.build(root)
            second = MOD.build(root)
            self.assertEqual(first, second)
            self.assertEqual(first[2]["tracked_file_count"], 2)
        finally:
            td.cleanup()

    def test_cyclonedx_identity_and_file_hashes(self) -> None:
        td, root = self.make_repo()
        try:
            bom, _, receipt = MOD.build(root)
            self.assertEqual(bom["bomFormat"], "CycloneDX")
            self.assertEqual(bom["specVersion"], "1.7")
            self.assertEqual(receipt["cyclonedx_spec_version"], "1.7")
            self.assertTrue(all(component["type"] == "file" for component in bom["components"]))
            self.assertTrue(all(component["hashes"][0]["alg"] == "SHA-256" for component in bom["components"]))
        finally:
            td.cleanup()

    def test_missing_root_license_is_fail_closed(self) -> None:
        td, root = self.make_repo()
        try:
            _, inventory, receipt = MOD.build(root)
            self.assertEqual(inventory["root_license_state"], TOKEN)
            self.assertEqual(inventory["owner_license_decision_state"], "TOKEN_VAZIO_OWNER_DECISION")
            self.assertFalse(inventory["claim_allowed"])
            self.assertFalse(receipt["claim_allowed"])
        finally:
            td.cleanup()

    def test_root_license_file_is_evidence_not_legal_conclusion(self) -> None:
        td, root = self.make_repo(with_license=True)
        try:
            _, inventory, _ = MOD.build(root)
            self.assertEqual(inventory["root_license_state"], "OBSERVED")
            license_records = [item for item in inventory["evidence"] if item["path"] == "LICENSE"]
            self.assertEqual(len(license_records), 1)
            self.assertFalse(license_records[0]["legal_conclusion"])
            self.assertEqual(inventory["owner_license_decision_state"], "REVIEW_REQUIRED")
        finally:
            td.cleanup()

    def test_spdx_header_is_observation_only(self) -> None:
        td, root = self.make_repo()
        try:
            bom, inventory, _ = MOD.build(root)
            observed = [item for item in inventory["evidence"] if item["evidence_type"] == "SPDX_HEADER"]
            self.assertEqual(observed[0]["expression_observed"], "MIT")
            self.assertFalse(observed[0]["legal_conclusion"])
            main = next(item for item in bom["components"] if item["name"] == "src/main.c")
            props = {item["name"]: item["value"] for item in main["properties"]}
            self.assertEqual(props["rafpolimata:license:spdx-expression-observed"], "MIT")
        finally:
            td.cleanup()

    def test_dependency_manifest_is_inventoried_not_resolved(self) -> None:
        td, root = self.make_repo()
        try:
            _, inventory, _ = MOD.build(root)
            self.assertEqual(inventory["dependency_manifests"][0]["path"], "requirements.txt")
            self.assertEqual(inventory["dependency_manifests"][0]["parser_state"], "REFERENCE_ONLY_NOT_DEPENDENCY_RESOLVED")
        finally:
            td.cleanup()

    def test_receipt_round_trips_json(self) -> None:
        td, root = self.make_repo()
        try:
            _, _, receipt = MOD.build(root)
            self.assertEqual(json.loads(json.dumps(receipt)), receipt)
            self.assertTrue(receipt["sbom_hash"].startswith("sha256:"))
            self.assertTrue(receipt["license_inventory_hash"].startswith("sha256:"))
        finally:
            td.cleanup()


if __name__ == "__main__":
    unittest.main()
