#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "configs" / "authorial-omega-assurance.v1.json"

MAPA_HEAD = "3d54e8e36f1de787e1a815d899490a0fb7f0b326"
MAPA_BLOB = "5ec155d2e67fb7e6f46fb2589c6e32149d570091"
EXEC_HEAD = "c949595817ac70144001c2a597aaac84eab06c10"
EXEC_BLOB = "dab95ef7489a72f24fccb22d822fcc5a59900b56"
BL0_BLOB = "132f948d199f6679fcae4f33912e0a3ad69691a3"

class ValidationError(ValueError):
    pass

def req(cond, msg):
    if not cond:
        raise ValidationError(msg)

def load():
    return json.loads(CONFIG.read_text(encoding="utf-8"))

def validate(d):
    req(d.get("schema") == "rafpolimata.authorial-omega-assurance.v1", "schema")
    req(d.get("claim_allowed") is False, "claim_allowed")
    req(d.get("role") == "ASSURANCE_BENCHMARK_FALSIFIER", "role")

    sb = d["source_bindings"]
    req(sb["federation"]["commit"] == MAPA_HEAD, "Mapa head")
    req(sb["federation"]["blob_sha1"] == MAPA_BLOB, "Mapa blob")
    req(sb["executor"]["commit"] == EXEC_HEAD, "executor head")
    req(sb["executor"]["blob_sha1"] == EXEC_BLOB, "executor blob")
    req(sb["federation"]["promotion"] == "NOT_AUTHORIZED", "Mapa promotion boundary")
    req(sb["executor"]["promotion"] == "NOT_AUTHORIZED", "executor promotion boundary")

    policy = d["authorial_policy"]
    req(policy["selection_mode"] == "AUTHORIAL_ONLY_FAIL_CLOSED", "selection mode")
    req(policy["admit_states"] == ["AUTHORIAL_PATH_PROVEN_IN_REPOSITORY_HISTORY"], "admit states")
    req(policy["whole_fork_authorship_allowed"] is False, "whole fork authorship")
    req(policy["legal_authorship_from_git_history"] is False, "legal authorship promotion")
    req(policy["external_primitive_rebranding_allowed"] is False, "external primitive rebranding")

    models = d["model_boundaries"]
    for k in ("Vectra", "PCR_Rafaelia_Code_seed"):
        req(models[k]["code_import_allowed"] is False, f"{k} import")
        req(models[k]["whole_repository_authorial"] is False, f"{k} whole authorial")

    payload = d["assured_payloads"]
    req(len(payload) == 1, "bounded payload count")
    req(payload[0]["artifact_id"] == "RAF_BL0_V0", "payload id")
    req(payload[0]["expected_blob_sha1"] == BL0_BLOB, "payload blob")
    req("CLOSURE_L12" in payload[0]["runtime_state"], "runtime closure")

    cycle = d["cycle_contract"]
    req(cycle == [
        "C01_INTENT","C02_AUTHORSHIP","C03_GAPS","C04_MOUNT",
        "C05_EXECUTE","C06_VERIFY","C07_CUSTODY","C08_OMEGA"
    ], "cycle order")

    falsifiers = d["falsifiers"]
    req(len(falsifiers) >= 6, "falsifier coverage")
    req(len({x["id"] for x in falsifiers}) == len(falsifiers), "duplicate falsifier")
    req(all(x.get("expected") == "REJECT" for x in falsifiers), "falsifier expectation")

    gates = d["assurance_gates"]
    req(len({x["id"] for x in gates}) == len(gates), "duplicate gate")
    by_id = {x["id"]: x for x in gates}
    req(by_id["A05_EXECUTOR_EXACT_HEAD"]["state"] == "TOKEN_VAZIO", "executor evidence must stay open")
    req(by_id["A05_EXECUTOR_EXACT_HEAD"]["closure"] == "CLOSURE_L11", "executor closure")
    req(by_id["A06_PHYSICAL_RUNTIME"]["state"] == "TOKEN_VAZIO", "physical runtime")
    req(by_id["A06_PHYSICAL_RUNTIME"]["closure"] == "CLOSURE_L12", "physical closure")
    req(by_id["A07_INDEPENDENT_REPRODUCTION"]["state"] == "TOKEN_VAZIO", "independent reproduction")

    req(bool(d.get("F_next")), "F_next")
    return True

def main():
    d = load()
    validate(d)
    print(json.dumps({
        "schema": d["schema"],
        "state": "PASS_STRUCTURAL_LOCAL",
        "mapa_head": MAPA_HEAD,
        "mapa_blob": MAPA_BLOB,
        "executor_head": EXEC_HEAD,
        "executor_blob": EXEC_BLOB,
        "payload_blob": BL0_BLOB,
        "claim_allowed": False
    }, sort_keys=True))

if __name__ == "__main__":
    main()
