#!/usr/bin/env python3
import argparse,json,pathlib,sys
P=argparse.ArgumentParser();P.add_argument("--contract",required=True);P.add_argument("--envelope");a=P.parse_args()
c=json.loads(pathlib.Path(a.contract).read_text());errors=[];req=c["required_fields"]
def validate(r):
 e=[]
 for k in req:
  if r.get(k) in (None,""):e.append("missing:"+k)
 if r.get("result_state") not in c["allowed_states"]:e.append("invalid_state")
 if r.get("result_state")=="PASS" and any(r.get(k) in (None,"") for k in req):e.append("false_pass")
 if r.get("performance_claim"):
  for k in c["benchmark_required_when_claimed"]:
   if r.get(k) in (None,""):e.append("benchmark_missing:"+k)
 return e
for t in c["self_test_cases"]:
 got="PASS" if not validate(t["record"]) else "FAIL"
 if got!=t["expect"]:errors.append("self-test:"+t["name"]+":"+got)
if a.envelope:errors.extend(validate(json.loads(pathlib.Path(a.envelope).read_text())))
print(json.dumps({"state":"PASS_LIMITED" if not errors else "FAIL","errors":errors,"claim_allowed":False},indent=2))
sys.exit(1 if errors else 0)
