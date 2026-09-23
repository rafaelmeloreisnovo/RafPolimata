#!/usr/bin/env python3
"""Validate additive T7 application bridge V2. Governance binding: CLOSURE_L9."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CFG = ROOT / "configs" / "t7-application-bridge-v2.json"
HDR = ROOT / "rafaelia" / "t7_workflow_apply_v2.h"
TEST = ROOT / "tests" / "test_t7_workflow_apply_v2.c"

def fail(msg):
    raise SystemExit("T7_APPLY_V2_FAIL: " + msg)

d = json.loads(CFG.read_text(encoding="utf-8"))
if d.get("schema") != "RAFPOLIMATA-T7-APPLICATION-BRIDGE-V2":
    fail("schema")
if d.get("version") != 2 or d.get("bridge_id") != "Pi_apply_V2":
    fail("identity")
if d.get("governance_closure") != "CLOSURE_L9":
    fail("closure")
if d.get("input_projection_version") != 1:
    fail("projection version")
if d.get("policy") != "PRESENT_ONLY":
    fail("policy")
if d.get("all_present_legacy_parity_required") is not True:
    fail("legacy parity")
if d.get("modifies_legacy_t7_map_input") is not False:
    fail("legacy mutation boundary")
if d.get("runtime_replacement_authorized") is not False:
    fail("runtime replacement gate")
if d.get("claim_allowed") is not False:
    fail("claim gate")
if not HDR.exists() or not TEST.exists():
    fail("source/test missing")

h = HDR.read_text(encoding="utf-8")
for required in (
    "T7WF_APPLY_POLICY_PRESENT_ONLY",
    "t7wf_apply_present_v2",
    "state->s[i] - (state->s[i] >> 2) + (q >> 2)",
    "r.preserved_mask |= bit",
):
    if required not in h:
        fail("header invariant: " + required)

print("T7_APPLY_V2_PASS policy=PRESENT_ONLY all_present_legacy_parity=required zero_present=apply absent=preserve legacy_replacement=blocked")
