#!/usr/bin/env python3
"""
Tests for CIGateValidator (Phase 3: CI Gate Enforcement)

Covers gate validation, precedence order, report generation,
and CI integration.
"""

import unittest
import json
import tempfile
from pathlib import Path
from datetime import datetime, timedelta

import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))

from ci_gate_validator import (
    CIGateValidator,
    GateStatus,
    GatePrecedence,
    GateCheckResult,
    CIGateReport,
)


class TestGateValidation(unittest.TestCase):
    """Test gate validation logic."""

    def setUp(self):
        self.validator = CIGateValidator("PR-293", "e58b7e5")
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _make_aggregation_report(
        self,
        claim_allowed=True,
        confidence=0.95,
        rejected_count=0,
        timeout_count=0
    ):
        """Helper to create valid aggregation report."""
        return {
            "schema": "rafaelia.federated_aggregation_report.routing.v1",
            "report_id": "REPORT-CLAIM-001-2026-08-15",
            "local_repo": "rafaelmeloreisnovo/RafPolimata",
            "local_commit": "abc123",
            "claim_id": "CLAIM-001",
            "original_claim": {
                "id": "CLAIM-001",
                "domain": "COMPILER",
                "source": "phase-21-type-system",
            },
            "receipts": [
                {
                    "authority": "rafaelmeloreisnovo/Matem-tica-",
                    "receipt_id": "RECEIPT-001",
                    "status": "VERIFIED",
                    "verdict": "PASSED_FORMAL_PROOF",
                    "confidence": confidence,
                    "timestamp_utc": (datetime.utcnow() - timedelta(hours=1)).isoformat() + "Z",
                    "signature": "ED25519(sig)",
                }
            ],
            "aggregated_verdict": {
                "status": "CLAIM_ALLOWED" if claim_allowed else "CLAIM_BLOCKED",
                "combined_confidence": confidence,
                "verified_count": 1 if claim_allowed else 0,
                "rejected_count": rejected_count,
                "timeout_count": timeout_count,
                "requires_additional_evidence": not claim_allowed,
                "recommendations": [],
            },
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            "report_hash": "sha256:abc123",
        }

    def test_valid_aggregation_passes_all_gates(self):
        """Test that valid aggregation passes all gates."""
        report_data = self._make_aggregation_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertEqual(gate_report.overall_status, GateStatus.PASS)
        self.assertGreater(len(gate_report.gate_checks), 0)

    def test_missing_report_fails(self):
        """Test that missing report fails."""
        report_path = Path(self.temp_dir.name) / "nonexistent.json"

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertEqual(gate_report.overall_status, GateStatus.FAIL)
        self.assertGreater(len(gate_report.gate_checks), 0)

    def test_invalid_json_fails(self):
        """Test that invalid JSON fails."""
        report_path = Path(self.temp_dir.name) / "invalid.json"
        report_path.write_text("not valid json")

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertEqual(gate_report.overall_status, GateStatus.FAIL)

    def test_rejected_claim_fails(self):
        """Test that rejected claim fails gate."""
        report_data = self._make_aggregation_report(claim_allowed=False, rejected_count=1)
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertEqual(gate_report.overall_status, GateStatus.BLOCKED)

    def test_low_confidence_fails(self):
        """Test that low confidence fails threshold gate."""
        report_data = self._make_aggregation_report(confidence=0.50)
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertEqual(gate_report.overall_status, GateStatus.BLOCKED)


class TestGatePrecedence(unittest.TestCase):
    """Test gate execution precedence."""

    def setUp(self):
        self.validator = CIGateValidator("PR-293", "e58b7e5")
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _make_report_with_invalid_signatures(self):
        """Create report with invalid signatures."""
        return {
            "schema": "rafaelia.federated_aggregation_report.routing.v1",
            "report_id": "REPORT-001",
            "claim_id": "CLAIM-001",
            "receipts": [
                {
                    "authority": "rafaelmeloreisnovo/Matem-tica-",
                    "receipt_id": "RECEIPT-001",
                    "status": "VERIFIED",
                    "verdict": "PASSED_FORMAL_PROOF",
                    "confidence": 0.95,
                    "timestamp_utc": datetime.utcnow().isoformat() + "Z",
                    "signature": "",  # Invalid: empty signature
                }
            ],
            "aggregated_verdict": {
                "status": "CLAIM_ALLOWED",
                "combined_confidence": 0.95,
                "verified_count": 1,
                "rejected_count": 0,
                "timeout_count": 0,
            },
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
        }

    def test_signature_validation_first(self):
        """Test that signature validation is checked first."""
        report_data = self._make_report_with_invalid_signatures()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        # First gate should be signature validation
        self.assertEqual(
            gate_report.gate_checks[0].gate,
            GatePrecedence.SIGNATURE_VALIDATION
        )
        self.assertEqual(gate_report.gate_checks[0].status, GateStatus.FAIL)


class TestGateCheckResults(unittest.TestCase):
    """Test individual gate check results."""

    def setUp(self):
        self.validator = CIGateValidator("PR-293", "e58b7e5")
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _make_valid_report(self):
        """Create valid aggregation report."""
        return {
            "schema": "rafaelia.federated_aggregation_report.routing.v1",
            "report_id": "REPORT-001",
            "claim_id": "CLAIM-001",
            "receipts": [
                {
                    "authority": "rafaelmeloreisnovo/Matem-tica-",
                    "receipt_id": "RECEIPT-001",
                    "status": "VERIFIED",
                    "verdict": "PASSED_FORMAL_PROOF",
                    "confidence": 0.95,
                    "timestamp_utc": (datetime.utcnow() - timedelta(hours=1)).isoformat() + "Z",
                    "signature": "ED25519(sig)",
                }
            ],
            "aggregated_verdict": {
                "status": "CLAIM_ALLOWED",
                "combined_confidence": 0.95,
                "verified_count": 1,
                "rejected_count": 0,
                "timeout_count": 0,
            },
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
        }

    def test_signature_check_passes(self):
        """Test signature validation check passes."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        sig_check = [c for c in gate_report.gate_checks
                     if c.gate == GatePrecedence.SIGNATURE_VALIDATION]
        self.assertEqual(len(sig_check), 1)
        self.assertEqual(sig_check[0].status, GateStatus.PASS)

    def test_freshness_check_passes(self):
        """Test freshness check passes for recent receipts."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        fresh_check = [c for c in gate_report.gate_checks
                       if c.gate == GatePrecedence.FRESHNESS_CHECK]
        self.assertEqual(len(fresh_check), 1)
        self.assertEqual(fresh_check[0].status, GateStatus.PASS)

    def test_freshness_check_fails_stale(self):
        """Test freshness check fails for stale receipts."""
        report_data = self._make_valid_report()
        # Make receipt 10 days old
        report_data["receipts"][0]["timestamp_utc"] = (
            datetime.utcnow() - timedelta(days=10)
        ).isoformat() + "Z"
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        fresh_check = [c for c in gate_report.gate_checks
                       if c.gate == GatePrecedence.FRESHNESS_CHECK]
        self.assertEqual(len(fresh_check), 1)
        self.assertEqual(fresh_check[0].status, GateStatus.FAIL)

    def test_confidence_check_passes(self):
        """Test confidence threshold check passes."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        conf_check = [c for c in gate_report.gate_checks
                      if c.gate == GatePrecedence.CONFIDENCE_THRESHOLD]
        self.assertEqual(len(conf_check), 1)
        self.assertEqual(conf_check[0].status, GateStatus.PASS)

    def test_confidence_check_fails_low(self):
        """Test confidence check fails for low confidence."""
        report_data = self._make_valid_report()
        report_data["aggregated_verdict"]["combined_confidence"] = 0.50
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        conf_check = [c for c in gate_report.gate_checks
                      if c.gate == GatePrecedence.CONFIDENCE_THRESHOLD]
        self.assertEqual(len(conf_check), 1)
        self.assertEqual(conf_check[0].status, GateStatus.FAIL)

    def test_no_rejections_check_passes(self):
        """Test no rejections check passes."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        rej_check = [c for c in gate_report.gate_checks
                     if c.gate == GatePrecedence.NO_REJECTIONS]
        self.assertEqual(len(rej_check), 1)
        self.assertEqual(rej_check[0].status, GateStatus.PASS)

    def test_no_rejections_check_fails(self):
        """Test no rejections check fails when claim rejected."""
        report_data = self._make_valid_report()
        report_data["aggregated_verdict"]["rejected_count"] = 1
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        rej_check = [c for c in gate_report.gate_checks
                     if c.gate == GatePrecedence.NO_REJECTIONS]
        self.assertEqual(len(rej_check), 1)
        self.assertEqual(rej_check[0].status, GateStatus.FAIL)


class TestReportGeneration(unittest.TestCase):
    """Test gate report generation."""

    def setUp(self):
        self.validator = CIGateValidator("PR-293", "e58b7e5")
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _make_valid_report(self):
        """Create valid aggregation report."""
        return {
            "schema": "rafaelia.federated_aggregation_report.routing.v1",
            "report_id": "REPORT-001",
            "claim_id": "CLAIM-001",
            "receipts": [
                {
                    "authority": "rafaelmeloreisnovo/Matem-tica-",
                    "receipt_id": "RECEIPT-001",
                    "status": "VERIFIED",
                    "verdict": "PASSED_FORMAL_PROOF",
                    "confidence": 0.95,
                    "timestamp_utc": (datetime.utcnow() - timedelta(hours=1)).isoformat() + "Z",
                    "signature": "ED25519(sig)",
                }
            ],
            "aggregated_verdict": {
                "status": "CLAIM_ALLOWED",
                "combined_confidence": 0.95,
                "verified_count": 1,
                "rejected_count": 0,
                "timeout_count": 0,
            },
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
        }

    def test_report_exported_to_json(self):
        """Test that gate report is exported to JSON."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        output_path = Path(self.temp_dir.name) / "gate_report.json"
        result = self.validator.export_report(output_path, gate_report)

        self.assertTrue(output_path.exists())
        self.assertEqual(result, output_path)

    def test_exported_json_is_valid(self):
        """Test that exported JSON is valid."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        output_path = Path(self.temp_dir.name) / "gate_report.json"
        self.validator.export_report(output_path, gate_report)

        data = json.loads(output_path.read_text())

        # Validate schema
        self.assertEqual(
            data["schema"],
            "rafaelia.ci_gate_report.routing.v1"
        )

        # Validate required fields
        self.assertIn("report_id", data)
        self.assertIn("gate_checks", data)
        self.assertIn("overall_status", data)

    def test_report_hash_deterministic(self):
        """Test that report hash is deterministic."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        hash1 = gate_report.report_hash

        # Recompute hash
        hash2 = self.validator._compute_report_hash(gate_report)

        self.assertEqual(hash1, hash2)

    def test_summary_computed(self):
        """Test that summary is correctly computed."""
        report_data = self._make_valid_report()
        report_path = Path(self.temp_dir.name) / "report.json"
        report_path.write_text(json.dumps(report_data))

        gate_report = self.validator.validate_aggregation_report(report_path)

        self.assertGreater(gate_report.summary["total_gates"], 0)
        self.assertIn("passed", gate_report.summary)
        self.assertIn("failed", gate_report.summary)
        self.assertIn("claim_allowed", gate_report.summary)


class TestGateCheckResult(unittest.TestCase):
    """Test GateCheckResult dataclass."""

    def test_result_to_dict(self):
        """Test that result can be converted to dict."""
        result = GateCheckResult(
            gate=GatePrecedence.SIGNATURE_VALIDATION,
            status=GateStatus.PASS,
            message="Test message",
            details={"key": "value"},
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )

        data = result.to_dict()

        self.assertEqual(data["gate"], "SIGNATURE_VALIDATION")
        self.assertEqual(data["status"], "PASS")
        self.assertEqual(data["message"], "Test message")
        self.assertEqual(data["details"]["key"], "value")


if __name__ == "__main__":
    unittest.main()
