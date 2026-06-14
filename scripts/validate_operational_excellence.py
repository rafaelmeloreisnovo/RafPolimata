#!/usr/bin/env python3
"""Valida o manifesto de excelência operacional sem dependências externas."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "configs" / "operational_excellence.yml"
DOC = ROOT / "docs" / "EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md"


def _read(path: Path) -> str:
    if not path.exists():
        raise SystemExit(f"missing_file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def _section_items(text: str, section: str) -> list[str]:
    lines = text.splitlines()
    start = None
    for index, line in enumerate(lines):
        if line == f"{section}:":
            start = index + 1
            break
    if start is None:
        raise SystemExit(f"missing_section: {section}")
    body: list[str] = []
    for line in lines[start:]:
        if line and not line.startswith((" ", "\t")):
            break
        if line.strip().startswith("- "):
            body.append(line.strip()[2:].strip())
    return body


def main() -> None:
    manifest = _read(MANIFEST)
    doc = _read(DOC)

    depth = re.search(r"^\s*traversal_depth:\s*(\d+)\b", manifest, re.MULTILINE)
    if not depth or int(depth.group(1)) != 5:
        raise SystemExit("invalid_traversal_depth")

    states = _section_items(manifest, "states")
    expected_states = ["VOID", "BASELINE", "CANDIDATE", "VALIDATED", "ROLLBACK"]
    if states != expected_states:
        raise SystemExit(f"invalid_states: {states!r}")

    architectures = re.findall(r"- id:\s*([^;]+);\s*required_fallback:\s*true;", manifest)
    if len(architectures) < 7:
        raise SystemExit(f"insufficient_architectures: {len(architectures)}")

    min_batch = re.search(r"^\s*min_batch_gpu:\s*(\d+)\b", manifest, re.MULTILINE)
    if not min_batch or int(min_batch.group(1)) < 1:
        raise SystemExit("invalid_min_batch_gpu")

    for key in ("implementation", "test"):
        match = re.search(rf"^\s*{key}:\s*(\S+)\s*$", manifest, re.MULTILINE)
        if not match:
            raise SystemExit(f"missing_runtime_router_{key}")
        target = ROOT / match.group(1)
        if not target.exists():
            raise SystemExit(f"missing_runtime_router_{key}_file: {match.group(1)}")

    required_terms = ["Fail-safe", "Failover", "Rollback", "Mitigação", "TOKEN_VAZIO", "morph-on-runtime"]
    missing_terms = [term for term in required_terms if term not in doc]
    if missing_terms:
        raise SystemExit("missing_doc_terms: " + ", ".join(missing_terms))

    print({
        "manifest": str(MANIFEST.relative_to(ROOT)),
        "doc": str(DOC.relative_to(ROOT)),
        "states": len(states),
        "architectures": len(architectures),
        "min_batch_gpu": int(min_batch.group(1)),
        "verdict": "PASS",
    })


if __name__ == "__main__":
    main()
