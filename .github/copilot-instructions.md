# GitHub Copilot Instructions — RafPolimata

## Repository role

RafPolimata is the deterministic low-level compiler/parser/indexer layer of the RAFAELIA ecosystem.

Its responsibility is to transform large raw sources into bounded, versioned, provenance-preserving artifacts that other components can consume safely.

Primary current vertical:

```text
conversations.json
  -> streaming JSON state machine
  -> conversation/message records
  -> deterministic binary segments
  -> temporal evidence
  -> manifest/audit/checkpoint
```

Read these before changing the conversation pipeline:

- `docs/RAFAELIA_DATA_INGEST_INDEX_PROTOCOL.md`
- `include/rafaelia_runtime_protocol.h`
- `runtime/conversation_indexer/raf_convscan.h`
- `runtime/conversation_indexer/raf_convscan.c`
- `runtime/conversation_indexer/test_convscan.c`
- `.github/workflows/conversation-indexer-ci.yml`

The current scanner already passed Clang, GCC, ASan, UBSan, ARM32, ARM64 and a realistic fixture. Extend it incrementally. Do not discard it for a DOM parser or a dynamic language implementation.

---

## Non-negotiable engineering rules

### Core runtime

- C first.
- No `malloc`, `calloc`, `realloc` or `free` in the freestanding core.
- No hidden allocator.
- No unbounded recursion.
- No libc calls in the freestanding core.
- Hosted CLIs may use libc only as an outer I/O shell.
- All buffers have pointer, current length and explicit capacity.
- All loops have a bound derived from input length or declared maximum.
- All persisted structures use explicit-width integers.
- Never persist compiler padding implicitly; serialize fields deliberately or assert exact layout.

### Strings

- Treat source strings as byte spans.
- Never assume NUL termination.
- Persist strings in a string pool as `offset + uint32 length` after checking the length fits.
- Track UTF-8 validity separately.
- Preserve source byte ranges even when a normalized view is generated.
- Never use unbounded `strcpy`, `strcat`, `sprintf` or `%s` on source data.

### Large files

- Stream input in caller-selected chunk sizes.
- Output must be invariant across chunk sizes.
- Never require the complete JSON document in memory.
- Checkpoint only at parser-safe boundaries.
- A resumed run must either reproduce the same artifact bytes or reject the checkpoint explicitly.

### Error model

- Return deterministic negative error codes.
- Preserve the first causal failure position.
- Distinguish syntax error, truncation, capacity exhaustion, unsupported shape, version mismatch, hash mismatch and I/O boundary failure.
- Do not return `RCS_OK` or `VERIFIED` for partial outputs.

### Optimization

- Correct portable C is the reference.
- SIMD/ASM may be added only after golden outputs exist.
- Every optimized backend must prove byte-for-byte or declared-tolerance equivalence.
- Do not remove bounds checks from parsers for speed.

---

## Content identity and integrity

Target identity is BLAKE3-256.

Rules:

- First audit the repository for an existing implementation.
- Never generate or invent cryptographic code.
- If vendoring is required, pin exact source revision and license.
- Add official golden vectors and cross-architecture tests.
- CRC32C remains useful for fast corruption detection and frame/header checks; it is not a cryptographic identity.

Each source and artifact must record:

```text
source locator
source byte size
source content hash
parser version
schema version
chunk size used for the run
artifact byte size
artifact content hash
record counts
first failure or completion state
```

Chunk size must not affect artifact identity.

---

## Binary format discipline

Every persisted format requires:

- 8-byte magic;
- major/minor version;
- flags;
- explicit little-endian encoding in v1;
- record count;
- section offsets and lengths;
- source identity;
- header integrity field;
- reserved bytes initialized to zero;
- reader rejection of unknown incompatible major versions;
- static assertions for in-memory helper structs when direct layout is used;
- golden binary fixtures checked into tests only when small.

Never change an existing layout in place. Add a new version or a backward-compatible reader.

---

## Conversation JSON semantics

The OpenAI export shape is graph-like, not a flat list.

Preserve:

- conversation ID;
- title;
- claimed create/update times;
- mapping node ID;
- message ID;
- parent/child relationships;
- author role/name/metadata when present;
- content type;
- content parts;
- message claimed time;
- source byte range;
- ordinal assigned by deterministic traversal;
- unknown fields as skipped-but-counted evidence, not silent schema proof.

Do not assume every mapping node has a message. Do not assume every message has text. Do not assume every conversation tree is connected or acyclic. Detect and report:

- missing root;
- multiple roots;
- dangling parent;
- cycle;
- duplicate ID;
- null message;
- non-text content;
- timestamp type mismatch;
- invalid UTF-8;
- unsupported nested content.

These are evidence states, not automatic fatal errors unless they violate output invariants.

---

## Temporal evidence

Do not overwrite or collapse dates.

Represent each timestamp observation with:

```text
entity_id
domain
raw representation
normalized epoch microseconds when possible
source byte range
confidence/state
contradiction group
```

Clock domains include at minimum:

- JSON claimed create time;
- JSON claimed update time;
- ZIP entry time when provided by upstream input;
- Drive created/modified time supplied in the job manifest;
- ingestion observed time;
- structural order.

Unknown normalized time uses `INT64_MIN`; it must not become zero epoch.

---

## Testing requirements

Every parser/extractor change must test:

- every byte boundary as a possible chunk split for a bounded fixture;
- one-byte chunks;
- large chunks;
- strings and escapes split across chunks;
- Unicode escape split across chunks;
- numbers split across chunks;
- nested object/array boundaries;
- truncation at representative positions;
- depth limit;
- capacity limit;
- malformed escape;
- invalid close delimiter;
- duplicate IDs;
- cycles/dangling parents for extractor stage;
- deterministic artifact hash across chunk sizes;
- little-endian encoding bytes;
- reader rejection of corrupt headers/sections;
- ASan and UBSan;
- Clang and GCC;
- ARM32 and ARM64 freestanding objects;
- no undefined symbols in the core object;
- no forbidden allocator/libc symbol strings in the core object.

Fuzzing must have explicit memory and time budgets and a reproducible seed corpus.

---

## CI and PR rules

Do not weaken existing workflows.

A PR must report:

- source files inspected;
- exact format version affected;
- memory model;
- input and output size limits;
- commands executed;
- test matrix result;
- artifact hashes;
- unsupported cases marked `TOKEN_VAZIO`;
- any semantic change to record ordering or identity.

Reject changes that:

- introduce a dynamic JSON tree;
- parse the full corpus in Python/Kotlin/Java;
- silently normalize dates;
- create variable-size persisted C structs with pointers;
- rely on native enum size;
- omit versioning;
- claim BLAKE3 while only computing CRC32C;
- merge semantically similar conversations automatically;
- emit partial segments as `VERIFIED`.

---

## Definition of done

The next conversation-indexing phase is complete only when:

- conversations and messages are extracted from a streamed input;
- source byte ranges are preserved;
- output segment bytes are deterministic across chunk sizes;
- a reader validates and enumerates the generated segments;
- temporal evidence is preserved without silent correction;
- interruption/resume is tested at safe boundaries;
- CI passes host, sanitizers, ARM32 and ARM64;
- artifacts and hashes are published by CI;
- all unsupported structures are counted and reported honestly.
