#!/usr/bin/env python3
import json
import re
import sys
from pathlib import Path

REQUIRED = {
    "schema_version", "artifact_id", "producer", "consumer", "source_commit",
    "target", "format", "toolchain", "commands", "hashes", "validators",
    "dependencies", "epistemic_state", "claim_allowed", "rollback", "receipt"
}
TARGETS = {"armv7", "aarch64", "x86_64", "TOKEN_VAZIO"}
FORMATS = {"ELF", "APK", "DEX", "ZIP", "REPORT", "TOKEN_VAZIO"}
STATES = {"TOKEN_VAZIO", "BLOCKED", "FAIL", "VALID_LIMITED", "VALID"}
VALIDATOR_STATES = {"PASS", "FAIL", "TOKEN_VAZIO"}
HEX40 = re.compile(r"^[0-9a-f]{40}$")
HEX = re.compile(r"^[0-9a-fA-F]+$")


def fail(msg: str) -> None:
    raise ValueError(msg)


def validate(obj: dict) -> None:
    missing = sorted(REQUIRED - obj.keys())
    if missing:
        fail(f"missing required fields: {missing}")
    extra = sorted(set(obj) - REQUIRED)
    if extra:
        fail(f"unexpected top-level fields: {extra}")
    if obj["schema_version"] != "multifilament-runtime-envelope.v1":
        fail("invalid schema_version")
    if obj["producer"] != "RafPolimata":
        fail("producer must be RafPolimata")
    if obj["target"] not in TARGETS:
        fail("invalid target")
    if obj["format"] not in FORMATS:
        fail("invalid format")
    if obj["epistemic_state"] not in STATES:
        fail("invalid epistemic_state")
    source = obj["source_commit"]
    if source != "TOKEN_VAZIO" and not HEX40.fullmatch(source):
        fail("source_commit must be TOKEN_VAZIO or a 40-char lowercase git SHA")
    if not isinstance(obj["claim_allowed"], bool):
        fail("claim_allowed must be boolean")

    toolchain = obj["toolchain"]
    if set(toolchain) != {"id", "version"} or not all(isinstance(toolchain[k], str) and toolchain[k] for k in toolchain):
        fail("toolchain must contain non-empty id and version")
    if not isinstance(obj["commands"], list) or not all(isinstance(x, str) and x for x in obj["commands"]):
        fail("commands must be a list of non-empty strings")
    if not isinstance(obj["dependencies"], list) or not all(isinstance(x, str) and x for x in obj["dependencies"]):
        fail("dependencies must be a list of non-empty strings")
    if not isinstance(obj["hashes"], dict):
        fail("hashes must be an object")
    for name, digest in obj["hashes"].items():
        if not isinstance(name, str) or not name or not isinstance(digest, str) or not HEX.fullmatch(digest):
            fail("hashes must map non-empty names to hex digests")

    if not isinstance(obj["validators"], list):
        fail("validators must be a list")
    for item in obj["validators"]:
        if not isinstance(item, dict) or set(item) - {"name", "status", "detail"}:
            fail("validator entry has invalid shape")
        if not isinstance(item.get("name"), str) or not item["name"]:
            fail("validator name must be non-empty")
        if item.get("status") not in VALIDATOR_STATES:
            fail("validator status invalid")

    rollback = obj["rollback"]
    if set(rollback) - {"strategy", "verified", "receipt_ref"} or not {"strategy", "verified"} <= set(rollback):
        fail("rollback has invalid shape")
    if not isinstance(rollback["strategy"], str) or not rollback["strategy"] or not isinstance(rollback["verified"], bool):
        fail("rollback strategy/verified invalid")

    receipt = obj["receipt"]
    if set(receipt) != {"status", "ref"} or receipt["status"] not in {"TOKEN_VAZIO", "PRESENT"}:
        fail("receipt has invalid shape/status")

    if obj["claim_allowed"]:
        if obj["epistemic_state"] not in {"VALID_LIMITED", "VALID"}:
            fail("claim_allowed=true requires VALID_LIMITED or VALID")
        if not HEX40.fullmatch(source):
            fail("claim_allowed=true requires concrete source_commit")
        if not obj["hashes"]:
            fail("claim_allowed=true requires at least one artifact hash")
        if not obj["validators"] or any(v["status"] != "PASS" for v in obj["validators"]):
            fail("claim_allowed=true requires non-empty all-PASS validators")
        if receipt["status"] != "PRESENT" or not isinstance(receipt["ref"], str) or not receipt["ref"]:
            fail("claim_allowed=true requires receipt PRESENT with ref")
        if rollback["verified"] is not True:
            fail("claim_allowed=true requires verified rollback")


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} ENVELOPE.json", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    try:
        obj = json.loads(path.read_text(encoding="utf-8"))
        validate(obj)
    except Exception as exc:
        print(f"FAIL {path}: {exc}", file=sys.stderr)
        return 1
    print(f"PASS {path}: claim_allowed={obj['claim_allowed']} state={obj['epistemic_state']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
