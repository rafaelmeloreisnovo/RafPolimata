#!/usr/bin/env python3
"""Offline deterministic integrity tests for the Science Learning Engine."""
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from scripts import science_learning_engine_v2 as sle  # noqa: E402


class ScienceLearningEngineIntegrityTests(unittest.TestCase):
    def test_doi_normalization(self):
        self.assertEqual(sle._normalize_doi("https://doi.org/10.1000/ABC."), "10.1000/abc")
        self.assertIsNone(sle._normalize_doi(None))

    def test_orcid_search_results_are_ids_not_works(self):
        payload = {
            "result": [
                {"orcid-identifier": {"path": "0000-0001-2345-6789"}},
                {"orcid-identifier": {"uri": "https://orcid.org/0000-0002-0000-0001"}},
            ]
        }
        self.assertEqual(
            sle._extract_orcid_ids(payload),
            ["0000-0001-2345-6789", "0000-0002-0000-0001"],
        )

    def test_orcid_two_phase_resolution(self):
        search = {"result": [{"orcid-identifier": {"path": "0000-0001-2345-6789"}}]}
        works = {
            "group": [{
                "work-summary": [{
                    "title": {"title": {"value": "Measured result"}},
                    "external-ids": {"external-id": [{
                        "external-id-type": "doi",
                        "external-id-value": "10.1234/XYZ",
                    }]},
                }]
            }]
        }
        with mock.patch.dict(os.environ, {"ORCID_ACCESS_TOKEN": "fixture-token"}, clear=True), \
             mock.patch.object(sle, "_http_get", side_effect=[search, works]) as http_get:
            records = sle._search_orcid("physics", 2)
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]["doi"], "10.1234/xyz")
        self.assertEqual(http_get.call_count, 2)
        self.assertIn("/works", http_get.call_args_list[1].args[0])

    def test_missing_orcid_token_is_explicit_and_offline(self):
        with mock.patch.dict(os.environ, {}, clear=True), mock.patch.object(sle, "_http_get") as http_get:
            self.assertEqual(sle._search_orcid("physics", 3), [])
            http_get.assert_not_called()

    def test_stage_three_means_repository_qualified(self):
        raw = {
            "metadata": {
                "title": "Quantum mechanics experiment",
                "description": "A" * 140,
                "keywords": ["quantum", "experiment"],
                "doi": "10.5281/zenodo.1",
                "license": {"id": "cc-by-4.0"},
                "communities": [{"id": "physics"}],
            },
            "files": [{"links": {"self": "https://zenodo.org/file/content"}}],
        }
        record = sle._normalize_zenodo(raw)
        record["_relevance_score"] = sle._relevance_score(record, "physics", "quantum mechanics")
        self.assertEqual(sle._classify_stage(record), 3)
        self.assertEqual(sle._stage_name(3), "repository_qualified")
        self.assertFalse(record["claim_allowed"])

    def test_candidate_requires_domain_relevance(self):
        record = {
            "source": "zenodo",
            "title": "Unrelated title",
            "doi": "10.1/test",
            "abstract": "A" * 150,
            "keywords": ["unrelated"],
            "_relevance_score": 0,
        }
        self.assertEqual(sle._classify_stage(record), 1)

    def test_cross_domain_candidate_does_not_mutate_inputs(self):
        left = {
            "source": "zenodo",
            "sources": ["zenodo"],
            "title": "Shared DOI",
            "doi": "10.1/shared",
            "abstract": "A" * 150,
            "keywords": ["quantum"],
            "_stage": 2,
            "claim_allowed": False,
        }
        right = dict(left)
        records = {"physics": [left], "mathematics": [right]}
        with tempfile.TemporaryDirectory() as temporary:
            synthesis, promoted = sle._promote_cross_domain(records, Path(temporary), False)
        self.assertEqual(left["_stage"], 2)
        self.assertEqual(right["_stage"], 2)
        self.assertEqual(promoted["physics"][0]["_stage"], 4)
        self.assertFalse(promoted["physics"][0]["claim_allowed"])
        self.assertIn("claim_allowed=false", synthesis["physics"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
