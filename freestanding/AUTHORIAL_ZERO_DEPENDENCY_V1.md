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
- `tools/rafbbs/rafbbs_baremetal.h` — fixed-buffer output and binary manifest;
- `tools/rafbbs/rafbbs_crc32_core.h` — pure CRC32;
- `tools/rafbbs/rafbbs_sha256_core.h` — pure SHA-256;
- `tools/rafbbs/tests/rafbbs_zero_dependency_test.c` — known-vector falsifier;
- `tools/rafbbs/tests/rafbbs_time_core_test.c` — monotonic arithmetic falsifier.

The hosted file wrappers remain deliberately outside the pure core:

- `tools/rafbbs/rafbbs_crc32.h`;
- `tools/rafbbs/rafbbs_sha256.h`;
- `tools/rafbbs/rafbbs_time_posix.h`;
- POSIX CLI/log/file adapters.

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
RafBBS context/monotonic-time value layer prepared for `-nostdinc` compilation,
with POSIX clock acquisition isolated behind an adapter.

`F_gap` = exact-head provider CI for this successor, physical execution and
repository-wide component migration are not yet evidence-bound.

`F_next` = execute the exact-head zero-dependency + semantic time falsifier;
only after PASS select the next smallest runtime-bearing cut.
