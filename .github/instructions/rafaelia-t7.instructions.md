---
applyTo: "rafaelia/**"
---

# RAFAELIA / T^7 — path-specific instructions

Read `AGENTS.md`, `docs/AGENTES.md`, the current module tests, and `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` before changing convergence or attractor language.

## Mathematical claim boundary

Do not state that the implementation proves "42 fixed-point attractors".

The current evidence distinguishes:

- a finite/indexed construction involving values in a 42-slot range;
- observed program behavior under specified inputs;
- a mathematical convergence theorem;
- a physical/cognitive interpretation.

These are not interchangeable.

The previous strong fixed-point convergence claim was falsified as stated by the current L9 work. Preserve that negative result unless a new definition and reproducible proof supersede it explicitly.

## Before promoting a claim

Require, as applicable:

```text
precise object and units
transition/update definition
H0/H1 or theorem statement
falsifier/test
initial conditions / seed
execution environment
observed output
receipt/hash
scope and limitations
```

A modulo/range operation that returns 0..41 proves only that bounded output property unless additional dynamics are demonstrated.

## Symbolic versus executable language

Symbolic names such as `phi`, `omega`, `ethica`, toroid or engram may be useful domain vocabulary. Do not silently promote metaphor/label into a scientific or mathematical theorem.

If a statement is heuristic or analogical, label it as such.

## Tests and handoff

Run the smallest relevant current falsifier/smoke test available for the touched path. Report negative results and `TOKEN_VAZIO` exactly; do not edit tests or documentation merely to restore a stronger historical claim.
