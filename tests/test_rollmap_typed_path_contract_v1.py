"""RafPolimata Roll-map typed contract: public synthetic assertions only."""
import json
import pathlib
import unittest

HERE = pathlib.Path(__file__).resolve().parents[1]
CONTRACT = HERE / "contracts" / "rollmap-typed-path-v1.json"

class TypedRollmapContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.d = json.loads(CONTRACT.read_text(encoding="utf-8"))

    def test_identity_and_version(self):
        self.assertEqual(self.d["schema"], "rafaelia.rollmap.typed-path.v1")
        self.assertEqual(self.d["version"], "1.0.0")

    def test_owner_and_consumer_separate(self):
        self.assertEqual(self.d["authority"], "rafaelmeloreisnovo/RafPolimata")
        self.assertEqual(self.d["consumer"], "rafaelmeloreisnovo/RafGitTools")

    def test_claim_is_not_promoted(self):
        self.assertIs(self.d["claim_allowed"], False)

    def test_zero_byte_distinct_unknown(self):
        field = self.d["field_semantics"]["size_bytes"]
        self.assertIn("(0 valid)", field)
        self.assertIn("TOKEN_VAZIO", field)

    def test_name_preservation_not_same_as_search_key(self):
        fields = self.d["field_semantics"]
        self.assertIn("raw_name", fields)
        self.assertIn("name_search_key", fields)
        self.assertIn("collisions retained", fields["name_search_key"])

    def test_structural_joins_implemented(self):
        impl = set(self.d["predicates"]["implemented_consumer_v2"])
        self.assertTrue({"AND", "OR", "NOT", "HAS_CHILD", "HAS_DESC"}.issubset(impl))

    def test_fragments_are_not_implemented_claim(self):
        impl = set(self.d["predicates"]["implemented_consumer_v2"])
        pending = set(self.d["predicates"]["contract_candidate_not_implemented"])
        self.assertNotIn("HASH_PIECE_ORDERED_JOIN", impl)
        self.assertIn("HASH_PIECE_ORDERED_JOIN", pending)

    def test_types_are_disjoint(self):
        rel = self.d["relations"]
        groups = [set(v) for v in rel.values()]
        self.assertEqual(len(set.union(*groups)), sum(map(len, groups)))

    def test_candidates_and_verification_distinct(self):
        rel = self.d["relations"]
        self.assertIn("HASH_FRAGMENT_CANDIDATE", rel["candidates"])
        self.assertIn("SAME_CONTENT_SHA256", rel["verified_scoped"])
        self.assertNotIn("HASH_FRAGMENT_CANDIDATE", rel["verified_scoped"])

    def test_sha_identity_distinct_git_oid(self):
        inv = self.d["invariants"]
        self.assertIn("content_sha256 != Git object ID", inv)

    def test_stopwalk_defined(self):
        self.assertTrue(any("stopwalk" in s.lower() for s in self.d["invariants"]))

    def test_negative_falsifiers_preserved(self):
        neg = self.d["validation_contract"]["negative"]
        self.assertGreaterEqual(len(neg), 5)
        self.assertTrue(any("unapproved" in x for x in neg))

    def test_no_implicit_authorization(self):
        self.assertIn("read-only discovery", self.d["transport"]["policy"])

if __name__ == "__main__":
    unittest.main()
