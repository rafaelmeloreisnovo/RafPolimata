# SEED Ω Evolution — Producer Contract V1

**Repository:** RafPolimata  
**Date:** 2026-09-15  
**State:** `CONTRACT_ONLY / IMPLEMENTATION_TOKEN_VAZIO`  
**claim_allowed:** `false`

## Role

RafPolimata is the intended **evidence producer** for SEED-Ω. This file does not claim that the evolution engine has been implemented.

The upstream authority split is:

- Mapa: schema, ontology, lineage and seed manifest;
- papers: methodological synthesis and claim ledger;
- RLL: falsifiable scientific/model contract;
- RafPolimata: future deterministic execution, tests and receipts.

## Non-linear lineage

The runtime must use a temporal typed hypergraph, not a single linear list.

A child may have multiple parents:

[
{P_1,P_2,ldots,A_k}ightarrow C.
]

The ancestor (A_k) may be used for explicit backcross.

## In-vitro bank

A stored seed can be quiescent:

[
activity(S_i)approx0
]

without being deleted.

Hard invariant:

[
QUIESCENT
eq DELETED.
]

Reactivation is explicit and receipt-bound.

## Backcross

[
C=B(P,A;M)
]

where (P) is current parent, (A) is preserved ancestor and (M) is a trait mask.

The producer must record:

- parent and ancestor IDs;
- trait mask;
- compatibility result;
- pre/post trait vectors;
- operator parameters;
- deterministic RNG seed;
- output hash.

Neither parent nor ancestor may be silently rewritten.

## Practice ↔ theory residual

The runtime stores only the mismatch first:

[
Delta_{PT}=d(O_{practice},P_{theory}).
]

Initial owner:

[
TOKEN_VAZIO_RESIDUAL_OWNER.
]

It may later be classified as measurement noise, discretization artifact, model misspecification, unmodeled structure, implementation gap, theory-only prediction, practice-only effect, anomaly or contradiction.

The producer must not assume that either theory or practice is the noise source.

## Hidden relations

[
HIDDEN_IN_PROJECTION

eq
ABSENT_FROM_GRAPH.
]

A latent-relation probe may materialize an already derivable relation, but its receipt must distinguish **discovery/recovery** from **creation**.

Control example:

[
h=(sqrt3/2)a
]

for an equilateral triangle.

## Execution boundary

Required before `IMPLEMENTED_UNTESTED` can become anything stronger:

1. finite fixture set;
2. JSON/schema validation;
3. deterministic replay;
4. illegal operator negative tests;
5. incompatible recombination rejection;
6. backcross ancestry preservation tests;
7. quiesce/reactivate roundtrip;
8. noise reclassification provenance;
9. receipt hashes;
10. independent rerun.

Current state:

`EVOLUTION_ENGINE=TOKEN_VAZIO_IMPLEMENTATION`  
`DETERMINISTIC_REPLAY=TOKEN_VAZIO`  
`SCIENTIFIC_GAIN=TOKEN_VAZIO`  
`claim_allowed=false`.

## R3

**F_ok:** producer interface and fail-closed gates are specified.  
**F_gap:** no engine, fixtures, runtime receipt or independent replay exists in this delta.  
**F_next:** implement fixture-only deterministic producer before any model-performance claim.
