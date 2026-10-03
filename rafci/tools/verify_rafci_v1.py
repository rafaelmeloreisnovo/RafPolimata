#!/usr/bin/env python3
"""Fail-closed source/graph validator for RafCI V1.

Stdlib-only by design. This validates semantic wiring and can emit a bounded
source-contract receipt. It does not execute target binaries or provider gates.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import sys
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
RAFCI = ROOT / "rafci"
GRAPH_PATH = RAFCI / "graph.v1.json"
HEADER_PATH = RAFCI / "contracts" / "rafci_wire_v1.h"
ANCHOR_PATH = RAFCI / "contracts" / "rafci_anchors_v1.c"

BIT_BLOCK_RE = re.compile(r"/\*\s*RAFCI-BIT(?P<body>.*?)\*/", re.DOTALL)
FIELD_RE = re.compile(r"(?m)^\s*\*?\s*(ID|KIND|ROUTE|AUTHORITY|EVIDENCE)=([^\r\n]+)\s*$")
DEFINE_RE = re.compile(r"(?m)^\s*#define\s+(RAFCI_(?:CAP|GATE)_[A-Z0-9_]+_BIT)\s+([0-9]+)u\s*$")


class ContractError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def load_graph() -> dict[str, Any]:
    with GRAPH_PATH.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def _unique(values: list[Any], label: str) -> None:
    require(len(values) == len(set(values)), f"duplicate {label}")


def validate_graph(graph: dict[str, Any]) -> None:
    require(graph.get("schema") == "rafaelia.rafci.graph/v1", "unexpected graph schema")
    require(graph.get("claim_allowed") is False, "claim_allowed must remain false in source contract")
    authority = graph.get("authority", {})
    require(authority.get("id") == "authority.rafpolimata.rafci", "unexpected authority id")
    require(authority.get("repository") == "rafaelmeloreisnovo/RafPolimata", "unexpected authority repository")
    require(authority.get("authority_transfer_allowed") is False, "authority transfer must be false")

    providers = graph.get("providers", [])
    provider_ids = [p.get("id") for p in providers]
    _unique(provider_ids, "provider id")
    provider_roles = [p.get("role") for p in providers]
    authority_providers = [p for p in providers if p.get("role") == "AUTHORITY"]
    require(len(authority_providers) == 1, "exactly one AUTHORITY provider required")
    require(authority_providers[0].get("id") == "provider.rafpolimata", "RafPolimata must be RafCI AUTHORITY")
    required_roles = {"AUTHORITY", "OPERATOR", "ACTION_PROVIDER", "HOST_PROVIDER", "TOOLCHAIN_PROVIDER"}
    require(required_roles.issubset(set(provider_roles)), "required provider role missing")

    stages = graph.get("stages", [])
    stage_ids = [s.get("id") for s in stages]
    _unique(stage_ids, "stage id")
    expected_stages = [
        "stage.source",
        "stage.artifact",
        "stage.execution",
        "stage.evidence",
        "stage.claim",
    ]
    require(stage_ids == expected_stages, "evidence stages must preserve canonical order")

    edges = graph.get("edges", [])
    edge_ids = [e.get("id") for e in edges]
    _unique(edge_ids, "edge id")
    stage_set = set(stage_ids)
    for edge in edges:
        require(edge.get("from") in stage_set, f"edge source missing: {edge.get('id')}")
        require(edge.get("to") in stage_set, f"edge target missing: {edge.get('id')}")
    expected_pairs = list(zip(expected_stages, expected_stages[1:]))
    actual_pairs = [(e.get("from"), e.get("to")) for e in edges]
    require(actual_pairs == expected_pairs, "evidence chain must be direct and acyclic")

    provider_set = set(provider_ids)
    for binding in graph.get("provider_bindings", []):
        require(binding.get("provider") in provider_set, "provider binding references unknown provider")
        for stage in binding.get("stages", []):
            require(stage in stage_set, "provider binding references unknown stage")

    all_bit_ids: list[str] = []
    for namespace in ("capabilities", "gates"):
        entries = graph.get("bit_dictionary", {}).get(namespace, [])
        ids = [entry.get("id") for entry in entries]
        bits = [entry.get("bit") for entry in entries]
        symbols = [entry.get("symbol") for entry in entries]
        _unique(ids, f"{namespace} semantic id")
        _unique(bits, f"{namespace} bit index")
        _unique(symbols, f"{namespace} C symbol")
        require(all(isinstance(bit, int) and 0 <= bit < 64 for bit in bits), f"{namespace} bit outside u64 mask")
        all_bit_ids.extend(ids)
    _unique(all_bit_ids, "cross-namespace bit semantic id")

    anchors = graph.get("anchors", [])
    _unique([a.get("id") for a in anchors], "anchor id")
    _unique([a.get("section") for a in anchors], "anchor section")
    _unique([a.get("symbol") for a in anchors], "anchor symbol")

    gaps = graph.get("gaps", [])
    _unique([g.get("id") for g in gaps], "gap id")
    for gap in gaps:
        require(gap.get("state") == "TOKEN_VAZIO", f"open gap must remain TOKEN_VAZIO: {gap.get('id')}")
        require(bool(gap.get("evidence_rule")), f"gap lacks evidence rule: {gap.get('id')}")

    global_ids = [
        authority.get("id"),
        *provider_ids,
        *stage_ids,
        *edge_ids,
        *[a.get("id") for a in anchors],
        *[g.get("id") for g in gaps],
        *all_bit_ids,
    ]
    _unique(global_ids, "global graph id")


def validate_header(graph: dict[str, Any], header_text: str) -> None:
    defines = {name: int(bit) for name, bit in DEFINE_RE.findall(header_text)}
    expected: dict[str, int] = {}
    for namespace in ("capabilities", "gates"):
        for entry in graph["bit_dictionary"][namespace]:
            expected[entry["symbol"]] = entry["bit"]
    require(defines == expected, "graph/header bit dictionary mismatch")
    require("RAFCI_STATUS_TOKEN_VAZIO 4u" in header_text, "TOKEN_VAZIO wire status missing")
    require("_Static_assert(sizeof(rafci_job_v1) == 88u" in header_text, "job layout assertion missing")
    require("_Static_assert(sizeof(rafci_receipt_v1) == 152u" in header_text, "receipt layout assertion missing")


def validate_bit_comments() -> int:
    seen: set[str] = set()
    count = 0
    for path in sorted(RAFCI.rglob("*")):
        if path.suffix not in {".c", ".h"}:
            continue
        text = path.read_text(encoding="utf-8")
        blocks = BIT_BLOCK_RE.findall(text)
        for body in blocks:
            fields = {key: value.strip() for key, value in FIELD_RE.findall(body)}
            missing = {"ID", "KIND", "ROUTE", "AUTHORITY", "EVIDENCE"} - set(fields)
            require(not missing, f"{path}: RAFCI-BIT missing fields {sorted(missing)}")
            bit_id = fields["ID"]
            require(bit_id not in seen, f"duplicate RAFCI-BIT ID: {bit_id}")
            require(fields["AUTHORITY"] == "authority.rafpolimata.rafci", f"{path}: unexpected RAFCI-BIT authority")
            seen.add(bit_id)
            count += 1
    require(count >= 7, "too few RAFCI-BIT blocks; semantic anchors may be missing")
    return count


def validate_anchors(graph: dict[str, Any], source_text: str) -> None:
    for anchor in graph["anchors"]:
        require(anchor["section"] in source_text, f"missing anchor section {anchor['section']}")
        require(anchor["symbol"] in source_text, f"missing anchor symbol {anchor['symbol']}")


def validate_all() -> tuple[dict[str, Any], int]:
    graph = load_graph()
    validate_graph(graph)
    header_text = HEADER_PATH.read_text(encoding="utf-8")
    anchor_text = ANCHOR_PATH.read_text(encoding="utf-8")
    validate_header(graph, header_text)
    bit_count = validate_bit_comments()
    validate_anchors(graph, anchor_text)
    return graph, bit_count


def expect_rejected(graph: dict[str, Any], mutator, label: str) -> None:
    mutant = copy.deepcopy(graph)
    mutator(mutant)
    try:
        validate_graph(mutant)
    except ContractError:
        return
    raise ContractError(f"selftest mutation was not rejected: {label}")


def selftest(graph: dict[str, Any]) -> None:
    expect_rejected(graph, lambda g: g.__setitem__("claim_allowed", True), "claim promotion")
    expect_rejected(
        graph,
        lambda g: g["providers"].__setitem__(1, copy.deepcopy(g["providers"][0])),
        "duplicate provider identity",
    )
    expect_rejected(
        graph,
        lambda g: g["bit_dictionary"]["gates"][1].__setitem__("bit", g["bit_dictionary"]["gates"][0]["bit"]),
        "duplicate gate bit",
    )
    expect_rejected(
        graph,
        lambda g: g["edges"][0].__setitem__("to", "stage.missing"),
        "missing route endpoint",
    )


def write_receipt(path: Path, graph: dict[str, Any], bit_count: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    receipt = {
        "schema": "rafaelia.rafci.source-contract-receipt/v1",
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "commit": os.environ.get("GITHUB_SHA", "TOKEN_VAZIO"),
        "scope": "graph+bit-dictionary+comments+wire-layout+anchor-source",
        "status": "PASS",
        "claim_allowed": False,
        "physical_execution": "TOKEN_VAZIO",
        "provider_enforcement": "TOKEN_VAZIO",
        "semantic_bit_blocks": bit_count,
        "inputs": {
            "graph_sha256": sha256_file(GRAPH_PATH),
            "wire_header_sha256": sha256_file(HEADER_PATH),
            "anchor_source_sha256": sha256_file(ANCHOR_PATH),
        },
    }
    path.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()

    try:
        graph, bit_count = validate_all()
        if args.selftest:
            selftest(graph)
        if args.receipt:
            write_receipt(args.receipt, graph, bit_count)
    except (ContractError, OSError, json.JSONDecodeError) as exc:
        print(f"RAFCI_CONTRACT_FAIL: {exc}", file=sys.stderr)
        return 1

    print(
        "RAFCI_CONTRACT_PASS "
        f"providers={len(graph['providers'])} "
        f"capabilities={len(graph['bit_dictionary']['capabilities'])} "
        f"gates={len(graph['bit_dictionary']['gates'])} "
        f"semantic_bits={bit_count}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
