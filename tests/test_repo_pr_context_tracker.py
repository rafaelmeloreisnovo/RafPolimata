#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "scripts"
sys.path.insert(0, str(SCRIPTS))
SPEC = importlib.util.spec_from_file_location(
    "repo_pr_context_tracker", SCRIPTS / "repo_pr_context_tracker.py"
)
assert SPEC and SPEC.loader
PR = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = PR
SPEC.loader.exec_module(PR)


def repo_record() -> dict:
    return {
        "full_name": "owner/repository",
        "priority": 0,
        "family": "test",
        "follow_forks": False,
        "enabled": True,
    }


def pull(updated: str = "2026-07-21T00:00:00Z", title: str = "feat: baseline") -> dict:
    return {
        "number": 1,
        "title": title,
        "state": "open",
        "draft": True,
        "merged_at": None,
        "created_at": "2026-07-20T00:00:00Z",
        "updated_at": updated,
        "head": {"sha": "a" * 40, "ref": "feature"},
        "base": {"ref": "main"},
        "user": {"login": "user"},
        "html_url": "https://example.invalid/pr/1",
    }


class FakeClient:
    def __init__(self, values):
        self.values = values
        self.requests_used = 0
        self.max_requests = 40

    def get(self, path, params=None):
        self.requests_used += 1
        return self.values


class PullContextTests(unittest.TestCase):
    def test_first_poll_is_baseline(self) -> None:
        result, events, current = PR.poll_pull_requests(
            FakeClient([pull()]), repo_record(), {}, 6
        )
        self.assertEqual(result["status"], "OK")
        self.assertEqual(events, [])
        self.assertIn("1", current)

    def test_changed_pull_request_becomes_event(self) -> None:
        first = PR.compact_pull_request(pull(), "owner/repository", "test")
        previous = {"1": first["change_fingerprint"]}
        result, events, _ = PR.poll_pull_requests(
            FakeClient([
                pull(updated="2026-07-21T01:00:00Z", title="fix: changed")
            ]),
            repo_record(),
            previous,
            6,
        )
        self.assertEqual(result["changed_count"], 1)
        self.assertEqual(events[0]["pr_number"], 1)
        self.assertIn("fix", events[0]["semantic_tags"])

    def test_report_never_allows_claim(self) -> None:
        state = {
            "next_counter": "00000000",
            "chain_head": "0" * 64,
            "history": [],
        }
        report, markdown = PR.build_report(
            "2026-07-21T00:00:00Z", [], [], state, FakeClient([])
        )
        self.assertFalse(report["claim_allowed"])
        self.assertEqual(report["weights"], "TOKEN_VAZIO_CALIBRATION")
        self.assertIn("Pull request context report", markdown)

    def test_history_is_bounded(self) -> None:
        state = {"history": []}
        event = PR.compact_pull_request(
            pull(), "owner/repository", "test"
        )
        PR.append_history(state, [event, event, event], limit=2)
        self.assertEqual(len(state["history"]), 2)


if __name__ == "__main__":
    unittest.main()
