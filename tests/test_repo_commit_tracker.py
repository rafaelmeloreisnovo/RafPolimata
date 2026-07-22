#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "repo_commit_tracker", ROOT / "scripts/repo_commit_tracker.py"
)
assert SPEC and SPEC.loader
TRACKER = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = TRACKER
SPEC.loader.exec_module(TRACKER)


def sample_config() -> dict:
    return {
        "schema": "raf.repository-tracker-config.v1",
        "owner": "rafaelmeloreisnovo",
        "poll_interval_minutes": 15,
        "api": {
            "max_requests": 100,
            "timeout_seconds": 10,
            "max_repositories": 10,
            "max_commits_per_repository": 10,
            "max_forks_per_repository": 4,
        },
        "shard": {
            "alphabet": "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~",
            "width": 6,
            "snapshot_after_stable_runs": 4,
            "max_history_events": 64,
        },
        "repositories": [
            {
                "full_name": "rafaelmeloreisnovo/RafPolimata",
                "priority": 0,
                "family": "orchestrator",
                "follow_forks": False,
                "aliases": [],
                "enabled": True,
            }
        ],
        "safety": {
            "read_only": True,
            "auto_fork": False,
            "auto_merge": False,
            "auto_push": False,
            "execute_external_code": False,
            "clone_external_repositories": False,
        },
    }


class TrackerTests(unittest.TestCase):
    def test_counter_carry(self) -> None:
        alphabet = "01A"
        self.assertEqual(TRACKER.increment_counter("00", alphabet), "01")
        self.assertEqual(TRACKER.increment_counter("01", alphabet), "0A")
        self.assertEqual(TRACKER.increment_counter("0A", alphabet), "10")
        with self.assertRaises(TRACKER.TrackerError):
            TRACKER.increment_counter("AA", alphabet)

    def test_alphabet_is_path_safe(self) -> None:
        TRACKER.validate_shard_alphabet("0123ABCxyz-._~")
        with self.assertRaises(TRACKER.TrackerError):
            TRACKER.validate_shard_alphabet("01/AB")
        with self.assertRaises(TRACKER.TrackerError):
            TRACKER.validate_shard_alphabet("001")

    def test_config_requires_read_only_safety(self) -> None:
        config = sample_config()
        TRACKER.validate_config(config)
        config["safety"]["auto_push"] = True
        with self.assertRaises(TRACKER.TrackerError):
            TRACKER.validate_config(config)

    def test_semantic_fingerprint_is_deterministic(self) -> None:
        message = "fix(ci): remove AVX warning in build pipeline"
        tags = TRACKER.semantic_tags(message)
        self.assertIn("ci", tags)
        self.assertIn("build", tags)
        self.assertIn("performance", tags)
        self.assertEqual(
            TRACKER.semantic_fingerprint(message, tags),
            TRACKER.semantic_fingerprint(message, reversed(tags)),
        )

    def test_chain_changes_with_payload(self) -> None:
        first = TRACKER.chain_hash(TRACKER.ZERO_CHAIN, "000000", "a" * 64)
        second = TRACKER.chain_hash(TRACKER.ZERO_CHAIN, "000000", "b" * 64)
        self.assertNotEqual(first, second)
        self.assertEqual(len(first), 64)

    def test_first_observation_is_baseline(self) -> None:
        class FakeClient:
            requests_used = 0

            def get(self, path, params=None):
                self.requests_used += 1
                if path.endswith("/commits"):
                    return [{
                        "sha": "a" * 40,
                        "commit": {
                            "message": "feat: initial baseline",
                            "author": {"date": "2026-07-21T00:00:00Z"},
                            "committer": {"date": "2026-07-21T00:00:00Z"},
                        },
                        "author": {"login": "rafaelmeloreisnovo"},
                        "html_url": "https://example.invalid/commit",
                    }]
                return {
                    "full_name": "rafaelmeloreisnovo/RafPolimata",
                    "default_branch": "main",
                    "private": True,
                    "fork": False,
                }

        record = sample_config()["repositories"][0]
        result, events = TRACKER.poll_repository(
            FakeClient(), record, {}, sample_config()["api"]
        )
        self.assertEqual(result["status"], "OK")
        self.assertEqual(events, [])
        self.assertTrue(result["next_state"]["initialized"])

    def test_stability_requires_consecutive_empty_runs(self) -> None:
        state = {"global_stable_runs": 0}
        self.assertFalse(TRACKER.update_stability(state, 0, 2))
        self.assertTrue(TRACKER.update_stability(state, 0, 2))
        self.assertFalse(TRACKER.update_stability(state, 1, 2))
        self.assertEqual(state["global_stable_runs"], 0)

    def test_deterministic_zip_uses_fixed_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "a.txt").write_text("A", encoding="utf-8")
            (root / "nested").mkdir()
            (root / "nested" / "b.txt").write_text("B", encoding="utf-8")
            target = root / "snapshot.zip"
            TRACKER.deterministic_zip(root, target)
            first = target.read_bytes()
            TRACKER.deterministic_zip(root, target)
            second = target.read_bytes()
            self.assertEqual(first, second)
            with zipfile.ZipFile(target) as archive:
                self.assertEqual(archive.namelist(), ["a.txt", "nested/b.txt"])

    def test_history_is_bounded(self) -> None:
        state = {"history": []}
        events = []
        for index in range(5):
            events.append({
                "repository": "o/r",
                "sha": f"{index:040x}",
                "event_type": "commit",
                "semantic_tags": ["test"],
                "semantic_fingerprint": "f" * 64,
                "committed_at": None,
            })
        TRACKER.append_history(state, events, 3)
        self.assertEqual(len(state["history"]), 3)
        self.assertEqual(state["history"][0]["sha"], f"{2:040x}")


if __name__ == "__main__":
    unittest.main()
