#!/usr/bin/env python3
"""
Phase 2: Federated Submitter

Submits verified claims to external repositories for cross-repo validation.
Handles claim packaging, signing, and submission tracking.

Protocol: RAFAELIA-PSC-1-FED (federated_claim_protocol.routing.v1)
"""

import json
import hashlib
from dataclasses import dataclass, asdict, field
from datetime import datetime, timedelta
from pathlib import Path
from typing import Any, Dict, List, Optional
from enum import Enum


class SubmissionStatus(str, Enum):
    """Status of a claim submission."""
    PENDING = "PENDING"
    SUBMITTED = "SUBMITTED"
    ACKNOWLEDGED = "ACKNOWLEDGED"
    PROCESSING = "PROCESSING"
    VERIFIED = "VERIFIED"
    REJECTED = "REJECTED"
    TIMEOUT = "TIMEOUT"


class AuthorityLevel(str, Enum):
    """Authority levels for verification."""
    FORMAL_PROOF = "FORMAL_PROOF"  # Matem-tica-, Rafaelia_Core
    PEER_REVIEW = "PEER_REVIEW"  # papers
    DOMAIN_EXPERT = "DOMAIN_EXPERT"  # ChipQuantum, Cosmos
    CROSS_REFERENCE = "CROSS_REFERENCE"  # Mapa
    NARRATIVE = "NARRATIVE"  # CientiEspiritual


# Trusted authorities for federated verification
TRUSTED_AUTHORITIES = {
    "rafaelmeloreisnovo/Rafaelia_Core": AuthorityLevel.FORMAL_PROOF,
    "rafaelmeloreisnovo/ZIPRAF_OMEGA_FULL": AuthorityLevel.FORMAL_PROOF,
    "rafaelmeloreisnovo/ChipQuantum": AuthorityLevel.DOMAIN_EXPERT,
    "rafaelmeloreisnovo/Matem-tica-": AuthorityLevel.FORMAL_PROOF,
    "rafaelmeloreisnovo/papers": AuthorityLevel.PEER_REVIEW,
    "rafaelmeloreisnovo/Mapa": AuthorityLevel.CROSS_REFERENCE,
    "rafaelmeloreisnovo/CientiEspiritual": AuthorityLevel.NARRATIVE,
    "rafaelmeloreisnovo/Cosmos": AuthorityLevel.DOMAIN_EXPERT,
    "instituto-Rafael/Eletron-efeitos-qu-ntico": AuthorityLevel.DOMAIN_EXPERT,
    "instituto-Rafael/relativity-living-light": AuthorityLevel.DOMAIN_EXPERT,
}


@dataclass
class ClaimSubmission:
    """Complete claim submission package."""
    schema: str = "rafaelia.federated_claim_protocol.routing.v1"
    claim_id: str = ""
    origin_repo: str = ""
    origin_commit: str = ""

    claim: Dict[str, Any] = field(default_factory=dict)
    verification: Dict[str, Any] = field(default_factory=dict)
    evidence: List[Dict[str, Any]] = field(default_factory=list)

    request_authorities: List[str] = field(default_factory=list)

    timestamp_utc: str = ""
    submission_hash: str = ""
    signature: str = ""  # ED25519 signature (placeholder)

    def compute_hash(self) -> str:
        """Compute deterministic submission hash."""
        hashable = {
            "claim_id": self.claim_id,
            "origin_repo": self.origin_repo,
            "origin_commit": self.origin_commit,
            "claim": self.claim,
            "verification": self.verification,
            "request_authorities": sorted(self.request_authorities),
        }
        content = json.dumps(hashable, sort_keys=True)
        return hashlib.sha256(content.encode()).hexdigest()

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "schema": self.schema,
            "claim_id": self.claim_id,
            "origin_repo": self.origin_repo,
            "origin_commit": self.origin_commit,
            "claim": self.claim,
            "verification": self.verification,
            "evidence": self.evidence,
            "request_authorities": self.request_authorities,
            "timestamp_utc": self.timestamp_utc,
            "submission_hash": self.submission_hash,
            "signature": self.signature,
        }


@dataclass
class SubmissionTracker:
    """Tracks submission status and receipts."""
    submission_id: str
    status: SubmissionStatus
    submitted_to: List[str] = field(default_factory=list)
    receipts_received: Dict[str, bool] = field(default_factory=dict)
    submitted_at: str = ""
    deadline: str = ""

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


class FederatedSubmitter:
    """Submits verified claims to external repositories."""

    def __init__(self, origin_repo: str, origin_commit: str):
        self.origin_repo = origin_repo
        self.origin_commit = origin_commit
        self.submissions: List[ClaimSubmission] = []
        self.trackers: Dict[str, SubmissionTracker] = {}

    def prepare_submission(
        self,
        claim: Dict[str, Any],
        verification: Dict[str, Any],
        evidence: List[Dict[str, Any]],
        request_authorities: List[str],
    ) -> ClaimSubmission:
        """Prepare a claim for submission."""
        claim_id = claim.get("id", "unknown")

        submission = ClaimSubmission(
            claim_id=claim_id,
            origin_repo=self.origin_repo,
            origin_commit=self.origin_commit,
            claim=claim,
            verification=verification,
            evidence=evidence,
            request_authorities=request_authorities,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )

        # Validate authorities
        submission.request_authorities = self._validate_authorities(request_authorities)

        # Compute hash and sign (placeholder)
        submission.submission_hash = submission.compute_hash()
        submission.signature = self._sign_submission(submission)

        self.submissions.append(submission)

        return submission

    def _validate_authorities(self, authorities: List[str]) -> List[str]:
        """Validate that all requested authorities are trusted."""
        validated = []
        for auth in authorities:
            if auth in TRUSTED_AUTHORITIES:
                validated.append(auth)
        return validated

    def _sign_submission(self, submission: ClaimSubmission) -> str:
        """Sign submission (placeholder ED25519)."""
        # In real implementation, would use cryptography library
        content = submission.submission_hash.encode()
        # Placeholder: return first 32 chars of double-hash
        sig = hashlib.sha256(content).hexdigest()
        return f"ED25519({sig[:32]})"

    def submit_claim(
        self,
        submission: ClaimSubmission,
        authorities: Optional[List[str]] = None
    ) -> SubmissionTracker:
        """Submit claim to authorities."""
        if authorities is None:
            authorities = submission.request_authorities

        # Validate submission gates
        if not self._validate_submission_gates(submission):
            raise ValueError("Submission failed gate validation")

        # Create tracker
        submission_id = f"SUB-{submission.claim_id}-{submission.timestamp_utc[:10]}"
        tracker = SubmissionTracker(
            submission_id=submission_id,
            status=SubmissionStatus.SUBMITTED,
            submitted_to=authorities,
            submitted_at=datetime.utcnow().isoformat() + "Z",
            deadline=(datetime.utcnow() + timedelta(hours=24)).isoformat() + "Z",
        )

        # Initialize receipt tracking
        for auth in authorities:
            tracker.receipts_received[auth] = False

        self.trackers[submission_id] = tracker

        return tracker

    def _validate_submission_gates(self, submission: ClaimSubmission) -> bool:
        """Validate that submission can be sent."""
        # Must have claim_gate = PASS
        if submission.verification.get("gate_status") != "PASS":
            return False

        # Must have authorities
        if not submission.request_authorities:
            return False

        # Must have claim_id
        if not submission.claim_id:
            return False

        # Claim must have all required fields
        required = ["domain", "source", "method", "artifact", "limitations", "falsifier"]
        for field in required:
            if field not in submission.claim or not submission.claim[field]:
                return False

        return True

    def record_receipt(
        self,
        submission_id: str,
        authority: str,
        receipt: Dict[str, Any]
    ) -> bool:
        """Record receipt from authority."""
        if submission_id not in self.trackers:
            return False

        tracker = self.trackers[submission_id]

        # Validate receipt signature (placeholder)
        if not self._verify_receipt_signature(receipt):
            return False

        # Mark receipt received
        if authority in tracker.receipts_received:
            tracker.receipts_received[authority] = True

        # Update status
        all_received = all(tracker.receipts_received.values())
        if all_received:
            tracker.status = SubmissionStatus.VERIFIED
        else:
            tracker.status = SubmissionStatus.PROCESSING

        return True

    def _verify_receipt_signature(self, receipt: Dict[str, Any]) -> bool:
        """Verify receipt signature (placeholder)."""
        # In real implementation, would verify ED25519 signature
        return receipt.get("signature", "") != ""

    def check_submission_status(self, submission_id: str) -> Optional[SubmissionTracker]:
        """Check status of a submission."""
        if submission_id in self.trackers:
            tracker = self.trackers[submission_id]

            # Check deadline
            deadline = datetime.fromisoformat(tracker.deadline.replace("Z", "+00:00"))
            if datetime.utcnow().replace(tzinfo=deadline.tzinfo) > deadline:
                if tracker.status == SubmissionStatus.PROCESSING:
                    tracker.status = SubmissionStatus.TIMEOUT

            return tracker

        return None

    def export_submissions(self, output_path: Path) -> Path:
        """Export all submissions to JSON."""
        submissions_data = {
            "schema": "rafaelia.federated_submissions.routing.v1",
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            "origin_repo": self.origin_repo,
            "origin_commit": self.origin_commit,
            "submissions": [s.to_dict() for s in self.submissions],
            "trackers": {k: v.to_dict() for k, v in self.trackers.items()},
            "summary": {
                "total_submissions": len(self.submissions),
                "pending": sum(1 for t in self.trackers.values()
                             if t.status == SubmissionStatus.PENDING),
                "submitted": sum(1 for t in self.trackers.values()
                               if t.status == SubmissionStatus.SUBMITTED),
                "verified": sum(1 for t in self.trackers.values()
                              if t.status == SubmissionStatus.VERIFIED),
                "rejected": sum(1 for t in self.trackers.values()
                              if t.status == SubmissionStatus.REJECTED),
                "timeout": sum(1 for t in self.trackers.values()
                             if t.status == SubmissionStatus.TIMEOUT),
            },
        }

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(json.dumps(submissions_data, indent=2))

        return output_path


def main():
    """CLI entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Submit claims to federated authorities")
    parser.add_argument("--claim", type=Path, help="Claim JSON file")
    parser.add_argument("--verification", type=Path, help="Verification report JSON")
    parser.add_argument("--authorities", nargs="+", help="Authority repos to submit to")
    parser.add_argument("--output", type=Path, help="Output submission JSON")
    parser.add_argument("--repo", default="rafaelmeloreisnovo/RafPolimata", help="Origin repo")
    parser.add_argument("--commit", help="Origin commit SHA")

    args = parser.parse_args()

    submitter = FederatedSubmitter(args.repo, args.commit or "unknown")

    if args.claim and args.claim.exists():
        claim = json.loads(args.claim.read_text())
        verification = {}

        if args.verification and args.verification.exists():
            verification = json.loads(args.verification.read_text())

        submission = submitter.prepare_submission(
            claim=claim,
            verification=verification,
            evidence=[],
            request_authorities=args.authorities or [],
        )

        tracker = submitter.submit_claim(submission)

        if args.output:
            submitter.export_submissions(args.output)
            print(f"Submission exported to {args.output}")

        print(f"Submission ID: {tracker.submission_id}")
        print(f"Status: {tracker.status.value}")
        print(f"Authorities: {', '.join(tracker.submitted_to)}")

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
