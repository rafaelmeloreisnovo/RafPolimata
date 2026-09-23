#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CFG = ROOT / "configs" / "t7-workflow-v1.json"
HDR = ROOT / "rafaelia" / "t7_workflow_projection_v1.h"
TEST = ROOT / "tests" / "test_t7_workflow_projection_v1.c"

def fail(msg):
    raise SystemExit("T7_FAIL: " + msg)

d = json.loads(CFG.read_text(encoding="utf-8"))
if d.get("schema") != "RAFPOLIMATA-T7-WORKFLOW-V1":
    fail("schema")
m = d.get("math", {})
if m.get("space") != "(R/Z)^7" or m.get("dimensions") != 7:
    fail("math object")
if m.get("q16_modulus") != 65536:
    fail("q16 modulus")

coords = d.get("coordinates", [])
if len(coords) != 7:
    fail("coordinate count")
if [c.get("index") for c in coords] != list(range(7)):
    fail("coordinate indexes")
if len({c.get("symbol") for c in coords}) != 7:
    fail("coordinate symbols")

tv = d.get("token_vazio", {})
if tv.get("numeric_zero_equivalent") is not False:
    fail("TOKEN_VAZIO must not equal numeric zero")
if tv.get("silent_imputation_forbidden") is not True:
    fail("silent imputation gate")

p = d.get("projection_adapter", {})
if p.get("id") != "Pi_wf_V1" or p.get("implemented") is not True:
    fail("projection adapter")
if p.get("presence_encoding") != "7-bit present_mask":
    fail("presence encoding")
if p.get("numeric_zero_is_valid_present_value") is not True:
    fail("zero-present distinction")
if p.get("silent_imputation") is not False:
    fail("projection silent imputation")
if p.get("mutates_legacy_t7_state") is not False:
    fail("legacy mutation boundary")
if not HDR.exists() or not TEST.exists():
    fail("projection source/test missing")

a = d.get("auxiliary_42_slot_machine", {})
if a.get("implemented") is not True:
    fail("42-slot implementation state")
for k in ("proven_as_exactly_42_dynamical_attractors",
          "global_convergence_proven",
          "stability_proven"):
    if a.get(k) is not False:
        fail(k + " must remain false until proof")

legacy = d.get("legacy_mapping", {})
if legacy.get("semantic_identity") is not False:
    fail("legacy semantic identity must remain false")
if legacy.get("runtime_behavior_changed") is not False:
    fail("legacy runtime must remain unchanged in V1 adapter")
if d.get("claim_allowed") is not False:
    fail("claim gate")

print("T7_PASS dimensions=7 Pi_wf_V1=implemented token_vazio=presence_bit legacy_runtime=unchanged convergence_proof=pending")
