#!/usr/bin/env python3
"""
Phase 1: Heuristic Evaluator

Implements sophisticated evaluation of 7 heuristics with confidence scoring.
Supports claim evaluation across multiple knowledge domains.

Protocol: RAFAELIA-PSC-1
Heuristics: DIRECT, CAUSAL_REVERSE, TEMPORAL_ANTIDERIVATIVE, LOCAL_DERIVATIVE,
            COUNTERFACTUAL, ROLE_INVERSION, MULTISCALE
"""

from dataclasses import dataclass, asdict, field
from enum import Enum
from typing import Dict, List, Optional, Tuple
from datetime import datetime
from hashlib import sha256
import json


class Heuristic(str, Enum):
    """Seven heuristics for claim evaluation."""
    DIRECT = "DIRECT"
    CAUSAL_REVERSE = "CAUSAL_REVERSE"
    TEMPORAL_ANTIDERIVATIVE = "TEMPORAL_ANTIDERIVATIVE"
    LOCAL_DERIVATIVE = "LOCAL_DERIVATIVE"
    COUNTERFACTUAL = "COUNTERFACTUAL"
    ROLE_INVERSION = "ROLE_INVERSION"
    MULTISCALE = "MULTISCALE"


class Domain(str, Enum):
    """Knowledge domains for claim evaluation."""
    COMPILER = "COMPILER"
    PROTOCOL = "PROTOCOL"
    CRYPTOGRAPHY = "CRYPTOGRAPHY"
    FORMAL_METHODS = "FORMAL_METHODS"
    COGNITIVE = "COGNITIVE"
    PHYSICS = "PHYSICS"
    UNKNOWN = "UNKNOWN"


@dataclass
class EvidencePoint:
    """Single piece of evidence."""
    type: str  # observation, measurement, proof, reference
    value: str
    confidence: float  # 0.0-1.0
    source: str
    timestamp_utc: str = ""

    def to_dict(self) -> Dict:
        return asdict(self)


@dataclass
class HeuristicEvaluation:
    """Result of evaluating a single heuristic."""
    heuristic: Heuristic
    domain: Domain
    confidence: float  # 0.0-1.0
    reasoning: str
    evidence_chain: List[EvidencePoint] = field(default_factory=list)
    score: float = 0.0  # Computed confidence with evidence weighting
    timestamp_utc: str = ""

    def to_dict(self) -> Dict:
        return {
            "heuristic": self.heuristic.value,
            "domain": self.domain.value,
            "confidence": self.confidence,
            "reasoning": self.reasoning,
            "evidence_chain": [e.to_dict() for e in self.evidence_chain],
            "score": self.score,
            "timestamp_utc": self.timestamp_utc,
        }


class DirectHeuristic:
    """H1: Direct observation/proof."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate claim using direct evidence."""
        artifact = claim.get("artifact", "")
        method = claim.get("method", "")

        # Direct heuristic requires concrete evidence
        direct_evidence = [e for e in evidence if e.type in ["observation", "proof"]]

        base_confidence = 0.9 if direct_evidence else 0.0
        evidence_boost = len(direct_evidence) * 0.05  # +5% per evidence point

        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Direct evidence: {len(direct_evidence)} proof(s), artifact={artifact!r}, "
            f"method={method!r}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.DIRECT,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=direct_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class CausalReverseHeuristic:
    """H2: Reverse causal inference."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate claim using reverse causal reasoning."""
        source = claim.get("source", "")
        limitations = claim.get("limitations", "")
        falsifier = claim.get("falsifier", "")

        # Causal reasoning supported by limitations and falsification conditions
        causal_evidence = [e for e in evidence if e.type == "reference"]

        has_limitations = limitations != ""
        has_falsifier = falsifier != ""

        base_confidence = 0.0
        if has_limitations and has_falsifier:
            base_confidence = 0.75
        elif has_limitations or has_falsifier:
            base_confidence = 0.5
        else:
            base_confidence = 0.2

        evidence_boost = min(0.2, len(causal_evidence) * 0.03)
        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Reverse causal: source={source!r}, limitations={has_limitations}, "
            f"falsifier={has_falsifier}, evidence={len(causal_evidence)}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.CAUSAL_REVERSE,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=causal_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class TemporalAntiderivativeHeuristic:
    """H3: Temporal antiderivative (time-backward reasoning)."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate using temporal backward reasoning."""
        domain = claim.get("domain", "")
        source_status = claim.get("source_status", "")

        # Temporal reasoning requires domain and source context
        temporal_evidence = [e for e in evidence if "time" in e.type.lower() or "historical" in e.type]

        has_domain = domain != ""
        has_status = source_status != ""

        base_confidence = 0.0
        if has_domain and has_status:
            base_confidence = 0.6
        elif has_domain or has_status:
            base_confidence = 0.3
        else:
            base_confidence = 0.1

        evidence_boost = min(0.25, len(temporal_evidence) * 0.05)
        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Temporal antiderivative: domain={domain!r}, "
            f"status={source_status!r}, temporal_evidence={len(temporal_evidence)}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.TEMPORAL_ANTIDERIVATIVE,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=temporal_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class LocalDerivativeHeuristic:
    """H4: Local scope derivation."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate using local scope derivation."""
        domain = claim.get("domain", "")
        method = claim.get("method", "")

        # Local derivation needs clear domain and method
        local_evidence = [e for e in evidence if e.type == "measurement"]

        has_domain = domain != ""
        has_method = method != ""

        base_confidence = 0.0
        if has_domain and has_method:
            base_confidence = 0.8
        elif has_domain or has_method:
            base_confidence = 0.5
        else:
            base_confidence = 0.0

        evidence_boost = min(0.15, len(local_evidence) * 0.04)
        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Local derivative: domain={domain!r}, method={method!r}, "
            f"measurements={len(local_evidence)}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.LOCAL_DERIVATIVE,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=local_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class CounterfactualHeuristic:
    """H5: Counterfactual thinking."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate using counterfactual conditions."""
        falsifier = claim.get("falsifier", "")
        limitations = claim.get("limitations", "")

        # Counterfactual reasoning supported by falsifiers
        falsifier_evidence = [e for e in evidence if e.type == "proof"]

        has_falsifier = falsifier != ""
        has_limitations = limitations != ""

        base_confidence = 0.0
        if has_falsifier and has_limitations:
            base_confidence = 0.85
        elif has_falsifier:
            base_confidence = 0.7
        elif has_limitations:
            base_confidence = 0.4
        else:
            base_confidence = 0.0

        evidence_boost = min(0.1, len(falsifier_evidence) * 0.03)
        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Counterfactual: falsifier={falsifier!r}, "
            f"limitations={limitations!r}, proofs={len(falsifier_evidence)}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.COUNTERFACTUAL,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=falsifier_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class RoleInversionHeuristic:
    """H6: Role/perspective swap."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate using role inversion."""
        # Role inversion: claim viewed from different stakeholder perspective
        # Simplified: check evidence from multiple sources

        unique_sources = {e.source for e in evidence}

        base_confidence = 0.0
        if len(unique_sources) >= 3:
            base_confidence = 0.7
        elif len(unique_sources) >= 2:
            base_confidence = 0.5
        else:
            base_confidence = 0.2

        reasoning = f"Role inversion: {len(unique_sources)} independent source(s)"

        return HeuristicEvaluation(
            heuristic=Heuristic.ROLE_INVERSION,
            domain=Domain.UNKNOWN,
            confidence=base_confidence,
            reasoning=reasoning,
            evidence_chain=evidence[:5],  # Sample of evidence
            score=base_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class MultiscaleHeuristic:
    """H7: Cross-scale reasoning."""

    @staticmethod
    def evaluate(claim: Dict, evidence: List[EvidencePoint]) -> HeuristicEvaluation:
        """Evaluate using multiscale reasoning."""
        domain = claim.get("domain", "")
        method = claim.get("method", "")
        artifact = claim.get("artifact", "")

        # Multiscale reasoning: local measurements scaling to system level
        measurement_evidence = [e for e in evidence if "measurement" in e.type or "observation" in e.type]

        has_full_context = domain != "" and method != "" and artifact != ""

        base_confidence = 0.0
        if has_full_context:
            base_confidence = 0.75
        elif domain != "" and method != "":
            base_confidence = 0.55
        else:
            base_confidence = 0.2

        evidence_boost = min(0.2, len(measurement_evidence) * 0.04)
        final_confidence = min(1.0, base_confidence + evidence_boost)

        reasoning = (
            f"Multiscale: domain={domain!r}, method={method!r}, "
            f"artifact={artifact!r}, scales={len(measurement_evidence)}"
        )

        return HeuristicEvaluation(
            heuristic=Heuristic.MULTISCALE,
            domain=Domain.UNKNOWN,
            confidence=final_confidence,
            reasoning=reasoning,
            evidence_chain=measurement_evidence,
            score=final_confidence,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )


class HeuristicEvaluator:
    """Orchestrates evaluation of all 7 heuristics."""

    HEURISTICS = {
        Heuristic.DIRECT: DirectHeuristic,
        Heuristic.CAUSAL_REVERSE: CausalReverseHeuristic,
        Heuristic.TEMPORAL_ANTIDERIVATIVE: TemporalAntiderivativeHeuristic,
        Heuristic.LOCAL_DERIVATIVE: LocalDerivativeHeuristic,
        Heuristic.COUNTERFACTUAL: CounterfactualHeuristic,
        Heuristic.ROLE_INVERSION: RoleInversionHeuristic,
        Heuristic.MULTISCALE: MultiscaleHeuristic,
    }

    def __init__(self):
        self.evaluations: List[HeuristicEvaluation] = []

    def evaluate_claim(
        self,
        claim: Dict,
        evidence: Optional[List[EvidencePoint]] = None
    ) -> List[HeuristicEvaluation]:
        """Evaluate claim against all 7 heuristics."""
        if evidence is None:
            evidence = []

        results = []

        for heuristic, evaluator_class in self.HEURISTICS.items():
            result = evaluator_class.evaluate(claim, evidence)
            results.append(result)
            self.evaluations.append(result)

        return results

    def compute_overall_score(self, evaluations: List[HeuristicEvaluation]) -> float:
        """Compute weighted overall score from all heuristics."""
        if not evaluations:
            return 0.0

        # Weight heuristics equally (can be customized)
        scores = [e.score for e in evaluations]
        return sum(scores) / len(scores)

    def export_results(self, output_path) -> str:
        """Export evaluation results to JSON."""
        report = {
            "schema": "rafaelia.heuristic_evaluator.v1",
            "timestamp_utc": datetime.utcnow().isoformat() + "Z",
            "evaluations": [e.to_dict() for e in self.evaluations],
            "summary": {
                "total_evaluations": len(self.evaluations),
                "average_confidence": sum(e.confidence for e in self.evaluations) / len(self.evaluations)
                    if self.evaluations else 0.0,
                "heuristic_distribution": {
                    h.value: len([e for e in self.evaluations if e.heuristic == h])
                    for h in Heuristic
                },
            },
        }

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(json.dumps(report, indent=2))
        return str(output_path)
