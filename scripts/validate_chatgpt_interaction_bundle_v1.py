#!/usr/bin/env python3
from __future__ import annotations
import argparse, json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
DEFAULT_CONTRACT=ROOT/"data/contracts/chatgpt_interaction_bundle.v1.json"
DEFAULT_BUNDLE=ROOT/"data/evidence/chatgpt/2026-10-20_three_interaction_bundle.v1.json"

def load(p): return json.loads(Path(p).read_text(encoding="utf-8"))

def validate(bundle, contract):
    errors=[]
    if bundle.get("schema")!="rafaelia.chatgpt-interaction-bundle/v1": errors.append("unexpected bundle schema")
    if bundle.get("interaction_count")!=contract["required_interaction_count"]: errors.append("interaction_count must equal 3")
    if len(bundle.get("interactions",[]))!=3: errors.append("exactly three interaction records required")
    if bundle.get("source_order_preserved") is not True: errors.append("source order must be preserved")
    if bundle.get("semantic_merge_as_one_interaction") is not True: errors.append("semantic merge flag required")
    if bundle.get("training_use") is not False: errors.append("training_use must be false")
    if bundle.get("dataset_use") is not False: errors.append("dataset_use must be false")
    if bundle.get("operational_evidence_use") is not True: errors.append("operational_evidence_use must be true")
    ids=[x.get("interaction_id") for x in bundle.get("interactions",[])]
    if ids!=["I1","I2","I3"]: errors.append("interaction order must be I1,I2,I3")
    if any(not x.get("prompt_text") or not x.get("response_capture") for x in bundle.get("interactions",[])):
        errors.append("every interaction requires prompt and response capture")
    urg={x.get("id") for x in bundle.get("urgency",[])}
    for req in contract["required_urgency_ids"]:
        if req not in urg: errors.append(f"missing urgency: {req}")
    gates=set(bundle.get("gates",[]))
    for req in contract["required_gates"]:
        if req not in gates: errors.append(f"missing gate: {req}")
    p0=bundle.get("convergence",{}).get("priority_p0_operator_ids",[])
    if len(p0)!=len(set(p0)) or any(not str(x).startswith("OP") for x in p0):
        errors.append("P0 operator IDs must be unique OP identifiers")
    for tv in bundle.get("token_vazios",[]):
        if not tv.get("id","").startswith("TOKEN_VAZIO_") or not tv.get("type") or not tv.get("closure"):
            errors.append("typed TOKEN_VAZIO entries require id/type/closure")
    if bundle.get("claim_allowed") is not False or bundle.get("promotion_allowed") is not False:
        errors.append("bundle cannot self-promote")
    return errors

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--bundle",type=Path,default=DEFAULT_BUNDLE)
    ap.add_argument("--contract",type=Path,default=DEFAULT_CONTRACT)
    a=ap.parse_args()
    errors=validate(load(a.bundle),load(a.contract))
    print(json.dumps({"schema":"rafaelia.chatgpt-interaction-bundle.validation/v1","state":"PASS" if not errors else "FAIL","errors":errors,"claim_allowed":False},indent=2))
    return 0 if not errors else 1
if __name__=="__main__": raise SystemExit(main())
