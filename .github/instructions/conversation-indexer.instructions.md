---
applyTo: "runtime/conversation_indexer/**"
---

# Conversation indexer — path-specific instructions

Read `AGENTS.md`, `docs/AGENTES.md`, `docs/RAFAELIA_DATA_INGEST_INDEX_PROTOCOL.md`, and the tests beside this subsystem before editing.

## Core contract

- C is the reference implementation for the bounded streaming core.
- Do not introduce a full in-memory DOM for large conversation exports.
- Freestanding/core paths must not gain hidden allocation, unbounded recursion, or unbounded libc dependencies.
- Hosted outer I/O shells may use facilities explicitly allowed by their contract; keep the core boundary explicit.
- Every buffer needs pointer/base, current length/position, and explicit capacity.
- Every loop must be bounded by input length or a declared maximum.
- Persist explicit-width integers with deliberate serialization; do not persist compiler padding or pointer values.

## Strings and source identity

- Treat source strings as byte spans, not assumed NUL-terminated C strings.
- Preserve source byte ranges even when a normalized view is derived.
- Track UTF-8 validity separately from byte preservation.
- Store string pool references as checked offset + length according to the current format.
- Never use unbounded source-data operations such as `strcpy`, `strcat`, or unchecked `%s`.

## Streaming determinism

- Input is processed in caller-selected bounded chunks.
- Output semantics and artifact identity must not depend on chunk size unless the format explicitly declares otherwise.
- Checkpoint only at parser-safe boundaries.
- Resume must reproduce the declared artifact or reject the checkpoint explicitly.
- Do not return a verified/success state for partial output.

## Error model

Keep deterministic distinctions between at least:

```text
syntax/truncation
capacity/depth limit
unsupported shape
version mismatch
integrity/hash mismatch
I/O boundary failure
```

Preserve the first causal failure position where the current API supports it.

## Conversation graph semantics

Do not assume:

- every mapping node has a message;
- every message has text;
- a unique root always exists;
- the graph is connected or acyclic;
- timestamps have one stable type.

Detect/report the cases supported by the current implementation/tests, including null messages, duplicate IDs, cycles/dangling parents, unsupported content, timestamp mismatch, invalid UTF-8, and unknown fields.

Unknown or unsupported structure is evidence. It is not automatic schema proof and must not be silently discarded.

## Integrity

- CRC32C is valid for fast frame/header corruption checks where specified; do not call it cryptographic identity.
- BLAKE3 claims require the actual pinned implementation/vectors/gates declared by the repository.
- Do not invent cryptographic code or silently replace one hash with another.

## Tests

For parser/extractor changes, select applicable tests covering chunk boundaries, truncation, limits, malformed input, deterministic output, corruption rejection, serialization bytes and architecture/compiler constraints.

Optimization comes after golden/reference behavior. SIMD/ASM backends must preserve the declared equivalence to the portable reference.

## Handoff

Report exact format version affected, memory model, commands actually run, unsupported structures, and any change to ordering, IDs, timestamps, byte ranges or artifact identity.
