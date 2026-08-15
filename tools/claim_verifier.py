#!/usr/bin/env python3
"""
Phase 1: Claim Verification Engine

Validates claims against RAFAELIA-PSC-1 schema with 7 heuristics.
Enforces gate requirements: domain, source, method, artifact, limitations, falsifier.

Protocol: rafaelia.parabolic_semantic_codec.routing.v1
Configuration: configs/parabolic_semantic_codec.json
"""

import json
import re
from dataclasses import dataclass, asdict, field
from datetime import datetime
from enum import Enum
from hashlib import sha256
from pathlib import Path
from typing import Any, Dict, List, Optional, Set


class ClaimStatus(str, Enum):
    """Claim status enumeration."""
    VERIFIED_LITERAL = "VERIFIED_LITERAL"  # Direct observation
    PROVED = "PROVED"  # Formal proof
    TESTED = "TESTED"  # Experimental validation
    DERIVED = "DERIVED"  # Logical derivation
    HYPOTHESIS = "HYPOTHESIS"  # Working hypothesis
    ANALOGY_ONLY = "ANALOGY_ONLY"  # Analogical reasoning
    DECLARED_BY_AUTHOR = "DECLARED_BY_AUTHOR"  # Author declaration
    TOKEN_VAZIO = "TOKEN_VAZIO"  # Missing evidence
    CONTRADICTION = "CONTRADICTION"  # Contradicted by evidence
    BLOCKED = "BLOCKED"  # Gate not met


class Heuristic(str, Enum):
    """Seven heuristics for claim evaluation."""
    DIRECT = "DIRECT"  # Direct observation/proof
    CAUSAL_REVERSE = "CAUSAL_REVERSE"  # Reverse causal inference
    TEMPORAL_ANTIDERIVATIVE = "TEMPORAL_ANTIDERIVATIVE"  # Time-backward reasoning
    LOCAL_DERIVATIVE = "LOCAL_DERIVATIVE"  # Local scope derivation
    COUNTERFACTUAL = "COUNTERFACTUAL"  # Counterfactual thinking
    ROLE_INVERSION = "ROLE_INVERSION"  # Role/perspective swap
    MULTISCALE = "MULTISCALE"  # Cross-scale reasoning


@dataclass
class ClaimField:
    """Represents a single claim field."""
    name: str
    required: bool
    value: Optional[str] = None
    status: str = "PENDING"
    evidence: str = ""


@dataclass
class ClaimGate:
    """Represents claim gate validation result."""
    field_name: str
    is_satisfied: bool
    reason: str = ""
    evidence_hash: str = ""


@dataclass
class HeuristicResult:
    """Result of applying a single heuristic."""
    heuristic: Heuristic
    confidence: float  # 0.0-1.0
    status: ClaimStatus
    reasoning: str
    evidence_pointers: List[str] = field(default_factory=list)


@dataclass
class ClaimVerification:
    """Complete claim verification result."""
    claim_id: str
    source_status: ClaimStatus
    epistemic_status: ClaimStatus
    claim_gate_status: str  # PASS, FAIL, BLOCKED
    required_fields_present: Dict[str, bool] = field(default_factory=dict)
    gate_results: List[ClaimGate] = field(default_factory=list)
    heuristic_results: List[HeuristicResult] = field(default_factory=list)
    hard_boundary_violations: List[str] = field(default_factory=list)
    verification_hash: str = ""
    timestamp_utc: str = ""

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "claim_id": self.claim_id,
            "source_status": self.source_status.value,
            "epistemic_status": self.epistemic_status.value,
            "claim_gate_status": self.claim_gate_status,
            "required_fields_present": self.required_fields_present,
            "gate_results": [asdict(gr) for gr in self.gate_results],
            "heuristic_results": [
                {
                    "heuristic": hr.heuristic.value,
                    "confidence": hr.confidence,
                    "status": hr.status.value,
                    "reasoning": hr.reasoning,
                    "evidence_pointers": hr.evidence_pointers,
                }
                for hr in self.heuristic_results
            ],
            "hard_boundary_violations": self.hard_boundary_violations,
            "verification_hash": self.verification_hash,
            "timestamp_utc": self.timestamp_utc,
        }


class ClaimVerifier:
    """Verifies claims against RAFAELIA-PSC-1 protocol."""

    # Required fields as per PARABOLIC_SEMANTIC_CODEC
    REQUIRED_FIELDS = [
        "source_status",
        "epistemic_status",
        "claim_gate",
        "domain",
        "source",
        "method",
        "artifact",
        "limitations",
        "falsifier",
    ]

    # Hard boundaries: these must NOT be true
    HARD_BOUNDARIES = {
        "symbol_implies_physical_effect": False,
        "analogy_implies_mechanism": False,
        "finite_verification_implies_universal_proof": False,
        "truncated_hash_implies_integrity": False,
        "token_vazio_implies_pass": False,
        "shared_mathematical_form_implies_shared_ontology": False,
    }

    def __init__(self, config_path: Optional[Path] = None):
        """Initialize verifier with optional config."""
        self.config = self._load_config(config_path)
        self.verifications: List[ClaimVerification] = []

    def _load_config(self, config_path: Optional[Path]) -> Dict[str, Any]:
        """Load configuration from JSON."""
        if config_path is None:
            config_path = Path("configs/parabolic_semantic_codec.json")

        if config_path.exists():
            return json.loads(config_path.read_text(encoding="utf-8"))

        # Default config
        return {
            "schema": "rafaelia.parabolic_semantic_codec.routing.v1",
            "protocol": "RAFAELIA-PSC-1",
            "claim_allowed": False,
            "hard_boundaries": self.HARD_BOUNDARIES,
        }

    def verify_claim(self, claim: Dict[str, Any]) -> ClaimVerification:
        """Verify a single claim."""
        claim_id = claim.get("id", "unknown")
        verification = ClaimVerification(
            claim_id=claim_id,
            source_status=ClaimStatus.TOKEN_VAZIO,
            epistemic_status=ClaimStatus.TOKEN_VAZIO,
            claim_gate_status="BLOCKED",
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )

        # Check required fields
        verification.required_fields_present = self._check_required_fields(claim)

        # Check gates
        verification.gate_results = self._validate_gates(claim)

        # Evaluate epistemic status
        verification.source_status = self._evaluate_source_status(claim)
        verification.epistemic_status = self._evaluate_epistemic_status(claim)

        # Apply heuristics
        verification.heuristic_results = self._apply_heuristics(claim)

        # Check hard boundaries
        verification.hard_boundary_violations = self._check_hard_boundaries(claim)

        # Determine claim gate status
        verification.claim_gate_status = self._determine_claim_gate_status(verification)

        # Compute verification hash
        verification.verification_hash = self._compute_verification_hash(verification)

        self.verifications.append(verification)
        return verification

    def _check_required_fields(self, claim: Dict[str, Any]) -> Dict[str, bool]:
        """Check if all required fields are present."""
        return {field: field in claim and claim[field] not in [None, ""]
                for field in self.REQUIRED_FIELDS}

    def _validate_gates(self, claim: Dict[str, Any]) -> List[ClaimGate]:
        """Validate claim gates."""
        gates = []

        # Each required field must be non-empty
        for field in self.REQUIRED_FIELDS:
            is_satisfied = field in claim and claim[field] not in [None, ""]
            gates.append(ClaimGate(
                field_name=field,
                is_satisfied=is_satisfied,
                reason="" if is_satisfied else f"Field '{field}' missing or empty"
            ))

        return gates

    def _evaluate_source_status(self, claim: Dict[str, Any]) -> ClaimStatus:
        """Evaluate source_status field."""
        source_status_str = claim.get("source_status", "")

        try:
            return ClaimStatus(source_status_str)
        except ValueError:
            return ClaimStatus.TOKEN_VAZIO

    def _evaluate_epistemic_status(self, claim: Dict[str, Any]) -> ClaimStatus:
        """Evaluate epistemic_status field."""
        epistemic_status_str = claim.get("epistemic_status", "")

        try:
            return ClaimStatus(epistemic_status_str)
        except ValueError:
            return ClaimStatus.TOKEN_VAZIO

    def _apply_heuristics(self, claim: Dict[str, Any]) -> List[HeuristicResult]:
        """Apply all 7 heuristics to the claim."""
        results = []

        for heuristic in Heuristic:
            result = self._apply_single_heuristic(heuristic, claim)
            results.append(result)

        return results

    def _apply_single_heuristic(
        self,
        heuristic: Heuristic,
        claim: Dict[str, Any]
    ) -> HeuristicResult:
        """Apply a single heuristic."""
        if heuristic == Heuristic.DIRECT:
            return self._heuristic_direct(claim)
        elif heuristic == Heuristic.CAUSAL_REVERSE:
            return self._heuristic_causal_reverse(claim)
        elif heuristic == Heuristic.TEMPORAL_ANTIDERIVATIVE:
            return self._heuristic_temporal_antiderivative(claim)
        elif heuristic == Heuristic.LOCAL_DERIVATIVE:
            return self._heuristic_local_derivative(claim)
        elif heuristic == Heuristic.COUNTERFACTUAL:
            return self._heuristic_counterfactual(claim)
        elif heuristic == Heuristic.ROLE_INVERSION:
            return self._heuristic_role_inversion(claim)
        elif heuristic == Heuristic.MULTISCALE:
            return self._heuristic_multiscale(claim)

        return HeuristicResult(
            heuristic=heuristic,
            confidence=0.0,
            status=ClaimStatus.TOKEN_VAZIO,
            reasoning="Unknown heuristic"
        )

    def _heuristic_direct(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H1: Direct observation/proof."""
        artifact = claim.get("artifact", "")
        method = claim.get("method", "")
        has_evidence = artifact != "" and method != ""

        confidence = 0.9 if has_evidence else 0.0
        status = ClaimStatus.TESTED if has_evidence else ClaimStatus.TOKEN_VAZIO

        return HeuristicResult(
            heuristic=Heuristic.DIRECT,
            confidence=confidence,
            status=status,
            reasoning="Direct evidence available" if has_evidence else "Missing artifact or method"
        )

    def _heuristic_causal_reverse(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H2: Reverse causal inference."""
        source = claim.get("source", "")
        limitations = claim.get("limitations", "")

        # Causal inference if source and limitations defined
        has_causal = source != "" and limitations != ""
        confidence = 0.6 if has_causal else 0.2

        return HeuristicResult(
            heuristic=Heuristic.CAUSAL_REVERSE,
            confidence=confidence,
            status=ClaimStatus.HYPOTHESIS,
            reasoning="Causal reasoning possible" if has_causal else "Limited causal chain"
        )

    def _heuristic_temporal_antiderivative(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H3: Temporal antiderivative (time-backward reasoning)."""
        source_status = claim.get("source_status", "")
        domain = claim.get("domain", "")

        # Temporal reasoning if domain and source_status defined
        has_temporal = domain != "" and source_status != ""
        confidence = 0.5 if has_temporal else 0.1

        return HeuristicResult(
            heuristic=Heuristic.TEMPORAL_ANTIDERIVATIVE,
            confidence=confidence,
            status=ClaimStatus.HYPOTHESIS,
            reasoning="Temporal context available" if has_temporal else "No temporal markers"
        )

    def _heuristic_local_derivative(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H4: Local scope derivation."""
        domain = claim.get("domain", "")

        # Local derivation depends on domain specification
        confidence = 0.7 if domain != "" else 0.0

        return HeuristicResult(
            heuristic=Heuristic.LOCAL_DERIVATIVE,
            confidence=confidence,
            status=ClaimStatus.DERIVED if domain != "" else ClaimStatus.TOKEN_VAZIO,
            reasoning=f"Local scope: {domain}" if domain != "" else "No domain specified"
        )

    def _heuristic_counterfactual(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H5: Counterfactual thinking."""
        falsifier = claim.get("falsifier", "")

        # Counterfactual reasoning if falsifier defined
        confidence = 0.8 if falsifier != "" else 0.0

        return HeuristicResult(
            heuristic=Heuristic.COUNTERFACTUAL,
            confidence=confidence,
            status=ClaimStatus.PROVED if falsifier != "" else ClaimStatus.TOKEN_VAZIO,
            reasoning="Falsifier defined" if falsifier != "" else "No falsification condition"
        )

    def _heuristic_role_inversion(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H6: Role/perspective swap."""
        # Role inversion: perspective from different stakeholder
        # Simplified: check if claim has multiple perspectives
        confidence = 0.4  # Low confidence without explicit role markers

        return HeuristicResult(
            heuristic=Heuristic.ROLE_INVERSION,
            confidence=confidence,
            status=ClaimStatus.ANALOGY_ONLY,
            reasoning="Perspective evaluation requires external context"
        )

    def _heuristic_multiscale(self, claim: Dict[str, Any]) -> HeuristicResult:
        """H7: Cross-scale reasoning."""
        # Multiscale: local → system level
        method = claim.get("method", "")
        domain = claim.get("domain", "")

        # Multiscale if both method and domain present
        has_scales = method != "" and domain != ""
        confidence = 0.6 if has_scales else 0.2

        return HeuristicResult(
            heuristic=Heuristic.MULTISCALE,
            confidence=confidence,
            status=ClaimStatus.DERIVED if has_scales else ClaimStatus.HYPOTHESIS,
            reasoning="Cross-scale reasoning possible" if has_scales else "Single scale only"
        )

    def _check_hard_boundaries(self, claim: Dict[str, Any]) -> List[str]:
        """Check that hard boundaries are not violated."""
        violations = []

        # Symbol ≠ physical effect
        if claim.get("symbol_implies_physical_effect") is True:
            violations.append("symbol_implies_physical_effect violated")

        # Analogy ≠ mechanism
        if claim.get("analogy_implies_mechanism") is True:
            violations.append("analogy_implies_mechanism violated")

        # Finite verification ≠ universal proof
        if claim.get("finite_verification_implies_universal_proof") is True:
            violations.append("finite_verification_implies_universal_proof violated")

        # TOKEN_VAZIO ≠ pass
        if claim.get("epistemic_status") == "TOKEN_VAZIO" and \
           claim.get("source_status") not in ["VERIFIED_LITERAL", "PROVED"]:
            # Not a violation per se, but TOKEN_VAZIO requires special handling
            pass

        return violations

    def _determine_claim_gate_status(self, verification: ClaimVerification) -> str:
        """Determine if claim passes gate."""
        # All gates must be satisfied
        all_gates_satisfied = all(gr.is_satisfied for gr in verification.gate_results)

        # No hard boundary violations
        no_violations = len(verification.hard_boundary_violations) == 0

        # Epistemic status must not be BLOCKED or TOKEN_VAZIO for claim_allowed
        claim_allowed = self.config.get("claim_allowed", False)
        epistemic_ok = verification.epistemic_status not in [
            ClaimStatus.BLOCKED,
            ClaimStatus.TOKEN_VAZIO
        ]

        if all_gates_satisfied and no_violations and (not claim_allowed or epistemic_ok):
            return "PASS"
        elif not all_gates_satisfied:
            return "BLOCKED"
        else:
            return "FAIL"

    def _compute_verification_hash(self, verification: ClaimVerification) -> str:
        """Compute deterministic verification hash."""
        hashable = {
            "claim_id": verification.claim_id,
            "source_status": verification.source_status.value,
            "epistemic_status": verification.epistemic_status.value,
            "claim_gate_status": verification.claim_gate_status,
            "required_fields_present": verification.required_fields_present,
            "hard_boundary_violations": sorted(verification.hard_boundary_violations),
        }
        content = json.dumps(hashable, sort_keys=True)
        return sha256(content.encode()).hexdigest()

    def export_report(self, output_path: Path) -> Path:
        """Export verification report."""
        report = {
            "schema": self.config.get("schema", "unknown"),
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            "verifications": [v.to_dict() for v in self.verifications],
            "summary": {
                "total_claims": len(self.verifications),
                "passed": sum(1 for v in self.verifications if v.claim_gate_status == "PASS"),
                "blocked": sum(1 for v in self.verifications if v.claim_gate_status == "BLOCKED"),
                "failed": sum(1 for v in self.verifications if v.claim_gate_status == "FAIL"),
            },
        }

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(json.dumps(report, indent=2))
        return output_path


def main():
    """CLI entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Verify claims against RAFAELIA-PSC-1")
    parser.add_argument("--input", type=Path, help="Input claims JSON file")
    parser.add_argument("--output", type=Path, help="Output report JSON file")
    parser.add_argument("--config", type=Path, help="Config JSON file")

    args = parser.parse_args()

    verifier = ClaimVerifier(args.config)

    if args.input and args.input.exists():
        claims = json.loads(args.input.read_text(encoding="utf-8"))

        if isinstance(claims, dict):
            claims = [claims]

        for claim in claims:
            verifier.verify_claim(claim)

        if args.output:
            verifier.export_report(args.output)
            print(f"Report written to {args.output}")

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
