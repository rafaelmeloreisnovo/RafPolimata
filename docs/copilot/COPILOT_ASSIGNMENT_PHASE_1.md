# COPILOT ASSIGNMENT — PHASE 1

Copy the block below into GitHub Copilot Coding Agent, or point Copilot to this file and ask it to execute the assignment exactly.

---

## PROMPT

You are working in `rafaelmeloreisnovo/RafPolimata`.

Your task is to implement **Phase 1: deterministic conversations/messages/timeline segments v1**.

Before changing code, read completely:

1. `.github/copilot-instructions.md`
2. `docs/copilot/TASK_02_CONVERSATION_SEGMENTS_V1.md`
3. `docs/RAFAELIA_DATA_INGEST_INDEX_PROTOCOL.md`
4. `include/rafaelia_runtime_protocol.h`
5. every file under `runtime/conversation_indexer/`
6. `.github/workflows/conversation-indexer-ci.yml`

Then inspect the current default branch and write a reconnaissance section in your PR description listing the real files, functions, build commands and current limitations. Do not invent paths or claim tests you did not execute.

Implement the complete vertical. The deliverable must parse a streamed OpenAI `conversations.json` without loading the whole file into memory and must generate real, non-empty:

- `source.manifest.json`
- `conversations.segment`
- `messages.segment`
- `timeline.segment`
- `audit.jsonl`
- `checkpoint.state`
- `coverage_report.json`

Implement:

- bounded JSON event emission;
- conversation and message extraction;
- deterministic graph traversal;
- duplicate/dangling/cycle/root anomaly reporting;
- exact source byte ranges;
- signed epoch-microsecond temporal evidence without converting missing values to zero;
- fixed, versioned little-endian binary segments;
- separate no-allocation segment reader/validator;
- BLAKE3-256 identity using a real audited/pinned implementation and official golden vectors;
- CRC32C for header/checkpoint integrity only;
- checkpoint/resume at safe parser boundaries;
- deterministic final artifact hashes across chunk sizes and after resume.

Core constraints:

- C low-level implementation;
- no malloc/calloc/realloc/free in the freestanding core;
- no hidden allocator;
- no libc in the freestanding core;
- no unbounded recursion;
- all persisted values have explicit width and endian encoding;
- strings are offset+length byte spans, never persisted pointers and never dependent on NUL termination;
- all capacities and overflow cases are checked;
- no silent timestamp correction;
- no semantic fusion/deduplication;
- no placeholder files;
- no partial output marked `VERIFIED`.

Run and report all required gates:

- Clang;
- GCC;
- ASan;
- UBSan;
- ARM32 freestanding object;
- ARM64 freestanding object;
- undefined-symbol audit;
- forbidden-allocation/libc audit;
- chunk split tests including one-byte chunks;
- corrupt segment/header/range tests;
- deterministic artifact hash tests;
- interruption/resume equivalence;
- fixed-seed fuzz smoke.

Update CI so these gates execute automatically and upload only small synthetic artifacts, reports and compiled tools. Never upload private corpus data.

Do not stop at a plan. Implement, test, correct failures and open a PR only after the focused CI is green. If a blocker cannot be resolved, leave the implementation in an honest non-verified state and document the exact blocker as `TOKEN_VAZIO`.

The PR description must contain:

- objective;
- files inspected;
- files changed;
- exact binary layouts and sizes;
- memory model and limits;
- cryptographic source/version/license;
- commands actually executed;
- CI run results;
- artifact hashes;
- unsupported structures and `TOKEN_VAZIO` gaps;
- compatibility impact for Termux and LlamaRafaelia consumers.

Do not redesign the architecture. Execute the architecture already defined in the repository documents.

---
