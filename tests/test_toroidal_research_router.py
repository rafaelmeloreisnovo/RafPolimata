from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "router", ROOT / "scripts" / "toroidal_research_router.py"
)
ROUTER = importlib.util.module_from_spec(SPEC)
assert SPEC and SPEC.loader
sys.modules[SPEC.name] = ROUTER
SPEC.loader.exec_module(ROUTER)


def load(path):
    return json.loads((ROOT / path).read_text(encoding="utf-8"))


class RouterTests(unittest.TestCase):
    def setUp(self):
        self.contract = load("contracts/toroidal_research_router.v1.json")
        self.manifest = load("examples/toroidal_research_router.example.json")

    def invalid_contract(self, contract, text):
        with self.assertRaises(ROUTER.RouterError) as error:
            ROUTER.validate_contract(contract)
        self.assertIn(text, str(error.exception))

    def invalid_manifest(self, manifest, text):
        with self.assertRaises(ROUTER.RouterError) as error:
            ROUTER.validate_manifest(self.contract, manifest)
        self.assertIn(text, str(error.exception))

    def test_contract_valid(self):
        self.assertIn("SCIENCE", ROUTER.validate_contract(self.contract)["authorities"])

    def test_example_deterministic(self):
        one = ROUTER.validate_manifest(self.contract, self.manifest)
        two = ROUTER.validate_manifest(self.contract, self.manifest)
        self.assertEqual(one, two)
        self.assertEqual(one["routed_limited_count"], 3)
        self.assertEqual(one["blocked_token_vazio_count"], 1)
        self.assertFalse(one["claim_allowed"])

    def test_router_cannot_promote(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["objects"][0]["requested_action"] = "PROMOTE"
        self.invalid_manifest(manifest, "cannot promote")

    def test_claim_must_originate_science(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["objects"][0]["source_repository"] = "rafaelmeloreisnovo/RafPolimata"
        self.invalid_manifest(manifest, "cannot originate")

    def test_formula_routes_to_orchestrator(self):
        output = ROUTER.validate_manifest(self.contract, self.manifest)
        decision = next(
            item for item in output["decisions"] if item["object_id"] == "O-FORMULA"
        )
        self.assertEqual(decision["target_role"], "ORCHESTRATION")
        self.assertEqual(decision["decision"], "ROUTED_LIMITED")

    def test_device_without_receipt_blocked(self):
        output = ROUTER.validate_manifest(self.contract, self.manifest)
        decision = next(
            item for item in output["decisions"] if item["object_id"] == "O-DEVICE"
        )
        self.assertEqual(decision["decision"], "BLOCKED_TOKEN_VAZIO")

    def test_device_with_receipt_can_route(self):
        manifest = copy.deepcopy(self.manifest)
        item = manifest["objects"][3]
        item["state"] = "EXECUTED_PASS"
        item["runtime_receipt"] = "sha256:abc"
        output = ROUTER.validate_manifest(self.contract, manifest)
        decision = next(
            value for value in output["decisions"] if value["object_id"] == "O-DEVICE"
        )
        self.assertEqual(decision["decision"], "ROUTED_LIMITED")

    def test_unknown_repository_rejected(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["objects"][1]["target_repository"] = "unknown/repo"
        self.invalid_manifest(manifest, "unknown target repository")

    def test_wrong_target_role_rejected(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["objects"][1]["target_repository"] = "rafaelmeloreisnovo/llamaRafaelia"
        self.invalid_manifest(manifest, "cannot target role MEMORY")

    def test_token_vazio_never_promotes(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["objects"][1]["state"] = "TOKEN_VAZIO"
        output = ROUTER.validate_manifest(self.contract, manifest)
        decision = next(
            value for value in output["decisions"] if value["object_id"] == "O-FORMULA"
        )
        self.assertEqual(decision["decision"], "BLOCKED_TOKEN_VAZIO")
        self.assertFalse(decision["claim_allowed"])

    def test_duplicate_authority_repository_rejected(self):
        contract = copy.deepcopy(self.contract)
        contract["authorities"][1]["repository"] = contract["authorities"][0]["repository"]
        self.invalid_contract(contract, "duplicate authority repository")

    def test_every_kind_requires_rule(self):
        contract = copy.deepcopy(self.contract)
        contract["route_rules"] = contract["route_rules"][:-1]
        self.invalid_contract(contract, "every object kind")

    def test_manifest_claim_allowed_rejected(self):
        manifest = copy.deepcopy(self.manifest)
        manifest["claim_allowed"] = True
        self.invalid_manifest(manifest, "claim_allowed must be false")


if __name__ == "__main__":
    unittest.main()
