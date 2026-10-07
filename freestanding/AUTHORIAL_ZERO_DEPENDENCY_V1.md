# RAFAELIA Authorial Zero-Dependency Contract V1

Status: `IMPLEMENTED_SOURCE / EXECUTION_PENDING_PROVIDER`  
Authority: `RafPolimata` producer repository  
Claim ceiling: source/build boundary only; physical execution remains `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.

## Intent

Move authorial hot cores toward a reconstructible freestanding form in which the
final core object does not require a hosted C runtime, external library, heap,
syscall layer, package manager, or provider SDK.

The boundary is strict:

```text
AUTHORIAL_CORE != HOST_ADAPTER != FACTORY_TOOL != PROVIDER != RUNTIME_DEPENDENCY
```

A compiler/assembler/CI runner may be needed to manufacture or inspect an
object. That does not make it a runtime dependency of the object. Full
self-hosting/bootstrap independence is a separate gate and is not claimed here.

## V1 enforced scope

Canonical L0 is already governed by `freestanding/AGENTS.md` and
`freestanding/tests/verify_contract.sh`.

This successor also closes the first RafBBS slice:

- `tools/rafbbs/rafbbs_types.h` — zero-include exact-width authorial types;
- `tools/rafbbs/rafbbs_status.h` — status vocabulary without hosted headers;
- `tools/rafbbs/rafbbs_time.h` — monotonic value/validity and elapsed arithmetic;
- `tools/rafbbs/rafbbs_freestanding.h` — watchdog/rollback/flags;
- `tools/rafbbs/rafbbs_core.h` — caller-owned context with no hosted time type;
- `tools/rafbbs/rafbbs_context_core.h` — seeded context initialization, bounded text/path composition and watchdog setup without memset/snprintf/libc;
- `tools/rafbbs/rafbbs_command_core.h` — deterministic executed/rc/optional → PASS/SKIP/FAIL/TOKEN_VAZIO policy with limited/failed bits; no command execution;
- `tools/rafbbs/rafbbs_manifest_core.h` — deterministic text-manifest rendering into caller-owned memory;
- `tools/rafbbs/rafbbs_filepicker_core.h` — static catalog + bounded selection state with no libc/string dependency;
- `tools/rafbbs/rafbbs_theme.h` — deterministic status-to-ANSI mapping over the authorial status vocabulary;
- `tools/rafbbs/rafbbs_log_core.h` — deterministic caller-owned log-line rendering with authorial 64-bit divmod and no printf/libgcc helper;
- `tools/rafbbs/rafbbs_format_core.h` — finite typed text/i32/hex32 formatter with no stdarg/vsnprintf/general printf grammar;
- `tools/rafbbs/rafbbs_git_core.h` — deterministic parser for caller-supplied HEAD/OID/packed-ref bytes; no filesystem or Git executable;
- `tools/rafbbs/rafbbs_recent_core.h` — fixed-memory prefix/suffix filter, lexicographic top-10 selector and byte renderer; no shell/find/sort/tail;
- `tools/rafbbs/rafbbs_runlog_core.h` — deterministic persisted run-log header/artifact/gap byte renderer with authorial fixed-width hex and no FILE/fprintf/libc;
- `tools/rafbbs/rafbbs_pipeline_core.h` — pipeline metadata + exact-byte lookup only; no context, status or execution callback;
- `tools/rafbbs/rafbbs_cli_core.h` — deterministic CLI action routing over caller-owned argv; no terminal, clock, filesystem or command execution;
- `tools/rafbbs/rafbbs_tui_core.h` — deterministic single-key TUI action/pipeline routing; no stdio, terminal input, rendering or execution;
- `tools/rafbbs/rafbbs_baremetal.h` — fixed-buffer output and binary manifest value model;
- `tools/rafbbs/rafbbs_manifest_bin_core.h` — deterministic 96-byte little-endian binary-manifest wire codec;
- `tools/rafbbs/rafbbs_crc32_core.h` — pure CRC32;
- `tools/rafbbs/rafbbs_sha256_core.h` — pure SHA-256;
- `tools/rafbbs/tests/rafbbs_zero_dependency_test.c` — known-vector falsifier;
- `tools/rafbbs/tests/rafbbs_time_core_test.c` — monotonic arithmetic falsifier;
- `tools/rafbbs/tests/rafbbs_manifest_core_test.c` — exact-byte, invalid-time and overflow falsifier;
- `tools/rafbbs/tests/rafbbs_filepicker_core_test.c` — catalog, valid selection and invalid-choice preservation falsifier;
- `tools/rafbbs/tests/rafbbs_theme_core_test.c` — status/color mapping and empty-default falsifier;
- `tools/rafbbs/tests/rafbbs_log_core_test.c` — exact line, invalid-time, long-minute and overflow falsifier;
- `tools/rafbbs/tests/rafbbs_format_core_test.c` — text/i32 extrema/fixed-hex/string-length/overflow formatter falsifier;
- `tools/rafbbs/tests/rafbbs_git_core_test.c` — symbolic/detached HEAD, OID, packed-ref, invalid-hex and truncation falsifier;
- `tools/rafbbs/tests/rafbbs_recent_core_test.c` — filter/top-10/order/render/overflow bounded-catalog falsifier;
- `tools/rafbbs/tests/rafbbs_runlog_core_test.c` — exact persisted header/tail bytes, fixed-width CRC/hash, TOKEN_VAZIO gap and overflow falsifier;
- `tools/rafbbs/tests/rafbbs_pipeline_core_test.c` — exact/case-sensitive lookup, flags and not-found falsifier;
- `tools/rafbbs/tests/rafbbs_cli_core_test.c` — help/list/run/invalid and case-sensitive CLI-route falsifier;
- `tools/rafbbs/tests/rafbbs_tui_core_test.c` — quit/list/files/run/default TUI-route falsifier with exact pipeline ids;
- `tools/rafbbs/tests/rafbbs_context_core_test.c` — seeded fields, deterministic paths, watchdog/zero state and truncation falsifier;
- `tools/rafbbs/tests/rafbbs_command_core_test.c` — executed/unexecuted, success, optional failure and required failure policy falsifier;
- `tools/rafbbs/tests/rafbbs_manifest_bin_core_test.c` — binary wire exact-byte/roundtrip/truncation falsifier.

The hosted file wrappers remain deliberately outside the pure core:

- `tools/rafbbs/rafbbs_crc32.h`;
- `tools/rafbbs/rafbbs_sha256.h`;
- `tools/rafbbs/rafbbs_time_posix.h`;
- `tools/rafbbs/rafbbs_manifest.h` — text FILE/filesystem persistence only;
- `tools/rafbbs/rafbbs_manifest_bin.h` — binary FILE/filesystem persistence only;
- `tools/rafbbs/rafbbs_filepicker.h` — hosted presentation/printf adapter only;
- `tools/rafbbs/rafbbs_log.h` — hosted clock observation, console and FILE persistence adapter; live detail formatting delegates to the typed pure formatter and persisted run-log byte layout delegates to the pure runlog core;
- `tools/rafbbs/rafbbs_pipeline.h` — hosted handler binding, command execution and provider/file/hash adapters; result policy delegates to `rafbbs_command_core.h`;
- `tools/rafbbs/rafbbs_git_posix.h` — hosted filesystem adapter for ordinary .git/HEAD, loose-ref and packed-refs observation; no external Git process;
- `tools/rafbbs/rafbbs_recent_posix.h` — hosted directory/stdout adapter for log/manifest catalogs; no shell/find/sort/tail process chain;
- `tools/rafbbs/rafbbs_cli.h` — hosted terminal, wall-clock, filesystem and command dispatch adapter; filesystem provenance observations seed the pure context core;
- `tools/rafbbs/rafbbs_tui.h` — hosted stdio rendering/input adapter that delegates key semantics to `rafbbs_tui_core.h`;
- POSIX file adapters.

The monotonic boundary does not reimplement a provider clock.  POSIX supplies the
observation; the authorial core owns only the value contract and deterministic
elapsed arithmetic. Missing/invalid observation is carried by an explicit
validity bit rather than by interpreting numeric zero as evidence.

## Required invariants

For an item declared `AUTHORIAL_FREESTANDING`:

```text
system/hosted header include = 0
external runtime library     = 0
allocator / heap / GC        = 0
syscall / OS ABI             = 0
provider SDK at runtime      = 0
caller-owned state/buffers   = required
unobserved execution         = TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)
```

The build probe uses `-nostdinc -ffreestanding -fno-builtin`. Passing that
probe establishes only that the selected source slice does not require hosted
headers at compile time. It does not prove physical execution or global
repository closure.

## Migration law

For each remaining subsystem:

```text
CLASSIFY
→ isolate pure semantic/math core
→ replace external runtime dependency with authorial primitive when sustainable
→ move unavoidable host/provider behavior behind an adapter
→ add a falsifier
→ compile with hosted headers disabled
→ inspect unresolved helpers
→ preserve TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE) for unexecuted targets
```

If a replacement would reduce correctness, security, interoperability or
auditability, do not silently rewrite it. Record that item as
`TOKEN_VAZIO / EXTERNAL_BOUNDARY_REQUIRES_AUTHORITY (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)` and keep the boundary
explicit.

## Evidence rule

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`.

V1 is complete only for the paths named above after the exact-head gate runs.
Repository-wide zero-dependency and self-hosting remain open until every
declared runtime surface has an equivalent source/build/runtime receipt.

## R3

`F_ok` = authorial zero-include type/status substrate + pure CRC32/SHA-256 +
RafBBS context/monotonic-time value layer + seeded context/path core + command-result policy core + deterministic text-manifest renderer
+ static filepicker catalog/selection + theme/status + deterministic log-line rendering
+ deterministic persisted run-log byte rendering without fprintf/libc
+ pipeline spec/lookup core + CLI route core + TUI route core + fixed 96-byte binary-manifest wire codec prepared for `-nostdinc` compilation,
with POSIX clock acquisition, filesystem persistence and console presentation isolated behind adapters; live detail formatting, Git provenance byte parsing and recent-catalog selection are typed/authorial; neither the external Git executable nor find/sort/tail are required by these paths.

`F_gap` = exact-head provider CI for this successor, physical execution and
repository-wide component migration are not yet evidence-bound.

`F_next` = execute the exact-head zero-dependency + semantic time/text/binary-manifest
falsifiers, including command-policy, seeded-context, filepicker, theme/status, log-line, pipeline-spec, CLI-route, TUI-route and binary-manifest;
only after PASS select the next smallest runtime-bearing cut.
