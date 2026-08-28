# RAF Hash Fabric V1

**Status:** `IMPLEMENTED` (source only; runtime evidence is `TOKEN_VAZIO`)  
**Owner:** `low-level-methods-maintainer`  
**Parent:** `README.md -> docs/INDEX.md -> this directory`  
**Base contract:** `Benchmark/raf_runtime_router.h` and `Benchmark/raf_types.h`

A seven-language, freestanding-oriented execution fabric for deterministic hash/Merkle workloads. It does **not** redefine MD5, SHA-2, SHA-3, BLAKE3, AES, Ed25519, or any other cryptographic primitive. Algorithm block sizes, rounds, constants and security boundaries remain those of their standards/reference implementations.

## Seven implementation languages

1. C11 freestanding (`c/`)
2. C++20 freestanding subset (`cpp/`)
3. Rust `#![no_std]` (`rust/`)
4. Zig freestanding (`zig/`)
5. D `-betterC` (`d/`)
6. Forth core (`forth/`)
7. Assembly (`asm/`) with ARMv7/AArch64/x86-64 kernels

The implementations share one small control contract: capability profile, fixed-point control values, table-driven state machine, watchdog, fail-safe/failover/rollback/failback and a 16-leaf Merkle scheduling primitive.

## Critical invariants

- `Q32.32` / `Q16.16` are **control-plane fixed-point formats**, not replacements for cryptographic word sizes.
- SIMD increases parallel independent work; it never shrinks a standard algorithm block. For 32-bit words: NEON-128 = 4 lanes, AVX2-256 = 8 lanes, AVX-512 = 16 lanes.
- Merkle reduction is binary: a 16-leaf batch reduces `16 -> 8 -> 4 -> 2 -> 1`. Hashing of each leaf/node must use the selected validated primitive.
- `TOKEN_VAZIO` means evidence/implementation is missing for a requested backend; it is not a cryptographic value.
- Reference scalar behavior is the oracle. Optimized backends must match it bit-for-bit before promotion to `VALIDATED`.
- No heap, malloc, GC or hidden runtime is permitted in the hot path of freestanding targets.

## State machine

`VOID -> PROBE -> BASELINE -> CANDIDATE -> VALIDATED`

Fault events can transition to `DEGRADED -> FAILOVER -> ROLLBACK -> FAILBACK -> VALIDATED`, or to `SAFE` if recovery is not proven. The transition table is explicit and bounded.

## SIMD/Merkle scheduling

For a 32-bit cryptographic word:

```text
scalar       1 lane
NEON 128     4 lanes
AVX2 256     8 lanes
AVX-512     16 lanes
```

This is scheduling parallelism, not a change from (for example) a 512-bit message block to 96 bits. BLAKE3, SHA-256 and other algorithms keep their specified block/chunk/state dimensions.

## Licensing and commercial boundary

Original files enumerated in `LICENSE_SCOPE_V1.json` are offered under **PolyForm Noncommercial License 1.0.0** through `LICENSE.md`. The scope is module-specific; it does not relicense unrelated RafPolimata material or any third-party content.

The authorization source for this module is deliberately limited to three repository surfaces:

1. `README.md` — human-readable orientation and response codes;
2. `LICENSE.md` — license notice and required notice;
3. `LICENSE_SCOPE_V1.json` — machine-readable scope and real file inventory.

No school, teacher, student, laboratory or researcher registration is required for a use that is already permitted by the applicable license. No approval database, allow-list, application form or personal-data collection is created by this authorization model.

The summary below does **not** expand, replace or override the PolyForm license. If the summary and the canonical license differ, the canonical license controls.

## Simplified authorization — schools and researchers

### School / educational institution

**Response: `EDU-PERMITTED`** when the use is by an educational institution and concerns material listed as original/covered in `LICENSE_SCOPE_V1.json`, subject to the license terms.

Typical examples include classroom teaching, coursework, educational laboratories, demonstrations, study, internal educational adaptation, and redistribution for the permitted purpose when the license's notice obligations are preserved.

No prior email, individual registration or authorization number is required. If software is copied or redistributed, recipients must receive the PolyForm terms or its canonical URL and the `Required Notice` supplied in `LICENSE.md`.

### Researcher / research organization

**Response: `RESEARCH-PERMITTED`** when the use is permitted by the PolyForm license, including personal research/experiment/testing without anticipated commercial application and use by a public research organization under the license terms.

Researchers may study, modify, test, benchmark, reproduce experiments and publish findings. If they distribute copies or modified software, the applicable license and notice obligations travel with the software.

No researcher registry is required. For reproducibility, citing the repository, revision/commit and file path is recommended but is not a substitute for the license conditions.

### When the answer is not automatically “permitted”

**Response: `REVIEW-REQUIRED`** when the facts do not clearly fit a permitted purpose, for example:

- research performed by or for a commercial business where commercial advantage/application is anticipated;
- a paid product, SaaS, resale, managed production service or commercial deployment;
- uncertainty about whether a file is original RAF material or separately licensed third-party material;
- redistribution that removes or conflicts with required notices or upstream terms;
- a request for rights not granted by the applicable copyright/patent license, or for trademark/brand rights.

**Response: `SEPARATE-LICENSE`** when the intended commercial/production use is outside the PolyForm Noncommercial grant. A separate written commercial agreement is then required from the applicable rightsholder(s).

### Canonical short answers

```text
EDU-PERMITTED
Uso educacional coberto, quando realizado dentro dos termos da licença aplicável.
Não exige cadastro individual. Preserve LICENSE/URL e Required Notice ao redistribuir.

RESEARCH-PERMITTED
Uso de pesquisa coberto, quando realizado dentro dos termos da licença aplicável.
Não exige cadastro de pesquisador. Preserve os termos e avisos ao redistribuir.

REVIEW-REQUIRED
Os fatos informados não permitem classificar o uso com segurança apenas pelo README.
Verifique LICENSE.md + LICENSE_SCOPE_V1.json e os termos próprios de qualquer terceiro.

SEPARATE-LICENSE
O uso pretendido está fora da concessão não comercial ou pede direitos adicionais.
É necessário acordo escrito separado com o(s) titular(es) aplicável(is).
```

## Real inventory — no parallel registry

`LICENSE_SCOPE_V1.json` is the inventory for this module. At the current PR scope it classifies **17 real files**:

- **14 covered original paths**;
- **2 license/authorization metadata files**: `LICENSE.md` and `LICENSE_SCOPE_V1.json`;
- **1 evidence-only receipt**: `evidence/BLAKE3_PORTABLE_LOCAL_KAT_2026-08-28.md`.

The evidence receipt is not a persisted BLAKE3 implementation and must not be interpreted as one. Third-party rights are determined from the third-party material's own license/SPDX/notice; an unclear third-party status yields `REVIEW-REQUIRED`, not an invented permission.

## Evidence boundary

Source presence = `IMPLEMENTED` only. The original change that introduced this directory did not execute compiler, ISA, device, benchmark or cryptographic vectors.

A later working-container experiment produced a **local** BLAKE3 known-answer-test PASS, documented at `evidence/BLAKE3_PORTABLE_LOCAL_KAT_2026-08-28.md`. The BLAKE3 implementation itself was not persisted to the repository because the available repository write path was blocked by a safety gate. Therefore current-commit BLAKE3 implementation/runtime evidence remains `TOKEN_VAZIO`.

## F_next

1. Compile/rerun the existing C/C++/ISA gates from the exact PR commit in an available build environment.
2. Persist the reviewed portable BLAKE3 oracle through an authorized repository path when permitted.
3. Re-run official known-answer vectors from that exact commit.
4. Add NEON 4-way, AVX2 8-way and AVX-512 16-way backends only after scalar equivalence.
5. Record cycles/byte, code size, symbol count, branches and rollback behavior separately.
