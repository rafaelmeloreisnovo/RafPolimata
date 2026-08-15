#!/usr/bin/env python3
"""
Phase 1 Tests: Claim Verifier and Heuristic Evaluator

Comprehensive test suite for RAFAELIA-PSC-1 claim verification.
Tests gate validation, heuristic evaluation, and hard boundary checks.
"""

import json
import tempfile
import unittest
from pathlib import Path
from sys import path as sys_path

sys_path.insert(0, str(Path(__file__).parent.parent / "tools"))

from claim_verifier import (
    ClaimVerifier,
    ClaimStatus,
    Heuristic,
    ClaimVerification,
)
from heuristic_evaluator import (
    HeuristicEvaluator,
    EvidencePoint,
)


class TestClaimGateValidation(unittest.TestCase):
    """Test claim gate validation."""

    def setUp(self):
        """Create test verifier."""
        self.verifier = ClaimVerifier()

    def _make_claim(self, **kwargs):
        """Helper to create claims with defaults."""
        claim = {
            "id": "test-claim-001",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "COMPILER",
            "source": "phase-21-tests",
            "method": "unification-algorithm",
            "artifact": "type_system.h",
            "limitations": "single-pass type checking",
            "falsifier": "ambiguous-type-context",
        }
        claim.update(kwargs)
        return claim

    def test_complete_claim_passes_gates(self):
        """Complete claim with all fields should pass gates."""
        claim = self._make_claim()
        verification = self.verifier.verify_claim(claim)

        # All gates should be satisfied
        all_satisfied = all(gr.is_satisfied for gr in verification.gate_results)
        self.assertTrue(all_satisfied)

    def test_missing_domain_blocks_claim(self):
        """Missing domain field should block claim."""
        claim = self._make_claim(domain="")
        verification = self.verifier.verify_claim(claim)

        # Domain gate should fail
        domain_gate = [g for g in verification.gate_results if g.field_name == "domain"]
        self.assertFalse(domain_gate[0].is_satisfied)
        self.assertEqual(verification.claim_gate_status, "BLOCKED")

    def test_missing_falsifier_blocks_claim(self):
        """Missing falsifier field should block claim."""
        claim = self._make_claim(falsifier="")
        verification = self.verifier.verify_claim(claim)

        falsifier_gate = [g for g in verification.gate_results if g.field_name == "falsifier"]
        self.assertFalse(falsifier_gate[0].is_satisfied)

    def test_all_required_fields_tracked(self):
        """All required fields should be tracked."""
        claim = self._make_claim()
        verification = self.verifier.verify_claim(claim)

        required_fields = ClaimVerifier.REQUIRED_FIELDS
        self.assertEqual(len(verification.required_fields_present), len(required_fields))

    def test_null_value_treated_as_missing(self):
        """Null/None values should be treated as missing."""
        claim = self._make_claim(source=None)
        verification = self.verifier.verify_claim(claim)

        self.assertFalse(verification.required_fields_present["source"])


class TestEpistemicStatus(unittest.TestCase):
    """Test epistemic status evaluation."""

    def setUp(self):
        self.verifier = ClaimVerifier()

    def test_verified_literal_status(self):
        """VERIFIED_LITERAL status should be recognized."""
        claim = {
            "id": "test",
            "epistemic_status": "VERIFIED_LITERAL",
            "source_status": "VERIFIED_LITERAL",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }
        verification = self.verifier.verify_claim(claim)
        self.assertEqual(verification.epistemic_status, ClaimStatus.VERIFIED_LITERAL)

    def test_hypothesis_status(self):
        """HYPOTHESIS status should be recognized."""
        claim = {
            "id": "test",
            "epistemic_status": "HYPOTHESIS",
            "source_status": "HYPOTHESIS",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }
        verification = self.verifier.verify_claim(claim)
        self.assertEqual(verification.epistemic_status, ClaimStatus.HYPOTHESIS)

    def test_invalid_status_defaults_to_token_vazio(self):
        """Invalid epistemic status should default to TOKEN_VAZIO."""
        claim = {
            "id": "test",
            "epistemic_status": "INVALID_STATUS",
            "source_status": "TESTED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }
        verification = self.verifier.verify_claim(claim)
        self.assertEqual(verification.epistemic_status, ClaimStatus.TOKEN_VAZIO)


class TestHeuristicApplication(unittest.TestCase):
    """Test heuristic application."""

    def setUp(self):
        self.verifier = ClaimVerifier()

    def _make_complete_claim(self):
        """Create a complete claim."""
        return {
            "id": "heuristic-test",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "FORMAL_METHODS",
            "source": "theorem-prover",
            "method": "proof-by-construction",
            "artifact": "proof.v",
            "limitations": "finite scope",
            "falsifier": "type-error",
        }

    def test_all_seven_heuristics_applied(self):
        """All 7 heuristics should be applied."""
        claim = self._make_complete_claim()
        verification = self.verifier.verify_claim(claim)

        self.assertEqual(len(verification.heuristic_results), 7)

    def test_heuristic_names(self):
        """All heuristic names should be correct."""
        claim = self._make_complete_claim()
        verification = self.verifier.verify_claim(claim)

        heuristic_names = {hr.heuristic for hr in verification.heuristic_results}
        expected = {
            Heuristic.DIRECT,
            Heuristic.CAUSAL_REVERSE,
            Heuristic.TEMPORAL_ANTIDERIVATIVE,
            Heuristic.LOCAL_DERIVATIVE,
            Heuristic.COUNTERFACTUAL,
            Heuristic.ROLE_INVERSION,
            Heuristic.MULTISCALE,
        }
        self.assertEqual(heuristic_names, expected)

    def test_heuristic_confidence_scores(self):
        """Heuristics should produce confidence scores."""
        claim = self._make_complete_claim()
        verification = self.verifier.verify_claim(claim)

        for result in verification.heuristic_results:
            self.assertGreaterEqual(result.confidence, 0.0)
            self.assertLessEqual(result.confidence, 1.0)

    def test_direct_heuristic_requires_artifact(self):
        """Direct heuristic should require artifact."""
        claim = self._make_complete_claim()
        claim["artifact"] = ""

        verification = self.verifier.verify_claim(claim)

        direct_result = [h for h in verification.heuristic_results
                         if h.heuristic == Heuristic.DIRECT][0]
        self.assertLess(direct_result.confidence, 0.5)

    def test_counterfactual_heuristic_requires_falsifier(self):
        """Counterfactual heuristic should require falsifier."""
        claim = self._make_complete_claim()
        claim["falsifier"] = ""

        verification = self.verifier.verify_claim(claim)

        counterfactual_result = [h for h in verification.heuristic_results
                                 if h.heuristic == Heuristic.COUNTERFACTUAL][0]
        self.assertLess(counterfactual_result.confidence, 0.5)


class TestHardBoundaries(unittest.TestCase):
    """Test hard boundary enforcement."""

    def setUp(self):
        self.verifier = ClaimVerifier()

    def _make_claim_with_boundary(self, boundary_name: str, value: bool):
        """Create claim with hard boundary violation."""
        claim = {
            "id": "boundary-test",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
            boundary_name: value,
        }
        return claim

    def test_symbol_implies_physical_effect_violation(self):
        """symbol_implies_physical_effect should be detected."""
        claim = self._make_claim_with_boundary("symbol_implies_physical_effect", True)
        verification = self.verifier.verify_claim(claim)

        self.assertIn("symbol_implies_physical_effect",
                      " ".join(verification.hard_boundary_violations))

    def test_analogy_implies_mechanism_violation(self):
        """analogy_implies_mechanism should be detected."""
        claim = self._make_claim_with_boundary("analogy_implies_mechanism", True)
        verification = self.verifier.verify_claim(claim)

        self.assertIn("analogy_implies_mechanism",
                      " ".join(verification.hard_boundary_violations))

    def test_no_boundary_violations_when_all_false(self):
        """No violations when all boundaries are false."""
        claim = {
            "id": "clean-claim",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
            "symbol_implies_physical_effect": False,
            "analogy_implies_mechanism": False,
        }
        verification = self.verifier.verify_claim(claim)

        self.assertEqual(len(verification.hard_boundary_violations), 0)


class TestClaimGateStatus(unittest.TestCase):
    """Test claim gate status determination."""

    def setUp(self):
        self.verifier = ClaimVerifier()

    def _make_complete_claim(self):
        return {
            "id": "status-test",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

    def test_complete_claim_passes(self):
        """Complete claim should pass gate."""
        claim = self._make_complete_claim()
        verification = self.verifier.verify_claim(claim)

        self.assertEqual(verification.claim_gate_status, "PASS")

    def test_incomplete_claim_blocked(self):
        """Incomplete claim should be blocked."""
        claim = self._make_complete_claim()
        claim["domain"] = ""

        verification = self.verifier.verify_claim(claim)

        self.assertEqual(verification.claim_gate_status, "BLOCKED")

    def test_claim_with_violations_fails(self):
        """Claim with hard boundary violations should fail."""
        claim = self._make_complete_claim()
        claim["symbol_implies_physical_effect"] = True

        verification = self.verifier.verify_claim(claim)

        self.assertEqual(verification.claim_gate_status, "FAIL")


class TestVerificationHash(unittest.TestCase):
    """Test verification hash generation."""

    def setUp(self):
        self.verifier = ClaimVerifier()

    def test_hash_deterministic(self):
        """Verification hash should be deterministic."""
        claim = {
            "id": "hash-test",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        v1 = self.verifier.verify_claim(claim)
        hash1 = v1.verification_hash

        # Create new verifier and verify same claim
        verifier2 = ClaimVerifier()
        v2 = verifier2.verify_claim(claim)
        hash2 = v2.verification_hash

        self.assertEqual(hash1, hash2)

    def test_hash_changes_with_claim(self):
        """Hash should change when claim changes."""
        claim1 = {
            "id": "hash-test-1",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test-domain-1",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        claim2 = {
            "id": "hash-test-2",
            "source_status": "TESTED",
            "epistemic_status": "HYPOTHESIS",
            "claim_gate": "PASS",
            "domain": "test-domain-2",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        v1 = self.verifier.verify_claim(claim1)
        verifier2 = ClaimVerifier()
        v2 = verifier2.verify_claim(claim2)

        self.assertNotEqual(v1.verification_hash, v2.verification_hash)


class TestHeuristicEvaluator(unittest.TestCase):
    """Test heuristic evaluator with evidence."""

    def setUp(self):
        self.evaluator = HeuristicEvaluator()

    def _make_evidence(self):
        """Create sample evidence."""
        return [
            EvidencePoint(
                type="observation",
                value="Type inference converged",
                confidence=0.95,
                source="test-suite",
                timestamp_utc="2026-08-15T10:00:00Z"
            ),
            EvidencePoint(
                type="proof",
                value="Unification theorem proved",
                confidence=0.99,
                source="coq-proof",
                timestamp_utc="2026-08-15T11:00:00Z"
            ),
        ]

    def test_evaluate_all_heuristics(self):
        """Evaluator should evaluate all 7 heuristics."""
        claim = {
            "id": "eval-test",
            "domain": "FORMAL_METHODS",
            "source": "coq",
            "method": "proof",
            "artifact": "proof.v",
            "limitations": "finite scope",
            "falsifier": "type-error",
        }

        evidence = self._make_evidence()
        results = self.evaluator.evaluate_claim(claim, evidence)

        self.assertEqual(len(results), 7)

    def test_direct_heuristic_with_evidence(self):
        """Direct heuristic should score higher with evidence."""
        claim = {"id": "test", "artifact": "test.h", "method": "direct-check"}
        evidence = self._make_evidence()

        results = self.evaluator.evaluate_claim(claim, evidence)
        direct_result = [r for r in results if r.heuristic.value == "DIRECT"][0]

        self.assertGreater(direct_result.score, 0.7)

    def test_overall_score_computation(self):
        """Overall score should be weighted average."""
        claim = {"id": "test", "domain": "test", "method": "test", "artifact": "test"}
        evidence = self._make_evidence()

        results = self.evaluator.evaluate_claim(claim, evidence)
        overall = self.evaluator.compute_overall_score(results)

        self.assertGreaterEqual(overall, 0.0)
        self.assertLessEqual(overall, 1.0)


class TestReportGeneration(unittest.TestCase):
    """Test report generation."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_path = Path(self.temp_dir.name)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_report_json_valid(self):
        """Generated report should be valid JSON."""
        verifier = ClaimVerifier()

        claim = {
            "id": "report-test",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        verifier.verify_claim(claim)

        output_file = self.temp_path / "report.json"
        report_path = verifier.export_report(output_file)

        # Should be valid JSON
        report_json = json.loads(output_file.read_text())
        self.assertIn("schema", report_json)
        self.assertIn("verifications", report_json)
        self.assertIn("summary", report_json)

    def test_report_summary_accurate(self):
        """Report summary should match actual counts."""
        verifier = ClaimVerifier()

        # Add two claims
        claim1 = {
            "id": "claim-1",
            "source_status": "TESTED",
            "epistemic_status": "PROVED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        claim2 = {
            "id": "claim-2",
            "source_status": "TESTED",
            "claim_gate": "PASS",
            "domain": "test",
            "source": "test",
            "method": "test",
            "artifact": "test",
            "limitations": "test",
            "falsifier": "test",
        }

        verifier.verify_claim(claim1)
        verifier.verify_claim(claim2)

        output_file = self.temp_path / "report.json"
        verifier.export_report(output_file)

        report_json = json.loads(output_file.read_text())
        summary = report_json["summary"]

        self.assertEqual(summary["total_claims"], 2)
        self.assertGreaterEqual(summary["passed"], 0)


if __name__ == "__main__":
    unittest.main()
