#!/usr/bin/env python3
"""
HAM v1 Consumer — Human-AI Middleware Job Processor

Consumes jobs from raf.human-ai.middleware.v1 contract and emits
bounded receipts with validation and error handling.

Schema validation, secret blocking, and idempotency guarantees.
"""

from __future__ import annotations

import hashlib
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Optional

# HAM v1 contract version
CONTRACT_VERSION = "raf.human-ai.middleware.v1"

# Forbidden patterns in job definitions (security blocklist)
FORBIDDEN_PATTERNS = {
    "secret": r"secret|password|token|api[_-]?key|credential",
    "sensor": r"gps|camera|microphone|sensor|location",
    "network": r"http|https|socket|dns|ping|fetch|request",
    "write": r"write|delete|remove|unlink|rm|mkdir",
}


class HAMContractError(Exception):
    """HAM contract validation error."""
    pass


class HAMJobValidator:
    """Validates HAM v1 job definitions against contract."""

    @staticmethod
    def validate_schema(job: dict[str, Any]) -> bool:
        """Validate job schema against HAM v1 contract."""
        required_fields = ["job_id", "target", "effect", "limits", "idempotency"]

        for field in required_fields:
            if field not in job:
                raise HAMContractError(f"Missing required field: {field}")

        # Validate limits
        limits = job.get("limits", {})
        if not isinstance(limits, dict):
            raise HAMContractError("limits must be a dict with max_bytes, timeout_sec, retries")

        if "max_bytes" not in limits or limits["max_bytes"] <= 0:
            raise HAMContractError("limits.max_bytes must be positive integer")

        if "timeout_sec" not in limits or limits["timeout_sec"] <= 0:
            raise HAMContractError("limits.timeout_sec must be positive integer")

        if "retries" not in limits or limits["retries"] < 0:
            raise HAMContractError("limits.retries must be non-negative integer")

        # Validate idempotency
        idempotency = job.get("idempotency")
        if idempotency not in ("IDEMPOTENT", "AT_LEAST_ONCE", "AT_MOST_ONCE"):
            raise HAMContractError(f"idempotency must be one of: IDEMPOTENT, AT_LEAST_ONCE, AT_MOST_ONCE")

        return True

    @staticmethod
    def check_forbidden_patterns(job: dict[str, Any]) -> list[str]:
        """Check for forbidden patterns (secrets, sensors, etc.)."""
        violations = []

        job_str = json.dumps(job, separators=(",", ":")).lower()

        for pattern_type, pattern in FORBIDDEN_PATTERNS.items():
            import re
            if re.search(pattern, job_str):
                violations.append(f"Forbidden {pattern_type} pattern detected")

        return violations


class HAMJobReceipt:
    """Receipt for completed HAM job with bounded evidence."""

    def __init__(self, job_id: str, target: str):
        self.job_id = job_id
        self.target = target
        self.timestamp = datetime.now(timezone.utc).isoformat() + "Z"
        self.receipt = {
            "contract_version": CONTRACT_VERSION,
            "job_id": job_id,
            "target": target,
            "timestamp": self.timestamp,
            "status": "PENDING",
            "input_hash": "TOKEN_VAZIO",
            "output_hash": "TOKEN_VAZIO",
            "stdout_hash": "TOKEN_VAZIO",
            "stderr_hash": "TOKEN_VAZIO",
            "exit_code": "TOKEN_VAZIO",
            "execution_time_ms": "TOKEN_VAZIO",
            "bounded_evidence": {},
        }

    def add_execution_result(
        self,
        exit_code: int,
        stdout: str = "",
        stderr: str = "",
        execution_time_ms: int = 0,
    ) -> None:
        """Add execution results to receipt."""
        self.receipt["exit_code"] = exit_code
        self.receipt["execution_time_ms"] = execution_time_ms

        if stdout:
            self.receipt["stdout_hash"] = hashlib.sha256(stdout.encode("utf-8")).hexdigest()
        if stderr:
            self.receipt["stderr_hash"] = hashlib.sha256(stderr.encode("utf-8")).hexdigest()

        # Determine status
        if exit_code == 0:
            self.receipt["status"] = "PASS"
        else:
            self.receipt["status"] = "FAIL"

    def add_bounded_evidence(self, key: str, value: str, max_bytes: int = 1000) -> None:
        """Add bounded evidence (truncated if necessary)."""
        if len(value) > max_bytes:
            truncated = value[:max_bytes]
            self.receipt["bounded_evidence"][key] = {
                "value": truncated,
                "truncated": True,
                "original_length": len(value),
            }
        else:
            self.receipt["bounded_evidence"][key] = {
                "value": value,
                "truncated": False,
            }

    def finalize(self) -> dict[str, Any]:
        """Finalize receipt with hash."""
        # Calculate receipt hash over all fields except receipt_hash
        receipt_copy = dict(self.receipt)
        receipt_bytes = json.dumps(
            receipt_copy, separators=(",", ":"), sort_keys=True
        ).encode("utf-8")
        receipt_hash = hashlib.sha256(receipt_bytes).hexdigest()

        self.receipt["receipt_hash"] = receipt_hash
        return self.receipt


class HAMConsumer:
    """Consumes HAM v1 jobs and emits receipts."""

    def __init__(self):
        self.validator = HAMJobValidator()
        self.jobs_processed = 0
        self.receipts: list[dict[str, Any]] = []

    def consume_job(self, job: dict[str, Any]) -> HAMJobReceipt:
        """
        Consume a job from the HAM v1 middleware.

        Validates schema, checks for forbidden patterns, creates receipt.
        """
        # Validate schema
        try:
            self.validator.validate_schema(job)
        except HAMContractError as e:
            raise HAMContractError(f"Schema validation failed: {e}")

        # Check for forbidden patterns
        violations = self.validator.check_forbidden_patterns(job)
        if violations:
            raise HAMContractError(f"Security violations: {'; '.join(violations)}")

        # Extract required fields
        job_id = job.get("job_id")
        target = job.get("target")

        # Create receipt
        receipt = HAMJobReceipt(job_id, target)

        # Add metadata from job
        receipt.add_bounded_evidence("target", target, max_bytes=200)
        receipt.add_bounded_evidence("effect", str(job.get("effect", "")), max_bytes=500)

        # Add limits info
        limits = job.get("limits", {})
        receipt.add_bounded_evidence(
            "limits",
            json.dumps(limits, separators=(",", ":")),
            max_bytes=300,
        )

        self.jobs_processed += 1
        receipt_dict = receipt.finalize()
        self.receipts.append(receipt_dict)

        return receipt

    def emit_receipts(self, output_path: Path | str) -> Path:
        """Write receipts to JSONL file."""
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, "w", encoding="utf-8") as f:
            for receipt in self.receipts:
                f.write(json.dumps(receipt, separators=(",", ":")) + "\n")

        return output_path

    def generate_summary(self) -> dict[str, Any]:
        """Generate summary of job processing."""
        passed = sum(1 for r in self.receipts if r["status"] == "PASS")
        failed = sum(1 for r in self.receipts if r["status"] == "FAIL")
        unknown = sum(1 for r in self.receipts if r["status"] == "PENDING")

        return {
            "contract_version": CONTRACT_VERSION,
            "jobs_processed": self.jobs_processed,
            "receipts_generated": len(self.receipts),
            "passed": passed,
            "failed": failed,
            "unknown": unknown,
            "timestamp": datetime.now(timezone.utc).isoformat() + "Z",
        }


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Consume HAM v1 jobs and emit receipts")
    parser.add_argument("--input", required=True, help="Input JSONL file with jobs")
    parser.add_argument("--output", required=True, help="Output JSONL file for receipts")
    parser.add_argument("--summary", help="Output JSON file for summary")

    args = parser.parse_args()

    consumer = HAMConsumer()

    # Read and process jobs
    try:
        with open(args.input, "r", encoding="utf-8") as f:
            for i, line in enumerate(f, 1):
                if not line.strip():
                    continue
                try:
                    job = json.loads(line)
                    receipt = consumer.consume_job(job)
                    print(f"✓ Job {i}: {job.get('job_id')} → {receipt.receipt['status']}")
                except HAMContractError as e:
                    print(f"✗ Job {i}: {e}")
                    sys.exit(1)
    except FileNotFoundError:
        print(f"ERROR: Input file not found: {args.input}")
        sys.exit(1)

    # Emit receipts
    output_path = consumer.emit_receipts(args.output)
    print(f"✓ Receipts written to {output_path}")

    # Write summary if requested
    if args.summary:
        summary = consumer.generate_summary()
        summary_path = Path(args.summary)
        summary_path.parent.mkdir(parents=True, exist_ok=True)
        summary_path.write_text(
            json.dumps(summary, indent=2, ensure_ascii=False)
        )
        print(f"✓ Summary written to {summary_path}")

    sys.exit(0)
