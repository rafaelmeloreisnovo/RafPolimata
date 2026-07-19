#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "validate_root_file_decisions", ROOT / "scripts/validate_root_file_decisions.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)


class RootFileDecisionTests(unittest.TestCase):
    def policy(self) -> dict:
        return {
            "root_policy": {
                "allowed_files": ["README.md", "safe-extended"],
                "allowed_prefixes": ["RAF_", "raf_"],
            }
        }

    def valid_decision(self) -> dict:
        return {
            "path": "loose.txt",
            "git_blob_sha": "a" * 40,
            "kind": "documentation",
            "content_state": "REFERENCE",
            "evidence_state": "PENDING",
            "route": "MOVE_PROPOSED",
            "target": "docs/archive/loose.txt",
            "area": "documentation",
            "owner_role": "documentation-governance",
            "risk": "MEDIUM",
            "findings": ["arquivo solto"],
            "required_gates": ["revisar conteúdo"],
            "delete_allowed": False,
            "human_approval_required": True,
        }

    def test_loose_root_files_respect_policy(self) -> None:
        paths = [
            "README.md",
            "safe-extended",
            "RAF_001_test.c",
            "raf_main.c",
            "docs/guide.md",
            "loose.txt",
        ]
        self.assertEqual(MOD.loose_root_files(paths, self.policy()), ["loose.txt"])

    def test_valid_decision(self) -> None:
        self.assertEqual(MOD.validate_decision(self.valid_decision()), [])

    def test_delete_and_approval_are_fail_closed(self) -> None:
        decision = self.valid_decision()
        decision["delete_allowed"] = True
        decision["human_approval_required"] = False
        errors = MOD.validate_decision(decision)
        self.assertTrue(any("delete_allowed" in item for item in errors))
        self.assertTrue(any("human_approval_required" in item for item in errors))

    def test_move_requires_non_root_target(self) -> None:
        decision = self.valid_decision()
        decision["target"] = "renamed.txt"
        errors = MOD.validate_decision(decision)
        self.assertTrue(any("destino fora da raiz" in item for item in errors))

    def test_unknown_route_is_rejected(self) -> None:
        decision = self.valid_decision()
        decision["route"] = "DELETE_NOW"
        errors = MOD.validate_decision(decision)
        self.assertTrue(any("route inválida" in item for item in errors))


if __name__ == "__main__":
    unittest.main()
