#!/usr/bin/env python3
import json
import subprocess
import tempfile
from datetime import datetime
from pathlib import Path
from jsonschema import Draft202012Validator
from intent_factory import make_valid_intent

ROOT = Path(__file__).resolve().parents[2]
FLOW = ROOT / "scripts/vertical_slice/run_readonly_flow.sh"
RESULT_SCHEMA = ROOT / "docs/contracts/execution_result.schema.json"

intent = make_valid_intent("intent-result-001")
intent["evidence_refs"] = ["chunk-3", "chunk-4"]

required_fields = [
    "intent_id",
    "executed_command",
    "args",
    "working_directory",
    "started_at",
    "ended_at",
    "exit_code",
    "stdout_truncated",
    "stderr_truncated",
    "stdout_sha256",
    "stderr_sha256",
    "artifacts",
    "final_state",
    "rollback_available",
    "source_chunk_refs",
]

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
    assert proc.returncode == 0, proc.stderr

    result_path = td_path / "execution_result.json"
    result = json.loads(result_path.read_text(encoding="utf-8"))
    schema = json.loads(RESULT_SCHEMA.read_text(encoding="utf-8"))
    Draft202012Validator(schema).validate(result)

    for field in required_fields:
        assert field in result, field

    start = datetime.fromisoformat(result["started_at"].replace("Z", "+00:00"))
    end = datetime.fromisoformat(result["ended_at"].replace("Z", "+00:00"))
    assert end >= start

    for key in ("stdout_sha256", "stderr_sha256"):
        value = result[key]
        assert len(value) == 64
        assert all(ch in "0123456789abcdef" for ch in value)

print("PASS: test_execution_result_schema")
