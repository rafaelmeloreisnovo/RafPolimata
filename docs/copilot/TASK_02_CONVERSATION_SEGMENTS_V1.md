# TASK 02 — DETERMINISTIC CONVERSATION SEGMENTS V1

## Objective

Extend the already-tested streaming scanner into a complete first extraction pipeline that emits deterministic, versioned, read-only conversation and message segments from a real OpenAI `conversations.json` export.

The implementation must remain bounded-memory, freestanding-oriented and independent of input chunk size.

Do not replace `raf_convscan` with a DOM parser. Reuse and extend the existing state machine or build a second bounded extractor layer that consumes scanner events.

---

## 1. Required outputs

The first complete run must generate:

```text
source.manifest.json
conversations.segment
messages.segment
timeline.segment
audit.jsonl
checkpoint.state
coverage_report.json
```

Optional in the same PR only if fully tested:

```text
relations.segment
objects.segment
```

Do not generate empty placeholder files and call the phase complete.

---

## 2. Proposed source map

Create or adapt these files according to existing project conventions:

```text
runtime/conversation_indexer/
  raf_json_events.h
  raf_json_events.c
  raf_convextract.h
  raf_convextract.c
  raf_segment_v1.h
  raf_segment_writer.c
  raf_segment_reader.c
  raf_timeline_v1.h
  raf_timeline_writer.c
  convsegment_cli.c
  test_json_events.c
  test_convextract.c
  test_segment_v1.c
  test_timeline_v1.c
  fuzz_convextract.c
  fixtures/
    minimal.json
    branches.json
    null_message.json
    dangling_parent.json
    duplicate_id.json
    mixed_content.json
```

Do not create multiple competing abstractions for the same scanner. Prefer a single event interface that can be used by counting, extraction and future matrix/log parsers.

---

## 3. Event interface

The tokenizer/parser must emit bounded events without allocating source strings.

Suggested shape:

```c
typedef enum rje_event_kind {
    RJE_OBJECT_BEGIN = 1,
    RJE_OBJECT_END,
    RJE_ARRAY_BEGIN,
    RJE_ARRAY_END,
    RJE_KEY,
    RJE_STRING,
    RJE_NUMBER,
    RJE_TRUE,
    RJE_FALSE,
    RJE_NULL
} rje_event_kind;

typedef struct rje_span {
    rcs_u64 source_offset;
    rcs_u32 byte_length;
    rcs_u32 flags;
} rje_span;

typedef struct rje_event {
    rcs_u32 kind;
    rcs_u32 depth;
    rje_span raw;
    rcs_u64 ordinal;
} rje_event;
```

The event must identify source byte ranges even when the token crosses chunks. If a token is too large for the temporary buffer, the parser must stream/copy it into a caller-provided sink or reject it with a distinct capacity error; silently truncating strings is forbidden.

Required flags:

```text
VALID_UTF8
HAS_ESCAPE
HAS_UNICODE_ESCAPE
NORMALIZATION_REQUIRED
TRUNCATED_FOR_PREVIEW_ONLY
```

`TRUNCATED_FOR_PREVIEW_ONLY` may never be used for persisted identity or content extraction.

---

## 4. Binary segment v1

### 4.1 File header — exactly 64 bytes

Use explicit little-endian serialization. Do not depend on compiler padding.

```c
struct raf_segment_header_v1 {
    uint8_t  magic[8];          /* "RAFSEG1\0" */
    uint32_t version;           /* 0x00010000 */
    uint32_t flags;
    uint64_t record_count;
    uint64_t index_offset;
    uint64_t payload_offset;
    uint64_t source_size;
    uint32_t source_crc32c;
    uint32_t header_crc32c;
    uint8_t  reserved[8];
};
```

Serialized size: 64 bytes.

Rules:

- reserved bytes are zero;
- header CRC is computed with `header_crc32c` serialized as zero;
- offsets are absolute from file start;
- sections must not overlap;
- reader checks integer overflow before `offset + length`;
- incompatible major version is rejected;
- source BLAKE3 is stored in the manifest and in a versioned extension section if available.

### 4.2 Conversation record — exactly 96 bytes

```c
struct raf_conversation_record_v1 {
    uint8_t  id_hash[32];
    uint64_t title_offset;
    uint32_t title_length;
    uint32_t flags;
    int64_t  create_time_us;
    int64_t  update_time_us;
    uint64_t first_message_index;
    uint64_t message_count;
    uint64_t source_start;
    uint64_t source_end;
};
```

Serialized size: 96 bytes.

### 4.3 Message record — exactly 128 bytes

```c
struct raf_message_record_v1 {
    uint8_t  id_hash[32];
    uint8_t  parent_id_hash[32];
    uint32_t role;
    uint32_t content_type;
    uint64_t ordinal;
    int64_t  create_time_us;
    uint64_t text_offset;
    uint32_t text_length;
    uint32_t flags;
    uint64_t source_start;
    uint64_t source_end;
    uint8_t  reserved[8];
};
```

Serialized size: 128 bytes.

### 4.4 String pool

- concatenated UTF-8/source bytes;
- no implicit NUL terminator;
- every offset/length validated;
- duplicate strings may be deduplicated only if deterministic and proven not to alter provenance;
- v1 may avoid string deduplication for simpler deterministic closure;
- source ranges remain in records regardless of string-pool strategy.

### 4.5 Roles

Stable numeric enum:

```text
0 UNKNOWN
1 SYSTEM
2 USER
3 ASSISTANT
4 TOOL
5 DEVELOPER
6 OTHER
```

Do not persist native enum width. Serialize `uint32_t`.

### 4.6 Content types

Stable numeric enum, minimum:

```text
0 UNKNOWN
1 TEXT
2 MULTIMODAL_TEXT
3 CODE
4 IMAGE_REFERENCE
5 AUDIO_REFERENCE
6 FILE_REFERENCE
7 TOOL_RESULT
8 OTHER
```

Unsupported content is represented and counted; it is not silently discarded.

---

## 5. Identity

### 5.1 Conversation ID

Preferred identity bytes:

```text
BLAKE3("conversation-id\0" || exact UTF-8 id bytes)
```

If the source ID is absent:

```text
BLAKE3("conversation-fallback\0" || source_hash || source_start || source_end)
```

Set a `FALLBACK_ID` flag.

### 5.2 Message ID

Preferred:

```text
BLAKE3("message-id\0" || exact UTF-8 id bytes)
```

Fallback must include conversation identity and deterministic node/source position.

### 5.3 Parent identity

Missing parent uses all-zero hash plus `NO_PARENT` flag. A dangling non-empty parent uses its hash plus `DANGLING_PARENT` evidence; do not convert it to root.

### 5.4 Cryptographic implementation gate

Before coding identity:

1. search repository for BLAKE3;
2. inspect license and tests;
3. if absent, add a pinned audited implementation with upstream commit and license manifest;
4. add official BLAKE3 vectors;
5. run on host, ARM32 and ARM64;
6. never substitute CRC32C while naming the field BLAKE3.

Until BLAKE3 is integrated, the extraction PR may remain `PENDING`, but it may not publish a v1 identity format falsely.

---

## 6. JSON extraction semantics

### 6.1 Conversation recognition

A top-level array element is a conversation candidate when it is an object containing one or more expected fields. Preserve unknown top-level elements as coverage anomalies.

Extract when present:

```text
id
title
create_time
update_time
mapping
current_node
conversation_template_id
gizmo_id
is_archived
```

Only v1 persisted fields enter the fixed record. Additional fields go to audit/coverage or a future extension section.

### 6.2 Mapping graph

Each `mapping` property is an object keyed by node ID.

For every node capture:

```text
node ID
parent node ID
children IDs
message object or null
source ranges
```

The extractor must not assume object iteration order is graph order.

After extraction, validate:

- duplicate node ID;
- dangling parent;
- child-parent mismatch;
- cycle;
- multiple roots;
- no root;
- unreachable nodes;
- current_node missing;
- message ID duplicate within and across conversations.

The traversal order for message records must be deterministic. Recommended order:

1. selected/current branch from root when resolvable;
2. remaining branches in lexical order of node ID hash;
3. unreachable nodes in lexical order of node ID hash.

Document and golden-test the chosen order. Never use hash-table iteration order.

### 6.3 Content parts

For text content:

- concatenate string parts using a deterministic separator only if the source model semantically defines multiple parts;
- alternatively persist each part as a future child record;
- for v1, clearly choose and test one rule;
- non-string parts are counted and represented as unsupported, not coerced through generic string conversion.

### 6.4 Numbers and timestamps

Accept JSON integer or finite decimal forms that can be converted safely.

Canonical target:

```text
signed epoch microseconds
```

Rules:

- detect overflow;
- reject NaN/Infinity because valid JSON does not contain them;
- preserve unknown as `INT64_MIN`;
- track whether precision was lost;
- keep original token range in temporal evidence;
- never treat missing/null as zero.

---

## 7. Timeline segment

Minimum timeline record:

```c
struct raf_time_record_v1 {
    uint8_t  entity_hash[32];
    uint32_t domain;
    uint32_t state;
    int64_t  normalized_time_us;
    uint64_t source_start;
    uint64_t source_end;
    uint64_t contradiction_group;
    uint32_t flags;
    uint32_t reserved;
};
```

Domains:

```text
JSON_CONVERSATION_CREATE
JSON_CONVERSATION_UPDATE
JSON_MESSAGE_CREATE
DRIVE_CREATED
DRIVE_MODIFIED
ZIP_ENTRY
INGEST_OBSERVED
STRUCTURAL_ORDINAL
```

Drive/ZIP values arrive through the runtime job/source manifest, not by inventing them inside the parser.

Contradiction grouping must be deterministic and must not choose a winner silently.

---

## 8. Checkpoint/resume

A checkpoint must include:

```text
format version
parser version
source size
source CRC32C and BLAKE3 when available
input byte offset
JSON depth and stack state
conversation ordinal
record counts
output committed sizes
rolling output hashes
safe-boundary marker
```

Do not checkpoint halfway through an unbounded token unless the token state is fully serializable and tested.

Atomicity:

- write checkpoint to temporary path;
- flush;
- rename atomically;
- outputs append only to committed sizes;
- on resume, truncate any uncommitted suffix after verifying hashes/state;
- changed source identity rejects resume.

Test interruption at multiple safe points and prove the final artifact hashes equal an uninterrupted run.

---

## 9. Manifest and coverage

### 9.1 Manifest

Must include:

```text
schema_version
parser_version
source name/locator
source size
source CRC32C
source BLAKE3
input chunk size
start/end observed times
peak arena/buffer usage
conversation count
message count
segment sizes
segment hashes
checkpoint/resume count
state
```

### 9.2 Coverage report

Count at minimum:

```text
top-level items
conversation objects
missing IDs
fallback IDs
null mappings
mapping nodes
null messages
text messages
non-text messages
duplicate IDs
dangling parents
cycles
multiple roots
unreachable nodes
invalid timestamps
unknown fields
invalid UTF-8
capacity rejections
```

No count may be fabricated from heuristics without labeling it.

---

## 10. Reader API

A segment is not closed until a separate reader validates it.

Required functions:

```c
int raf_segment_open(const void *bytes, uint64_t size, raf_segment_view *out);
int raf_segment_validate(const raf_segment_view *view);
int raf_segment_get_conversation(const raf_segment_view *view, uint64_t index, raf_conversation_view *out);
int raf_segment_get_message(const raf_segment_view *view, uint64_t index, raf_message_view *out);
int raf_segment_get_string(const raf_segment_view *view, uint64_t offset, uint32_t length, raf_span *out);
```

Reader rules:

- no allocation;
- all section ranges validated once and at access;
- no pointer returned outside mapped bytes;
- corrupt length/offset rejected;
- unknown flags handled according to version policy;
- record count multiplication overflow checked.

---

## 11. Test matrix

### Parser boundaries

- every split point for small fixtures;
- chunk sizes 1, 2, 3, 7, 64, 4096, 1 MiB;
- escaped characters split;
- Unicode escapes split;
- 4-byte UTF-8 sequence split;
- large title/content crossing many chunks.

### Graph cases

- single root chain;
- branch;
- multiple roots;
- dangling parent;
- cycle;
- duplicate node;
- unreachable node;
- null message;
- missing current node.

### Format cases

- golden 64-byte header;
- exact record byte sizes;
- zeroed reserved fields;
- header CRC mismatch;
- overlapping sections;
- offset overflow;
- count overflow;
- truncated record table;
- invalid string-pool range;
- incompatible version.

### Determinism

For every fixture, all chunk sizes must produce identical:

```text
conversations.segment hash
messages.segment hash
timeline.segment hash
manifest semantic fields except timing/observational fields
```

Separate deterministic manifest content from nondeterministic observed timestamps if necessary.

### Toolchain

- Clang;
- GCC;
- ASan;
- UBSan;
- ARM32 freestanding object;
- ARM64 freestanding object;
- undefined-symbol audit;
- forbidden allocator/libc audit;
- fuzz smoke with fixed seed corpus.

---

## 12. CI artifacts

Upload:

```text
convsegment CLI
freestanding objects
small generated segment fixtures
golden hash report
coverage report
fuzz seed corpus manifest
```

Never upload a private real corpus to CI.

---

## 13. Definition of done

This task is complete only when:

- extraction works on fixture and configurable local real files;
- no full JSON is loaded into memory;
- output is byte-identical across chunk sizes;
- reader validates and enumerates all records;
- graph anomalies are represented honestly;
- timeline retains conflicting evidence;
- interruption/resume produces the same final hashes;
- BLAKE3 identity is real and tested or the format remains explicitly pre-v1/PENDING;
- all CI gates pass;
- unsupported structures appear in coverage as `TOKEN_VAZIO`/anomaly, not silent loss.
