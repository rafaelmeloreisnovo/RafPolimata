#!/usr/bin/env python3
"""Fail-closed validator for RAFAELIA session materialization 2026-10-05 V1."""
from __future__ import annotations
import argparse, copy, hashlib, json
from pathlib import Path
from typing import Any, Callable

ROOT=Path(__file__).resolve().parents[2]
DEFAULT=ROOT/"rafci"/"closures"/"session_materialization_20261005_v1.json"
SCHEMA="rafaelia.rafci.session-materialization/v1"
ID="RAFAELIA_SESSION_MATERIALIZATION_20261005_V1"
EVENT="RAFAELIA-SESSION-MATERIALIZATION-2026-10-05-V1"
TV="TOKEN_VAZIO"
STATES={"PASS","FAIL","PENDING","NOT_RUN","OBSERVED_UNPROMOTED",TV,"CONTRADICTION","OUT_OF_SCOPE","RECOMMENDATION"}
INVARIANTS={"SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM","TOKEN_VAZIO != 0","IMPLEMENTED_UNTESTED != PASS"}
GATES=["G0_ORIGIN","G1_CLAIM_BOUNDARY","G2_PRIOR_ART","G3_FORMAL_SPEC","G4_REFERENCE_MACHINE","G5_EQUIVALENCE","G6_MEASUREMENT","G7_PHYSICAL_SCOPE","G8_ROLLBACK","G9_PROMOTION"]
LOCKED={"C03":("FAIL",False),"C04":("FAIL",False),"C06":(TV,False),"C07":(TV,False),"C08":("NOT_RUN",False),"C09":("NOT_RUN",False),"C10":(TV,False),"C11":("PASS",True)}
UNRESOLVED={"world_novelty":TV,"patentability":TV,"turing_completeness":"NOT_RUN","performance_advantage":"NOT_RUN","energy_advantage":"NOT_RUN","physical_backend_equivalence":"NOT_RUN","asset_valuation":TV}

class ClosureError(RuntimeError): pass
def req(v: bool, m: str)->None:
    if not v: raise ClosureError(m)
def text(v: Any)->bool: return isinstance(v,str) and bool(v.strip())
def sha256(p: Path)->str:
    h=hashlib.sha256()
    with p.open("rb") as f:
        for b in iter(lambda:f.read(65536),b""): h.update(b)
    return h.hexdigest()

def validate(c: dict[str,Any])->None:
    req(c.get("schema")==SCHEMA,"schema drift")
    req(c.get("id")==ID,"id drift")
    req(c.get("event_id")==EVENT,"EVENT_ID drift")
    req(c.get("authority")=="rafaelmeloreisnovo/RafPolimata","authority drift")
    req(c.get("claim_allowed") is False,"session claim_allowed must remain false")
    req(c.get("automatic_promotion_forbidden") is True,"automatic promotion must remain forbidden")
    req(c.get("human_authorization_required") is True,"human authorization boundary drift")
    req(INVARIANTS.issubset(set(c.get("truth_invariants",[]))),"core invariant missing")
    req(set(c.get("allowed_states",[]))==STATES,"state set drift")
    b=c.get("bindings",{})
    req(b.get("evidence_garden")=="docs/EVIDENCE_GARDEN_V1.md","Evidence Garden binding drift")
    req(b.get("synaptic_closure")=="rafci/closures/sgpt_synaptic_closure_v1.json","synaptic closure binding drift")
    req(text(b.get("drive_document")) and text(b.get("drive_ledger")),"Drive custody anchors missing")

    chain=c.get("semantic_chain")
    req(isinstance(chain,list) and len(chain)>=2 and all(text(x) and "->" in x for x in chain),"semantic chain invalid")

    claims=c.get("locked_claims")
    req(isinstance(claims,dict) and set(claims)==set(LOCKED),"locked claim set drift")
    for cid,(state,allowed) in LOCKED.items():
        x=claims[cid]
        req(x.get("state")==state,f"{cid}: state drift")
        req(x.get("claim_allowed") is allowed,f"{cid}: promotion drift")
        req(text(x.get("falsifier")),f"{cid}: falsifier missing")
        if allowed:
            req(text(x.get("evidence")),f"{cid}: PASS claim requires evidence")

    rp=c.get("risk_policy",{})
    req(rp.get("rpn")=="severity*likelihood*detectability","RPN rule drift")
    req(set(rp.get("urgency",{}))=={"U0","U1","U2"},"urgency set drift")
    controls=rp.get("mandatory_controls")
    req(isinstance(controls,list) and len(controls)>=8 and all(text(x) for x in controls),"mandatory controls incomplete")

    req(c.get("gate_order")==GATES,"gate order drift")
    gates=c.get("gates")
    req(isinstance(gates,dict) and list(gates)==GATES,"gate map/order drift")
    req(all(v in STATES for v in gates.values()),"unsupported gate state")
    if gates["G9_PROMOTION"]=="PASS":
        req(all(gates[g]=="PASS" for g in GATES[:-1]),"promotion cannot PASS with open material gates")

    custody=c.get("custody")
    req(isinstance(custody,list) and [x.get("seq") for x in custody]==list(range(len(custody))),"custody sequence drift")
    for x in custody:
        req(x.get("state") in STATES,"custody state invalid")
        req(text(x.get("surface")) and text(x.get("class")) and text(x.get("receipt")),"custody field missing")
        if x.get("state")=="PASS": req(x.get("receipt")!=TV,"PASS custody cannot have TOKEN_VAZIO receipt")

    req(c.get("unresolved")==UNRESOLVED,"unresolved boundary drift")
    p=c.get("promotion",{})
    req(p.get("gate")=="G9_PROMOTION" and p.get("state")==TV and p.get("automatic") is False,"promotion block drift")
    req(p.get("human_authorization_required") is True,"human authorization drift")
    req(text(c.get("rollback")),"rollback missing")

def load(p: Path)->dict[str,Any]:
    x=json.loads(p.read_text(encoding="utf-8")); req(isinstance(x,dict),"root must be object"); return x

def reject(base:dict[str,Any], mutate:Callable[[dict[str,Any]],None], label:str)->None:
    x=copy.deepcopy(base); mutate(x)
    try: validate(x)
    except ClosureError: return
    raise ClosureError("selftest mutation not rejected: "+label)

def selftest(c:dict[str,Any])->list[str]:
    cases=[
      ("session-promotion",lambda x:x.__setitem__("claim_allowed",True)),
      ("novelty-promotion",lambda x:x["locked_claims"]["C06"].__setitem__("claim_allowed",True)),
      ("patentability-pass",lambda x:x["locked_claims"]["C07"].__setitem__("state","PASS")),
      ("pass-without-evidence",lambda x:x["locked_claims"]["C11"].__setitem__("evidence","")),
      ("missing-falsifier",lambda x:x["locked_claims"]["C03"].__setitem__("falsifier","")),
      ("open-gate-promotion",lambda x:x["gates"].__setitem__("G9_PROMOTION","PASS")),
      ("unknown-to-pass",lambda x:x["unresolved"].__setitem__("world_novelty","PASS")),
      ("event-drift",lambda x:x.__setitem__("event_id","OTHER")),
      ("missing-invariant",lambda x:x["truth_invariants"].remove("TOKEN_VAZIO != 0")),
      ("pass-custody-empty",lambda x:(x["custody"][4].__setitem__("state","PASS"),x["custody"][4].__setitem__("receipt",TV))),
    ]
    out=[]
    for label,mut in cases: reject(c,mut,label); out.append(label)
    return out

def main()->int:
    ap=argparse.ArgumentParser(); ap.add_argument("--closure",type=Path,default=DEFAULT); ap.add_argument("--selftest",action="store_true")
    ns=ap.parse_args(); p=ns.closure if ns.closure.is_absolute() else ROOT/ns.closure
    c=load(p); validate(c); rejected=selftest(c) if ns.selftest else []
    print(json.dumps({"state":"PASS","scope":"STRUCTURAL_CONTRACT_ONLY","event_id":EVENT,"closure_sha256":sha256(p),"selftest_rejected_mutations":rejected,"promotion_allowed":False},ensure_ascii=False,sort_keys=True))
    return 0
if __name__=="__main__":
    try: raise SystemExit(main())
    except ClosureError as e:
        print(json.dumps({"state":"FAIL","reason":str(e),"promotion_allowed":False},ensure_ascii=False,sort_keys=True))
        raise SystemExit(1)
