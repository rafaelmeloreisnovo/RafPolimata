"""Compile a finite federated route for the RLL session capsule."""
from __future__ import annotations
from hashlib import sha256
import itertools, json
from pathlib import Path
from typing import Any

STAGES = ("normalize", "permute", "prune", "score", "route", "evidence")

def canonical_digest(value: Any) -> str:
    return sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest()

def validate_config(config: dict) -> None:
    required = {"schema","authorities","stages","variants","gates","claim_allowed"}
    missing = required - config.keys()
    if missing: raise ValueError(f"missing keys: {sorted(missing)}")
    if tuple(config["stages"]) != STAGES: raise ValueError("canonical avalanche stage order required")
    if config["claim_allowed"] is not False: raise ValueError("claim_allowed must remain false")
    if config["authorities"]["physics"] != "instituto-Rafael/relativity-living-light":
        raise ValueError("RLL must remain physics authority")
    if config["authorities"]["governance"] != "rafaelmeloreisnovo/RafGitTools":
        raise ValueError("RafGitTools must remain governance authority")
    if config["authorities"]["map"] != "rafaelmeloreisnovo/Mapa":
        raise ValueError("Mapa must remain pointer-only")
    if any(g.get("required") and not g.get("failure_state") for g in config["gates"]):
        raise ValueError("required gates need failure_state")

def select_backends(capabilities: set[str]) -> tuple[str, ...]:
    out = ["scalar"]
    for item in ("simd","gpu","dsp","npu"):
        if item in capabilities: out.append(item)
    return tuple(out)

def compile_plan(config: dict, capabilities: set[str]) -> dict:
    validate_config(config)
    backends = select_backends(capabilities)
    v = config["variants"]
    jobs = []
    for backend, frequency, thermal, magnetization in itertools.product(
        backends, v["frequency_hz"], v["thermal_regime"], v["magnetization_regime"]
    ):
        if backend == "npu" and thermal == "fully_ionized":
            continue
        jobs.append({
          "job_id":f"{backend}-{int(frequency)}-{thermal}-{magnetization}",
          "backend":backend,"frequency_hz":frequency,"thermal_regime":thermal,
          "magnetization_regime":magnetization,
          "physics_authority":config["authorities"]["physics"],
          "required_gates":[g["id"] for g in config["gates"] if g["required"]],
          "automatic_cross_repo_write":False,"automatic_merge":False,"claim_allowed":False
        })
    jobs.sort(key=lambda x:x["job_id"])
    if len(jobs) > config["scale_policy"]["max_jobs"]:
        raise ValueError("compiled plan exceeds finite max_jobs")
    return {"schema":"rafpolimata.rll_session_avalanche.plan.v1","stages":list(STAGES),
            "backend_count":len(backends),"job_count":len(jobs),"jobs":jobs,
            "digest_sha256":canonical_digest(jobs),"claim_allowed":False}

def load_config(path: str | Path) -> dict:
    return json.loads(Path(path).read_text())

if __name__=="__main__":
    import argparse
    parser=argparse.ArgumentParser()
    parser.add_argument("--config",default="configs/rll-session-avalanche-route-v1.json")
    parser.add_argument("--capability",action="append",default=[])
    parser.add_argument("--output")
    args=parser.parse_args()
    plan=compile_plan(load_config(args.config),set(args.capability))
    payload=json.dumps(plan,indent=2,sort_keys=True)
    if args.output: Path(args.output).write_text(payload+"\n")
    else: print(payload)
