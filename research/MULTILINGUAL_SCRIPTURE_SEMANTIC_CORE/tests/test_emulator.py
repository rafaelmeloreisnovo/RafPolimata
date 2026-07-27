from __future__ import annotations

import copy
import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("mssc_emulator", ROOT / "core/emulator.py")
assert SPEC and SPEC.loader
EMULATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EMULATOR)


def load(name: str):
    return json.loads((ROOT / name).read_text(encoding="utf-8"))


class MultilingualScriptureCoreTests(unittest.TestCase):
    def setUp(self):
        self.config = load("core_config.v1.json")
        self.fixture = load("fixtures/demo_units.v1.json")

    def test_emulation_passes_without_activation(self):
        receipt = EMULATOR.build_receipt(self.config, self.fixture)
        self.assertEqual(receipt["state"], "PASS_WITH_TOKEN_VAZIO")
        self.assertFalse(receipt["enabled"])
        self.assertFalse(receipt["claim_allowed"])
        self.assertEqual(receipt["language_registry"], ["arc", "grc", "hbo", "pt-BR"])
        self.assertEqual(receipt["unit_count"], 6)
        self.assertEqual(receipt["relation_count"], 4)

    def test_receipt_is_deterministic(self):
        first = EMULATOR.build_receipt(self.config, self.fixture)
        second = EMULATOR.build_receipt(self.config, self.fixture)
        self.assertEqual(first, second)
        self.assertEqual(first["receipt_sha256"], second["receipt_sha256"])

    def test_core_cannot_be_activated(self):
        modified = copy.deepcopy(self.config)
        modified["enabled"] = True
        with self.assertRaisesRegex(AssertionError, "must remain disabled"):
            EMULATOR.build_receipt(modified, self.fixture)

    def test_authorial_intent_claim_is_blocked(self):
        modified = copy.deepcopy(self.fixture)
        modified["relations"][3]["authorial_intent_claimed"] = True
        with self.assertRaisesRegex(AssertionError, "authorial intent claim forbidden"):
            EMULATOR.build_receipt(self.config, modified)

    def test_aramaic_requires_dialect(self):
        modified = copy.deepcopy(self.fixture)
        aramaic = next(unit for unit in modified["units"] if unit["language_tag"] == "arc")
        aramaic["grammar"]["dialect"] = "TOKEN_VAZIO_DIALECT"
        with self.assertRaisesRegex(AssertionError, "Aramaic dialect required"):
            EMULATOR.build_receipt(self.config, modified)

    def test_script_mismatch_is_blocked(self):
        modified = copy.deepcopy(self.fixture)
        greek = next(unit for unit in modified["units"] if unit["language_tag"] == "grc")
        greek["script"] = "Latn"
        with self.assertRaisesRegex(AssertionError, "configured script mismatch"):
            EMULATOR.build_receipt(self.config, modified)

    def test_nfc_is_idempotent(self):
        sample = "ἀρχῇ"
        self.assertEqual(EMULATOR.normalize_nfc(sample), EMULATOR.normalize_nfc(EMULATOR.normalize_nfc(sample)))


if __name__ == "__main__":
    unittest.main()
