#!/usr/bin/env python3

def make_valid_intent(intent_id: str = "intent-valid-001") -> dict:
    return {
        "schema": "rafaelia.intent.v1",
        "intent_id": intent_id,
        "action": "inspect_repo",
        "target": {},
        "inputs": [],
        "constraints": [],
        "evidence_refs": ["chunk-1"],
        "requested_capabilities": ["git.read", "git.diff", "filesystem.read"],
        "risk": "low",
        "execution_gate": "allow",
    }
