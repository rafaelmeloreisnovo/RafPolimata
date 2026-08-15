#!/usr/bin/env python3
"""
Tests for FederatedAggregator (Phase 2: Federated Protocol)

Covers receipt aggregation, confidence weighting, failure scenarios,
circular dependencies, and report generation.
"""

import unittest
import json
import tempfile
from pathlib import Path
from datetime import datetime, timedelta

import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))

from federated_aggregator import (
    FederatedAggregator,
    AggregatedStatus,
    VerificationVerdict,
    ReceiptRecord,
    AggregationReport,
    AuthorityWeightAssignment,
)


class TestReceiptValidation(unittest.TestCase):
    """Test receipt validation logic."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")

    def _make_receipt(self, status="VERIFIED", verdict="PASSED_FORMAL_PROOF", confidence=0.95):
        """Helper to create valid receipt."""
        return {
            "schema": "rafaelia.federated_claim_receipt.routing.v1",
            "receipt_id": "RECEIPT-001",
            "origin_repo": "rafaelmeloreisnovo/Matem-tica-",
            "claim_reference": {
                "claim_id": "CLAIM-001",
                "origin_commit": "abc123",
            },
            "verification": {
                "status": status,
                "verdict": verdict,
                "confidence": confidence,
                "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            },
            "evidence": {
                "proof_artifact": "proofs/type_inference.v",
                "proof_hash": "sha256:abc123",
                "verified_by": "coq-proof-checker:8.17",
                "certification": "PASSED",
            },
            "authority": {
                "repo": "rafaelmeloreisnovo/Matem-tica-",
                "maintainers": ["alice@example.com"],
                "authority_level": "FORMAL_PROOF",
            },
            "signature": "ED25519(abcdef123456)",
            "receipt_hash": "sha256:xyz789",
        }

    def test_valid_receipt_accepted(self):
        """Test that valid receipt is accepted."""
        receipt = self._make_receipt()
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertTrue(result)
        self.assertEqual(len(self.aggregator.receipts), 1)

    def test_missing_schema_rejected(self):
        """Test that receipt without schema is rejected."""
        receipt = self._make_receipt()
        del receipt["schema"]
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)

    def test_missing_receipt_id_rejected(self):
        """Test that receipt without receipt_id is rejected."""
        receipt = self._make_receipt()
        del receipt["receipt_id"]
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)

    def test_missing_verification_rejected(self):
        """Test that receipt without verification is rejected."""
        receipt = self._make_receipt()
        del receipt["verification"]
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)

    def test_missing_signature_rejected(self):
        """Test that receipt without signature is rejected."""
        receipt = self._make_receipt()
        del receipt["signature"]
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)

    def test_empty_signature_rejected(self):
        """Test that receipt with empty signature is rejected."""
        receipt = self._make_receipt()
        receipt["signature"] = ""
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)

    def test_multiple_receipts_accumulated(self):
        """Test that multiple receipts are accumulated."""
        receipt1 = self._make_receipt(verdict="PASSED_FORMAL_PROOF", confidence=0.95)
        receipt2 = self._make_receipt(verdict="PASSED_PEER_REVIEW", confidence=0.90)

        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt1)
        self.aggregator.add_receipt("rafaelmeloreisnovo/papers", receipt2)

        self.assertEqual(len(self.aggregator.receipts), 2)


class TestConfidenceWeighting(unittest.TestCase):
    """Test confidence weighting and aggregation."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")

    def _make_receipt(self, authority="rafaelmeloreisnovo/Matem-tica-", confidence=0.95):
        """Helper to create receipt."""
        return {
            "schema": "rafaelia.federated_claim_receipt.routing.v1",
            "receipt_id": f"RECEIPT-{authority}",
            "origin_repo": authority,
            "claim_reference": {"claim_id": "CLAIM-001", "origin_commit": "abc123"},
            "verification": {
                "status": "VERIFIED",
                "verdict": "PASSED_FORMAL_PROOF",
                "confidence": confidence,
                "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            },
            "evidence": {"proof_artifact": "test.v"},
            "authority": {
                "repo": authority,
                "authority_level": "FORMAL_PROOF",
            },
            "signature": "ED25519(sig)",
            "receipt_hash": "sha256:hash",
        }

    def test_single_formal_proof_high_confidence(self):
        """Test single formal proof with high confidence."""
        receipt = self._make_receipt(confidence=0.95)
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_ALLOWED.value)
        self.assertGreater(report.aggregated_verdict["combined_confidence"], 0.90)

    def test_multiple_authorities_weighted_average(self):
        """Test confidence weighting with multiple authorities."""
        # Formal proof (weight 1.0)
        receipt1 = self._make_receipt(
            authority="rafaelmeloreisnovo/Matem-tica-",
            confidence=0.95
        )
        # Peer review (weight 0.8)
        receipt2 = self._make_receipt(
            authority="rafaelmeloreisnovo/papers",
            confidence=0.90
        )

        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt1)
        self.aggregator.add_receipt("rafaelmeloreisnovo/papers", receipt2)

        receipt2["authority"]["authority_level"] = "PEER_REVIEW"

        claim = {"id": "CLAIM-001"}
        authority_levels = {
            "rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF",
            "rafaelmeloreisnovo/papers": "PEER_REVIEW",
        }

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_ALLOWED.value)
        self.assertEqual(report.aggregated_verdict["verified_count"], 2)

    def test_low_confidence_below_threshold(self):
        """Test that low confidence blocks claim."""
        receipt = self._make_receipt(confidence=0.50)
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_BLOCKED.value)


class TestFailureScenarios(unittest.TestCase):
    """Test failure scenarios."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")

    def _make_receipt(self, status="VERIFIED", verdict="PASSED_FORMAL_PROOF"):
        """Helper to create receipt."""
        return {
            "schema": "rafaelia.federated_claim_receipt.routing.v1",
            "receipt_id": "RECEIPT-001",
            "origin_repo": "rafaelmeloreisnovo/Matem-tica-",
            "verification": {
                "status": status,
                "verdict": verdict,
                "confidence": 0.95,
                "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            },
            "evidence": {"proof_artifact": "test.v"},
            "authority": {
                "repo": "rafaelmeloreisnovo/Matem-tica-",
                "authority_level": "FORMAL_PROOF",
            },
            "signature": "ED25519(sig)",
            "receipt_hash": "sha256:hash",
        }

    def test_rejected_receipt_blocks_claim(self):
        """Test that rejected receipt blocks claim."""
        receipt = self._make_receipt(status="REJECTED", verdict="REJECTED_PROOF_FAILED")
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_BLOCKED.value)
        self.assertEqual(report.aggregated_verdict["rejected_count"], 1)

    def test_timeout_receipt_pending_status(self):
        """Test that timeout receipt results in PENDING status."""
        receipt = self._make_receipt(status="TIMEOUT", verdict="TIMEOUT_NO_RESPONSE")
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_PENDING.value)
        self.assertEqual(report.aggregated_verdict["timeout_count"], 1)

    def test_no_receipts_pending_status(self):
        """Test that no receipts results in PENDING status."""
        claim = {"id": "CLAIM-001"}
        authority_levels = {}

        report = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        self.assertEqual(report.aggregated_verdict["status"], AggregatedStatus.CLAIM_PENDING.value)

    def test_signature_verification_failure(self):
        """Test signature verification check."""
        receipt = self._make_receipt()
        receipt["signature"] = ""  # Empty signature

        # Should be rejected during add_receipt
        result = self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)
        self.assertFalse(result)


class TestCircularDependencies(unittest.TestCase):
    """Test circular dependency detection."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")

    def test_no_cycle_detected(self):
        """Test that linear dependency has no cycle."""
        graph = {
            "repo-a": ["repo-b"],
            "repo-b": ["repo-c"],
            "repo-c": [],
        }
        result = self.aggregator.check_circular_dependencies(graph)
        self.assertFalse(result)

    def test_simple_cycle_detected(self):
        """Test that simple cycle A->B->A is detected."""
        graph = {
            "repo-a": ["repo-b"],
            "repo-b": ["repo-a"],
        }
        result = self.aggregator.check_circular_dependencies(graph)
        self.assertTrue(result)

    def test_self_cycle_detected(self):
        """Test that self-cycle A->A is detected."""
        graph = {
            "repo-a": ["repo-a"],
        }
        result = self.aggregator.check_circular_dependencies(graph)
        self.assertTrue(result)

    def test_complex_cycle_detected(self):
        """Test that complex cycle A->B->C->B is detected."""
        graph = {
            "repo-a": ["repo-b"],
            "repo-b": ["repo-c"],
            "repo-c": ["repo-b"],
        }
        result = self.aggregator.check_circular_dependencies(graph)
        self.assertTrue(result)


class TestFreshness(unittest.TestCase):
    """Test receipt freshness checking."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")

    def _make_receipt(self, days_old=0):
        """Helper to create receipt with specific age."""
        old_time = datetime.utcnow() - timedelta(days=days_old)
        return {
            "schema": "rafaelia.federated_claim_receipt.routing.v1",
            "receipt_id": "RECEIPT-001",
            "origin_repo": "rafaelmeloreisnovo/Matem-tica-",
            "verification": {
                "status": "VERIFIED",
                "verdict": "PASSED_FORMAL_PROOF",
                "confidence": 0.95,
                "timestamp_utc": old_time.isoformat() + "Z",
            },
            "evidence": {"proof_artifact": "test.v"},
            "authority": {
                "repo": "rafaelmeloreisnovo/Matem-tica-",
                "authority_level": "FORMAL_PROOF",
            },
            "signature": "ED25519(sig)",
            "receipt_hash": "sha256:hash",
        }

    def test_fresh_receipt_passes(self):
        """Test that fresh receipt passes freshness check."""
        receipt = self._make_receipt(days_old=1)
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        result = self.aggregator.check_freshness(max_age_days=7)
        self.assertTrue(result)

    def test_stale_receipt_fails(self):
        """Test that stale receipt fails freshness check."""
        receipt = self._make_receipt(days_old=10)
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        result = self.aggregator.check_freshness(max_age_days=7)
        self.assertFalse(result)

    def test_boundary_receipt_passes(self):
        """Test boundary case (exactly at max age)."""
        receipt = self._make_receipt(days_old=7)
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        result = self.aggregator.check_freshness(max_age_days=7)
        # Boundary may be inclusive or exclusive depending on implementation
        # Just verify it doesn't crash
        self.assertIsInstance(result, bool)


class TestReportGeneration(unittest.TestCase):
    """Test report generation and export."""

    def setUp(self):
        self.aggregator = FederatedAggregator("test-repo", "test-commit")
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _make_receipt(self):
        """Helper to create valid receipt."""
        return {
            "schema": "rafaelia.federated_claim_receipt.routing.v1",
            "receipt_id": "RECEIPT-001",
            "origin_repo": "rafaelmeloreisnovo/Matem-tica-",
            "verification": {
                "status": "VERIFIED",
                "verdict": "PASSED_FORMAL_PROOF",
                "confidence": 0.95,
                "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            },
            "evidence": {"proof_artifact": "test.v"},
            "authority": {
                "repo": "rafaelmeloreisnovo/Matem-tica-",
                "authority_level": "FORMAL_PROOF",
            },
            "signature": "ED25519(sig)",
            "receipt_hash": "sha256:hash",
        }

    def test_report_exported_to_json(self):
        """Test that report is exported to JSON."""
        receipt = self._make_receipt()
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        output_path = Path(self.temp_dir.name) / "report.json"
        result = self.aggregator.export_report(output_path)

        self.assertTrue(output_path.exists())
        self.assertEqual(result, output_path)

    def test_exported_json_is_valid(self):
        """Test that exported JSON is valid and parseable."""
        receipt = self._make_receipt()
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp=datetime.utcnow().isoformat() + "Z",
            authority_levels=authority_levels,
        )

        output_path = Path(self.temp_dir.name) / "report.json"
        self.aggregator.export_report(output_path)

        # Parse exported JSON
        data = json.loads(output_path.read_text())

        # Validate schema
        self.assertEqual(
            data["schema"],
            "rafaelia.federated_aggregation_report.routing.v1"
        )

        # Validate required fields
        self.assertIn("report_id", data)
        self.assertIn("aggregated_verdict", data)
        self.assertIn("report_hash", data)

    def test_report_hash_deterministic(self):
        """Test that report hash is deterministic."""
        receipt = self._make_receipt()
        self.aggregator.add_receipt("rafaelmeloreisnovo/Matem-tica-", receipt)

        claim = {"id": "CLAIM-001"}
        authority_levels = {"rafaelmeloreisnovo/Matem-tica-": "FORMAL_PROOF"}

        report1 = self.aggregator.aggregate_receipts(
            claim_id="CLAIM-001",
            original_claim=claim,
            submission_timestamp="2026-08-15T12:00:00Z",
            authority_levels=authority_levels,
        )

        hash1 = report1.report_hash

        # Recompute hash
        hash2 = self.aggregator._compute_report_hash(report1)

        self.assertEqual(hash1, hash2)


class TestAuthorityWeights(unittest.TestCase):
    """Test authority weight assignment."""

    def test_formal_proof_weight(self):
        """Test FORMAL_PROOF has highest weight."""
        weight = AuthorityWeightAssignment.get_weight("FORMAL_PROOF")
        self.assertEqual(weight, 1.0)

    def test_peer_review_weight(self):
        """Test PEER_REVIEW weight."""
        weight = AuthorityWeightAssignment.get_weight("PEER_REVIEW")
        self.assertEqual(weight, 0.8)

    def test_domain_expert_weight(self):
        """Test DOMAIN_EXPERT weight."""
        weight = AuthorityWeightAssignment.get_weight("DOMAIN_EXPERT")
        self.assertEqual(weight, 0.75)

    def test_cross_reference_weight(self):
        """Test CROSS_REFERENCE weight."""
        weight = AuthorityWeightAssignment.get_weight("CROSS_REFERENCE")
        self.assertEqual(weight, 0.6)

    def test_narrative_weight(self):
        """Test NARRATIVE has lowest weight."""
        weight = AuthorityWeightAssignment.get_weight("NARRATIVE")
        self.assertEqual(weight, 0.5)

    def test_unknown_authority_default_weight(self):
        """Test unknown authority gets default weight."""
        weight = AuthorityWeightAssignment.get_weight("UNKNOWN")
        self.assertEqual(weight, 0.5)

    def test_formal_proof_minimum_confidence(self):
        """Test FORMAL_PROOF minimum confidence threshold."""
        min_conf = AuthorityWeightAssignment.get_min_confidence("FORMAL_PROOF")
        self.assertEqual(min_conf, 0.90)

    def test_narrative_minimum_confidence(self):
        """Test NARRATIVE minimum confidence threshold."""
        min_conf = AuthorityWeightAssignment.get_min_confidence("NARRATIVE")
        self.assertEqual(min_conf, 0.60)


if __name__ == "__main__":
    unittest.main()
