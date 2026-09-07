---
name: compiler-pattern-leaf
description: Apply reusable architecture patterns to RafPolimata compiler, ApkC, freestanding, syscall, indexing, or low-level tooling work without weakening target-specific contracts or inflating evidence. Use when an older language/runtime architecture suggests a useful typing, state, trigger, validation, serialization, storage-layout, memory-map, or failure mechanism.
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

## Machine-geometry transfers

Older machine/storage architectures can be normalized into compiler/runtime invariants without copying their historical constants.

### Identifier and allocation geometry

Map historical auto-number, variable conversion and record-allocation ideas to:

```text
source value
-> explicit-width conversion
-> range/overflow guard
-> deterministic identifier allocation
-> logical address/index
-> serialized representation
```

Do not conflate identifier identity with a physical address. Reuse/wrap/collision behavior must be explicit.

### Memory-map geometry

Translate conventional/high-memory windows, EMS/XMS-like banking, DMA/IRQ or MMIO reasoning into:

```text
address region
-> ownership
-> alignment/width
-> access capability
-> transfer/event boundary
-> conflict/failure state
```

For freestanding code, the current architecture contract owns exact addresses, alignment, ABI, registers and trap semantics. Historical addresses are references only.

### Storage-layout geometry

Map sector/block/page/extent and fragmentation reasoning to:

```text
record
-> serialized bytes
-> alignment/padding
-> block/page allocation
-> locality/cost model
-> fragmentation/compaction policy
```

A compiler or packer may optimize layout only when object identity, offsets and reproducibility remain testable. Current-device measurements, not remembered HDD/SSD timing, govern performance claims.

### Binary/header geometry

Executable/file-header, partition/boot-record and loader-layout reasoning transfers as:

```text
magic/version
-> fixed-width fields
-> validated offsets/lengths
-> alignment
-> mapped sections/segments
-> activation/load order
-> reject-on-malformed
```

Do not reuse historical magic values or offsets unless the target format specification requires them.

### Protocol and device geometry

Serial/parallel/network/modem examples may supply framing/state-machine ideas, but compiler/runtime code must keep:

```text
physical/electrical layer
!= framing
!= protocol state
!= application command
```

Electrical success is outside a compiler PASS unless a hardware fixture explicitly observes it.

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

Record `source_pattern`, `compiler_leaf`, affected target contract, memory/storage/binary geometry if relevant, falsifier, commands actually executed, `F_ok`, `F_gap`, `F_next`, rollback and scoped `claim_allowed`.
