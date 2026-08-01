import copy, hashlib, json, pathlib, tempfile, unittest
from scripts.validate_rafaelia_handoff import validate, ValidationError

BASE={
 "schema_version":"1.0.0","artifact_id":"ARTIFACT-0001","producer":"RafPolimata",
 "source_commit":"0"*40,"artifact":{"path":"artifact.bin","format":"OTHER","size_bytes":3},
 "hashes":{"sha256":hashlib.sha256(b"abc").hexdigest()},
 "target":{"runtime":"Vectras-VM-Android","abi":"armeabi-v7a"},
 "dependencies":[],"limits":{"timeout_seconds":30,"memory_mb":64,"network_allowed":False},
 "rollback":{"strategy":"abort_only","previous_artifact_sha256":"TOKEN_VAZIO"},
 "claim_allowed":False,"epistemic_state":"EVIDENCIADO"
}

class HandoffTests(unittest.TestCase):
 def test_valid_with_artifact(self):
  with tempfile.TemporaryDirectory() as d:
   pathlib.Path(d,"artifact.bin").write_bytes(b"abc")
   self.assertIn("artifact_sha256_verified",validate(copy.deepcopy(BASE),pathlib.Path(d)))
 def test_reject_claim_promotion(self):
  x=copy.deepcopy(BASE); x["claim_allowed"]=True
  with self.assertRaises(ValidationError): validate(x)
 def test_reject_hash_mismatch(self):
  with tempfile.TemporaryDirectory() as d:
   pathlib.Path(d,"artifact.bin").write_bytes(b"xyz")
   with self.assertRaises(ValidationError): validate(copy.deepcopy(BASE),pathlib.Path(d))
 def test_reject_network(self):
  x=copy.deepcopy(BASE); x["limits"]["network_allowed"]=True
  with self.assertRaises(ValidationError): validate(x)
 def test_reject_unknown_field(self):
  x=copy.deepcopy(BASE); x["surprise"]="unsafe"
  with self.assertRaises(ValidationError): validate(x)
 def test_reject_path_escape(self):
  x=copy.deepcopy(BASE); x["artifact"]["path"]="../artifact.bin"
  with tempfile.TemporaryDirectory() as d:
   with self.assertRaises(ValidationError): validate(x,pathlib.Path(d))

if __name__=="__main__": unittest.main()
