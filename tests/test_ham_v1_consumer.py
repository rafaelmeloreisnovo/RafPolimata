#!/usr/bin/env python3
"""
Tests for HAM v1 Consumer — Human-AI Middleware Job Processor.
"""

from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "ham_v1_consumer", ROOT / "tools/ham_v1_consumer.py"
)
assert SPEC and SPEC.loader
HAM = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = HAM
SPEC.loader.exec_module(HAM)


class HAMValidatorTests(unittest.TestCase):
    """Tests for HAMJobValidator."""

    def test_validate_schema_valid_job(self) -> None:
        """Validator should accept valid job schema."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {
                "max_bytes": 10000,
                "timeout_sec": 60,
                "retries": 3,
            },
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        self.assertTrue(validator.validate_schema(job))

    def test_validate_schema_missing_job_id(self) -> None:
        """Validator should reject job missing job_id."""
        job = {
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("job_id", str(ctx.exception))

    def test_validate_schema_invalid_limits_type(self) -> None:
        """Validator should reject non-dict limits."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": "not-a-dict",
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("limits", str(ctx.exception))

    def test_validate_schema_negative_max_bytes(self) -> None:
        """Validator should reject negative max_bytes."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": -100, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("max_bytes", str(ctx.exception))

    def test_validate_schema_zero_timeout(self) -> None:
        """Validator should reject zero timeout_sec."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 0, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("timeout_sec", str(ctx.exception))

    def test_validate_schema_negative_retries(self) -> None:
        """Validator should reject negative retries."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": -1},
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("retries", str(ctx.exception))

    def test_validate_schema_invalid_idempotency(self) -> None:
        """Validator should reject invalid idempotency value."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "UNKNOWN",
        }

        validator = HAM.HAMJobValidator()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            validator.validate_schema(job)
        self.assertIn("idempotency", str(ctx.exception))

    def test_check_forbidden_patterns_clean_job(self) -> None:
        """Validator should pass clean job with no forbidden patterns."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertEqual(len(violations), 0)

    def test_check_forbidden_patterns_secret(self) -> None:
        """Validator should detect secret keyword."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "api_secret": "sk-1234567890",
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertGreater(len(violations), 0)
        self.assertTrue(any("secret" in v.lower() for v in violations))

    def test_check_forbidden_patterns_password(self) -> None:
        """Validator should detect password keyword."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "password": "hunter2",
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertGreater(len(violations), 0)

    def test_check_forbidden_patterns_sensor(self) -> None:
        """Validator should detect sensor keyword."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "sensor_data": {"gps": "41.8781,-87.6298"},
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertGreater(len(violations), 0)

    def test_check_forbidden_patterns_network(self) -> None:
        """Validator should detect network operations."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "command": "curl https://example.com",
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertGreater(len(violations), 0)

    def test_check_forbidden_patterns_write(self) -> None:
        """Validator should detect write operations."""
        job = {
            "job_id": "test-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "cleanup": "rm -rf /tmp/*",
        }

        validator = HAM.HAMJobValidator()
        violations = validator.check_forbidden_patterns(job)
        self.assertGreater(len(violations), 0)


class HAMReceiptTests(unittest.TestCase):
    """Tests for HAMJobReceipt."""

    def test_receipt_initialization(self) -> None:
        """Receipt should initialize with required fields."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")

        self.assertEqual(receipt.job_id, "job-123")
        self.assertEqual(receipt.target, "/usr/bin/gcc")
        self.assertIn("contract_version", receipt.receipt)
        self.assertEqual(receipt.receipt["status"], "PENDING")
        self.assertEqual(receipt.receipt["exit_code"], "TOKEN_VAZIO")

    def test_receipt_add_execution_result_success(self) -> None:
        """Receipt should record successful execution."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")
        receipt.add_execution_result(
            exit_code=0,
            stdout="compilation successful",
            stderr="",
            execution_time_ms=1500,
        )

        self.assertEqual(receipt.receipt["exit_code"], 0)
        self.assertEqual(receipt.receipt["status"], "PASS")
        self.assertEqual(receipt.receipt["execution_time_ms"], 1500)
        self.assertNotEqual(receipt.receipt["stdout_hash"], "TOKEN_VAZIO")

    def test_receipt_add_execution_result_failure(self) -> None:
        """Receipt should record failed execution."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")
        receipt.add_execution_result(
            exit_code=1,
            stdout="",
            stderr="compilation error",
            execution_time_ms=500,
        )

        self.assertEqual(receipt.receipt["exit_code"], 1)
        self.assertEqual(receipt.receipt["status"], "FAIL")
        self.assertNotEqual(receipt.receipt["stderr_hash"], "TOKEN_VAZIO")

    def test_receipt_add_bounded_evidence_within_limit(self) -> None:
        """Receipt should store evidence within size limit."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")
        evidence_text = "short evidence"
        receipt.add_bounded_evidence("output", evidence_text, max_bytes=100)

        evidence = receipt.receipt["bounded_evidence"]["output"]
        self.assertEqual(evidence["value"], evidence_text)
        self.assertFalse(evidence["truncated"])

    def test_receipt_add_bounded_evidence_truncated(self) -> None:
        """Receipt should truncate evidence exceeding size limit."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")
        evidence_text = "a" * 200
        receipt.add_bounded_evidence("output", evidence_text, max_bytes=50)

        evidence = receipt.receipt["bounded_evidence"]["output"]
        self.assertEqual(len(evidence["value"]), 50)
        self.assertTrue(evidence["truncated"])
        self.assertEqual(evidence["original_length"], 200)

    def test_receipt_finalize_adds_hash(self) -> None:
        """Receipt finalize should compute receipt_hash."""
        receipt = HAM.HAMJobReceipt("job-123", "/usr/bin/gcc")
        receipt.add_execution_result(exit_code=0)

        finalized = receipt.finalize()

        self.assertIn("receipt_hash", finalized)
        self.assertEqual(len(finalized["receipt_hash"]), 64)  # SHA256 hex
        # Verify it's a valid hex string
        self.assertTrue(all(c in "0123456789abcdef" for c in finalized["receipt_hash"]))


class HAMConsumerTests(unittest.TestCase):
    """Tests for HAMConsumer."""

    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def test_consumer_initialization(self) -> None:
        """Consumer should initialize with empty state."""
        consumer = HAM.HAMConsumer()

        self.assertEqual(consumer.jobs_processed, 0)
        self.assertEqual(len(consumer.receipts), 0)

    def test_consume_valid_job(self) -> None:
        """Consumer should process valid job."""
        job = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        consumer = HAM.HAMConsumer()
        receipt = consumer.consume_job(job)

        self.assertEqual(receipt.job_id, "job-001")
        self.assertEqual(consumer.jobs_processed, 1)
        self.assertEqual(len(consumer.receipts), 1)

    def test_consume_invalid_schema_raises_error(self) -> None:
        """Consumer should reject job with invalid schema."""
        job = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            # Missing required fields
        }

        consumer = HAM.HAMConsumer()
        with self.assertRaises(HAM.HAMContractError):
            consumer.consume_job(job)

    def test_consume_job_with_forbidden_pattern_raises_error(self) -> None:
        """Consumer should reject job with forbidden patterns."""
        job = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
            "api_key": "sk-12345",  # Secret pattern
        }

        consumer = HAM.HAMConsumer()
        with self.assertRaises(HAM.HAMContractError) as ctx:
            consumer.consume_job(job)
        self.assertIn("Security violations", str(ctx.exception))

    def test_consume_multiple_jobs(self) -> None:
        """Consumer should track multiple jobs."""
        jobs = [
            {
                "job_id": f"job-{i:03d}",
                "target": "/usr/bin/gcc",
                "effect": "compile",
                "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
                "idempotency": "IDEMPOTENT",
            }
            for i in range(5)
        ]

        consumer = HAM.HAMConsumer()
        for job in jobs:
            consumer.consume_job(job)

        self.assertEqual(consumer.jobs_processed, 5)
        self.assertEqual(len(consumer.receipts), 5)

    def test_emit_receipts_creates_jsonl_file(self) -> None:
        """Consumer should write receipts to JSONL file."""
        job = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        consumer = HAM.HAMConsumer()
        consumer.consume_job(job)

        output_path = self.temp_root / "receipts.jsonl"
        written_path = consumer.emit_receipts(output_path)

        self.assertTrue(written_path.exists())

        # Verify JSONL format
        lines = written_path.read_text(encoding="utf-8").strip().split("\n")
        self.assertEqual(len(lines), 1)

        # Verify JSON validity
        receipt = json.loads(lines[0])
        self.assertIn("job_id", receipt)
        self.assertIn("receipt_hash", receipt)

    def test_emit_receipts_multiple_jobs(self) -> None:
        """Consumer should write multiple receipts to JSONL."""
        consumer = HAM.HAMConsumer()
        for i in range(3):
            job = {
                "job_id": f"job-{i:03d}",
                "target": "/usr/bin/gcc",
                "effect": "compile",
                "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
                "idempotency": "IDEMPOTENT",
            }
            consumer.consume_job(job)

        output_path = self.temp_root / "receipts.jsonl"
        consumer.emit_receipts(output_path)

        lines = output_path.read_text(encoding="utf-8").strip().split("\n")
        self.assertEqual(len(lines), 3)

    def test_generate_summary_empty(self) -> None:
        """Summary should handle empty consumer."""
        consumer = HAM.HAMConsumer()
        summary = consumer.generate_summary()

        self.assertEqual(summary["jobs_processed"], 0)
        self.assertEqual(summary["receipts_generated"], 0)
        self.assertEqual(summary["passed"], 0)
        self.assertEqual(summary["failed"], 0)
        self.assertEqual(summary["unknown"], 0)

    def test_generate_summary_with_jobs(self) -> None:
        """Summary should count job statuses."""
        consumer = HAM.HAMConsumer()

        # Add jobs
        for i in range(3):
            job = {
                "job_id": f"job-{i:03d}",
                "target": "/usr/bin/gcc",
                "effect": "compile",
                "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
                "idempotency": "IDEMPOTENT",
            }
            consumer.consume_job(job)

        summary = consumer.generate_summary()

        self.assertEqual(summary["jobs_processed"], 3)
        self.assertEqual(summary["receipts_generated"], 3)
        # All jobs start as PENDING
        self.assertEqual(summary["unknown"], 3)


class HAMIntegrationTests(unittest.TestCase):
    """Integration tests for HAM v1 consumer."""

    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def test_end_to_end_job_processing(self) -> None:
        """End-to-end: input JSONL → consumer → output receipts + summary."""
        input_path = self.temp_root / "jobs.jsonl"
        output_path = self.temp_root / "receipts.jsonl"
        summary_path = self.temp_root / "summary.json"

        # Create input jobs
        jobs = [
            {
                "job_id": "compile-1",
                "target": "/usr/bin/gcc",
                "effect": "compile source.c",
                "limits": {"max_bytes": 50000, "timeout_sec": 120, "retries": 2},
                "idempotency": "IDEMPOTENT",
            },
            {
                "job_id": "link-1",
                "target": "/usr/bin/ld",
                "effect": "link object files",
                "limits": {"max_bytes": 100000, "timeout_sec": 60, "retries": 1},
                "idempotency": "AT_LEAST_ONCE",
            },
            {
                "job_id": "test-1",
                "target": "/usr/bin/pytest",
                "effect": "run tests",
                "limits": {"max_bytes": 200000, "timeout_sec": 300, "retries": 3},
                "idempotency": "AT_MOST_ONCE",
            },
        ]

        # Write input jobs
        with open(input_path, "w", encoding="utf-8") as f:
            for job in jobs:
                f.write(json.dumps(job, separators=(",", ":")) + "\n")

        # Process jobs
        consumer = HAM.HAMConsumer()
        with open(input_path, "r", encoding="utf-8") as f:
            for line in f:
                if line.strip():
                    job = json.loads(line)
                    receipt = consumer.consume_job(job)

        # Emit receipts
        consumer.emit_receipts(output_path)

        # Generate summary
        summary = consumer.generate_summary()
        summary_path.write_text(
            json.dumps(summary, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

        # Verify output files
        self.assertTrue(output_path.exists())
        self.assertTrue(summary_path.exists())

        # Verify receipts
        output_lines = output_path.read_text(encoding="utf-8").strip().split("\n")
        self.assertEqual(len(output_lines), 3)

        # Verify summary
        summary_json = json.loads(summary_path.read_text(encoding="utf-8"))
        self.assertEqual(summary_json["jobs_processed"], 3)
        self.assertEqual(summary_json["receipts_generated"], 3)

    def test_error_recovery_continues_on_invalid_job(self) -> None:
        """Consumer should skip invalid job and continue processing."""
        consumer = HAM.HAMConsumer()

        # First valid job
        job1 = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }
        consumer.consume_job(job1)

        # Invalid job (missing fields)
        job2_invalid = {
            "job_id": "job-002",
            "target": "/usr/bin/gcc",
        }

        # Third valid job
        job3 = {
            "job_id": "job-003",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        # Process first job
        self.assertEqual(consumer.jobs_processed, 1)

        # Invalid job should raise
        with self.assertRaises(HAM.HAMContractError):
            consumer.consume_job(job2_invalid)

        # Process third job
        consumer.consume_job(job3)

        # Summary should reflect successfully processed jobs
        self.assertEqual(consumer.jobs_processed, 2)
        self.assertEqual(len(consumer.receipts), 2)

    def test_receipt_hash_chain_consistency(self) -> None:
        """Each receipt should have consistent, unique hash."""
        consumer = HAM.HAMConsumer()

        job1 = {
            "job_id": "job-001",
            "target": "/usr/bin/gcc",
            "effect": "compile",
            "limits": {"max_bytes": 10000, "timeout_sec": 60, "retries": 3},
            "idempotency": "IDEMPOTENT",
        }

        job2 = {
            "job_id": "job-002",
            "target": "/usr/bin/ld",
            "effect": "link",
            "limits": {"max_bytes": 20000, "timeout_sec": 120, "retries": 2},
            "idempotency": "AT_LEAST_ONCE",
        }

        receipt1 = consumer.consume_job(job1)
        receipt2 = consumer.consume_job(job2)

        hash1 = receipt1.finalize()["receipt_hash"]
        hash2 = receipt2.finalize()["receipt_hash"]

        # Hashes should be different for different receipts
        self.assertNotEqual(hash1, hash2)

        # Hashes should be valid SHA256
        for h in [hash1, hash2]:
            self.assertEqual(len(h), 64)
            self.assertTrue(all(c in "0123456789abcdef" for c in h))


if __name__ == "__main__":
    unittest.main()
