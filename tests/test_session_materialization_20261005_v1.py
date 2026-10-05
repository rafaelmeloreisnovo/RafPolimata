from __future__ import annotations
import copy, importlib.util, json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
TOOL=ROOT/"rafci/tools/verify_session_materialization_20261005_v1.py"
CLOSURE=ROOT/"rafci/closures/session_materialization_20261005_v1.json"
spec=importlib.util.spec_from_file_location("v",TOOL); v=importlib.util.module_from_spec(spec); spec.loader.exec_module(v)
class ContractTests(unittest.TestCase):
    def setUp(self): self.base=json.loads(CLOSURE.read_text(encoding="utf-8"))
    def bad(self,mut):
        x=copy.deepcopy(self.base); mut(x)
        with self.assertRaises(v.ClosureError): v.validate(x)
    def test_source(self): v.validate(self.base)
    def test_selftest(self): self.assertEqual(len(v.selftest(self.base)),10)
    def test_token_vazio_not_promotable(self): self.bad(lambda x:x["locked_claims"]["C06"].__setitem__("claim_allowed",True))
    def test_pass_needs_evidence(self): self.bad(lambda x:x["locked_claims"]["C11"].__setitem__("evidence",""))
    def test_falsifier_required(self): self.bad(lambda x:x["locked_claims"]["C03"].__setitem__("falsifier",""))
    def test_open_gates_block_promotion(self): self.bad(lambda x:x["gates"].__setitem__("G9_PROMOTION","PASS"))
    def test_unknown_stays_unknown(self): self.bad(lambda x:x["unresolved"].__setitem__("world_novelty","PASS"))
    def test_event_id_locked(self): self.bad(lambda x:x.__setitem__("event_id","OTHER"))
    def test_invariant_locked(self): self.bad(lambda x:x["truth_invariants"].remove("TOKEN_VAZIO != 0"))
    def test_pass_custody_needs_receipt(self):
        def mut(x): x["custody"][4].update({"state":"PASS","receipt":v.TV})
        self.bad(mut)
if __name__=="__main__": unittest.main()
