#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "document_governance", ROOT / "scripts/document_governance.py"
)
assert SPEC and SPEC.loader
DG = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = DG
SPEC.loader.exec_module(DG)


class DocumentGovernanceTests(unittest.TestCase):
    def make_repo(self) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        td = tempfile.TemporaryDirectory()
        root = Path(td.name)
        for name in ("docs", "scripts", "configs", "tests", "results"):
            (root / name).mkdir()
        (root / ".git").mkdir()
        policy = {
            "schema": "raf.document-governance-policy.v1",
            "version": "1.0.0",
            "canonical_indexes": ["README.md", "docs/INDEX.md"],
            "scan": {
                "max_text_bytes": 100000,
                "exclude_globs": ["results/document-governance/**"],
                "generated_prefixes": ["results/", "docs/generated/"],
            },
            "root_policy": {
                "allowed_files": ["README.md"],
                "allowed_prefixes": ["RAF_"],
            },
            "areas": [
                {
                    "id": "docs",
                    "prefixes": ["docs/"],
                    "owner_role": "doc-owner",
                    "classification": "PUBLIC",
                    "review_interval_days": 180,
                    "lifecycle": "ACTIVE",
                },
                {
                    "id": "root",
                    "prefixes": [""],
                    "owner_role": "maintainer",
                    "classification": "INTERNAL",
                    "review_interval_days": 90,
                    "lifecycle": "ACTIVE",
                },
            ],
            "sensitivity": {
                "filename_globs": [
                    {"id": "private_key_filename", "glob": "*private*.key"}
                ]
            },
            "blocking_routes": ["QUARANTINE_REVIEW"],
            "outputs": {
                "summary": "results/document-governance/summary.json",
                "catalog": "results/document-governance/catalog.jsonl",
                "relations": "results/document-governance/relations.jsonl",
                "duplicates": "results/document-governance/duplicates.json",
                "review_json": "results/document-governance/review-queue.json",
                "index_markdown": "docs/generated/DOCUMENT_GOVERNANCE_INDEX.md",
                "review_markdown": "docs/generated/DOCUMENT_REVIEW_QUEUE.md",
            },
        }
        (root / "configs/document-governance.v1.json").write_text(
            json.dumps(policy), encoding="utf-8"
        )
        return td, root

    def test_reference_graph_and_broken_link(self) -> None:
        td, root = self.make_repo()
        try:
            (root / "README.md").write_text(
                "# Root\n[Índice](docs/INDEX.md)\n", encoding="utf-8"
            )
            (root / "docs/INDEX.md").write_text(
                "# Index\n[Documento](guide.md)\n[Quebrado](missing.md)\n",
                encoding="utf-8",
            )
            (root / "docs/guide.md").write_text("# Guide\nPENDING\n", encoding="utf-8")
            paths = [root / "README.md", root / "docs/INDEX.md", root / "docs/guide.md"]
            relations, broken = DG.extract_relations(
                root, paths, DG.load_policy(root / "configs/document-governance.v1.json")
            )
            edges = {(r.source, r.target) for r in relations}
            self.assertIn(("README.md", "docs/INDEX.md"), edges)
            self.assertIn(("docs/INDEX.md", "docs/guide.md"), edges)
            self.assertEqual(broken["docs/INDEX.md"], ["missing.md"])
        finally:
            td.cleanup()

    def test_duplicate_and_normalized_duplicate_detection(self) -> None:
        record_a = DG.Record(
            path="docs/a.md", sha256="same", normalized_sha256="norm",
            size_bytes=300, line_count=2, media_class="documentation",
            area="docs", owner_role="doc-owner", classification="PUBLIC",
            lifecycle="ACTIVE", evidence_grade="E1", indexed=False,
            inbound_references=0, outbound_references=0,
            last_commit="x", last_modified_at="2026-01-01T00:00:00Z",
            age_days=1, review_interval_days=180, review_due=False,
        )
        record_b = DG.Record(
            path="docs/b.md", sha256="same", normalized_sha256="norm",
            size_bytes=300, line_count=2, media_class="documentation",
            area="docs", owner_role="doc-owner", classification="PUBLIC",
            lifecycle="ACTIVE", evidence_grade="E1", indexed=False,
            inbound_references=0, outbound_references=0,
            last_commit="x", last_modified_at="2026-01-01T00:00:00Z",
            age_days=1, review_interval_days=180, review_due=False,
        )
        self.assertEqual(
            DG.duplicate_groups([record_a, record_b], "sha256"),
            {"same": ["docs/a.md", "docs/b.md"]},
        )
        self.assertEqual(
            DG.duplicate_groups([record_a, record_b], "normalized_sha256"),
            {"norm": ["docs/a.md", "docs/b.md"]},
        )

    def test_sensitive_private_key_is_blocking(self) -> None:
        td, root = self.make_repo()
        try:
            policy = DG.load_policy(root / "configs/document-governance.v1.json")
            flags = DG.sensitivity_flags(
                "private-signing.key",
                "-----BEGIN PRIVATE KEY-----\nabc\n",
                policy,
            )
            self.assertIn("private_key_filename", flags)
            self.assertIn("private_key_block", flags)
            record = DG.Record(
                path="private-signing.key", sha256="x", normalized_sha256=None,
                size_bytes=20, line_count=2, media_class="text",
                area="root", owner_role="maintainer", classification="INTERNAL",
                lifecycle="ACTIVE", evidence_grade="E0", indexed=False,
                inbound_references=0, outbound_references=0,
                last_commit="x", last_modified_at="2026-01-01T00:00:00Z",
                age_days=1, review_interval_days=90, review_due=False,
                sensitivity_flags=flags,
            )
            route, _ = DG.route_record(record, set())
            self.assertEqual(route, "QUARANTINE_REVIEW")
        finally:
            td.cleanup()

    def test_root_policy_violation_is_reviewed(self) -> None:
        td, root = self.make_repo()
        try:
            policy = DG.load_policy(root / "configs/document-governance.v1.json")
            self.assertEqual(
                DG.root_policy_flags("mystery.txt", policy),
                ["root_file_outside_policy"],
            )
            self.assertEqual(DG.root_policy_flags("RAF_001_test.c", policy), [])
        finally:
            td.cleanup()

    def test_output_is_deterministic_for_same_report(self) -> None:
        report = {
            "schema": DG.SCHEMA,
            "generated_at": "2026-07-19T00:00:00Z",
            "commit": "abc",
            "state": "PASS",
            "claim_allowed": True,
            "policy_version": "1.0.0",
            "summary": {
                "files": 0,
                "relations": 0,
                "broken_reference_sources": 0,
                "exact_duplicate_groups": 0,
                "normalized_duplicate_groups": 0,
                "review_queue": 0,
                "blockers": 0,
                "missing_canonical_indexes": [],
                "routes": {},
                "areas": {},
                "classifications": {},
            },
            "records": [],
            "relations": [],
            "duplicates": {"exact": {}, "normalized_candidates": {}},
            "review_queue": [],
            "blockers": [],
            "broken_references": {},
        }
        policy = {
            "outputs": {
                "summary": "summary.json",
                "catalog": "catalog.jsonl",
                "relations": "relations.jsonl",
                "duplicates": "duplicates.json",
                "review_json": "review.json",
                "index_markdown": "index.md",
                "review_markdown": "queue.md",
            }
        }
        self.assertEqual(
            DG.output_payloads(report, policy),
            DG.output_payloads(report, policy),
        )


if __name__ == "__main__":
    unittest.main()
