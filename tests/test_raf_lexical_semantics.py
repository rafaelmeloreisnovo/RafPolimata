from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "raf_lexical_semantics.py"
REGISTRY = ROOT / "data" / "semantics" / "lexemes.seed.v1.jsonl"
SPEC = importlib.util.spec_from_file_location("raf_lexical_semantics", SCRIPT)
assert SPEC and SPEC.loader
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class LexicalSemanticTests(unittest.TestCase):
    def records(self):
        return module.load_jsonl(REGISTRY)

    def test_seed_registry_passes(self):
        receipt = module.validate_registry(REGISTRY)
        self.assertEqual(receipt["state"], "PASS_LEXICAL_SEMANTICS_AND_DECLARED_PHONEMES")
        self.assertEqual(receipt["record_count"], 6)
        self.assertFalse(receipt["automatic_grapheme_to_phoneme"])
        self.assertFalse(receipt["claim_allowed"])

    def test_unicode_normalization_is_deterministic(self):
        self.assertEqual(module.normalize_surface("  SEMÂNTICA  "), "semântica")
        self.assertEqual(module.normalize_surface("sema\u0302ntica"), "semântica")

    def test_stress_must_reference_a_syllable(self):
        record = copy.deepcopy(self.records()[0])
        record["phonology"]["stress_index"] = 7
        with self.assertRaises(module.LexicalError):
            module.validate_record(record)

    def test_token_vazio_cannot_carry_invented_ipa(self):
        record = copy.deepcopy(self.records()[0])
        record["phonology"]["state"] = "TOKEN_VAZIO_PHONEME_NOT_DECLARED"
        with self.assertRaises(module.LexicalError):
            module.validate_record(record)

    def test_claim_true_is_rejected(self):
        record = copy.deepcopy(self.records()[0])
        record["claim_allowed"] = True
        with self.assertRaises(module.LexicalError):
            module.validate_record(record)

    def test_duplicate_form_fails_closed(self):
        records = self.records()
        duplicate = copy.deepcopy(records[0])
        duplicate["id"] = "lex-luz-duplicate"
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "dup.jsonl"
            path.write_text("\n".join(json.dumps(item, ensure_ascii=False) for item in records + [duplicate]) + "\n", encoding="utf-8")
            with self.assertRaises(module.LexicalError):
                module.validate_registry(path)


if __name__ == "__main__":
    unittest.main()
