#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "validate_canari_integration", ROOT / "scripts/validate_canari_integration.py"
)
assert SPEC and SPEC.loader
CANARI = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = CANARI
SPEC.loader.exec_module(CANARI)


class CanariIntegrationTests(unittest.TestCase):
    def make_repo(self) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        td = tempfile.TemporaryDirectory()
        root = Path(td.name)
        for name in ("configs", "docs", "scripts", "tests"):
            (root / name).mkdir(parents=True, exist_ok=True)

        manifest = {
            "schema": "raf.canari-integration.v1",
            "integration_id": "CANARI-MALTEGO-001",
            "name": "Canari",
            "purpose": "test",
            "mode": "external-isolated",
            "upstream": {
                "repository": "malleum-inc/canari3",
                "license": "GPL-3.0",
                "source_pin": "TOKEN_VAZIO",
            },
            "compatibility": {
                "isolated_environment_required": True,
                "network_execution_default": "disabled",
            },
            "legal": {
                "vendoring_allowed": False,
                "compatibility_review": "TOKEN_VAZIO",
            },
            "evidence": {
                "runtime_verified": "TOKEN_VAZIO",
                "maltego_profile_verified": "TOKEN_VAZIO",
                "termux_verified": "TOKEN_VAZIO",
            },
            "claims": {"claim_allowed": False},
        }
        (root / "configs/canari-integration.v1.json").write_text(
            json.dumps(manifest), encoding="utf-8"
        )
        (root / "docs/LICENSE_DECISION_RECORD.md").write_text(
            "Estado: TOKEN_VAZIO_OWNER_DECISION\n", encoding="utf-8"
        )
        return td, root

    def test_structural_contract_passes_without_runtime_claim(self) -> None:
        td, root = self.make_repo()
        try:
            receipt = CANARI.validate(root, Path("configs/canari-integration.v1.json"))
            self.assertEqual(receipt["contract_status"], "PASS")
            self.assertFalse(receipt["claim_allowed"])
            self.assertEqual(receipt["runtime_state"]["runtime_verified"], "TOKEN_VAZIO")
        finally:
            td.cleanup()

    def test_direct_core_import_is_blocked(self) -> None:
        td, root = self.make_repo()
        try:
            (root / "scripts/unsafe.py").write_text("import canari\n", encoding="utf-8")
            receipt = CANARI.validate(root, Path("configs/canari-integration.v1.json"))
            self.assertEqual(receipt["contract_status"], "FAIL")
            self.assertIn("boundary.no_core_import", receipt["failed_checks"])
        finally:
            td.cleanup()

    def test_vendored_canari_is_blocked_before_license_closure(self) -> None:
        td, root = self.make_repo()
        try:
            (root / "third_party/canari").mkdir(parents=True)
            receipt = CANARI.validate(root, Path("configs/canari-integration.v1.json"))
            self.assertEqual(receipt["contract_status"], "FAIL")
            self.assertIn("boundary.no_vendored_canari", receipt["failed_checks"])
        finally:
            td.cleanup()

    def test_claim_promotion_requires_pin_license_and_runtime_evidence(self) -> None:
        td, root = self.make_repo()
        try:
            path = root / "configs/canari-integration.v1.json"
            manifest = json.loads(path.read_text(encoding="utf-8"))
            manifest["claims"]["claim_allowed"] = True
            path.write_text(json.dumps(manifest), encoding="utf-8")

            receipt = CANARI.validate(root, Path("configs/canari-integration.v1.json"))
            self.assertEqual(receipt["contract_status"], "FAIL")
            self.assertFalse(receipt["claim_allowed"])
            self.assertIn("claims.promotion_gate", receipt["failed_checks"])
        finally:
            td.cleanup()


if __name__ == "__main__":
    unittest.main()
