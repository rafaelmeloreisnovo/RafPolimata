import importlib.util
import pathlib
import sys
import unittest

MODULE_PATH = pathlib.Path(__file__).parents[1] / "tools" / "validate_peer_review_math_7d.py"
spec = importlib.util.spec_from_file_location("validate_peer_review_math_7d", MODULE_PATH)
validator = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = validator
assert spec.loader is not None
spec.loader.exec_module(validator)


def valid_manifest():
    directions = list(validator.DIRECTIONS)
    packages = []
    states = (
        ["READY_FOR_SCOPE_REVIEW"] * 5
        + ["EXPERIMENT_REQUIRED"] * 5
        + ["RESEARCH_AGENDA_ONLY"] * 2
    )
    boundaries = {
        "PRM-08": "NAVIER_STOKES_ORIGINAL_NOT_CLAIMED",
        "PRM-09": "YANG_MILLS_MASS_GAP_NOT_CLAIMED",
        "PRM-11": "RIEMANN_PROOF_NOT_CLAIMED",
    }
    for number, state in enumerate(states, start=1):
        pid = f"PRM-{number:02d}"
        item = {
            "id": pid,
            "title": f"title {pid}",
            "original_scope": f"scope {pid}",
            "open_problem_boundary": boundaries.get(pid, "NO_OPEN_PROBLEM_SOLUTION_CLAIM"),
            "state": state,
            "next_gate": "a verifiable next step",
            "claim_allowed": False,
        }
        item.update({direction: f"{direction} {pid}" for direction in directions})
        packages.append(item)

    return {
        "schema": validator.SCHEMA,
        "method": {"directions": directions},
        "immutability": {
            "immutable_fields": ["id", "title", "original_scope", "open_problem_boundary"],
            "mutable_fields": ["state", "gap", "variant", "proof", "feedback", "next_gate"],
        },
        "privacy": {
            "claim_allowed": False,
            "private_corpus_body": "FORBIDDEN",
        },
        "counts": {
            "canonical_formulations": 60,
            "packages": 12,
            "READY_FOR_SCOPE_REVIEW": 5,
            "EXPERIMENT_REQUIRED": 5,
            "RESEARCH_AGENDA_ONLY": 2,
            "open_problem_solution_claims": 0,
        },
        "packages": packages,
        "global_gates": [{"id": f"G{n}", "name": "gate", "pass_when": "evidence"} for n in range(8)],
        "decision": {"automatic_merge": False},
    }


class PeerReviewMath7DValidatorTests(unittest.TestCase):
    def test_valid_manifest_passes(self):
        facts = validator.validate_manifest(valid_manifest())
        self.assertIn("packages=12", facts)
        self.assertIn("directions=7", facts)

    def test_missing_direction_fails(self):
        data = valid_manifest()
        del data["packages"][0]["proof"]
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_claim_promotion_fails(self):
        data = valid_manifest()
        data["packages"][1]["claim_allowed"] = True
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_private_corpus_exposure_fails(self):
        data = valid_manifest()
        data["privacy"]["private_corpus_body"] = "ALLOWED"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_navier_stokes_boundary_cannot_be_weakened(self):
        data = valid_manifest()
        data["packages"][7]["open_problem_boundary"] = "SOLVED"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_yang_mills_boundary_cannot_be_weakened(self):
        data = valid_manifest()
        data["packages"][8]["open_problem_boundary"] = "MASS_GAP_PROVED"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_research_agenda_overclaim_fails(self):
        data = valid_manifest()
        data["packages"][10]["feedback"] = "problem solved"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_duplicate_id_fails(self):
        data = valid_manifest()
        data["packages"][11]["id"] = "PRM-11"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_count_mismatch_fails(self):
        data = valid_manifest()
        data["counts"]["EXPERIMENT_REQUIRED"] = 4
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_mutable_immutable_overlap_fails(self):
        data = valid_manifest()
        data["immutability"]["mutable_fields"].append("id")
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)

    def test_gate_order_fails(self):
        data = valid_manifest()
        data["global_gates"][0]["id"] = "G1"
        with self.assertRaises(validator.ValidationError):
            validator.validate_manifest(data)


if __name__ == "__main__":
    unittest.main()
