#!/usr/bin/env python3
import importlib.util
import tempfile
import unittest
from pathlib import Path

P = Path(__file__).parents[1] / "scripts" / "daily_bibliography_evolution.py"
spec = importlib.util.spec_from_file_location("daily_biblio", P)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


class DailyBibliographyInvariantTests(unittest.TestCase):
    def test_doi_normalization(self):
        self.assertEqual(mod.norm_doi("https://doi.org/10.1234/ABC."), "10.1234/abc")

    def test_identity_prefers_doi(self):
        r = {"source": "arxiv", "doi": "10.1/x", "arxiv_id": "2608.00001"}
        self.assertEqual(mod.source_identity(r), "doi:10.1/x")

    def test_arxiv_versions_share_stable_identity(self):
        a = {"source": "arxiv", "arxiv_id": "2608.00001v1"}
        b = {"source": "arxiv", "arxiv_id": "2608.00001v4"}
        self.assertEqual(mod.source_identity(a), "arxiv:2608.00001")
        self.assertEqual(mod.source_identity(a), mod.source_identity(b))
        self.assertEqual(mod.arxiv_version("2608.00001v4"), 4)

    def test_occurrences_preserved_and_identity_merged(self):
        records = [
            {"source": "zenodo", "doi": "10.1/x", "provider_id": "1", "raw_sha256": "a", "title": "A", "claim_allowed": False},
            {"source": "arxiv", "doi": "10.1/x", "provider_id": "2608.1", "raw_sha256": "b", "title": "A", "claim_allowed": False},
        ]
        ids, occ = mod.dedupe(records)
        self.assertEqual(len(ids), 1)
        self.assertEqual(len(occ), 2)
        self.assertEqual(ids[0]["occurrence_count"], 2)
        self.assertEqual(sorted(ids[0]["sources"]), ["arxiv", "zenodo"])

    def test_field_comparison_detects_title_conflict(self):
        records = [
            {
                "source": "zenodo", "doi": "10.1/x", "provider_id": "1",
                "title": "Alpha Result", "published": "2026-01-01",
                "creators": [{"name": "A. Author"}], "raw_sha256": "a", "claim_allowed": False,
            },
            {
                "source": "arxiv", "doi": "10.1/x", "provider_id": "2",
                "title": "Different Result", "published": "2026-02-01",
                "creators": ["A. Author"], "raw_sha256": "b", "claim_allowed": False,
            },
        ]
        rows = mod.field_comparison_receipts(records)
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["state"], "CONTRADICTED")
        self.assertEqual(rows[0]["checks"]["title"]["state"], "CONTRADICTED")
        self.assertIs(rows[0]["claim_allowed"], False)

    def test_field_comparison_preserves_missing_as_token_vazio(self):
        records = [
            {
                "source": "zenodo", "doi": "10.1/x", "provider_id": "1",
                "title": "Alpha Result", "published": "2026-01-01",
                "creators": [{"name": "A. Author"}], "raw_sha256": "a", "claim_allowed": False,
            },
            {
                "source": "orcid", "doi": "10.1/x", "provider_id": "2",
                "title": "Alpha Result", "published": {"year": {"value": "2026"}},
                "creators": [], "raw_sha256": "b", "claim_allowed": False,
            },
        ]
        row = mod.field_comparison_receipts(records)[0]
        self.assertEqual(row["checks"]["creator_overlap"]["state"], "TOKEN_VAZIO")
        self.assertEqual(row["state"], "TOKEN_VAZIO")

    def test_hypothesis_binds_field_test_state(self):
        identities = [{"identity": "doi:10.1/x", "sources": ["zenodo", "arxiv"]}]
        comparisons = [{"identity": "doi:10.1/x", "state": "CONTRADICTED"}]
        h = mod.hypothesis_candidates(identities, comparisons)
        self.assertEqual(len(h), 1)
        self.assertEqual(h[0]["state"], "TESTABLE")
        self.assertEqual(h[0]["field_test_state"], "CONTRADICTED")
        self.assertIs(h[0]["claim_allowed"], False)

    def test_manifest_verification_detects_tamper(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "a.json").write_text('{"x":1}\n', encoding="utf-8")
            mod.write_manifest(root, ["a.json"])
            ok, problems = mod.verify_manifest(root)
            self.assertTrue(ok)
            self.assertEqual(problems, [])
            (root / "a.json").write_text('{"x":2}\n', encoding="utf-8")
            ok, problems = mod.verify_manifest(root)
            self.assertFalse(ok)
            self.assertTrue(any(x.startswith("sha256:a.json:") for x in problems))


if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(DailyBibliographyInvariantTests)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        raise SystemExit(1)
    print("daily bibliography offline invariants: PASS")
