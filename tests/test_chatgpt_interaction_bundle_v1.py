import copy, importlib.util, json
from pathlib import Path
import unittest

GOVERNING_CLOSURE="CLOSURE_L13"
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/"scripts/validate_chatgpt_interaction_bundle_v1.py"
spec=importlib.util.spec_from_file_location("bundle_validator",P)
m=importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
C=json.loads((ROOT/"data/contracts/chatgpt_interaction_bundle.v1.json").read_text())
B=json.loads((ROOT/"data/evidence/chatgpt/2026-10-20_three_interaction_bundle.v1.json").read_text())

class T(unittest.TestCase):
    def test_current_bundle_passes(self): self.assertEqual(m.validate(B,C),[])
    def test_training_rejected(self):
        b=copy.deepcopy(B); b["training_use"]=True
        self.assertIn("training_use must be false",m.validate(b,C))
    def test_missing_response_rejected(self):
        b=copy.deepcopy(B); b["interactions"][1]["response_capture"]=""
        self.assertIn("every interaction requires prompt and response capture",m.validate(b,C))
    def test_order_rejected(self):
        b=copy.deepcopy(B); b["interactions"][0],b["interactions"][1]=b["interactions"][1],b["interactions"][0]
        self.assertIn("interaction order must be I1,I2,I3",m.validate(b,C))
    def test_untyped_gap_rejected(self):
        b=copy.deepcopy(B); b["typed_gaps"][0]["closure"]=""
        self.assertIn("typed governance-gap entries require id/type/closure",m.validate(b,C))
    def test_self_promotion_rejected(self):
        b=copy.deepcopy(B); b["promotion_allowed"]=True
        self.assertIn("bundle cannot self-promote",m.validate(b,C))
if __name__=="__main__": unittest.main()
