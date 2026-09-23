#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CFG = ROOT / "configs" / "t7-workflow-v1.json"

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

a = d.get("auxiliary_42_slot_machine", {})
if a.get("implemented") is not True:
    fail("42-slot implementation state")
for k in ("proven_as_exactly_42_dynamical_attractors",
          "global_convergence_proven",
          "stability_proven"):
    if a.get(k) is not False:
        fail(k + " must remain false until proof")

if d.get("legacy_mapping", {}).get("semantic_identity") is not False:
    fail("legacy semantic identity must remain false")
if d.get("claim_allowed") is not False:
    fail("claim gate")

print("T7_PASS dimensions=7 q16_modulus=65536 semantic_projection=typed convergence_proof=pending")
