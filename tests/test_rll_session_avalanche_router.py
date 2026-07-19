import importlib.util, json, sys
from pathlib import Path
import unittest
ROOT=Path(__file__).parents[1]
SPEC=importlib.util.spec_from_file_location("router",ROOT/"scripts/rll_session_avalanche_router.py")
m=importlib.util.module_from_spec(SPEC); sys.modules[SPEC.name]=m; assert SPEC.loader; SPEC.loader.exec_module(m)
C=json.loads((ROOT/"configs/rll-session-avalanche-route-v1.json").read_text())
class T(unittest.TestCase):
 def test_valid(self): m.validate_config(C)
 def test_authority(self): self.assertEqual(C["authorities"]["physics"],"instituto-Rafael/relativity-living-light")
 def test_scalar(self): self.assertEqual(m.select_backends(set()),("scalar",))
 def test_selection(self): self.assertEqual(m.select_backends({"gpu","simd"}),("scalar","simd","gpu"))
 def test_finite(self):
  p=m.compile_plan(C,{"simd","gpu"}); self.assertLessEqual(p["job_count"],C["scale_policy"]["max_jobs"]); self.assertGreater(p["job_count"],0)
 def test_digest(self): self.assertEqual(m.compile_plan(C,{"simd"})["digest_sha256"],m.compile_plan(C,{"simd"})["digest_sha256"])
 def test_claim(self):
  p=m.compile_plan(C,{"gpu"}); self.assertFalse(p["claim_allowed"]); self.assertTrue(all(not j["claim_allowed"] for j in p["jobs"]))
 def test_no_auto(self):
  p=m.compile_plan(C,{"gpu","dsp"}); self.assertTrue(all(not j["automatic_cross_repo_write"] and not j["automatic_merge"] for j in p["jobs"]))
 def test_stages(self): self.assertEqual(tuple(m.compile_plan(C,set())["stages"]),m.STAGES)
 def test_bad_authority(self):
  x=json.loads(json.dumps(C)); x["authorities"]["physics"]="wrong/repo"
  with self.assertRaises(ValueError): m.validate_config(x)
 def test_npu_prune(self):
  p=m.compile_plan(C,{"npu"}); self.assertFalse(any(j["backend"]=="npu" and j["thermal_regime"]=="fully_ionized" for j in p["jobs"]))
 def test_gates(self):
  req={g["id"] for g in C["gates"] if g["required"]}; p=m.compile_plan(C,set())
  self.assertTrue(all(set(j["required_gates"])==req for j in p["jobs"]))
if __name__=="__main__": unittest.main()
