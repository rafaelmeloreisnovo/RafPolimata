#!/usr/bin/env python3
"""Static closure audit for Evidence Garden known falsifier domain."""
# Governance anchor: CLOSURE_L2.
import argparse, itertools, json, pathlib, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT = ROOT / "Benchmark" / "evidence_garden" / "falsifier_matrix.v2.json"
STATIONS = {f"S{i}_{name}" for i,name in enumerate(["identity","correctness","observability","performance","intervention","statistics","reproduction","claim_gate"])}


def audit(matrix):
    errors=[]
    if matrix.get("schema")!="rafpolimata.evidence-garden.falsifier-matrix.v2":
        errors.append("schema")
    if matrix.get("claim_allowed") is not False:
        errors.append("claim_allowed_must_be_false")
    dims=matrix.get("void_dimensions",[])
    ids=[d.get("id") for d in dims if isinstance(d,dict)]
    if not ids or len(ids)!=len(set(ids)) or any(not x for x in ids):
        errors.append("void_dimension_ids")
    dimset=set(ids)
    fals=matrix.get("falsifiers",[])
    fids=[f.get("id") for f in fals if isinstance(f,dict)]
    if len(fids)!=len(set(fids)) or any(not x for x in fids):
        errors.append("falsifier_ids")
    covered=set()
    station_covered=set()
    for f in fals:
        ds=f.get("dimensions",[])
        if not ds or any(d not in dimset for d in ds):
            errors.append(f"falsifier_dimensions:{f.get('id')}")
        covered.update(ds)
        station_covered.add(f.get("station"))
        if not f.get("expected") or not f.get("kind"):
            errors.append(f"falsifier_contract:{f.get('id')}")
    missing_dims=sorted(dimset-covered)
    if missing_dims:
        errors.append("uncovered_dimensions:"+",".join(missing_dims))
    declared_stations={d.get("station") for d in dims}
    missing_stations=sorted(s for s in declared_stations if s not in station_covered)
    if missing_stations:
        errors.append("uncovered_stations:"+",".join(missing_stations))
    pairs=list(itertools.combinations(sorted(dimset),2))
    expected=matrix.get("pairwise_rule",{}).get("expected_count")
    if expected!=len(pairs):
        errors.append(f"pairwise_count:{expected}!={len(pairs)}")
    principle=matrix.get("principle",{})
    if principle.get("open_world_state")!="TOKEN_VAZIO":
        errors.append("open_world_must_remain_TOKEN_VAZIO")
    stop=matrix.get("stop_rule",{})
    if stop.get("universal_stop")!="FORBIDDEN":
        errors.append("universal_stop_must_be_FORBIDDEN")
    return {
        "schema":"rafpolimata.evidence-garden.falsifier-audit.v1",
        "state":"PASS" if not errors else "FAIL",
        "claim_allowed":False,
        "dimension_count":len(dimset),
        "falsifier_count":len(fals),
        "station_count":len(station_covered),
        "pairwise_permutation_count":len(pairs),
        "known_falsifier_closure":"PASS" if not errors else "FAIL",
        "open_world_unknown_unknown":"TOKEN_VAZIO",
        "errors":errors,
        "pairwise_permutations":[{"a":a,"b":b,"disposition":"COVERED_BY_FAIL_CLOSED_COMPOSITION"} for a,b in pairs]
    }


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--matrix",default=str(DEFAULT))
    ap.add_argument("--out")
    a=ap.parse_args()
    p=pathlib.Path(a.matrix)
    if not p.is_absolute(): p=ROOT/p
    result=audit(json.loads(p.read_text(encoding="utf-8")))
    if a.out:
        o=pathlib.Path(a.out)
        if not o.is_absolute(): o=ROOT/o
        o.parent.mkdir(parents=True,exist_ok=True)
        o.write_text(json.dumps(result,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    print(json.dumps({k:v for k,v in result.items() if k!="pairwise_permutations"},sort_keys=True))
    return 0 if result["state"]=="PASS" else 1

if __name__=="__main__":
    raise SystemExit(main())
