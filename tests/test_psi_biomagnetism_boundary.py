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

    def test_alphaxiv_custody_has_seven_papers(self):
        papers=self.d["alphaXiv_library"]["papers"]
        self.assertEqual(self.d["alphaXiv_library"]["paper_count_bound"],7)
        self.assertEqual(
            {p["id"] for p in papers},
            {"2306.16292","2204.09147","2402.10113","2212.03101","2603.20312","2607.20513","2410.07186"},
        )

    def test_species_transfer_guard_stays_closed(self):
        guard=self.d["species_transfer_guard"]
        self.assertEqual(guard["bee_to_octopus"],"FORBIDDEN_INFERENCE_WITHOUT_OCTOPUS_SPECIFIC_EVIDENCE")
        self.assertEqual(guard["amphibian_to_octopus"],"FORBIDDEN_INFERENCE_WITHOUT_OCTOPUS_SPECIFIC_EVIDENCE")
        self.assertEqual(guard["human_biomagnetism_to_identity_detection"],"FORBIDDEN_INFERENCE_WITHOUT_DISCRIMINATION_EVIDENCE")

    def test_new_animal_magnetism_claims_are_bounded(self):
        claims={x["id"]:x for x in self.d["claim_ladder"]}
        self.assertEqual(claims["C1B"]["state"],"OBSERVED_PHYSICAL_PROXY_UNPROMOTED")
        self.assertEqual(claims["C1C"]["state"],"OBSERVED_MECHANISM_CONSISTENT")
        self.assertEqual(claims["C1D"]["state"],"THEORETICAL_CONSISTENCY_ONLY")


if __name__=="__main__":
    unittest.main()
