import json
import tempfile
import unittest
from pathlib import Path

from scripts.conversation_pair_binder import build_report


def message(mid, role, text, t):
    return {"id": mid, "author": {"role": role}, "content": {"content_type": "text", "parts": [text]}, "create_time": t, "metadata": {}}


class ConversationPairBinderTests(unittest.TestCase):
    def test_parent_lineage_and_feedback_scope(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            shard = root / "conversations-000.json"
            feedback = root / "message_feedback.json"
            shard.write_text(json.dumps([{
                "id": "c1", "title": "fixture", "mapping": {
                    "u1": {"id": "u1", "message": message("um1", "user", "question", 1.0), "parent": None},
                    "sys": {"id": "sys", "message": message("s1", "system", "meta", 1.5), "parent": "u1"},
                    "a1": {"id": "a1", "message": message("am1", "assistant", "answer", 2.0), "parent": "sys"},
                }
            }]), encoding="utf-8")
            feedback.write_text(json.dumps([{"id":"f1","conversation_id":"c1","rating":"thumbs_down","create_time":"x","update_time":"y"}]), encoding="utf-8")
            report = build_report([shard], feedback)
            self.assertEqual(report["conversation_count"], 1)
            self.assertEqual(report["interaction_pair_count"], 1)
            row = report["interactions"][0]
            self.assertEqual(row["user_message_id"], "um1")
            self.assertEqual(row["assistant_message_id"], "am1")
            self.assertEqual(row["conversation_feedback"][0]["rating"], "thumbs_down")
            self.assertEqual(row["conversation_feedback"][0]["scope"], "conversation")
            self.assertEqual(row["operational_metrics"]["state"], "TOKEN_VAZIO_UNDERIVED")
            self.assertFalse(report["operational_metrics_derived"])
            self.assertFalse(report["claim_allowed"])

    def test_missing_shard_detection(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            files = []
            for n in (0, 2):
                p = root / f"conversations-{n:03d}.json"
                p.write_text("[]", encoding="utf-8")
                files.append(p)
            report = build_report(files)
            self.assertEqual(report["missing_shard_numbers_within_observed_range"], [1])
            self.assertIn("TOKEN_VAZIO_MISSING_SHARD_001", report["F_gap"])


if __name__ == "__main__":
    unittest.main()
