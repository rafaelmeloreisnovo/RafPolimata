#!/usr/bin/env python3
"""CLOSURE_L13 observer/observation intake validator."""
from __future__ import annotations
import argparse,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
C=ROOT/"data/contracts/observer_observation_intake.v1.json"
E=ROOT/"data/evidence/chatgpt/2026-10-20_observer_observation_intake.v1.json"
def load(p): return json.loads(Path(p).read_text(encoding="utf-8"))
def validate(c,e):
  errors=[]
  if e.get("observer_id")==e.get("observation_id"): errors.append("observer_observation_collision")
  if e.get("epistemic_state") not in c.get("accepted_states",[]): errors.append("state")
  for x in e.get("conduct",[]):
    if x not in c.get("accepted_conduct",[]): errors.append(f"conduct:{x}")
  routes=e.get("science_routes",[])
  if not routes or not any(x.get("role")=="PRIMARY" for x in routes): errors.append("primary_science_route")
  if e.get("epistemic_state") in {"UNCERTAIN","UNKNOWN"} and not e.get("typed_gaps"): errors.append("unknown_without_gap")
  for g in e.get("typed_gaps",[]):
    if not str(g.get("id","")).startswith(("TOKEN_VAZIO_GOVERNANCE_","TOKEN_VAZIO_REPRODUCTION_")) or not g.get("type") or not g.get("closure"):
      errors.append("untyped_gap")
  if e.get("raw_private_session_text_stored") is not False: errors.append("raw_private_text_boundary")
  if e.get("claim_allowed") is not False: errors.append("self_promotion")
  if e.get("epistemic_state") in {"NEW_OBSERVED","OPPORTUNITY","UNCERTAIN","UNKNOWN","ANOMALY"} and e.get("result_state")!="INTAKE_ACCEPTED_NON_PROMOTING":
    errors.append("nonpromotion_state")
  return errors
def main():
  ap=argparse.ArgumentParser(); ap.add_argument("--contract",type=Path,default=C); ap.add_argument("--envelope",type=Path,default=E); a=ap.parse_args()
  errs=validate(load(a.contract),load(a.envelope))
  print(json.dumps({"schema":"rafaelia.observer-observation-intake.validation/v1","state":"PASS" if not errs else "FAIL","errors":errs,"claim_allowed":False},indent=2))
  return 0 if not errs else 1
if __name__=="__main__": raise SystemExit(main())
