#!/usr/bin/env python3
"""RafPolimata Evidence Garden V1: bounded benchmark/evidence runner."""
# Governance anchor: CLOSURE_L2.
import argparse, hashlib, json, math, os, pathlib, platform, shutil, statistics, subprocess, time
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
TOKEN_VAZIO = "TOKEN_VAZIO"
SCHEMA_CONFIG = "rafpolimata.evidence-garden.experiment.v1"
SCHEMA_RECEIPT = "rafpolimata.evidence-garden.receipt.v1"
SCHEMA_REPRO = "rafpolimata.evidence-garden.reproduction.v1"


def sha256_bytes(data): return hashlib.sha256(data).hexdigest()
def sha256_file(path):
    h=hashlib.sha256()
    with open(path,"rb") as f:
        for b in iter(lambda:f.read(1<<20),b""): h.update(b)
    return h.hexdigest()
def utc_now(): return datetime.now(timezone.utc).isoformat()
def run_capture(argv, timeout=30, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE):
    t0=time.monotonic_ns()
    try:
        p=subprocess.run(argv,cwd=cwd,stdout=stdout,stderr=stderr,timeout=timeout,check=False)
        return {"state":"PASS" if p.returncode==0 else "FAIL","exit_code":p.returncode,"wall_ns":time.monotonic_ns()-t0,
                "stdout":p.stdout if isinstance(p.stdout,(bytes,bytearray)) else b"",
                "stderr":p.stderr if isinstance(p.stderr,(bytes,bytearray)) else b""}
    except subprocess.TimeoutExpired as e:
        return {"state":"FAIL","exit_code":None,"wall_ns":time.monotonic_ns()-t0,"timeout":True,
                "stdout":e.stdout or b"","stderr":e.stderr or b""}
    except OSError as e:
        return {"state":"FAIL","exit_code":None,"wall_ns":time.monotonic_ns()-t0,"error":f"{type(e).__name__}: {e}","stdout":b"","stderr":b""}


def safe_repo_path(rel):
    p=(ROOT/pathlib.Path(rel)).resolve()
    try: p.relative_to(ROOT.resolve())
    except ValueError: raise ValueError(f"path escapes repository: {rel}")
    return p


def load_config(path):
    raw=path.read_bytes(); cfg=json.loads(raw)
    if cfg.get("schema") != SCHEMA_CONFIG: raise ValueError("unsupported experiment schema")
    ids=[v.get("id") for v in cfg.get("variants",[])]
    if len(ids)<1 or any(not x for x in ids) or len(set(ids))!=len(ids): raise ValueError("variant ids must be unique and non-empty")
    if cfg.get("baseline_variant") not in ids: raise ValueError("baseline_variant must name a variant")
    for v in cfg["variants"]:
        if not isinstance(v.get("command"),list) or not v["command"] or not all(isinstance(x,str) for x in v["command"]):
            raise ValueError("variant command must be a non-empty argv array")
    return cfg, sha256_bytes(raw)


def git_text(*args):
    r=run_capture(["git",*args],10)
    return r["stdout"].decode("utf-8","replace").strip() if r["exit_code"]==0 else TOKEN_VAZIO


def host_identity():
    d={"system":platform.system(),"release":platform.release(),"machine":platform.machine(),"python":platform.python_version(),"cpu_count":os.cpu_count()}
    for rel,key in [("/proc/cpuinfo","cpuinfo_sha256"),("/proc/meminfo","meminfo_sha256")]:
        p=pathlib.Path(rel); d[key]=sha256_file(p) if p.is_file() else TOKEN_VAZIO
    d["github_runner"]={k:os.environ.get(k,TOKEN_VAZIO) for k in ("RUNNER_OS","RUNNER_ARCH","RUNNER_NAME","GITHUB_RUN_ID","GITHUB_RUN_ATTEMPT")}
    return d


def command_identity(argv):
    exe=shutil.which(argv[0]) if "/" not in argv[0] else str(safe_repo_path(argv[0]))
    return {"argv":argv,"executable":exe or TOKEN_VAZIO,"executable_sha256":sha256_file(exe) if exe and pathlib.Path(exe).is_file() else TOKEN_VAZIO}


def artifact_identity(cfg):
    out=[]
    for a in cfg.get("artifacts",[]):
        p=safe_repo_path(a["path"]); exists=p.is_file()
        out.append({"path":a["path"],"required":bool(a.get("required",False)),"exists":exists,"sha256":sha256_file(p) if exists else TOKEN_VAZIO,"size":p.stat().st_size if exists else TOKEN_VAZIO})
    return out


def summarize_exec(r, keep_wall=True):
    x={"state":r["state"],"exit_code":r.get("exit_code"),"stdout_sha256":sha256_bytes(r.get("stdout",b"")),"stderr_sha256":sha256_bytes(r.get("stderr",b""))}
    if keep_wall: x["wall_ns"]=r.get("wall_ns")
    for k in ("timeout","error"):
        if k in r: x[k]=r[k]
    return x


def median_ci_nonparametric(values, confidence=0.95):
    xs=sorted(values); n=len(xs)
    if n<1: return {"state":TOKEN_VAZIO,"reason":"no samples"}
    alpha=1.0-confidence; cum=0.0; k=None
    for j in range(0,n//2+1):
        cum += math.comb(n,j)*(0.5**n)
        if cum <= alpha/2: k=j
        else: break
    if k is None or k+1 > n-k:
        return {"state":TOKEN_VAZIO,"reason":"sample count insufficient for requested distribution-free interval","confidence":confidence,"n":n}
    return {"state":"PASS","method":"exact-binomial order-statistic interval for population median","confidence":confidence,"n":n,"lower":xs[k],"upper":xs[n-k-1],"order_indices_1_based":[k+1,n-k],"assumptions":["independent observations or a defensible sampling design","continuous or tie-tolerant interpretation"]}


def percentile_nearest(xs,p):
    ys=sorted(xs); return ys[max(0,min(len(ys)-1,math.ceil(p*len(ys))-1))]


def stats(values):
    xs=list(values); n=len(xs)
    if not xs: return {"state":TOKEN_VAZIO,"reason":"no successful samples"}
    med=statistics.median(xs); mad=statistics.median([abs(x-med) for x in xs]); mean=statistics.fmean(xs)
    return {"state":"PASS","n":n,"min":min(xs),"p05":percentile_nearest(xs,.05),"median":med,"mean":mean,"p95":percentile_nearest(xs,.95),"max":max(xs),"sample_stdev":statistics.stdev(xs) if n>1 else 0.0,"mad":mad,"cv":(statistics.stdev(xs)/mean if n>1 and mean else TOKEN_VAZIO),"median_ci_95":median_ci_nonparametric(xs,.95)}


def factors_delta(a,b):
    keys=sorted(set(a)|set(b)); return {k:{"baseline":a.get(k,TOKEN_VAZIO),"variant":b.get(k,TOKEN_VAZIO)} for k in keys if a.get(k,TOKEN_VAZIO)!=b.get(k,TOKEN_VAZIO)}


def correctness_station(cfg):
    lane=cfg.get("correctness",{}); expected=lane.get("expected_exit",0); results={}
    if not lane.get("enabled",True): return {"state":TOKEN_VAZIO,"reason":"disabled"}
    for v in cfg["variants"]:
        r=run_capture(v["command"]+lane.get("args",[]),lane.get("timeout_seconds",30)); s=summarize_exec(r)
        s["expected_exit"]=expected; s["exit_matches_expected"]=r.get("exit_code")==expected; results[v["id"]]=s
    ok=all(x["exit_matches_expected"] for x in results.values())
    hashes={x["stdout_sha256"] for x in results.values()}
    eq={"state":"PASS" if len(hashes)==1 and ok else "FAIL","basis":"sha256(stdout) over executed correctness cases"} if lane.get("compare_stdout",False) else {"state":TOKEN_VAZIO,"reason":"compare_stdout disabled"}
    ref=lane.get("reference_stdout_sha256")
    accuracy={"state":"PASS" if ref and all(x["stdout_sha256"]==ref for x in results.values()) else ("FAIL" if ref else TOKEN_VAZIO),"reference_stdout_sha256":ref or TOKEN_VAZIO}
    return {"state":"PASS" if ok and eq.get("state")!="FAIL" and accuracy.get("state")!="FAIL" else "FAIL","variants":results,"cross_variant_stdout_equivalence":eq,"reference_accuracy":accuracy,"scope":"executed cases only"}


def observability_station(cfg,out_dir):
    lane=cfg.get("observability",{}); probes=lane.get("probes",[])
    if not lane.get("enabled",False): return {"state":TOKEN_VAZIO,"reason":"disabled"}
    out_dir.mkdir(parents=True,exist_ok=True); results=[]; blocking=False
    base=cfg["variants"][0]["command"]+lane.get("args",[])
    for i,p in enumerate(probes):
        kind=p.get("kind"); required=bool(p.get("required",False)); tool="strace" if kind.startswith("strace") else "perf" if kind=="perf_stat" else None
        rec={"kind":kind,"required":required,"tool":tool or TOKEN_VAZIO}
        if not tool or not shutil.which(tool): rec.update({"state":TOKEN_VAZIO,"reason":"probe tool unavailable or unsupported"}); blocking |= required; results.append(rec); continue
        dest=out_dir/f"{i:02d}-{kind}.txt"
        if kind=="strace_summary": argv=[tool,"-f","-c","-o",str(dest),*base]
        elif kind=="strace_full": argv=[tool,"-f","-qq","-ttt","-T","-o",str(dest),*base]
        else: argv=[tool,"stat","-x,","-e","task-clock,cycles,instructions,branches,branch-misses,cache-references,cache-misses","-o",str(dest),"--",*base]
        r=run_capture(argv,lane.get("timeout_seconds",30)); rec.update(summarize_exec(r)); rec["output_path"]=str(dest.relative_to(ROOT)) if dest.is_relative_to(ROOT) else str(dest); rec["output_sha256"]=sha256_file(dest) if dest.is_file() else TOKEN_VAZIO
        if rec["state"]!="PASS": blocking |= required
        results.append(rec)
    return {"state":"FAIL" if blocking else "PASS","probes":results,"physical_signal_visibility":TOKEN_VAZIO,"scope":"OS/runner exposed channels only"}


def performance_station(cfg):
    lane=cfg.get("performance",{}); rounds=int(lane.get("rounds",31)); warmup=int(lane.get("warmup",3)); timeout=lane.get("timeout_seconds",30)
    if not lane.get("enabled",True): return {"state":TOKEN_VAZIO,"reason":"disabled"}
    variants=cfg["variants"]
    for v in variants:
        for _ in range(warmup): run_capture(v["command"]+lane.get("args",[]),timeout,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    raw=[]
    for rno in range(rounds):
        order=variants[rno%len(variants):]+variants[:rno%len(variants)]
        for pos,v in enumerate(order):
            x=run_capture(v["command"]+lane.get("args",[]),timeout,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            raw.append({"variant":v["id"],"round":rno,"order_index":pos,"wall_ns":x["wall_ns"],"exit_code":x.get("exit_code"),"state":x["state"],"timeout":bool(x.get("timeout",False))})
    by={v["id"]:[x for x in raw if x["variant"]==v["id"] and x["state"]=="PASS"] for v in variants}
    st={k:stats([x["wall_ns"] for x in vals]) for k,vals in by.items()}
    base=cfg["baseline_variant"]; comparisons={}; vmap={v["id"]:v for v in variants}
    bround={x["round"]:x["wall_ns"] for x in by[base]}
    for vid,vals in by.items():
        if vid==base: continue
        vround={x["round"]:x["wall_ns"] for x in vals}; rs=sorted(set(bround)&set(vround)); ds=[vround[r]-bround[r] for r in rs]
        medb=st[base].get("median"); medv=st[vid].get("median"); fd=factors_delta(vmap[base].get("factors",{}),vmap[vid].get("factors",{}))
        comparisons[vid]={"state":"OBSERVED_UNPROMOTED","paired_rounds":len(ds),"median_delta_ns":statistics.median(ds) if ds else TOKEN_VAZIO,"relative_median_delta_pct":(((medv-medb)/medb)*100 if medb not in (0,TOKEN_VAZIO) and medv!=TOKEN_VAZIO else TOKEN_VAZIO),"delta_median_ci_95":median_ci_nonparametric(ds,.95),"sign_counts":{"variant_faster":sum(d<0 for d in ds),"equal":sum(d==0 for d in ds),"variant_slower":sum(d>0 for d in ds)},"declared_factor_differences":fd,"single_declared_factor":len(fd)==1,"causality":"NOT_PROMOTED"}
    state="PASS" if all(len(v)==rounds for v in by.values()) else "FAIL"
    return {"state":state,"timer":"time.monotonic_ns","warmup":warmup,"rounds":rounds,"order":lane.get("order","rotating_interleaved"),"raw_samples":raw,"statistics":st,"baseline_variant":base,"comparisons_to_baseline":comparisons}


def run_experiment(cfg, config_sha, out_path):
    artifacts=artifact_identity(cfg); missing=[a["path"] for a in artifacts if a["required"] and not a["exists"]]
    s0={"state":"FAIL" if missing else "PASS","config_sha256":config_sha,"repository_head":git_text("rev-parse","HEAD"),"git_status_sha256":sha256_bytes(git_text("status","--porcelain=v1").encode()),"artifacts":artifacts,"commands":{v["id"]:command_identity(v["command"]) for v in cfg["variants"]},"missing_required_artifacts":missing}
    s1=correctness_station(cfg)
    s2=observability_station(cfg,out_path.parent/"observability")
    s3=performance_station(cfg)
    comps=s3.get("comparisons_to_baseline",{}) if isinstance(s3,dict) else {}
    s4={"state":"OBSERVED_UNPROMOTED" if comps else TOKEN_VAZIO,"comparisons":comps,"rule":"paired deltas and declared factor differences do not by themselves establish causality"}
    s5={"state":"PASS" if s3.get("state")=="PASS" else s3.get("state",TOKEN_VAZIO),"per_variant":s3.get("statistics",{}),"method_note":"descriptive metrics + exact-binomial order-statistic interval for median where n permits"}
    s6={"state":"PENDING","reason":"single receipt; use compare with two or more receipts"}
    s7={"state":"AUDIT","claim_allowed":False,"bounded_claims":["declared commands/artifacts/config were identified for this run","correctness statements are limited to executed cases","performance statements are limited to this run/environment"],"not_claimed":["universal performance superiority","universal semantic equivalence","isolated physical causality","constant-time behavior","bare-metal physical proof","independent-provider reproduction"],"physical_signal_visibility":TOKEN_VAZIO}
    blocking=[s0.get("state"),s1.get("state"),s3.get("state")]
    receipt={"schema":SCHEMA_RECEIPT,"experiment_id":cfg["experiment_id"],"created_at":utc_now(),"run_state":"PASS" if all(x=="PASS" for x in blocking) else "FAIL","claim_allowed":False,"host":host_identity(),"stations":{"S0_identity":s0,"S1_correctness":s1,"S2_observability":s2,"S3_performance":s3,"S4_intervention":s4,"S5_statistics":s5,"S6_reproduction":s6,"S7_claim_gate":s7}}
    out_path.parent.mkdir(parents=True,exist_ok=True); out_path.write_text(json.dumps(receipt,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    return receipt


def correctness_signature(r):
    v=r.get("stations",{}).get("S1_correctness",{}).get("variants",{})
    return {k:(x.get("exit_code"),x.get("stdout_sha256"),x.get("stderr_sha256")) for k,x in sorted(v.items())}


def compare_receipts(receipts):
    if len(receipts)<2: return {"schema":SCHEMA_REPRO,"state":TOKEN_VAZIO,"claim_allowed":False,"reason":"need at least two receipts"}
    exps={r.get("experiment_id") for r in receipts}; configs={r.get("stations",{}).get("S0_identity",{}).get("config_sha256") for r in receipts}; srcs={r.get("stations",{}).get("S0_identity",{}).get("repository_head") for r in receipts}; sigs=[correctness_signature(r) for r in receipts]
    identity_ok=len(exps)==1 and len(configs)==1 and len(srcs)==1; corr_ok=all(s==sigs[0] for s in sigs[1:]) and all(r.get("run_state")=="PASS" for r in receipts)
    return {"schema":SCHEMA_REPRO,"state":"PASS" if identity_ok and corr_ok else "FAIL","claim_allowed":False,"receipt_count":len(receipts),"identity":{"experiment_same":len(exps)==1,"config_same":len(configs)==1,"source_same":len(srcs)==1},"correctness":{"state":"PASS" if corr_ok else "FAIL","basis":"receipt correctness signatures"},"performance":{"state":"OBSERVED_UNPROMOTED","reason":"cross-environment timing is retained as observation, not winner/causality proof"},"independent_provider_reproduction":TOKEN_VAZIO}


def main():
    ap=argparse.ArgumentParser(); sub=ap.add_subparsers(dest="cmd",required=True)
    rp=sub.add_parser("run"); rp.add_argument("--config",required=True); rp.add_argument("--out",required=True)
    cp=sub.add_parser("compare"); cp.add_argument("--out",required=True); cp.add_argument("receipts",nargs="+")
    a=ap.parse_args()
    if a.cmd=="run":
        cfg,cs=load_config(safe_repo_path(a.config)); r=run_experiment(cfg,cs,safe_repo_path(a.out)); print(f"EVIDENCE_GARDEN_{r['run_state']} out={a.out} experiment={cfg['experiment_id']}"); raise SystemExit(0 if r["run_state"]=="PASS" else 1)
    rec=[json.loads(safe_repo_path(p).read_text(encoding="utf-8")) for p in a.receipts]; s=compare_receipts(rec); out=safe_repo_path(a.out); out.parent.mkdir(parents=True,exist_ok=True); out.write_text(json.dumps(s,indent=2,sort_keys=True)+"\n",encoding="utf-8"); print(f"EVIDENCE_GARDEN_REPRO_{s['state']} out={a.out}"); raise SystemExit(0 if s["state"]=="PASS" else 1)

if __name__=="__main__": main()
