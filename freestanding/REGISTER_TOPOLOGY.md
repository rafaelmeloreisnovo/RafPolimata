# RAFAELIA L0 — Priority Register Topology

This map is for scheduling/codegen geometry. Alias views are not counted as independent physical storage. Privileged/debug/control registers are documented but are **not** touched by generic L0 primitives.

## Priority 1 — x86-64 / SSE2 / AVX2 / AVX-512 / AMX

- GPR: `RAX RBX RCX RDX RSI RDI RBP RSP R8..R15`; APX `R16..R31` is a future explicit profile, not the baseline.
- Flow/status: `RIP`, `RFLAGS`.
- FP legacy: `ST0..ST7`, x87 control/status/tag.
- SIMD/vector alias family: `XMM0..15` -> `YMM0..15` -> `ZMM0..15`; AVX-512 64-bit mode extends to `ZMM16..31`.
- Predicate: `K0..K7` with AVX-512; L0 currently gates a native `K1` residual memory path.
- Matrix/tile: `TMM0..TMM7` with AMX-TILE; L0 currently gates direct `TMM0` ownership via `tilezero` with external state/configuration precondition.
- Control/extended state: `MXCSR`, `XCR0`, CR*/DR*/MSR families. Generic L0 does not issue privileged access.

## Priority 2 — i686 / IA-32

- GPR: `EAX EBX ECX EDX ESI EDI EBP ESP`; `EIP` and `EFLAGS` are separate flow/status state.
- FP: x87 `ST0..ST7`.
- MMX: `MM0..MM7` aliases x87 physical state and therefore must not be treated as independent storage.
- SIMD: `XMM0..XMM7` in the explicit SSE2 profile; later AVX views require an explicitly raised ISA floor.
- Segmentation/control/debug: CS/SS/DS/ES/FS/GS, CR*/DR* families. They are outside generic hot-core ownership.

## Priority 3 — ARMv7-A / AArch32 + NEON

- GPR/flow: `R0..R12`, `R13/SP`, `R14/LR`, `R15/PC`.
- Status: APSR/CPSR/SPSR family depending execution mode.
- FP/SIMD alias family: `S0..S31`, `D0..D31`, `Q0..Q15`; aliases are views, not three independent register banks.
- FP status/control: `FPSCR`.
- System/debug/PMU: CP15/system register families are profile/privilege dependent and excluded from generic L0 access.

## Priority 4 — AArch64 / ARM64

`ARM64` and `AArch64` refer to the same 64-bit Arm execution state here.

- GPR: `X0..X30`, with `W0..W30` as 32-bit views; `SP` and zero-register semantics are special.
- Flow/status: PC architectural state, `NZCV`, PSTATE fields.
- FP/SIMD alias family: `V0..V31` with Q/D/S/H/B views.
- SVE: `Z0..Z31`, `P0..P15`, `FFR`; L0 has a one-stage `P0/Z0` path with explicit consumed lanes.
- SME: `ZA`; L0 gates direct `ZA` ownership via `zero {za}` while caller/environment owns SME state enablement. SME2 `ZT0` remains a later extension.
- FP state: `FPCR`, `FPSR`.
- System/MMU/debug/PMU/virtualization/security register families exist at EL0..EL3 as permitted; generic L0 does not assume privilege.

## Priority 5 — RISC-V RV32/RV64

- GPR: `x0..x31` (RV32E/RV64E profiles can reduce the architectural GPR set; the current main L0 profile uses 32).
- Flow: `pc`.
- FP: `f0..f31` when F/D/Q extensions are enabled; `fcsr` state.
- Vector: `v0..v31` when V is enabled. `v0` also participates in mask semantics.
- Vector control: `vl`, `vtype`, `vstart`, `vxrm`, `vxsat`, `vcsr`, `vlenb` as extension-defined CSR state. L0 currently consumes one `VL`-bounded block per stage.
- Privileged/virtualization/debug/PMU: m*/s*/h*/vs* CSR families, PMP and counters; accessibility depends on privilege and extension.

## Priority 6 — POWER64 / VSX / MMA — metadata build-proven

- GPR: `r0..r31`.
- Flow/status: `NIP/PC` architectural flow state plus `CR`, `LR`, `CTR`, `XER`.
- FP/vector aliasing: `FPR0..31`, `VR0..31`, and the unified VSX view `VSR0..63`; these views overlap and are not counted as independent banks.
- Matrix: POWER MMA uses `ACC0..ACC7` accumulators in supporting profiles; executor support is not yet claimed by generic L0.
- Control/system: `MSR`, SPR families, debug, PMU and virtualization state are privilege-dependent.
- Current evidence: OS-neutral metadata object compiles successfully; this is topology evidence, not a VSX/MMA executor claim.

## Priority 7 — LoongArch64 / LSX / LASX — metadata build-proven

- GPR: `r0..r31`; ABI aliases such as zero/ra/tp/sp are views of that bank.
- FP/vector alias family: 32 FP registers; LSX exposes 128-bit views and LASX exposes 256-bit views of the same architectural vector storage family rather than independent banks.
- Flow/status: PC plus floating/status state.
- Control/system: CSR, TLB/MMU, debug, PMU and virtualization families are privilege-dependent and excluded from generic L0 access.
- Current evidence: OS-neutral LoongArch64 metadata object compiles; LSX/LASX executors remain a separate future gate.

## Priority 8 — IBM z / s390x — metadata build-proven

- GPR: `r0..r15` (64-bit).
- Access registers: `AR0..AR15` are a distinct address-space selection class, not GPR aliases.
- Control: `CR0..CR15` plus PSW/system state.
- FP: `FPR0..FPR15`.
- Vector: `VR0..VR31`, 128-bit where the vector facility/profile is enabled; the low-number vector/FP state has architectural overlap and must not be double-counted.
- Current evidence: OS-neutral s390x metadata object compiles; vector execution remains separately gated.

## Next metadata/executor priorities

1. POWER VSX/MMA executor profiles;
2. LoongArch LSX/LASX executor profiles;
3. IBM z vector executor profile;
4. Qualcomm Hexagon/HVX topology/executor;
5. ARM M-profile + MVE/Helium topology/executor;
6. x86 APX `R16..R31` when the available assembler/compiler profile can gate it independently.

Metadata success never promotes an executor claim; executor support requires its own instruction/codegen gate.