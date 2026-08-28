#!/usr/bin/env python3
import importlib.util
from pathlib import Path

P = Path(__file__).parents[1] / "scripts" / "daily_bibliography_evolution.py"
spec = importlib.util.spec_from_file_location("daily_biblio", P)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def test_doi_normalization():
    assert mod.norm_doi("https://doi.org/10.1234/ABC.") == "10.1234/abc"


def test_identity_prefers_doi():
    r = {"source":"arxiv", "doi":"10.1/x", "arxiv_id":"2608.00001"}
    assert mod.source_identity(r) == "doi:10.1/x"


def test_occurrences_preserved_and_identity_merged():
    records = [
        {"source":"zenodo", "doi":"10.1/x", "provider_id":"1", "raw_sha256":"a", "title":"A", "claim_allowed":False},
        {"source":"arxiv", "doi":"10.1/x", "provider_id":"2608.1", "raw_sha256":"b", "title":"A", "claim_allowed":False},
    ]
    ids, occ = mod.dedupe(records)
    assert len(ids) == 1
    assert len(occ) == 2
    assert ids[0]["occurrence_count"] == 2
    assert sorted(ids[0]["sources"]) == ["arxiv", "zenodo"]


def test_hypothesis_requires_independent_sources():
    one = [{"identity":"doi:10.1/x", "sources":["zenodo"]}]
    two = [{"identity":"doi:10.1/x", "sources":["zenodo", "arxiv"]}]
    assert mod.hypothesis_candidates(one) == []
    h = mod.hypothesis_candidates(two)
    assert len(h) == 1
    assert h[0]["state"] == "TESTABLE"
    assert h[0]["claim_allowed"] is False


if __name__ == "__main__":
    test_doi_normalization()
    test_identity_prefers_doi()
    test_occurrences_preserved_and_identity_merged()
    test_hypothesis_requires_independent_sources()
    print("daily bibliography offline invariants: PASS")
