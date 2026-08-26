# RAFAELIA Stage0 + Fluent Event ABI V1

Status: `MATERIALIZED_FOR_REVIEW / claim_allowed=false`

Closure binding for unresolved execution/runtime evidence: `CLOSURE_L2`.

## Decision

RAFAELIA separates the compile artifact plane from the event/receipt plane.

### Artifact plane

`SOURCE -> RAFIR -> ISA ENCODER -> ELF/DEX/ZIP`

No SQL database, Fluent daemon, JSON translator, assembler or external linker is required by the target architecture of the Stage0 compiler.

### Event/receipt plane

Every meaningful state transition emits a canonical event directly as Fluent Forward Message-mode MessagePack:

`[tag, time, record]`

The implementation authority is `Apkc/raf_fluent_event.h`.

The codec is caller-buffer based, has no heap, no libc and no external MessagePack dependency.

## Why SQL is not canonical

A relational database is useful as a local query/read model, but forcing compiler/runtime evidence through SQL creates an unnecessary transformation boundary:

`event -> SQL schema -> query/export -> transport`

The canonical route becomes:

`event -> MessagePack Forward envelope -> append-only sink / Fluent-compatible receiver`

SQLite/Room, when used by consumers, is a derived projection and must be rebuildable from authority/evidence streams.

Invariant:

`EVENT_AUTHORITY != SQL_PROJECTION`

## Why Fluent is not the IR

Fluent Forward is a transport/event envelope, not a compiler intermediate representation.

Invariant:

`RAFIR != FLUENT_EVENT != ELF != DEX != ZIP`

Therefore no compiler backend is required to parse Fluent records in order to emit machine artifacts.

## Canonical event record V1

Fields are emitted in deterministic order:

- `schema = RAFAELIA_FLUENT_EVENT/v1`
- `event`
- `component`
- `arch`
- `artifact_kind`
- `state`
- `source_sha256`
- `artifact_sha256`
- `seq`
- `claim_allowed`

Unknown evidence is serialized as `TOKEN_VAZIO`; it is never silently replaced by an empty value or success state.

## Stage0 route

### S0.1 — existing authorities

- ARM32 A32 encoder: `Apkc/arch_arm32.h`
- syscall layer: `Apkc/sys.h`
- ELF writer: `Apkc/fmt_elf.h`
- current compiler surface: `Apkc/apkc.c`
- Fluent-compatible event codec: `Apkc/raf_fluent_event.h`

### S0.2 — first executable closure

Build a minimal `rafcc-stage0-arm32` able to consume a bounded RAFIR/ASM subset and emit ELF32 through the internal encoder/writer.

Pass requires:

- no libc imports;
- no heap;
- no `DT_NEEDED`;
- no PT_INTERP;
- ARM `e_machine`;
- deterministic artifact hash for identical input/tool identity;
- direct event receipt emitted for source accepted, code generated, artifact sealed, gate result.

### S0.3 — remove external linker from normal path

Use `fmt_elf.h` as the output authority instead of `lld` for Stage0-supported artifacts.

### S0.4 — remove external assembler from normal path

Use `arch_arm32.h`/ARM64 encoder functions directly.

### S0.5 — remove Python lowering from the Stage0 subset

Move the minimal lowering/parser necessary for RAFIR/ASM into freestanding C with fixed storage.

### S0.6 — bootstrap

`stage0 -> stage1 -> stage2`

Then verify reproducible self-host convergence where applicable:

`hash(stage2_a) == hash(stage2_b)`

This equality is a build reproducibility claim only; it does not by itself prove authorship, legal clearance or semantic equivalence.

## Sink model

The canonical bytes may be:

1. written append-only to a local `.msgpack` receipt stream;
2. sent to a Fluent-compatible Forward endpoint when networking is available;
3. mirrored into a derived JSONL/Room/SQLite projection for human/UI querying.

The first two consume the canonical event representation. Projections must not become authority.

## Cross-system property

A target backend changes only the artifact encoder/writer:

- ARM32 -> A32 + ELF32
- ARM64 -> A64 + ELF64
- Android VM -> DEX
- Android package -> ZIP/APK + AXML

The event ABI remains the same, so provenance/telemetry does not require schema translation merely because the target artifact changes.

## R3

- `F_ok`: direct Fluent-compatible freestanding codec materialized; ARM32 encoder/syscalls/ELF writer already exist.
- `F_gap`: Stage0 executable still needs closure without Python/clang/lld in its normal path; Forward network sink not yet implemented; physical ARM32 execution receipt missing.
- `F_next`: integrate the codec at Stage0 state transitions, implement bounded RAFIR->A32->ELF32 route, then execute on physical ARM32 and bind hashes/exit evidence.
