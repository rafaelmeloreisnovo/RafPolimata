# Formula Authority Bridge V1

**State:** `IMPLEMENTED_UNTESTED`  
**claim_allowed:** `false`

## Purpose

Give humans and coding agents one bounded route from symbolic formula names to executable reference,
source authority and evidence without copying the whole mathematical corpus.

```text
formula token
→ semantic authority
→ local reference producer
→ golden vectors
→ cross-language adapter
→ equivalence gate
```

Machine index: `configs/formula-authority-index.v1.json`.

## Recurrence matrix reference

The local producer `research/recurrence_matrix_reference/reference.py` implements exact integer
matrix exponentiation for two conventional recurrence definitions:

```text
F0=0, F1=1
[F(n+1)]   [1 1]^n [1]
[F(n)  ] = [1 0]   [0]

T0=0, T1=0, T2=1
[T(n+2)]   [1 1 1]^n [1]
[T(n+1)] = [1 0 0]   [0]
[T(n)  ]   [0 1 0]   [0]
```

The definitions are explicit because repositories may contain other Fibonacci/Tribonacci indexing
conventions. Equal names do not imply equal initial conditions.

## Trinity633

```text
Trinity633(A,L,C) = A^6 · L^3 · C^3
```

It is included in the same reproducibility packet but **not represented as a recurrence matrix**.
The relation is `CO_PUBLISHED_NOT_FUSED`.

## Golden vectors

`data/formulas/recurrence-matrix-vectors.v1.json` is the interoperability surface for C, ASM,
Java, Kotlin or other implementations. An adapter should reproduce those vectors before claiming
equivalence.

## Gates

```bash
python3 -m unittest -v tests/test_recurrence_matrix_reference.py
python3 scripts/validate_formula_authority_index.py
```

A PASS means exact reference/vector consistency in this repository. It does not establish global
mathematical authority, scientific interpretation or runtime/device parity in another repository.

## Relationship to Spiral√3/2 and π·φ

The matrix package does not absorb the spiral family. `sqrt(3)/2` remains separately routed.
Likewise, `π·φ` is not introduced into Fibonacci or Tribonacci unless a specific formula contract
requires it.

This separation is deliberate:

```text
shared navigation != shared semantics
shared evidence bundle != algebraic identity
analogy != claim
```

## R3

F_ok = deterministic matrix reference + vectors + tests + authority index created.  
F_gap = cross-repository canonical authority and independent C/ASM/Java parity remain open.  
F_next = execute the local gates, then bind one consumer implementation at a time to the golden vectors.
