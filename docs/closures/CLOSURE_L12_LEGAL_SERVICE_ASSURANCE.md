# CLOSURE_L12 — Legal & Service Assurance Structured Gaps

**Closure ID:** `CLOSURE_L12`  
**Status:** `GOVERNANCE_CLOSURE / DOES_NOT_PROMOTE_MATERIAL_GAPS`  
**Owner:** `human-authorizer` + `security-license` + privacy review  
**Scope:** Legal & Service Assurance Pack V1 and RAF Hash Fabric licensing/evidence fields explicitly bound by the TOKEN_VAZIO validator.

## 1. Purpose

This closure binds intentional unresolved fields in legal/service templates to a reproducible closing route. It does **not** convert missing legal facts, signatures, runtime evidence, supplier evidence, privacy facts or cryptographic implementation into PASS.

The invariant is:

```text
TOKEN_VAZIO + CLOSURE_L12
= known structured gap with owner and closing evidence
!= completed contract
!= legal compliance
!= certification
!= runtime proof
```

## 2. Why these gaps are expected

Reusable templates necessarily contain fields that cannot be truthfully completed before a concrete transaction exists, including:

- party identity and signatory authority;
- customer/provider legal entities;
- governing law, forum and contract dates;
- service scope, pricing, acceptance and SLA values;
- controller/processor/operator roles based on actual facts;
- data categories, purposes, retention and locations;
- selected international-transfer mechanism and official clause artifacts;
- selected suppliers/subprocessors and actual service entities;
- actual security evidence/certifications and validity periods;
- current-commit cryptographic/runtime evidence not yet produced.

Replacing these unknowns with invented defaults would be less correct than preserving them explicitly.

## 3. Bound paths

The validator binds L12 only to the exact paths registered in `tools/validate_token_vazio_gates.py`, covering:

- `docs/legal/*` artifacts in Legal & Service Assurance Pack V1;
- the RAF Hash Fabric module license-scope/README/spec/header needed by this PR;
- the local BLAKE3 KAT receipt whose repository implementation remains unresolved.

The mapping is exact-path scoped. Other repository files do not inherit L12 automatically.

## 4. Closing evidence by gap family

### Identity / contract
Close with legal-party identity, authority, final version, dates and signatures.

### Commercial scope
Close with signed MSA/Order/SOW identifying artifact/version/hash, rights, fees, acceptance, liability and applicable law.

### IP / license
Close with authorship/provenance inventory, third-party license compatibility, scope manifest and required notices. Root-wide licensing remains separately governed.

### Privacy / LGPD / GDPR
Close with factual processing inventory, legal roles, purposes/bases/instructions, data categories, retention, rights route, DPA and public notice where applicable.

### International transfer
Close with actual origin/destination, lawful mechanism, official ANPD clauses or EU SCC module/annexes when selected, supplementary safeguards, onward-transfer chain and transparency evidence.

### Supplier / BigTech dependency
Close with exact service and contracting entity, current terms/DPA, locations, subprocessors, security evidence, transfer basis, continuity and exit route. Brand reputation alone cannot close the gap.

### Security
Close specific controls with current evidence. Certification fields close only with an actual valid certification/report whose issuer, scope and validity are recorded.

### Cryptographic/runtime evidence
Close only after reviewed source is persisted, built and tested from the exact commit/environment. A working-container KAT does not by itself close current-commit implementation evidence.

## 5. Promotion rules

No L12-bound gap may be promoted merely because:

- this closure exists;
- a template exists;
- a PR is mergeable;
- another workflow passed;
- a vendor is well known;
- an algorithm is standardized;
- a local experiment passed outside the repository commit.

Promotion requires the evidence named by the corresponding artifact/control matrix.

## 6. Negative states

`FAIL`, contradiction, stale evidence, expired certification, rejected transfer mechanism, incompatible license or unsuccessful runtime test remain first-class evidence and must not be rewritten as TOKEN_VAZIO merely to avoid a failing result.

## 7. Rollback

If L12 is too broad or a bound path changes semantics, remove/revise only the exact binding and re-run the strict validator. Do not disable the TOKEN_VAZIO gate or globally whitelist the legal/native directories.

## 8. F_next

1. Keep exact-path bindings synchronized with the Legal Assurance pack.
2. Add customer-specific instantiated contracts as separate controlled artifacts rather than overwriting templates.
3. Close root-license provenance independently.
4. Persist and rerun the BLAKE3 oracle from a commit when the repository write path permits it.
5. Re-run strict TOKEN_VAZIO validation after every scope change.
