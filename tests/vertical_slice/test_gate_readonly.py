#!/usr/bin/env python3
import json
import subprocess
import tempfile
from pathlib import Path
from intent_factory import make_valid_intent

ROOT = Path(__file__).resolve().parents[2]
FLOW = ROOT / "scripts/vertical_slice/run_readonly_flow.sh"

intent = make_valid_intent("intent-gate-001")
intent["evidence_refs"] = ["chunk-2"]
intent["requested_capabilities"] = ["git.read", "network.write"]
intent["risk"] = "medium"

with tempfile.TemporaryDirectory() as td:
    td_path = Path(td)
    intent_path = td_path / "intent.json"
    intent_path.write_text(json.dumps(intent), encoding="utf-8")
    proc = subprocess.run(
        [str(FLOW), str(intent_path), str(td_path)],
        capture_output=True,
        text=True,
        check=False,
    )
    assert proc.returncode != 0
    assert "fora da allowlist" in proc.stderr

print("PASS: test_gate_readonly")
