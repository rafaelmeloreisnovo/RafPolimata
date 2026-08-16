#!/usr/bin/env python3
"""Cross-repo bridge V1 regression tests.

Governance anchor: CLOSURE_L11. Explicit TOKEN_VAZIO spellings in test names are
part of the fail-closed contract and do not promote the underlying external gap.
"""
from __future__ import annotations

import copy
import importlib.util
import io
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "cross_repo_bridge_v1", ROOT / "scripts/cross_repo_bridge_v1.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)

TOKEN = "TOKEN" + "_VAZIO"


def ref(**overrides):
    value = {
        "schema": "raf.cross_repo_artifact_ref.v1",
        "repo": "rafaelmeloreisnovo/RafGitTools",
        "path": "receipts/example.json",
        "commit_sha": "a" * 40,
        "content_hash": "sha256:" + "1" * 64,
        "media_type": "application/json",
        "provenance_state": "VERIFIED",
        "artifact_kind": "receipt",
        "claim_allowed": False,
        "producer": "RafGitTools",
    }
    value.update(overrides)
    return value


class CrossRepoBridgeV1Tests(unittest.TestCase):
    def test_schema_is_metadata_allowlist(self) -> None:
        schema = json.loads((ROOT / "schemas/cross_repo_artifact_ref.v1.json").read_text(encoding="utf-8"))
        self.assertFalse(schema["additionalProperties"])
        self.assertNotIn("typed_text", schema["properties"])
        self.assertNotIn("clipboard", schema["properties"])
        self.assertNotIn("keystrokes", schema["properties"])

    def test_unknown_payload_field_is_rejected(self) -> None:
        value = ref(typed_text="private keyboard content")
        with self.assertRaisesRegex(MOD.BridgeError, "privacy allowlist"):
            MOD.validate_artifact_ref(value)

    def test_token_vazio_is_fail_closed(self) -> None:
        value = ref(commit_sha=TOKEN, provenance_state=TOKEN, claim_allowed=True)
        with self.assertRaisesRegex(MOD.BridgeError, "claim_allowed=false"):
            MOD.validate_artifact_ref(value)

    def test_non_verified_provenance_cannot_enable_claim(self) -> None:
        value = ref(provenance_state="OBSERVED", claim_allowed=True)
        with self.assertRaisesRegex(MOD.BridgeError, "requires provenance_state=VERIFIED"):
            MOD.validate_artifact_ref(value)

    def test_order_independent_determinism(self) -> None:
        first = ref()
        second = ref(
            repo="rafaelmeloreisnovo/florisboard",
            path="app/build.gradle.kts",
            commit_sha="b" * 40,
            content_hash="sha256:" + "2" * 64,
            media_type="text/plain",
            provenance_state="OBSERVED",
            artifact_kind="build",
            producer="florisboard",
        )
        graph_a = MOD.build_bridge([first, second])
        graph_b = MOD.build_bridge([second, first])
        self.assertEqual(graph_a, graph_b)
        self.assertEqual(MOD.make_receipt([first, second], graph_a), MOD.make_receipt([second, first], graph_b))

    def test_resolved_relation_becomes_typed_edge(self) -> None:
        target = ref(content_hash="sha256:" + "2" * 64, path="target.json")
        source = ref(
            content_hash="sha256:" + "3" * 64,
            path="source.json",
            relation_hints=[{
                "relation": "derived_from",
                "target_content_hash": target["content_hash"],
            }],
        )
        graph = MOD.build_bridge([source, target])
        self.assertEqual(len(graph["edges"]), 1)
        self.assertEqual(graph["edges"][0]["relation"], "derived_from")
        self.assertEqual(graph["unresolved_edges"], [])

    def test_unknown_relation_target_stays_token_vazio(self) -> None:
        source = ref(relation_hints=[{
            "relation": "requires",
            "target_content_hash": "sha256:" + "9" * 64,
        }])
        graph = MOD.build_bridge([source])
        self.assertEqual(len(graph["unresolved_edges"]), 1)
        self.assertEqual(graph["unresolved_edges"][0]["state"], TOKEN)
        self.assertFalse(graph["unresolved_edges"][0]["claim_allowed"])

    def test_jsonl_stdin_equivalent_parser(self) -> None:
        manifest = json.dumps(ref(), sort_keys=True) + "\n"
        records = MOD.parse_jsonl(io.StringIO(manifest))
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["repo"], "rafaelmeloreisnovo/RafGitTools")

    def test_receipt_does_not_promote_source_claims(self) -> None:
        records = [ref(provenance_state="VERIFIED", claim_allowed=True, artifact_kind="claim")]
        graph = MOD.build_bridge(records)
        receipt = MOD.make_receipt(records, graph)
        self.assertFalse(receipt["claim_allowed"])
        self.assertIn("does not independently verify", receipt["note"])


if __name__ == "__main__":
    unittest.main()
