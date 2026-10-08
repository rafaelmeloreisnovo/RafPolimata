import copy
import json
import unittest
from pathlib import Path
from scripts.latin.build_latin_artifact import build, validate

ROOT=Path(__file__).resolve().parents[2]
SEED=json.loads((ROOT/"research/LATIN/seed.v1.json").read_text(encoding="utf-8"))

class LatinGraphTest(unittest.TestCase):
    def test_valid_seed(self):
        self.assertTrue(validate(SEED))
    def test_build_is_deterministic(self):
        a=build(SEED,"a"*40); b=build(SEED,"a"*40)
        self.assertEqual(a,b)
        self.assertFalse(a["claim_allowed"])
    def test_unauthorized_claim(self):
        s=copy.deepcopy(SEED); s["claim_allowed"]=True
        with self.assertRaises(ValueError):validate(s)
    def test_forbidden_passage(self):
        s=copy.deepcopy(SEED); s["nodes"][0]["passage"]="secret"
        with self.assertRaises(ValueError):validate(s)
    def test_dangling_edge(self):
        s=copy.deepcopy(SEED); s["edges"][0]["to"]="absent"
        with self.assertRaises(ValueError):validate(s)
    def test_translation_gate(self):
        s=copy.deepcopy(SEED); s["cultural_translation_state"]="PASS"
        with self.assertRaises(ValueError):validate(s)
    def test_fake_evidence(self):
        s=copy.deepcopy(SEED); s["edges"][0]["evidence_ref"]="proof"
        with self.assertRaises(ValueError):validate(s)
    def test_duplicate_node(self):
        s=copy.deepcopy(SEED); s["nodes"].append(copy.deepcopy(s["nodes"][0]))
        with self.assertRaises(ValueError):validate(s)

if __name__=="__main__":
    unittest.main()
