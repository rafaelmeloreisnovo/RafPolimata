# RAFAELIA Ω — Cognitive Freestanding L0 V1

Authority: RafPolimata producer. Exploratory authorship: Rafael Melo Reis.
Status: IMPLEMENTED_SOURCE / LOCAL_HOST_AND_CROSS_OBJECT_PASS / PROVIDER_NOT_RUN.
Source: docs/COGNITIVE_SPIRAL_CUBE_10X10X10_V1.md.
Rights: root redistribution license TOKEN_VAZIO. Claim_allowed=false.

## Pure contract

- Strict 10x10x10 coordinate bijection: k=x+10*y+100*z; k in [0,999].
- First valid Q16 sample initializes observed state; subsequent samples yield EMA=(3*old+sample+2)/4.
- Typed missing observation differs from observed numeric zero. No mutation on missing or invalid input; distinct caller-owned before and after.
- Each candidate has caller-provided source/evidence/receipt opaque refs and an explicit L9 bitmask. These refs are NOT authenticated by L0.
- Status CANDIDATE is an arithmetic transition, not scientific, physical, or evidence gate PASS.
- No libc, heap, network, hosted headers, syscall, hidden allocation, retry or provider SDK.
- No self-directed learning, training, 42-attractor or neurological outcome claim.

## Supabase hosted boundary (NOT deployed)

Verified provider: Rafcodephi PG17, project ehwltgfrvbkmjqjjfoqn. Existing private tables:
raf_private.mapa_driver_catalog_v1, mapa_driver_projection_v1, mapa_driver_run_receipts_v1, mapa_cross_source_edges_v1.
Read-only snapshot on 2026-11-03: drivers=4, projections=12, edges=32, receipts=2, promoted claims=0. anon/authenticated: raf_private USAGE denied, projection SELECT denied.

No schema is changed in this slice. These tables describe source mappings, NOT numeric cognition; do not reuse them as unsourced measured EMA. A future separate authorized adapter must validate user permissions, rights, source SHA, input schema, replay/idempotency, evidence authenticity, RLS/API negative tests, backup/restore and rollback before importing any private event. Stable record ID + SHA + event identity belongs at the provider boundary. L0 only operates on caller-supplied values.

## Gates and falsifier

Run sh freestanding/tests/verify_cognitive_v1.sh for 37 host assertions at O0/O2/O3, 1000 cell roundtrips and six OS-neutral architecture objects with unresolved-symbol count 0. Initial hosted-test struct copying generated a riscv32 memcpy import; this was fixed by isolating a freestanding cross-ISA probe. This proves a bounded source/compile property, not arbitrary calling code or device runtime. No Supabase API E2E, Android physical test or full CI claim is made.

Rollback: close unmerged draft PR, preserve all historical evidence; main and database are untouched. Exact-head CI + independent review are required before merge.
