import unittest

from scripts.genesis_8x3_materializer import build_manifest, validate_manifest


class Genesis8x3MaterializerTests(unittest.TestCase):
    def test_core_counts_and_gates(self):
        manifest = build_manifest()
        validation = validate_manifest(manifest)
        self.assertEqual(validation["status"], "PASS")
        self.assertEqual(len(manifest["dimensions"]), 8)
        self.assertEqual(len(manifest["stages"]), 3)
        self.assertEqual(len(manifest["matrix_8x3_cells"]), 24)
        self.assertEqual(len(manifest["parables"]), 8)
        self.assertEqual(len(manifest["formulas_50"]), 50)
        self.assertEqual(manifest["counts"]["A_pairs"], 780)
        self.assertEqual(manifest["counts"]["B_pairs"], 210)
        self.assertEqual(manifest["counts"]["A_B_cross"], 840)
        self.assertEqual(manifest["counts"]["pairs_of_pairs"], 163800)
        self.assertFalse(manifest["claim_allowed"])

    def test_token_vazio_boundaries(self):
        manifest = build_manifest()
        states = {g["gate"]: g["state"] for g in manifest["gates"]}
        self.assertEqual(states["C_8x8_IDENTITY"], "TOKEN_VAZIO")
        self.assertEqual(states["PHONETIC_IPA_DIALECTS"], "TOKEN_VAZIO")
        self.assertEqual(states["GMAIL_SEND"], "WITHHELD_NO_RECIPIENT")
        self.assertEqual(states["CALENDAR_CREATE"], "WITHHELD_NO_TIME")
        self.assertIn("METAPHOR", manifest["metaphor_boundary"]["quantum_language"])


if __name__ == "__main__":
    unittest.main()
