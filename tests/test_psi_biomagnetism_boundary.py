# Governance anchor: CLOSURE_L2.
import json
import pathlib
import unittest

ROOT=pathlib.Path(__file__).resolve().parents[1]
PATH=ROOT/"research/evidence_garden/psi_biomagnetism_boundary.v1.json"


class PsiBiomagnetismBoundaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.d=json.loads(PATH.read_text(encoding="utf-8"))

    def test_claim_gate_closed(self):
        self.assertFalse(self.d["claim_allowed"])

    def test_open_world_guard_preserved(self):
        self.assertEqual(self.d["open_world"]["unknown_unknown"],"TOKEN_VAZIO")
        self.assertEqual(self.d["open_world"]["universal_completeness"],"FORBIDDEN")

    def test_no_parapsychology_promotion(self):
        claims={x["id"]:x for x in self.d["claim_ladder"]}
        self.assertEqual(claims["C3"]["state"],"TOKEN_VAZIO")
        self.assertEqual(claims["C4"]["state"],"TOKEN_VAZIO")
        self.assertEqual(claims["C6"]["state"],"TOKEN_VAZIO")

    def test_physical_biomagnetism_is_separate_lane(self):
        claims={x["id"]:x for x in self.d["claim_ladder"]}
        self.assertEqual(claims["C0"]["state"],"PASS_BOUNDED")
        self.assertNotEqual(claims["C0"]["state"],claims["C4"]["state"])

    def test_historical_machine_claim_is_not_promoted(self):
        self.assertEqual(
            self.d["candidate_identity_resolution"]["separate_1972_magnetometer_story"]["state"],
            "HISTORICAL_CLAIM_CONTESTED",
        )

    def test_alphaxiv_custody_has_four_papers(self):
        papers=self.d["alphaXiv_library"]["papers"]
        self.assertEqual({p["id"] for p in papers},{"2306.16292","2204.09147","2402.10113","2212.03101"})


if __name__=="__main__":
    unittest.main()
