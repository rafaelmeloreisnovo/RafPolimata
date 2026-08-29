import unittest

from scripts.conversation_permutation_feedback import FeedbackError, evaluate


class ConversationPermutationFeedbackTests(unittest.TestCase):
    def test_exhaustive_and_chronology_preserved(self):
        payload = {
            "schema_version": "rafaelia.conversation-permutation-feedback/v1",
            "seed": "test-seed",
            "blocking_gaps": ["TOKEN_VAZIO_FULL_CONVERSATION_BINDING"],
            "interactions": [
                {"interaction_id":"i0","chronology_index":0,"metrics":{"evidence":0.8,"gap":0.2,"risk":0.3,"urgency":0.7}},
                {"interaction_id":"i1","chronology_index":1,"metrics":{"evidence":0.3,"gap":0.8,"risk":0.6,"urgency":0.9}},
            ],
        }
        result = evaluate(payload)
        self.assertTrue(result["chronology_preserved"])
        self.assertEqual(result["interaction_count"], 2)
        self.assertEqual(result["permutations_per_interaction"], 24)
        self.assertTrue(all(x["permutation_count"] == 24 for x in result["interactions"]))
        self.assertFalse(result["claim_allowed"])
        self.assertEqual(result["urgency_queue"][0], "i1")

    def test_rejects_chronology_reorder(self):
        payload = {
            "schema_version": "rafaelia.conversation-permutation-feedback/v1",
            "interactions": [
                {"interaction_id":"i1","chronology_index":1,"metrics":{"evidence":1,"gap":0,"risk":0,"urgency":0}},
                {"interaction_id":"i0","chronology_index":0,"metrics":{"evidence":1,"gap":0,"risk":0,"urgency":0}},
            ],
        }
        with self.assertRaises(FeedbackError):
            evaluate(payload)


if __name__ == "__main__":
    unittest.main()
