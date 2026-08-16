#!/usr/bin/env python3
"""Deterministic read-only cross-repository artifact bridge V1.

Governance anchor: CLOSURE_L11.

The bridge consumes JSONL artifact references from stdin or a file, validates a
strict allowlist, projects typed nodes/edges, and emits a deterministic receipt.
It never reads source repositories by path and never accepts arbitrary payload
fields such as typed text, clipboard contents, keystrokes, or telemetry blobs.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any, Iterable

SCHEMA = "raf.cross_repo_artifact_ref.v1"
OUTPUT_SCHEMA = "raf.cross_repo_bridge.v1"
RECEIPT_SCHEMA = "raf.cross_repo_bridge_receipt.v1"
TOKEN = "TOKEN_VAZIO"
ALLOWED_KEYS = {
    "schema", "repo", "path", "commit_sha", "content_hash", "media_type",
    "provenance_state", "artifact_kind", "claim_allowed", "producer", "relation_hints",
}
REQUIRED_KEYS = {
    "schema", "repo", "path", "commit_sha", "content_hash", "media_type",
    "provenance_state", "artifact_kind", "claim_allowed",
}
PROVENANCE_STATES = {"VERIFIED", "OBSERVED", "REFERENCE", TOKEN}
ARTIFACT_KINDS = {"source", "code", "build", "runtime", "receipt", "claim", TOKEN}
RELATIONS = {"derived_from", "supports", "contradicts", "supersedes", "requires", "observed_in"}
SHA40 = re.compile(r"^[a-f0-9]{40}$")
SHA256 = re.compile(r"^sha256:[a-f0-9]{64}$")


class BridgeError(ValueError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


def digest(value: Any) -> str:
    return "sha256:" + hashlib.sha256(canonical_bytes(value)).hexdigest()


def _require_string(record: dict[str, Any], key: str) -> str:
    value = record.get(key)
    if not isinstance(value, str) or not value:
        raise BridgeError(f"{key}: non-empty string required")
    return value


def validate_artifact_ref(record: Any) -> dict[str, Any]:
    if not isinstance(record, dict):
        raise BridgeError("artifact_ref must be an object")
    unknown = sorted(set(record) - ALLOWED_KEYS)
    if unknown:
        raise BridgeError(f"unknown fields rejected by privacy allowlist: {unknown}")
    missing = sorted(REQUIRED_KEYS - set(record))
    if missing:
        raise BridgeError(f"missing required fields: {missing}")
    if record.get("schema") != SCHEMA:
        raise BridgeError(f"schema must be {SCHEMA}")

    repo = _require_string(record, "repo")
    path = _require_string(record, "path")
    commit_sha = _require_string(record, "commit_sha")
    content_hash = _require_string(record, "content_hash")
    media_type = _require_string(record, "media_type")
    provenance_state = record.get("provenance_state")
    artifact_kind = record.get("artifact_kind")
    claim_allowed = record.get("claim_allowed")

    if commit_sha != TOKEN and not SHA40.fullmatch(commit_sha):
        raise BridgeError("commit_sha must be 40 lowercase hex or TOKEN_VAZIO")
    if content_hash != TOKEN and not SHA256.fullmatch(content_hash):
        raise BridgeError("content_hash must be sha256:<64 lowercase hex> or TOKEN_VAZIO")
    if provenance_state not in PROVENANCE_STATES:
        raise BridgeError(f"invalid provenance_state: {provenance_state!r}")
    if artifact_kind not in ARTIFACT_KINDS:
        raise BridgeError(f"invalid artifact_kind: {artifact_kind!r}")
    if not isinstance(claim_allowed, bool):
        raise BridgeError("claim_allowed must be boolean")

    if TOKEN in {commit_sha, content_hash, provenance_state, artifact_kind} and claim_allowed:
        raise BridgeError("TOKEN_VAZIO artifact_ref requires claim_allowed=false")
    if provenance_state != "VERIFIED" and claim_allowed:
        raise BridgeError("claim_allowed=true requires provenance_state=VERIFIED")
    if artifact_kind == "claim" and claim_allowed and content_hash == TOKEN:
        raise BridgeError("claim cannot be allowed without content identity")

    producer = record.get("producer")
    if producer is not None and (not isinstance(producer, str) or not producer):
        raise BridgeError("producer must be a non-empty string when present")

    relation_hints = record.get("relation_hints", [])
    if not isinstance(relation_hints, list):
        raise BridgeError("relation_hints must be an array")
    normalized_hints: list[dict[str, str]] = []
    for index, hint in enumerate(relation_hints):
        if not isinstance(hint, dict):
            raise BridgeError(f"relation_hints[{index}] must be an object")
        if set(hint) != {"relation", "target_content_hash"}:
            raise BridgeError(f"relation_hints[{index}] has forbidden or missing fields")
        relation = hint.get("relation")
        target_hash = hint.get("target_content_hash")
        if relation not in RELATIONS:
            raise BridgeError(f"relation_hints[{index}]: invalid relation")
        if target_hash != TOKEN and (not isinstance(target_hash, str) or not SHA256.fullmatch(target_hash)):
            raise BridgeError(f"relation_hints[{index}]: invalid target_content_hash")
        normalized_hints.append({"relation": relation, "target_content_hash": target_hash})

    normalized = {
        "schema": SCHEMA,
        "repo": repo,
        "path": path,
        "commit_sha": commit_sha,
        "content_hash": content_hash,
        "media_type": media_type,
        "provenance_state": provenance_state,
        "artifact_kind": artifact_kind,
        "claim_allowed": claim_allowed,
    }
    if producer is not None:
        normalized["producer"] = producer
    if normalized_hints:
        normalized["relation_hints"] = sorted(
            normalized_hints, key=lambda item: (item["relation"], item["target_content_hash"])
        )
    return normalized


def parse_jsonl(lines: Iterable[str]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for line_number, raw in enumerate(lines, 1):
        raw = raw.strip()
        if not raw:
            continue
        try:
            value = json.loads(raw)
        except json.JSONDecodeError as exc:
            raise BridgeError(f"line {line_number}: invalid JSON: {exc.msg}") from exc
        try:
            records.append(validate_artifact_ref(value))
        except BridgeError as exc:
            raise BridgeError(f"line {line_number}: {exc}") from exc
    if not records:
        raise BridgeError("manifest is empty")
    return records


def node_id(record: dict[str, Any]) -> str:
    identity = {
        "repo": record["repo"],
        "path": record["path"],
        "commit_sha": record["commit_sha"],
        "content_hash": record["content_hash"],
        "artifact_kind": record["artifact_kind"],
    }
    return digest(identity)


def build_bridge(records: list[dict[str, Any]]) -> dict[str, Any]:
    ordered = sorted(records, key=lambda item: canonical_bytes(item))
    nodes: list[dict[str, Any]] = []
    by_hash: dict[str, str] = {}
    for record in ordered:
        identifier = node_id(record)
        node = {
            "id": identifier,
            "type": record["artifact_kind"],
            "repo": record["repo"],
            "path": record["path"],
            "commit_sha": record["commit_sha"],
            "content_hash": record["content_hash"],
            "media_type": record["media_type"],
            "provenance_state": record["provenance_state"],
            "claim_allowed": record["claim_allowed"],
        }
        if "producer" in record:
            node["producer"] = record["producer"]
        nodes.append(node)
        if record["content_hash"] != TOKEN:
            by_hash[record["content_hash"]] = identifier

    edges: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []
    for record in ordered:
        source = node_id(record)
        for hint in record.get("relation_hints", []):
            target_hash = hint["target_content_hash"]
            target = by_hash.get(target_hash)
            if target is None:
                unresolved.append({
                    "from": source,
                    "relation": hint["relation"],
                    "target_content_hash": target_hash,
                    "state": TOKEN,
                    "claim_allowed": False,
                })
                continue
            edges.append({
                "from": source,
                "to": target,
                "relation": hint["relation"],
            })

    nodes.sort(key=lambda item: item["id"])
    edges.sort(key=lambda item: (item["from"], item["relation"], item["to"]))
    unresolved.sort(key=lambda item: (item["from"], item["relation"], item["target_content_hash"]))
    graph = {
        "schema": OUTPUT_SCHEMA,
        "version": "1.0.0",
        "mode": "READ_ONLY",
        "privacy_boundary": "METADATA_ALLOWLIST_NO_TYPED_CONTENT",
        "nodes": nodes,
        "edges": edges,
        "unresolved_edges": unresolved,
    }
    graph["graph_hash"] = digest(graph)
    return graph


def make_receipt(records: list[dict[str, Any]], graph: dict[str, Any]) -> dict[str, Any]:
    normalized_input = sorted(records, key=lambda item: canonical_bytes(item))
    return {
        "schema": RECEIPT_SCHEMA,
        "bridge_version": "1.0.0",
        "artifact_ref_schema": SCHEMA,
        "input_count": len(records),
        "input_hash": digest(normalized_input),
        "output_hash": digest(graph),
        "graph_hash": graph["graph_hash"],
        "claim_allowed": False,
        "note": "Receipt proves deterministic transformation only; it does not independently verify producer claims.",
    }


def _open_input(path: str):
    if path == "-":
        return sys.stdin
    return Path(path).open("r", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Build deterministic cross_repo_bridge_v1 from artifact_ref JSONL")
    parser.add_argument("input", nargs="?", default="-", help="JSONL manifest path or - for stdin")
    parser.add_argument("--output", help="write deterministic graph JSON")
    parser.add_argument("--receipt", help="write deterministic receipt JSON")
    args = parser.parse_args(argv)

    try:
        handle = _open_input(args.input)
        close_handle = handle is not sys.stdin
        try:
            records = parse_jsonl(handle)
        finally:
            if close_handle:
                handle.close()
        graph = build_bridge(records)
        receipt = make_receipt(records, graph)
    except (OSError, BridgeError) as exc:
        print(f"cross_repo_bridge_v1: FAIL: {exc}", file=sys.stderr)
        return 2

    graph_payload = json.dumps(graph, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.output:
        Path(args.output).parent.mkdir(parents=True, exist_ok=True)
        Path(args.output).write_text(graph_payload, encoding="utf-8")
    else:
        sys.stdout.write(graph_payload)
    if args.receipt:
        Path(args.receipt).parent.mkdir(parents=True, exist_ok=True)
        Path(args.receipt).write_text(
            json.dumps(receipt, ensure_ascii=False, sort_keys=True, indent=2) + "\n",
            encoding="utf-8",
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
