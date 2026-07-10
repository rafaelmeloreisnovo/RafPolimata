#!/usr/bin/env python3
import json
import subprocess
import tempfile
from pathlib import Path
from intent_factory import make_valid_intent

ROOT = Path(__file__).resolve().parents[2]
FLOW = ROOT / "scripts/vertical_slice/run_readonly_flow.sh"


def run_flow(intent: dict):
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
        result_exists = (td_path / "execution_result.json").exists()
        return proc, result_exists


valid_intent = make_valid_intent("intent-valid-001")

proc_ok, result_exists = run_flow(valid_intent)
assert proc_ok.returncode == 0, proc_ok.stderr
assert result_exists

invalid_intent = dict(valid_intent)
del invalid_intent["action"]
proc_bad, _ = run_flow(invalid_intent)
assert proc_bad.returncode != 0
assert "intent_ir inválido" in proc_bad.stderr

print("PASS: test_intent_ir_validation")
