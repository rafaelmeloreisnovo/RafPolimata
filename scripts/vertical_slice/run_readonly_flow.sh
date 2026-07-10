#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
INTENT_PATH="${1:-}"
OUT_DIR="${2:-$ROOT_DIR/output/vertical_slice}"

if [[ -z "$INTENT_PATH" ]]; then
  echo "usage: $0 <intent_ir.json> [output_dir]" >&2
  exit 64
fi

mkdir -p "$OUT_DIR"
PLAN_PATH="$OUT_DIR/execution_plan.json"
RESULT_PATH="$OUT_DIR/execution_result.json"
STDOUT_PATH="$OUT_DIR/stdout.log"
STDERR_PATH="$OUT_DIR/stderr.log"

python3 - "$ROOT_DIR" "$INTENT_PATH" "$PLAN_PATH" <<'PY'
import json
import sys
from datetime import datetime, timezone

root_dir, intent_path, plan_path = sys.argv[1:4]

with open(intent_path, "r", encoding="utf-8") as f:
    intent = json.load(f)
with open(f"{root_dir}/internal/governance/capabilities.json", "r", encoding="utf-8") as f:
    caps = json.load(f)
with open(f"{root_dir}/internal/governance/policy.json", "r", encoding="utf-8") as f:
    policy = json.load(f)

required = [
    "schema", "intent_id", "action", "target", "inputs", "constraints",
    "evidence_refs", "requested_capabilities", "risk", "execution_gate"
]
for key in required:
    if key not in intent:
        raise SystemExit(f"intent_ir inválido: campo ausente '{key}'")

if intent["schema"] != "rafaelia.intent.v1":
    raise SystemExit("intent_ir inválido: schema deve ser rafaelia.intent.v1")
if intent["risk"] not in {"low", "medium", "high", "critical"}:
    raise SystemExit("intent_ir inválido: risk fora do enum")
if intent["execution_gate"] not in {"allow", "sandbox_only", "human_review", "blocked"}:
    raise SystemExit("intent_ir inválido: execution_gate fora do enum")

cap_map = caps.get("capabilities", {})
default_gate = caps.get("default_policy", policy.get("default_gate", "blocked"))
gate_rank = {"allow": 0, "sandbox_only": 1, "human_review": 2, "blocked": 3}

requested_caps = intent.get("requested_capabilities", [])
if not isinstance(requested_caps, list):
    raise SystemExit("intent_ir inválido: requested_capabilities deve ser array")

resolved = []
for capability in requested_caps:
    gate = cap_map.get(capability, default_gate)
    resolved.append((capability, gate))

max_cap_gate = "allow"
for _, gate in resolved:
    if gate_rank[gate] > gate_rank[max_cap_gate]:
        max_cap_gate = gate

intent_gate = intent["execution_gate"]
final_gate = max_cap_gate if gate_rank[max_cap_gate] > gate_rank[intent_gate] else intent_gate

unknown = [cap for cap, _ in resolved if cap not in cap_map]
if unknown:
    raise SystemExit(f"gate bloqueado: capability fora da allowlist: {', '.join(unknown)}")
if final_gate == "blocked":
    raise SystemExit("gate bloqueado: execution_gate final = blocked")

plan = {
    "schema": "rafaelia.execution_plan.v1",
    "plan_id": f"plan-{intent['intent_id']}",
    "intent_id": intent["intent_id"],
    "gate": final_gate,
    "commands": [
        {"command": "git", "args": ["status"], "read_only": True},
        {"command": "git", "args": ["diff", "--stat"], "read_only": True}
    ],
    "created_at": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")
}

allowed = {(item["command"], tuple(item["args"])) for item in policy.get("allowed_readonly_commands", [])}
for cmd in plan["commands"]:
    key = (cmd["command"], tuple(cmd["args"]))
    if key not in allowed:
        raise SystemExit("policy inválida: plano contém comando não permitido")

with open(plan_path, "w", encoding="utf-8") as f:
    json.dump(plan, f, ensure_ascii=False, indent=2)
PY

STARTED_AT="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
set +e
{
  echo "$ git status"
  git -C "$ROOT_DIR" status
  echo
  echo "$ git diff --stat"
  git -C "$ROOT_DIR" diff --stat
} >"$STDOUT_PATH" 2>"$STDERR_PATH"
EXIT_CODE=$?
set -e
ENDED_AT="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"

STDOUT_SHA256="$(sha256sum "$STDOUT_PATH" | awk '{print $1}')"
STDERR_SHA256="$(sha256sum "$STDERR_PATH" | awk '{print $1}')"

python3 - "$INTENT_PATH" "$RESULT_PATH" "$ROOT_DIR" "$STARTED_AT" "$ENDED_AT" "$EXIT_CODE" "$STDOUT_SHA256" "$STDERR_SHA256" <<'PY'
import json
import sys

(
    intent_path,
    result_path,
    root_dir,
    started_at,
    ended_at,
    exit_code,
    stdout_sha256,
    stderr_sha256,
) = sys.argv[1:9]

with open(intent_path, "r", encoding="utf-8") as f:
    intent = json.load(f)

result = {
    "schema": "rafaelia.execution_result.v1",
    "intent_id": intent["intent_id"],
    "executed_command": "git status && git diff --stat",
    "args": [["status"], ["diff", "--stat"]],
    "working_directory": root_dir,
    "started_at": started_at,
    "ended_at": ended_at,
    "exit_code": int(exit_code),
    "stdout_truncated": False,
    "stderr_truncated": False,
    "stdout_sha256": stdout_sha256,
    "stderr_sha256": stderr_sha256,
    "artifacts": [
        result_path.replace("execution_result.json", "execution_plan.json"),
        result_path.replace("execution_result.json", "stdout.log"),
        result_path.replace("execution_result.json", "stderr.log")
    ],
    "final_state": "success" if int(exit_code) == 0 else "failed",
    "rollback_available": False,
    "source_chunk_refs": intent.get("evidence_refs", [])
}

with open(result_path, "w", encoding="utf-8") as f:
    json.dump(result, f, ensure_ascii=False, indent=2)
PY

echo "execution_plan=$PLAN_PATH"
echo "execution_result=$RESULT_PATH"
exit "$EXIT_CODE"
