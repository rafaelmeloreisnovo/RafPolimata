#!/usr/bin/env python3
"""
Phase 5: Parable Compiler

Translates narrative/parable proofs into formal claims.
Converts metaphorical reasoning into structured verification.

Protocol: RAFAELIA-PSC-1 Narrative Integration
Authority Level: CientiEspiritual (NARRATIVE)
"""

import json
import hashlib
import re
from dataclasses import dataclass, asdict, field
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple
from enum import Enum


class ParableType(str, Enum):
    """Type of parable/narrative proof."""
    ANALOGY = "ANALOGY"
    METAPHOR = "METAPHOR"
    NARRATIVE = "NARRATIVE"
    ALLEGORY = "ALLEGORY"
    PARABLE = "PARABLE"


class CompilationStatus(str, Enum):
    """Status of parable compilation."""
    PARSED = "PARSED"
    EXTRACTED = "EXTRACTED"
    MAPPED = "MAPPED"
    COMPILED = "COMPILED"
    VERIFIED = "VERIFIED"
    FAILED = "FAILED"


@dataclass
class ParableElement:
    """Single element extracted from parable."""
    category: str  # "protagonist", "setting", "conflict", "resolution", "moral"
    content: str
    confidence: float  # 0.0-1.0
    symbolic_mapping: Optional[str] = None  # Maps to formal claim element

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "category": self.category,
            "content": self.content,
            "confidence": self.confidence,
            "symbolic_mapping": self.symbolic_mapping,
        }


@dataclass
class CompiledClaim:
    """Claim compiled from parable."""
    schema: str = "rafaelia.parable_compiled_claim.routing.v1"
    claim_id: str = ""
    parable_id: str = ""
    parable_type: ParableType = ParableType.NARRATIVE

    # Original parable
    parable_text: str = ""

    # Extracted elements
    elements: List[ParableElement] = field(default_factory=list)

    # Formal claim
    formal_claim: Dict[str, Any] = field(default_factory=dict)

    # Compilation metadata
    status: CompilationStatus = CompilationStatus.PARSED
    confidence_score: float = 0.0
    interpretation: str = ""
    caveats: List[str] = field(default_factory=list)

    timestamp_utc: str = ""
    claim_hash: str = ""

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "schema": self.schema,
            "claim_id": self.claim_id,
            "parable_id": self.parable_id,
            "parable_type": self.parable_type.value,
            "parable_text": self.parable_text,
            "elements": [e.to_dict() for e in self.elements],
            "formal_claim": self.formal_claim,
            "status": self.status.value,
            "confidence_score": self.confidence_score,
            "interpretation": self.interpretation,
            "caveats": self.caveats,
            "timestamp_utc": self.timestamp_utc,
            "claim_hash": self.claim_hash,
        }


class ParableCompiler:
    """Compiles parables into formal claims."""

    # Keyword mappings for element extraction
    ELEMENT_KEYWORDS = {
        "protagonist": [
            "hero", "protagonist", "character", "person", "who", "someone",
            "the main", "central figure", "agent"
        ],
        "setting": [
            "place", "setting", "location", "where", "land", "world", "realm",
            "kingdom", "city", "forest", "mountain", "sea"
        ],
        "conflict": [
            "challenge", "problem", "conflict", "obstacle", "struggle", "test",
            "trial", "adversity", "danger", "threat", "opposition"
        ],
        "resolution": [
            "solution", "resolution", "overcome", "triumph", "victory", "success",
            "achieved", "accomplished", "discovered", "learned", "solved"
        ],
        "moral": [
            "moral", "lesson", "teaching", "principle", "virtue", "wisdom",
            "truth", "insight", "enlightenment", "understand", "means"
        ],
    }

    # Symbolic mappings (parable concepts to formal claim elements)
    SYMBOLIC_MAPPINGS = {
        "journey": "method",
        "obstacle": "limitation",
        "guide": "source",
        "test": "verification",
        "transformation": "proof",
        "wisdom": "artifact",
        "darkness": "problem",
        "light": "solution",
    }

    def __init__(self):
        self.compiled_claims: List[CompiledClaim] = []

    def compile_parable(
        self,
        parable_text: str,
        parable_type: ParableType = ParableType.NARRATIVE,
        parable_id: Optional[str] = None
    ) -> CompiledClaim:
        """Compile parable into formal claim."""
        claim = CompiledClaim(
            parable_id=parable_id or f"PARABLE-{datetime.utcnow().isoformat()[:10]}",
            parable_type=parable_type,
            parable_text=parable_text,
            status=CompilationStatus.PARSED,
            timestamp_utc=datetime.utcnow().isoformat() + "Z",
        )

        # Extract narrative elements
        elements = self._extract_elements(parable_text)
        claim.elements = elements
        claim.status = CompilationStatus.EXTRACTED

        # Map symbols to formal elements
        self._map_symbols(claim)
        claim.status = CompilationStatus.MAPPED

        # Compile formal claim
        self._compile_formal_claim(claim)
        claim.status = CompilationStatus.COMPILED

        # Compute confidence and hash
        claim.confidence_score = self._compute_confidence(claim)
        claim.claim_id = f"CLAIM-{claim.parable_id}-PARABLE"
        claim.claim_hash = self._compute_hash(claim)

        self.compiled_claims.append(claim)

        return claim

    def _extract_elements(self, text: str) -> List[ParableElement]:
        """Extract narrative elements from parable text."""
        elements = []
        text_lower = text.lower()

        # Extract by sentence and keyword matching
        sentences = re.split(r'[.!?]+', text)

        for category, keywords in self.ELEMENT_KEYWORDS.items():
            for sentence in sentences:
                sentence_lower = sentence.lower()
                for keyword in keywords:
                    if keyword in sentence_lower:
                        # Found matching sentence
                        confidence = 0.7 + (0.3 * (len(keyword) / 20))
                        element = ParableElement(
                            category=category,
                            content=sentence.strip()[:200],  # Truncate long sentences
                            confidence=min(1.0, confidence),
                        )
                        elements.append(element)
                        break  # One element per sentence

        # Deduplicate by (category, content) pair
        unique_elements = []
        seen_pairs = set()
        for elem in elements:
            pair = (elem.category, elem.content)
            if pair not in seen_pairs:
                unique_elements.append(elem)
                seen_pairs.add(pair)

        return unique_elements

    def _map_symbols(self, claim: CompiledClaim) -> None:
        """Map symbolic elements to formal claim structure."""
        for element in claim.elements:
            # Look for symbolic words in content
            for symbol, formal_elem in self.SYMBOLIC_MAPPINGS.items():
                if symbol in element.content.lower():
                    element.symbolic_mapping = formal_elem
                    break

    def _compile_formal_claim(self, claim: CompiledClaim) -> None:
        """Compile extracted elements into formal claim."""
        # Build formal claim from elements
        formal_claim = {
            "id": claim.claim_id,
            "source_status": "NARRATIVE",
            "epistemic_status": "ANALOGY_ONLY",  # Narratives are analogies
            "claim_gate": "PASS" if self._validate_claim(claim) else "FAIL",
            "domain": self._infer_domain(claim),
            "source": f"parable:{claim.parable_id}",
            "method": "narrative-reasoning",
            "artifact": "narrative-proof",
            "limitations": self._infer_limitations(claim),
            "falsifier": self._infer_falsifier(claim),
        }

        claim.formal_claim = formal_claim
        claim.interpretation = self._generate_interpretation(claim)
        claim.caveats = self._generate_caveats(claim)

    def _validate_claim(self, claim: CompiledClaim) -> bool:
        """Validate that narrative has sufficient structure."""
        # Must have at least 3 different element types
        element_types = set(e.category for e in claim.elements)
        return len(element_types) >= 3

    def _infer_domain(self, claim: CompiledClaim) -> str:
        """Infer claim domain from parable content."""
        text_lower = claim.parable_text.lower()

        domains = {
            "PHILOSOPHY": ["wisdom", "virtue", "truth", "knowledge", "being"],
            "ETHICS": ["good", "bad", "moral", "right", "wrong", "justice"],
            "SPIRITUALITY": ["sacred", "divine", "spirit", "soul", "transcend"],
            "HUMAN_NATURE": ["character", "heart", "mind", "desire", "passion"],
            "SOCIETY": ["community", "society", "people", "tradition", "culture"],
        }

        for domain, keywords in domains.items():
            for keyword in keywords:
                if keyword in text_lower:
                    return domain

        return "PHILOSOPHY"  # Default domain

    def _infer_limitations(self, claim: CompiledClaim) -> str:
        """Infer limitations of narrative reasoning."""
        limitations = [
            "Narrative reasoning is analogical, not deductive",
            "Symbolic mappings are interpretive",
        ]

        # Add specific limitations
        if claim.parable_type == ParableType.ANALOGY:
            limitations.append("Analogy may not capture all nuances")
        elif claim.parable_type == ParableType.METAPHOR:
            limitations.append("Metaphorical language is inherently ambiguous")

        return "; ".join(limitations)

    def _infer_falsifier(self, claim: CompiledClaim) -> str:
        """Infer falsification conditions."""
        return (
            "Narrative interpretation contradicted by empirical evidence or "
            "formal logical proof"
        )

    def _generate_interpretation(self, claim: CompiledClaim) -> str:
        """Generate human-readable interpretation."""
        if not claim.elements:
            return "Unable to extract narrative elements"

        # Build narrative summary
        protagonist = next(
            (e for e in claim.elements if e.category == "protagonist"),
            None
        )
        moral = next((e for e in claim.elements if e.category == "moral"), None)

        interpretation = f"Narrative about "
        if protagonist:
            interpretation += f"{protagonist.content}. "
        if moral:
            interpretation += f"Moral: {moral.content}"

        return interpretation

    def _generate_caveats(self, claim: CompiledClaim) -> List[str]:
        """Generate important caveats about narrative proof."""
        caveats = []

        if claim.confidence_score < 0.7:
            caveats.append("Low confidence in extracted elements")

        if not any(e.symbolic_mapping for e in claim.elements):
            caveats.append("No symbolic mappings found to formal claim elements")

        if len([e for e in claim.elements if e.category == "moral"]) == 0:
            caveats.append("No explicit moral/lesson identified")

        return caveats

    def _compute_confidence(self, claim: CompiledClaim) -> float:
        """Compute overall confidence in compilation."""
        if not claim.elements:
            return 0.0

        # Average element confidence
        avg_confidence = sum(e.confidence for e in claim.elements) / len(claim.elements)

        # Adjust for element diversity
        element_types = len(set(e.category for e in claim.elements))
        diversity_bonus = (element_types / len(self.ELEMENT_KEYWORDS))

        return min(1.0, avg_confidence * (0.7 + 0.3 * diversity_bonus))

    def _compute_hash(self, claim: CompiledClaim) -> str:
        """Compute deterministic hash of compiled claim."""
        hashable = {
            "claim_id": claim.claim_id,
            "parable_id": claim.parable_id,
            "parable_type": claim.parable_type.value,
            "formal_claim": claim.formal_claim,
            "confidence_score": claim.confidence_score,
        }

        content = json.dumps(hashable, sort_keys=True)
        return hashlib.sha256(content.encode()).hexdigest()

    def export_claim(self, claim: CompiledClaim, output_path: Path) -> Path:
        """Export compiled claim to JSON."""
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(json.dumps(claim.to_dict(), indent=2))
        return output_path


def main():
    """CLI entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Compile parable into formal claim")
    parser.add_argument("--parable", type=Path, help="Parable text file")
    parser.add_argument("--type", choices=["ANALOGY", "METAPHOR", "NARRATIVE", "ALLEGORY", "PARABLE"],
                        default="NARRATIVE", help="Type of parable")
    parser.add_argument("--output", type=Path, help="Output claim JSON")
    parser.add_argument("--id", help="Parable ID")

    args = parser.parse_args()

    compiler = ParableCompiler()

    if args.parable and args.parable.exists():
        parable_text = args.parable.read_text()
        claim = compiler.compile_parable(
            parable_text,
            parable_type=ParableType[args.type],
            parable_id=args.id
        )

        if args.output:
            compiler.export_claim(claim, args.output)
            print(f"Claim exported to {args.output}")

        print(f"Claim ID: {claim.claim_id}")
        print(f"Status: {claim.status.value}")
        print(f"Confidence: {claim.confidence_score:.2f}")
        print(f"Interpretation: {claim.interpretation}")

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
