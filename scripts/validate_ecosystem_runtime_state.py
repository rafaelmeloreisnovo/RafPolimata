#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def require(value: bool, message: str) -> None:
    if not value:
        raise AssertionError(message)


def main() -> int:
    state_path = ROOT / "ECOSYSTEM_RUNTIME_STATE.json"
    schema_path = ROOT / "contracts/ecosystem-runtime-state.schema.json"
    state = json.loads(state_path.read_text(encoding="utf-8"))
    schema = json.loads(schema_path.read_text(encoding="utf-8"))

    require(state["schema"] == "raf.ecosystem-runtime-state.v1", "wrong state schema")
    require(schema["properties"]["schema"]["const"] == state["schema"], "schema/state drift")
    require(state["ci_execution"]["state"] == "OUT_OF_SCOPE_NO_CREDIT",
            "CI must not be represented as executed")

    implementation_states = {
        "IMPLEMENTED", "PARTIAL", "ADAPTER_IMPLEMENTED", "STUB",
        "EXPERIMENTAL", "DEVICE_REQUIRED", "REFERENCE",
    }
    evidence_states = {"VERIFIED", "DECLARED_BY_AUTHOR", "TOKEN_VAZIO", "CONTRADICTION"}
    ids: set[str] = set()

    for component in state["components"]:
        identifier = component["id"]
        require(identifier not in ids, f"duplicate component id: {identifier}")
        ids.add(identifier)
        require(component["implementation_state"] in implementation_states,
                f"invalid implementation state: {identifier}")
        require(component["evidence_state"] in evidence_states,
                f"invalid evidence state: {identifier}")
        require(isinstance(component["evidence"], list), f"evidence is not a list: {identifier}")
        require(isinstance(component["gaps"], list), f"gaps is not a list: {identifier}")
        require(bool(component["next_action"]), f"missing next action: {identifier}")

    segment = next(item for item in state["components"] if item["id"] == "rafpolimata.segment-v1-header")
    require(segment["evidence_state"] == "VERIFIED", "segment header local proof missing")

    apk = next(item for item in state["components"] if item["id"] == "rafpolimata.apkc-device-runtime")
    require(apk["evidence_state"] == "TOKEN_VAZIO", "device runtime must remain TOKEN_VAZIO")

    print("PASS ecosystem-runtime-state")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, KeyError, StopIteration, json.JSONDecodeError, OSError) as error:
        print(f"FAIL ecosystem-runtime-state: {error}", file=sys.stderr)
        raise SystemExit(1)
