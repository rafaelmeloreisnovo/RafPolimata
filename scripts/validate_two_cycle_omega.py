#!/usr/bin/env python3
"""Valida o manifesto de dois ciclos ômega sem dependências externas."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "configs" / "two_cycle_omega.yml"
DOC = ROOT / "docs" / "PROTOCOLO_DOIS_CICLOS_OMEGA.md"

EXPECTED_FIELDS = [
    "matrix_id", "row", "col", "cell_id", "value", "layer", "state",
    "tag14", "rafbit10", "epoch", "cycle", "timestamp",
]
EXPECTED_INVARIANTS = {
    "no_heap_hot_path", "no_gc_runtime", "explicit_arch_flags",
    "fail_safe", "failover", "rollback_ready", "token_vazio_allowed",
}


def section_items(text: str, section: str) -> list[str]:
    lines = text.splitlines()
    start = None
    for idx, line in enumerate(lines):
        if line == f"{section}:":
            start = idx + 1
            break
    if start is None:
        raise SystemExit(f"missing_section:{section}")
    items: list[str] = []
    for line in lines[start:]:
        if line and not line.startswith((" ", "\t")):
            break
        stripped = line.strip()
        if stripped.startswith("- "):
            items.append(stripped[2:].strip())
    return items


def scalar_int(text: str, key: str) -> int:
    match = re.search(rf"^\s*{re.escape(key)}:\s*(\d+)\b", text, re.MULTILINE)
    if not match:
        raise SystemExit(f"missing_int:{key}")
    return int(match.group(1))


def validate() -> dict[str, object]:
    if not CONFIG.exists():
        raise SystemExit("missing_config")
    if not DOC.exists():
        raise SystemExit("missing_doc")
    text = CONFIG.read_text(encoding="utf-8")
    doc = DOC.read_text(encoding="utf-8")

    if scalar_int(text, "cycles") != 2:
        raise SystemExit("cycles_must_equal_2")
    if scalar_int(text, "depth_limit") != 5:
        raise SystemExit("depth_limit_must_equal_5")
    if "empty_token: TOKEN_VAZIO" not in text:
        raise SystemExit("missing_token_vazio")
    if section_items(text, "canonical_fields") != EXPECTED_FIELDS:
        raise SystemExit("invalid_canonical_fields")
    invariants = set(section_items(text, "technical_invariants"))
    missing = EXPECTED_INVARIANTS - invariants
    if missing:
        raise SystemExit("missing_invariants:" + ",".join(sorted(missing)))
    for required in ("SEMANTIC_READY", "EXEC_PASS", "EXEC_FAIL", "EXEC_SKIPPED", "ROLLBACK_READY"):
        if required not in doc:
            raise SystemExit(f"missing_doc_status:{required}")
    return {"protocol": "two_cycle_omega", "fields": len(EXPECTED_FIELDS), "verdict": "PASS"}


if __name__ == "__main__":
    result = validate()
    print(result)
