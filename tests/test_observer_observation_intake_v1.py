import copy,importlib.util,json
from pathlib import Path
import unittest
GOVERNING_CLOSURE="CLOSURE_L13"
ROOT=Path(__file__).resolve().parents[1]
P=ROOT/"scripts/validate_observer_observation_intake_v1.py"
spec=importlib.util.spec_from_file_location("v",P); m=importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
C=json.loads((ROOT/"data/contracts/observer_observation_intake.v1.json").read_text())
E=json.loads((ROOT/"data/evidence/chatgpt/2026-10-20_observer_observation_intake.v1.json").read_text())
class T(unittest.TestCase):
 def test_current(self): self.assertEqual(m.validate(C,E),[])
 def test_collision(self):
  x=copy.deepcopy(E); x["observer_id"]=x["observation_id"]; self.assertIn("observer_observation_collision",m.validate(C,x))
 def test_primary_route_required(self):
  x=copy.deepcopy(E); x["science_routes"]=[{"id":"SCI-NETWORK","role":"SECONDARY"}]; self.assertIn("primary_science_route",m.validate(C,x))
 def test_unknown_requires_gap(self):
  x=copy.deepcopy(E); x["epistemic_state"]="UNKNOWN"; x["typed_gaps"]=[]; self.assertIn("unknown_without_gap",m.validate(C,x))
 def test_new_cannot_self_promote(self):
  x=copy.deepcopy(E); x["result_state"]="PASS"; self.assertIn("nonpromotion_state",m.validate(C,x))
 def test_raw_private_text_blocked(self):
  x=copy.deepcopy(E); x["raw_private_session_text_stored"]=True; self.assertIn("raw_private_text_boundary",m.validate(C,x))
if __name__=="__main__": unittest.main()
