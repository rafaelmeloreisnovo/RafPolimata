# Federated Claim Protocol (RAFAELIA-PSC-1-FED)

**Version:** 1.0.0  
**Date:** 2026-08-15  
**Schema:** rafaelia.federated_claim_protocol.routing.v1  
**Governance:** PARABOLIC_SEMANTIC_CODEC

## Overview

Federated Claim Protocol enables cross-repository claim submission, validation, and aggregation.
Coordinates with 11 autonomous repositories using standardized claim submission and receipt verification.

## Protocol Flow

```
Local Claim (RafPolimata)
    ↓
[Phase 1: Verify locally] → ClaimVerifier + HeuristicEvaluator
    ↓ (claim_gate_status = PASS)
[Phase 2: Submit to external] → FederatedSubmitter
    ↓
External Repository [Rafaelia_Core, ZIPRAF_OMEGA_FULL, ChipQuantum, ...]
    ↓ (process, evaluate, sign)
Receipt + Signature
    ↓
[Phase 2: Aggregate results] → FederatedAggregator
    ↓
Cross-Repository Verification Report
    ↓
[Phase 4: CI Gate] → claim_allowed = true/false
```

## Repository Authority Mapping

| Layer | Repository | Role | Authority |
|-------|-----------|------|-----------|
| Protocol | `rafaelmeloreisnovo/Rafaelia_Core` | Language/type system definition | Semantic truth |
| Codec | `rafaelmeloreisnovo/ZIPRAF_OMEGA_FULL` | Round-trip serialization | Integrity reference |
| Geometry | `rafaelmeloreisnovo/ChipQuantum` | Mathematical foundations | Formal specification |
| Proofs | `rafaelmeloreisnovo/Matem-tica-` | Formal mathematics | Proof verification |
| Publication | `rafaelmeloreisnovo/papers` | Peer review | Scientific validation |
| Governance | `rafaelmeloreisnovo/RafPolimata` | Workflow orchestration | Local gate keeper |
| Navigation | `rafaelmeloreisnovo/Mapa` | Cross-repo linking | Discovery/traceability |
| Parables | `rafaelmeloreisnovo/CientiEspiritual` | Narrative formalism | Analogical reasoning |
| Cosmology | `rafaelmeloreisnovo/Cosmos` + RLL | Physics/relativistic adapters | Domain specialization |
| Quantum | `instituto-Rafael/Eletron-efeitos-qu-ntico` | Quantum mechanics | Domain specialization |

## Claim Submission Format

```json
{
  "schema": "rafaelia.federated_claim_protocol.routing.v1",
  "claim_id": "CLAIM-20260815-001",
  "origin_repo": "rafaelmeloreisnovo/RafPolimata",
  "origin_commit": "abc123...",
  
  "claim": {
    "id": "unique-claim-id",
    "source_status": "TESTED",
    "epistemic_status": "PROVED",
    "claim_gate": "PASS",
    "domain": "COMPILER",
    "source": "phase-21-type-system",
    "method": "hindley-milner-unification",
    "artifact": "Apkc/sem_type_inference.h",
    "limitations": "single-pass, no polymorphic recursion",
    "falsifier": "ambiguous-constraint-set"
  },
  
  "verification": {
    "gate_status": "PASS",
    "heuristic_scores": {
      "DIRECT": 0.95,
      "CAUSAL_REVERSE": 0.72,
      "TEMPORAL_ANTIDERIVATIVE": 0.55,
      "LOCAL_DERIVATIVE": 0.88,
      "COUNTERFACTUAL": 0.90,
      "ROLE_INVERSION": 0.65,
      "MULTISCALE": 0.80
    },
    "overall_score": 0.78,
    "hard_boundaries_satisfied": true
  },
  
  "evidence": [
    {
      "type": "unit-test",
      "reference": "tests/test_type_inference.c:line:42",
      "hash": "sha256:...",
      "status": "PASS"
    }
  ],
  
  "request_authorities": [
    "rafaelmeloreisnovo/Matem-tica-",
    "rafaelmeloreisnovo/papers"
  ],
  
  "timestamp_utc": "2026-08-15T14:30:00Z",
  "signature": "ED25519(...)"
}
```

## Receipt Format

External repository responds with receipt:

```json
{
  "schema": "rafaelia.federated_claim_receipt.routing.v1",
  "receipt_id": "RECEIPT-20260815-001",
  "origin_repo": "rafaelmeloreisnovo/Matem-tica-",
  
  "claim_reference": {
    "claim_id": "CLAIM-20260815-001",
    "origin_commit": "abc123..."
  },
  
  "verification": {
    "status": "VERIFIED",
    "verdict": "PASSED_FORMAL_PROOF",
    "confidence": 0.99,
    "timestamp_utc": "2026-08-15T15:00:00Z"
  },
  
  "evidence": {
    "proof_artifact": "proofs/type_inference.v",
    "proof_hash": "sha256:...",
    "verified_by": "coq-proof-checker:8.17",
    "certification": "PASSED"
  },
  
  "authority": {
    "repo": "rafaelmeloreisnovo/Matem-tica-",
    "maintainers": ["alice@example.com", "bob@example.com"],
    "authority_level": "FORMAL_PROOF"
  },
  
  "signature": "ED25519(...)",
  "receipt_hash": "sha256:..."
}
```

## Aggregation Process

FederatedAggregator collects receipts from multiple repositories:

```json
{
  "schema": "rafaelia.federated_aggregation_report.routing.v1",
  "report_id": "REPORT-20260815-001",
  "local_repo": "rafaelmeloreisnovo/RafPolimata",
  "local_commit": "def456...",
  
  "original_claim": { /* full claim JSON */ },
  
  "receipts": [
    {
      "authority": "rafaelmeloreisnovo/Matem-tica-",
      "status": "VERIFIED",
      "verdict": "PASSED_FORMAL_PROOF",
      "confidence": 0.99
    },
    {
      "authority": "rafaelmeloreisnovo/papers",
      "status": "VERIFIED",
      "verdict": "PASSED_PEER_REVIEW",
      "confidence": 0.92
    }
  ],
  
  "aggregated_verdict": {
    "status": "CLAIM_ALLOWED",
    "combined_confidence": 0.96,
    "requires_additional_evidence": false,
    "recommendations": []
  },
  
  "timestamp_utc": "2026-08-15T15:30:00Z",
  "report_hash": "sha256:..."
}
```

## Security Considerations

### Signature Verification

All submissions and receipts are signed with ED25519. Verify:

```python
crypto.verify(
    public_key=repo_public_key,
    message=claim_bytes,
    signature=claim_signature
)
```

### Blocked Operations

Federated protocol forbids:

- ✗ Modifying claims after submission
- ✗ Creating circular dependencies (repo A → B → A)
- ✗ Submitting unverified claims (gate_status != PASS)
- ✗ Accepting receipts from unauthorized repos
- ✗ Ignoring hard boundary violations
- ✗ Claiming credit for upstream work

### Trusted Repos

Only these repositories accept submissions:

```
TRUSTED_AUTHORITIES = {
    "rafaelmeloreisnovo/Rafaelia_Core",
    "rafaelmeloreisnovo/ZIPRAF_OMEGA_FULL",
    "rafaelmeloreisnovo/ChipQuantum",
    "rafaelmeloreisnovo/Matem-tica-",
    "rafaelmeloreisnovo/papers",
    "rafaelmeloreisnovo/Mapa",
    "rafaelmeloreisnovo/CientiEspiritual",
    "rafaelmeloreisnovo/Cosmos",
    "instituto-Rafael/Eletron-efeitos-qu-ntico",
    "instituto-Rafael/relativity-living-light",
}
```

## CI Gate Integration

CI gate checks:

1. **Local gate:** claim_gate_status == PASS
2. **Federated gate:** aggregated_verdict.status == CLAIM_ALLOWED
3. **Hard boundaries:** no violations detected
4. **Signature validation:** all signatures valid
5. **Freshness:** receipts < 7 days old

If all gates pass:

```bash
export CLAIM_ALLOWED=true
export FEDERATED_VERDICT=APPROVED
export COMBINED_CONFIDENCE=0.96
```

## Error Handling

### Timeouts

If external repo doesn't respond within 24 hours:

```json
{
  "receipt_id": "TIMEOUT-...",
  "status": "PENDING",
  "reason": "authority-timeout",
  "fallback_verdict": "TOKEN_VAZIO"
}
```

### Signature Mismatch

If receipt signature doesn't verify:

```json
{
  "receipt_id": "INVALID-...",
  "status": "REJECTED",
  "reason": "signature-verification-failed",
  "fallback_verdict": "CLAIM_BLOCKED"
}
```

### Circular Dependencies

If repo A references repo B which references repo A:

```json
{
  "error": "circular_dependency_detected",
  "cycle": ["repo-a", "repo-b", "repo-a"],
  "action": "block-submission"
}
```

## Specification Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2026-08-15 | Initial specification (10 authorities, ED25519 signing) |

## References

- `docs/AGENTES.md` — Operational protocol
- `configs/parabolic_semantic_codec.json` — Gate definitions
- `CLAUDEMD.md` — Codebase truth model
- `tools/claim_verifier.py` — Local verification engine
- `tools/federated_submitter.py` — Cross-repo submission
- `tools/federated_aggregator.py` — Receipt aggregation
