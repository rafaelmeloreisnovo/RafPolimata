# AGENTS.md — RAFAELIA Freestanding L0

Scope: everything under `freestanding/`.

This file narrows the repository-wide `AGENTS.md`. It is the canonical maintenance contract for the OS-agnostic L0. When rules conflict, preserve the stricter evidence and zero-runtime boundary.

## Read order

Before editing L0:

1. repository `AGENTS.md` and `docs/AGENTES.md`;
2. this file;
3. `README.md`, `ARCH_CONTRACT.md`, `ABI_CONTRACT.md`, `REGISTER_TOPOLOGY.md`;
4. `FLAGS.md`, `NO_SHADOW_NO_TAIL.md`, `COMMENT_CONTRACT.md`, `STUB_POLICY.md`, `GAPS.md`;
5. the tests/gates under `freestanding/tests/`.

## Non-negotiable L0 invariants

```text
OS dependency          = 0
libc / hosted headers  = 0
heap / malloc / GC     = 0
syscall / OS ABI       = 0
hidden scalar tail     = 0
undeclared shadow      = 0
hidden retry           = 0
external hot helper    = 0
```

- `void` is used where caller-owned state makes return-object traffic unnecessary; it is not treated as a magic optimization by itself.
- Prefer compile-time constants, fixed-width macros, forced `static inline`, then inline assembly.
- Raw hexadecimal opcodes are last resort only when the selected assembler cannot express the required instruction. Document ISA, encoding, readable mnemonic, operands and clobbers.
- A branch used only for value selection should prefer mask/select lowering when the generated code is better or equal. A branch may transfer ownership to the next pipeline stage.
- Never introduce a compatibility loop merely to consume residual lanes. Residual ownership must stay explicit.
- Do not raise the baseline ISA silently. NEON/SVE/SME, SSE/AVX/AVX-512/AMX and RVV are explicit build profiles.
- Do not confuse ISA profiles with host ABI. SysV, Windows x64, AAPCS/EABI and OS entry/exit rules stay outside generic L0.
- Register aliases are geometry views, not independent storage: XMM/YMM/ZMM, S/D/Q, V/Q/D/S/H/B, FPR/VR/VSR and similar families must not be double-counted.
- Privileged/control/debug/PMU/virtualization/security registers may be described as topology but generic L0 must not access them without a separately gated privilege contract.
- Metadata compilation is not executor evidence. POWER/LoongArch/s390x metadata may be build-proven while their instruction executors remain `TOKEN_VAZIO (CLOSURE_L11)`.

## Stub rule

A stub may exist only to preserve structure, compile-time contracts or documentation boundaries. It must never report success for unavailable behavior.

Allowed forms:

- compile-time `#error` for unsupported target;
- explicit capability/state marker;
- declaration-only contract not linked into production;
- test/probe symbol under `tests/`.

Forbidden forms:

- no-op implementation presented as working;
- fake return value that means success;
- hidden fallback to hosted/runtime code;
- silent scalar tail;
- silent allocation or retry.

Unknown runtime/device evidence remains `TOKEN_VAZIO (CLOSURE_L12)`.

## File-comment contract

Every canonical L0 implementation header must carry the marker `RAFAELIA-L0-FILE-CONTRACT` and document:

```text
PURPOSE
SCOPE
PRECONDITIONS
REGISTER_OWNERSHIP
CLOBBERS
MEMORY_ORDER
TAIL_SHADOW
EVIDENCE
```

Comments are part of the engineering contract: update them in the same change when semantics, clobbers, ISA floor or evidence changes.

## Required gates

For a source change, run when applicable:

```sh
sh freestanding/tests/verify_contract.sh
sh freestanding/tests/verify_matrix.sh
sh freestanding/tests/verify_profiles.sh
sh freestanding/tests/verify_scalable.sh
sh freestanding/tests/verify_matrix_accel.sh
sh freestanding/tests/verify_register_metadata.sh
```

- `verify_profiles.sh`: SSE2/AVX2/AVX-512/K/NEON/Advanced-SIMD fixed-vector codegen.
- `verify_scalable.sh`: SVE/RVV one-stage predicate/VL codegen.
- `verify_matrix_accel.sh`: direct AMX TMM and SME ZA register ownership/codegen; it does not imply complete matrix data paths.
- `verify_register_metadata.sh`: POWER64/LoongArch64/s390x metadata only; it must never be reported as executor PASS.

Codegen-sensitive changes must reject unexpected calls, stack traffic and unresolved helpers rather than documenting them away.

Do not call a physical architecture/runtime proven until a same-scope receipt exists. `TOKEN_VAZIO (CLOSURE_L12)` is valid and preferable to an invented PASS.

## Handoff

Report:

```text
F_ok   = implemented and actually verified
F_gap  = open capability/evidence gaps
F_next = smallest reproducible next gate
```
