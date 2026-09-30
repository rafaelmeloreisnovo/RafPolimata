# AUTHORIAL OMEGA ASSURANCE V1

Status: `IMPLEMENTED_UNTESTED`  
Claim gate: `false`  
Governance: topology/implementation gaps → `CLOSURE_L11`; runtime/device gaps → `CLOSURE_L12`.

## Purpose

RafPolimata is the assurance/falsifier plane of the Authorial Omega federation. It does not own the federated topology (Mapa) and does not own the executor (RafGitTools).

The bounded chain is:

```text
Mapa PR#717 exact manifest
  -> RafGitTools PR#591 exact executor
  -> RafPolimata falsifiers / benchmarks / evidence discipline
  -> producer/device receipt when runtime is claimed
```

No Vectra or PCR inherited implementation is imported here. Vectra remains a structural reference; PCR remains the documentary custody-cycle reference. Ambiguous or upstream material is reference-only.

## Exact pins

- Mapa PR #717 head: `3d54e8e36f1de787e1a815d899490a0fb7f0b326`
- Mapa manifest blob: `5ec155d2e67fb7e6f46fb2589c6e32149d570091`
- RafGitTools PR #591 head: `c949595817ac70144001c2a597aaac84eab06c10`
- RafGitTools executor config blob: `dab95ef7489a72f24fccb22d822fcc5a59900b56`
- RAF_BL0_V0 expected Git blob: `132f948d199f6679fcae4f33912e0a3ad69691a3`

A Git blob proves object identity in Git. It does not by itself establish legal authorship, physical execution, novelty, patentability or scientific validity.

## Assurance dimensions

The first pass tests six failure modes: whole-fork authorship inflation; model-reference/code-inheritance confusion; build-to-runtime promotion; invalid C08 reentry; private payload leakage to public control plane; and relation-label-to-causality promotion.

This preserves the session's relational theme: `DIRECT`, `INDIRECT`, `TRANSVERSAL`, `ORTHOGONAL`, derivational or latent labels are graph semantics. They are not automatically causal evidence.

## Evidence boundary

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
IMPLEMENTED_UNTESTED != PASS
TOKEN_VAZIO != 0
```

The current RafGitTools head is pinned, but its fresh exact-head terminal pipeline result is intentionally not hardcoded as PASS in this assurance cut. That state must come from fresh provider readback.

Physical device execution and independent reproduction remain `TOKEN_VAZIO (CLOSURE_L12)`.

## Local gate

```sh
python3 -m unittest -v tests.test_authorial_omega_assurance
python3 scripts/validate_authorial_omega_assurance.py
```

A passing local/CI validator proves only the pinned configuration and its negative falsifiers. It does not promote device/runtime or legal-authorship claims.

## R3

**F_ok:** assurance contract, exact pins, authorial-only admission policy and six falsifiers are materialized.

**F_gap:** exact-head execution of this RafPolimata branch, fresh terminal RafGitTools PR#591 state, physical runtime and independent reproduction remain evidence-bound.

**F_next:** execute exact-head RafPolimata tests, then fresh-read PR#591's exact-head pipeline and append a successor receipt without changing the physical-runtime boundary.
