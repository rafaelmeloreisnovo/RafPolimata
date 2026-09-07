---
name: compiler-pattern-leaf
description: Apply reusable architecture patterns to RafPolimata compiler, ApkC, freestanding, syscall, indexing, or low-level tooling work without weakening target-specific contracts or inflating evidence. Use when an older language/runtime architecture suggests a useful typing, state, trigger, validation, serialization, or failure mechanism.
---

# Compiler / Freestanding Pattern Leaf

Read `AGENTS.md` and the nearest scoped `AGENTS.md` before applying this skill.
Governance anchor for unresolved operational gaps in this leaf: `CLOSURE_L11`.

## Local projection

```text
extracted mechanism
-> target subsystem
-> target-specific invariant
-> representation / state / guard mapping
-> smallest code or contract delta
-> compiler/static/runtime falsifier as applicable
-> receipt
```

Reference transfers:

- Pascal-like discipline -> explicit widths/types, range checks, initialized state, deterministic control flow, overflow/error policy and clear compile-time/runtime boundaries.
- InterBase-like trigger discipline -> explicit phase/event activation, guarded transformations, durable audit events and deterministic failure transitions where an event model already belongs.

These are mechanisms, not a mandate to introduce Pascal or SQL syntax.

## Freestanding boundary

For `freestanding/**`, preserve the local contract exactly:

```text
freestanding -> no libc, heap, GC, hosted runtime or syscall
syscall      -> optional OS ABI bindings; never imported back into L0
```

A hosted development path never justifies weakening a freestanding target.

## Evidence boundary

```text
concept != implementation != execution != evidence != validated claim
syntax PASS != device runtime
bounded/indexing construction != theorem
pattern similarity != proof
skill != authority
```

Use the smallest applicable target (`make syntax`, compiler contract/self-test, freestanding matrix, document-governance gate, or path-specific test) when execution is available. Otherwise preserve execution as `TOKEN_VAZIO` under `CLOSURE_L11` until the applicable gate is actually executed.

Private derivation may supply a sanitized mechanism only; do not publish raw private corpus or private source pointers.

## Completion

Record `source_pattern`, `compiler_leaf`, affected target contract, falsifier, commands actually executed, `F_ok`, `F_gap`, `F_next`, rollback and scoped `claim_allowed`.
