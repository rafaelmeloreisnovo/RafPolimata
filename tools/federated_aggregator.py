#!/usr/bin/env python3
"""
Phase 2: Federated Aggregator

Collects and aggregates receipts from multiple external repositories.
Computes combined confidence scores and determines overall claim verdict.

Protocol: RAFAELIA-PSC-1-FED (federated_claim_protocol.routing.v1)
"""

import json
import hashlib
from dataclasses import dataclass, asdict, field
from datetime import datetime, timedelta
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple
from enum import Enum


class AggregationStatus(str, Enum):
    """Status of aggregation process."""
    PENDING = "PENDING"
    IN_PROGRESS = "IN_PROGRESS"
    COMPLETE = "COMPLETE"
    FAILED = "FAILED"


class VerificationVerdict(str, Enum):
    """Final verdict from an authority."""
    PASSED_FORMAL_PROOF = "PASSED_FORMAL_PROOF"
    PASSED_PEER_REVIEW = "PASSED_PEER_REVIEW"
    PASSED_DOMAIN_EXPERT = "PASSED_DOMAIN_EXPERT"
    PASSED_CROSS_REFERENCE = "PASSED_CROSS_REFERENCE"
    PASSED_NARRATIVE = "PASSED_NARRATIVE"
    REJECTED_PROOF_FAILED = "REJECTED_PROOF_FAILED"
    REJECTED_REVIEW_FAILED = "REJECTED_REVIEW_FAILED"
    REJECTED_SIGNATURE_INVALID = "REJECTED_SIGNATURE_INVALID"
    TIMEOUT_NO_RESPONSE = "TIMEOUT_NO_RESPONSE"
    BLOCKED_CIRCULAR_DEPENDENCY = "BLOCKED_CIRCULAR_DEPENDENCY"


class AggregatedStatus(str, Enum):
    """Overall aggregation status."""
    CLAIM_ALLOWED = "CLAIM_ALLOWED"
    CLAIM_BLOCKED = "CLAIM_BLOCKED"
    CLAIM_PENDING = "CLAIM_PENDING"


@dataclass
class ReceiptRecord:
    """Single receipt from an authority."""
    authority: str
    receipt_id: str
    status: str  # VERIFIED, REJECTED, TIMEOUT
    verdict: str  # VerificationVerdict value
    confidence: float  # 0.0-1.0
    timestamp_utc: str
    signature: str
    proof_artifact: str = ""
    receipt_hash: str = ""


@dataclass
class AuthorityWeight:
    """Weight assignment for different authority levels."""
    level: str
    weight: float  # Importance multiplier
    min_confidence: float  # Minimum confidence threshold


@dataclass
class AggregationReport:
    """Final aggregation report."""
    schema: str = "rafaelia.federated_aggregation_report.routing.v1"
    report_id: str = ""
    local_repo: str = ""
    local_commit: str = ""
    claim_id: str = ""

    original_claim: Dict[str, Any] = field(default_factory=dict)
    submission_timestamp_utc: str = ""

    receipts: List[ReceiptRecord] = field(default_factory=list)

    aggregated_verdict: Dict[str, Any] = field(default_factory=dict)

    timestamp_utc: str = ""
    report_hash: str = ""

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "schema": self.schema,
            "report_id": self.report_id,
            "local_repo": self.local_repo,
            "local_commit": self.local_commit,
            "claim_id": self.claim_id,
            "original_claim": self.original_claim,
            "submission_timestamp_utc": self.submission_timestamp_utc,
            "receipts": [
                {
                    "authority": r.authority,
                    "receipt_id": r.receipt_id,
                    "status": r.status,
                    "verdict": r.verdict,
                    "confidence": r.confidence,
                    "timestamp_utc": r.timestamp_utc,
                }
                for r in self.receipts
            ],
            "aggregated_verdict": self.aggregated_verdict,
            "timestamp_utc": self.timestamp_utc,
            "report_hash": self.report_hash,
        }


class AuthorityWeightAssignment:
    """Assigns weights to authorities based on their verification level."""

    # Authority level mappings
    AUTHORITY_WEIGHTS = {
        "FORMAL_PROOF": AuthorityWeight("FORMAL_PROOF", 1.0, 0.90),
        "PEER_REVIEW": AuthorityWeight("PEER_REVIEW", 0.8, 0.80),
        "DOMAIN_EXPERT": AuthorityWeight("DOMAIN_EXPERT", 0.75, 0.75),
        "CROSS_REFERENCE": AuthorityWeight("CROSS_REFERENCE", 0.6, 0.70),
        "NARRATIVE": AuthorityWeight("NARRATIVE", 0.5, 0.60),
    }

    @classmethod
    def get_weight(cls, authority_level: str) -> float:
        """Get weight for authority level."""
        if authority_level in cls.AUTHORITY_WEIGHTS:
            return cls.AUTHORITY_WEIGHTS[authority_level].weight
        return 0.5  # Default weight

    @classmethod
    def get_min_confidence(cls, authority_level: str) -> float:
        """Get minimum confidence threshold for authority level."""
        if authority_level in cls.AUTHORITY_WEIGHTS:
            return cls.AUTHORITY_WEIGHTS[authority_level].min_confidence
        return 0.70  # Default minimum


class FederatedAggregator:
    """Aggregates receipts from multiple external repositories."""

    def __init__(self, local_repo: str, local_commit: str):
        self.local_repo = local_repo
        self.local_commit = local_commit
        self.receipts: List[ReceiptRecord] = []
        self.reports: List[AggregationReport] = []
        self.authority_verdicts: Dict[str, Tuple[str, float]] = {}

    def add_receipt(
        self,
        authority: str,
        receipt: Dict[str, Any]
    ) -> bool:
        """Add receipt from authority."""
        # Validate receipt format
        if not self._validate_receipt(receipt):
            return False

        # Extract receipt information
        record = ReceiptRecord(
            authority=authority,
            receipt_id=receipt.get("receipt_id", ""),
            status=receipt.get("verification", {}).get("status", "UNKNOWN"),
            verdict=receipt.get("verification", {}).get("verdict", "UNKNOWN"),
            confidence=receipt.get("verification", {}).get("confidence", 0.0),
            timestamp_utc=receipt.get("verification", {}).get("timestamp_utc", ""),
            signature=receipt.get("signature", ""),
            proof_artifact=receipt.get("evidence", {}).get("proof_artifact", ""),
            receipt_hash=receipt.get("receipt_hash", ""),
        )

        self.receipts.append(record)
        self.authority_verdicts[authority] = (record.verdict, record.confidence)

        return True

    def _validate_receipt(self, receipt: Dict[str, Any]) -> bool:
        """Validate receipt structure and content."""
        # Must have required fields
        if "schema" not in receipt:
            return False
        if "receipt_id" not in receipt:
            return False
        if "verification" not in receipt:
            return False
        if "signature" not in receipt:
            return False

        verification = receipt["verification"]
        if "status" not in verification or "verdict" not in verification:
            return False

        # Signature must not be empty
        if not receipt["signature"]:
            return False

        return True

    def aggregate_receipts(
        self,
        claim_id: str,
        original_claim: Dict[str, Any],
        submission_timestamp: str,
        authority_levels: Dict[str, str]
    ) -> AggregationReport:
        """Aggregate all receipts and compute verdict."""
        # Create report
        report = AggregationReport(
            report_id=f"REPORT-{claim_id}-{datetime.utcnow().isoformat()[:10]}",
            local_repo=self.local_repo,
            local_commit=self.local_commit,
            claim_id=claim_id,
            original_claim=original_claim,
            submission_timestamp_utc=submission_timestamp,
            receipts=self.receipts,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )

        # Compute aggregated verdict
        verdict = self._compute_aggregated_verdict(authority_levels)
        report.aggregated_verdict = verdict

        # Compute report hash
        report.report_hash = self._compute_report_hash(report)

        self.reports.append(report)

        return report

    def _compute_aggregated_verdict(
        self,
        authority_levels: Dict[str, str]
    ) -> Dict[str, Any]:
        """Compute overall verdict from individual receipts."""
        if not self.receipts:
            return {
                "status": AggregatedStatus.CLAIM_PENDING.value,
                "combined_confidence": 0.0,
                "requires_additional_evidence": True,
                "recommendations": ["No receipts received from authorities"],
            }

        # Separate receipts by status
        verified = []
        rejected = []
        timeout = []

        for receipt in self.receipts:
            if receipt.status == "VERIFIED":
                verified.append(receipt)
            elif receipt.status == "REJECTED":
                rejected.append(receipt)
            elif receipt.status == "TIMEOUT":
                timeout.append(receipt)

        # Check for critical failures
        if rejected:
            return {
                "status": AggregatedStatus.CLAIM_BLOCKED.value,
                "combined_confidence": 0.0,
                "verified_count": len(verified),
                "rejected_count": len(rejected),
                "timeout_count": len(timeout),
                "requires_additional_evidence": True,
                "recommendations": [
                    f"Receipts rejected by {len(rejected)} authorities",
                    "Address reviewer feedback and resubmit"
                ],
            }

        # Compute weighted confidence
        total_weight = 0.0
        weighted_confidence = 0.0

        for receipt in verified:
            authority_level = authority_levels.get(receipt.authority, "NARRATIVE")
            weight = AuthorityWeightAssignment.get_weight(authority_level)

            # Check minimum confidence threshold
            min_conf = AuthorityWeightAssignment.get_min_confidence(authority_level)
            if receipt.confidence < min_conf:
                # Authority confidence below threshold - counts as weak support
                weight *= (receipt.confidence / min_conf)

            total_weight += weight
            weighted_confidence += receipt.confidence * weight

        # Normalize confidence
        if total_weight > 0:
            combined_confidence = min(1.0, weighted_confidence / total_weight)
        else:
            combined_confidence = 0.0

        # Determine overall status
        status = AggregatedStatus.CLAIM_ALLOWED.value
        recommendations = []

        if len(verified) == 0:
            status = AggregatedStatus.CLAIM_PENDING.value
            recommendations.append("Awaiting verification receipts")
        elif combined_confidence < 0.80:
            status = AggregatedStatus.CLAIM_BLOCKED.value
            recommendations.append(f"Combined confidence {combined_confidence:.2f} below threshold 0.80")
        elif len(timeout) > 0:
            status = AggregatedStatus.CLAIM_PENDING.value
            recommendations.append(f"Waiting on {len(timeout)} authority/authorities (timeout)")

        return {
            "status": status,
            "combined_confidence": combined_confidence,
            "verified_count": len(verified),
            "rejected_count": len(rejected),
            "timeout_count": len(timeout),
            "requires_additional_evidence": status != AggregatedStatus.CLAIM_ALLOWED.value,
            "recommendations": recommendations,
        }

    def _compute_report_hash(self, report: AggregationReport) -> str:
        """Compute deterministic hash of report."""
        hashable = {
            "report_id": report.report_id,
            "local_repo": report.local_repo,
            "local_commit": report.local_commit,
            "claim_id": report.claim_id,
            "receipts": [
                {
                    "authority": r.authority,
                    "verdict": r.verdict,
                    "confidence": r.confidence,
                }
                for r in sorted(report.receipts, key=lambda x: x.authority)
            ],
            "aggregated_verdict": report.aggregated_verdict,
        }

        content = json.dumps(hashable, sort_keys=True)
        return hashlib.sha256(content.encode()).hexdigest()

    def verify_all_signatures(self) -> bool:
        """Verify signatures on all receipts."""
        for receipt in self.receipts:
            # In real implementation, would verify ED25519 signature
            # For now, just check that signature is not empty
            if not receipt.signature:
                return False

        return True

    def check_circular_dependencies(
        self,
        dependency_graph: Dict[str, List[str]]
    ) -> bool:
        """Detect circular dependencies in repository graph."""
        visited = set()
        rec_stack = set()

        def has_cycle(repo):
            visited.add(repo)
            rec_stack.add(repo)

            for neighbor in dependency_graph.get(repo, []):
                if neighbor not in visited:
                    if has_cycle(neighbor):
                        return True
                elif neighbor in rec_stack:
                    return True

            rec_stack.remove(repo)
            return False

        for repo in dependency_graph:
            if repo not in visited:
                if has_cycle(repo):
                    return True

        return False

    def check_freshness(self, max_age_days: int = 7) -> bool:
        """Check that all receipts are fresh (within max_age)."""
        from datetime import timezone
        cutoff = datetime.now(timezone.utc) - timedelta(days=max_age_days)

        for receipt in self.receipts:
            try:
                receipt_time = datetime.fromisoformat(
                    receipt.timestamp_utc.replace("Z", "+00:00")
                )
                if receipt_time < cutoff:
                    return False
            except (ValueError, AttributeError):
                # Invalid timestamp format
                return False

        return True

    def export_report(self, output_path: Path) -> Path:
        """Export aggregation report to JSON."""
        if not self.reports:
            raise ValueError("No aggregation reports to export")

        # Export the most recent report
        report = self.reports[-1]

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(json.dumps(report.to_dict(), indent=2))

        return output_path


def main():
    """CLI entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Aggregate federated claim receipts")
    parser.add_argument("--receipt", type=Path, action="append", help="Receipt JSON file")
    parser.add_argument("--claim", type=Path, help="Original claim JSON file")
    parser.add_argument("--output", type=Path, help="Output report JSON")
    parser.add_argument("--repo", default="rafaelmeloreisnovo/RafPolimata", help="Local repo")
    parser.add_argument("--commit", help="Local commit SHA")
    parser.add_argument("--freshness-days", type=int, default=7, help="Max receipt age in days")

    args = parser.parse_args()

    aggregator = FederatedAggregator(args.repo, args.commit or "unknown")

    # Load receipts
    if args.receipt:
        authority_levels = {}

        for receipt_path in args.receipt:
            if receipt_path.exists():
                receipt_data = json.loads(receipt_path.read_text())
                authority = receipt_data.get("authority", {}).get("repo", "unknown")
                authority_level = receipt_data.get("authority", {}).get("authority_level", "NARRATIVE")

                aggregator.add_receipt(authority, receipt_data)
                authority_levels[authority] = authority_level

    # Load original claim
    claim = {}
    if args.claim and args.claim.exists():
        claim = json.loads(args.claim.read_text())

    # Check freshness
    if not aggregator.check_freshness(args.freshness_days):
        print(f"ERROR: Receipts older than {args.freshness_days} days")
        return 1

    # Verify signatures
    if not aggregator.verify_all_signatures():
        print("ERROR: Invalid receipt signatures")
        return 1

    # Aggregate receipts
    submission_timestamp = datetime.utcnow().isoformat() + "Z"
    claim_id = claim.get("id", "unknown")

    report = aggregator.aggregate_receipts(
        claim_id=claim_id,
        original_claim=claim,
        submission_timestamp=submission_timestamp,
        authority_levels=authority_levels
    )

    # Export report
    if args.output:
        aggregator.export_report(args.output)
        print(f"Report exported to {args.output}")

    # Print summary
    print(f"Report ID: {report.report_id}")
    print(f"Status: {report.aggregated_verdict.get('status')}")
    print(f"Combined Confidence: {report.aggregated_verdict.get('combined_confidence'):.2f}")
    print(f"Verified: {report.aggregated_verdict.get('verified_count', 0)}")
    print(f"Rejected: {report.aggregated_verdict.get('rejected_count', 0)}")
    print(f"Timeout: {report.aggregated_verdict.get('timeout_count', 0)}")

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
