#!/usr/bin/env python3
from __future__ import annotations
import argparse, copy, hashlib, json, os, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SPEC = ROOT / "rafci" / "pathways.v1.json"
CEIL = {
 "AUTHORIAL_FREESTANDING_CORE":"FREESTANDING_STRUCTURAL",
 "HOSTED_NATIVE_TOOLCHAIN_ADAPTER":"ANDROID_NATIVE_HOSTED",
 "MANAGED_NATIVE_BOUNDARY":"MANAGED_NATIVE_BOUNDARY",
 "MANAGED_RUNTIME_COMPILER":"MANAGED_RUNTIME",
 "HOSTED_PACKAGING_TOOLCHAIN":"PACKAGING_TRANSFORM",
 "HOSTED_DEX_TRANSFORM":"DEX_TRANSFORM",
 "PHYSICAL_USERSPACE_RUNTIME":"PHYSICAL_USERSPACE",
 "EMULATED_RUNTIME":"EMULATED_RUNTIME",
 "PHYSICAL_BAREMETAL_RUNTIME":"PHYSICAL_BAREMETAL",
}
IDS={"freestanding_native","android_ndk","jni_bridge","art_jit_aot","android_sdk_buildtools","r8_dex_optimizer","physical_termux_native","qemu_emulated","baremetal_firmware"}
NON_GATE={"infer","analyze","condition","doubt","catalyze","heuristic"}

class E(RuntimeError): pass
def req(v,m):
    if not v: raise E(m)
def uniq(xs,m): req(len(xs)==len(set(xs)),f"duplicate {m}")
def load(): return json.loads(SPEC.read_text())
def validate(s):
    req(s.get("schema")=="rafaelia.rafci.pathways/v1","schema")
    req(s.get("authority")=="authority.rafpolimata.rafci","authority")
    req(s.get("repository")=="rafaelmeloreisnovo/RafPolimata","repository")
    req(s.get("claim_allowed") is False,"claim_allowed")
    req(s.get("unknown")=="TOKEN_VAZIO" and not isinstance(s.get("unknown"),(int,float)),"TOKEN_VAZIO")
    req(s.get("selection_law")=="ADMISSIBILITY_THEN_PARETO","selection")
    req(s.get("weighted_score_policy")=="FORBIDDEN_UNTIL_OBJECTIVE_AND_WEIGHTS_ARE_PREREGISTERED","weights")
    req(s.get("incomparable_state")=="INCOMPARABLE","incomparable")
    keys=s.get("comparison_keys",[]); uniq(keys,"comparison key")
    req(set(("workload_id","semantics_digest","protocol_version","artifact_role","hardware_class","runtime_state")).issubset(keys),"comparison keys")
    m=s.get("measurement_policy",{})
    req(isinstance(m.get("timing_min_samples"),int) and m["timing_min_samples"]>=31,"sample floor")
    req(m.get("timing_recommended_samples",0)>=m["timing_min_samples"],"recommended samples")
    req(m.get("missing_sensor_policy")=="TOKEN_VAZIO","missing sensors")
    for x in ("energy_claim_requires_measured_energy","pmu_claim_requires_measured_pmu","thermal_claim_requires_measured_thermal"): req(m.get(x) is True,x)
    req(set(("median","p95","p99","mad","min","max","failures")).issubset(m.get("required_distribution_fields",[])),"distribution")
    ops=s.get("epistemic_operations",[]); uniq([o.get("id") for o in ops],"operation")
    for o in ops:
        req(isinstance(o.get("can_close_gate"),bool),f"operation {o.get('id')}")
        if o.get("id") in NON_GATE: req(o["can_close_gate"] is False,f"non-gate promoted {o['id']}")
    ps=s.get("pathways",[]); uniq([p.get("id") for p in ps],"pathway id"); uniq([p.get("class") for p in ps],"pathway class")
    req({p.get("id") for p in ps}==IDS,"pathway set")
    by={p["id"]:p for p in ps}
    for p in ps:
        req(p.get("class") in CEIL and p.get("claim_ceiling")==CEIL[p["class"]],f"ceiling {p.get('id')}")
        req(bool(p.get("boundary")) and bool(p.get("comparability_group")),"boundary/group")
        req(bool(p.get("required_gates")) and bool(p.get("required_measurements")),"gates/measurements")
        req(isinstance(p.get("forbidden_promotions"),list),"forbidden promotions")
    req("FREESTANDING_STRUCTURAL" in by["android_ndk"]["forbidden_promotions"],"NDK->freestanding")
    req("PHYSICAL_BAREMETAL" in by["physical_termux_native"]["forbidden_promotions"],"userspace->baremetal")
    req("PHYSICAL_USERSPACE" in by["qemu_emulated"]["forbidden_promotions"],"QEMU->physical")
    req("RUNTIME_PASS" in by["android_sdk_buildtools"]["forbidden_promotions"],"SDK->runtime")
    req("NATIVE_RUNTIME_PASS" in by["r8_dex_optimizer"]["forbidden_promotions"],"R8->runtime")
    req("runtime_state" in by["art_jit_aot"]["required_measurements"],"JIT state")
    fs=s.get("falsifiers",[]); uniq([f.get("id") for f in fs],"falsifier"); req(len(fs)>=12,"falsifiers")
    rules={f.get("rule"):f.get("result") for f in fs}
    expected={
     "TOKEN_VAZIO_AS_ZERO_OR_PASS":"FAIL","HOSTED_OR_EMULATED_AS_PHYSICAL":"FAIL",
     "PHYSICAL_USERSPACE_AS_BAREMETAL":"FAIL","HEAD_HASH_ABI_MISMATCH":"FAIL",
     "TRANSFORM_WITHOUT_PROVENANCE":"FAIL","TIMING_BELOW_SAMPLE_FLOOR":"NO_PRECISION_CLAIM",
     "JIT_COLD_WARM_PROFILE_COLLAPSED":"INCOMPARABLE","CROSS_HARDWARE_WITHOUT_MATCHED_PROTOCOL":"INCOMPARABLE",
     "R8_OR_SDK_PROMOTED_TO_RUNTIME":"FAIL","NDK_NATIVE_PROMOTED_TO_FREESTANDING":"FAIL",
     "HEURISTIC_PROMOTED_TO_GATE_EVIDENCE":"FAIL","UNPREREGISTERED_WEIGHTED_WINNER_SCORE":"FAIL"}
    req(rules==expected,"falsifier dictionary")
    for k,v in s.get("operational_method_mapping",{}).items():
        req("NOT_CERTIFICATION" in v or "NOT_IETF_CONFORMANCE" in v,f"method overclaim {k}")
def reject(s,fn,label):
    x=copy.deepcopy(s); fn(x)
    try: validate(x)
    except E: return
    raise E(f"selftest not rejected: {label}")
def selftest(s):
    reject(s,lambda x:x.__setitem__("unknown",0),"empty->zero")
    reject(s,lambda x:x.__setitem__("claim_allowed",True),"claim")
    reject(s,lambda x:next(p for p in x["pathways"] if p["id"]=="android_ndk").__setitem__("claim_ceiling","FREESTANDING_STRUCTURAL"),"NDK")
    reject(s,lambda x:next(p for p in x["pathways"] if p["id"]=="physical_termux_native").__setitem__("claim_ceiling","PHYSICAL_BAREMETAL"),"baremetal")
    reject(s,lambda x:next(p for p in x["pathways"] if p["id"]=="qemu_emulated")["forbidden_promotions"].remove("PHYSICAL_USERSPACE"),"QEMU")
    reject(s,lambda x:next(p for p in x["pathways"] if p["id"]=="art_jit_aot")["required_measurements"].remove("runtime_state"),"JIT")
    reject(s,lambda x:x.__setitem__("weighted_score_policy","ALLOW"),"weights")
    reject(s,lambda x:next(o for o in x["epistemic_operations"] if o["id"]=="heuristic").__setitem__("can_close_gate",True),"heuristic")
def receipt(path,s):
    path.parent.mkdir(parents=True,exist_ok=True)
    r={"schema":"rafaelia.rafci.pathway-contract-receipt/v1","repository":"rafaelmeloreisnovo/RafPolimata",
       "commit":os.environ.get("RAFCI_SOURCE_SHA") or os.environ.get("GITHUB_SHA","TOKEN_VAZIO"),
       "pathways":len(s["pathways"]),"falsifiers":len(s["falsifiers"]),"selection_law":s["selection_law"],
       "contract_sha256":hashlib.sha256(SPEC.read_bytes()).hexdigest(),"contract_gate":"PASS",
       "benchmark_execution":"TOKEN_VAZIO","physical_execution":"TOKEN_VAZIO","baremetal_execution":"TOKEN_VAZIO","claim_allowed":False}
    path.write_text(json.dumps(r,indent=2,sort_keys=True)+"\n")
def main():
    a=argparse.ArgumentParser(); a.add_argument("--selftest",action="store_true"); a.add_argument("--receipt",type=Path); z=a.parse_args()
    try:
        s=load(); validate(s)
        if z.selftest:selftest(s)
        if z.receipt:receipt(z.receipt,s)
    except (OSError,json.JSONDecodeError,E) as e:
        print(f"RAFCI_PATHWAY_FAIL: {e}",file=sys.stderr); return 1
    print(f"RAFCI_PATHWAY_PASS pathways={len(s['pathways'])} falsifiers={len(s['falsifiers'])}"); return 0
if __name__=="__main__": raise SystemExit(main())
