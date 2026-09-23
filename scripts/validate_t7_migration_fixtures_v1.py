#!/usr/bin/env python3
"""Validate T7 normalization/migration fixtures. Governance binding: CLOSURE_L9."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REG = ROOT / "configs" / "t7-normalization-registry-v1.json"
FIX = ROOT / "tests" / "fixtures" / "t7_projection_migration_v1.json"
MASK = 0xFFFF
ALL = 0x7F

def fail(msg):
    raise SystemExit("T7_FIXTURE_FAIL: " + msg)

def legacy_raw(inp):
    h = int(inp["data_hash"])
    entropy = int(inp["entropy"])
    hw = int(inp["hw_state"])
    return [
        h & MASK,
        (h >> 16) & MASK,
        entropy & MASK,
        (entropy >> 16) & MASK,
        hw & MASK,
        (hw >> 8) & MASK,
        (h ^ hw) & MASK,
    ]

def legacy_after(seed, raw):
    return [
        (int(seed[i]) - (int(seed[i]) >> 2) + (int(raw[i]) >> 2)) & MASK
        for i in range(7)
    ]

def equal_mask(event, raw):
    present = int(event["present_mask"]) & ALL
    q = [int(x) & MASK for x in event["q"]]
    mask = 0
    for i in range(7):
        bit = 1 << i
        if (present & bit) and q[i] == raw[i]:
            mask |= bit
    return mask

reg = json.loads(REG.read_text(encoding="utf-8"))
if reg.get("schema") != "RAFPOLIMATA-T7-NORMALIZATION-REGISTRY-V1":
    fail("registry schema")
if reg.get("governance_closure") != "CLOSURE_L9":
    fail("registry closure")
entries = {int(x["id"]): x for x in reg.get("entries", [])}
if 1 not in entries:
    fail("normalization id 1")
if entries[1].get("transform") != "q_out = q_in & 0xFFFF":
    fail("identity transform")
if entries[1].get("scientific_axis_calibration") is not False:
    fail("representation/science boundary")
if reg.get("external_source_to_axis_normalization_status") != "TOKEN_VAZIO":
    fail("unknown external normalization must remain explicit")
if reg.get("silent_imputation") is not False:
    fail("silent imputation")
if reg.get("migration_authorized") is not False:
    fail("migration gate")

doc = json.loads(FIX.read_text(encoding="utf-8"))
if doc.get("schema") != "RAFPOLIMATA-T7-MIGRATION-FIXTURES-V1":
    fail("fixture schema")
if doc.get("governance_closure") != "CLOSURE_L9":
    fail("fixture closure")
if doc.get("normalization_id") != 1:
    fail("fixture normalization id")
if doc.get("runtime_migration_authorized") is not False:
    fail("fixture migration gate")

fixtures = doc.get("fixtures", [])
if len(fixtures) != 3:
    fail("fixture count")
base = fixtures[0]
raw = legacy_raw(base["legacy_input"])
if raw != base["expected_legacy_raw"]:
    fail("legacy raw vector")
after = legacy_after(doc["legacy_init_seed"], raw)
if after != base["expected_legacy_after_map"]:
    fail("legacy after-map vector")

for item in fixtures:
    ref_raw = raw
    if equal_mask(item["semantic_event"], ref_raw) != int(item["expected_equal_mask"]):
        fail("equal mask " + item["id"])

if fixtures[0]["semantic_event"]["q"][3] != 0:
    fail("zero fixture")
if (fixtures[0]["semantic_event"]["present_mask"] & (1 << 3)) == 0:
    fail("numeric zero must be present in reference fixture")
if (fixtures[2]["semantic_event"]["present_mask"] & (1 << 3)) != 0:
    fail("absent chi fixture")

print("T7_FIXTURE_PASS registry=1 fixtures=3 legacy_raw=PASS legacy_after=PASS masks=127,126,119 migration=blocked")
