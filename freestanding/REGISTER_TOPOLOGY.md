# RAFAELIA L0 — Priority Register Topology

This map is for scheduling/codegen geometry. Alias views are not counted as independent physical storage. Privileged/debug/control registers are documented but are **not** touched by generic L0 primitives.

## Priority 1 — x86-64 / AVX2 / AVX-512 / AMX

- GPR: `RAX RBX RCX RDX RSI RDI RBP RSP R8..R15`; APX `R16..R31` is a future explicit profile, not the baseline.
- Flow/status: `RIP`, `RFLAGS`.
- FP legacy: `ST0..ST7`, x87 control/status/tag.
- SIMD/vector alias family: `XMM0..15` -> `YMM0..15` -> `ZMM0..15`; AVX-512 64-bit mode extends to `ZMM16..31`.
- Predicate: `K0..K7` with AVX-512.
- Matrix/tile: `TMM0..TMM7` with AMX profile.
- Control/extended state: `MXCSR`, `XCR0`, CR*/DR*/MSR families. Generic L0 does not issue privileged access.

## Priority 2 — i686 / IA-32

- GPR: `EAX EBX ECX EDX ESI EDI EBP ESP`; `EIP` and `EFLAGS` are separate flow/status state.
- FP: x87 `ST0..ST7`.
- MMX: `MM0..MM7` aliases x87 physical state and therefore must not be treated as independent storage.
- SIMD: `XMM0..XMM7` when SSE profile is explicitly enabled; AVX profile extends views to YMM where the selected CPU/toolchain permits it.
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
- SVE: `Z0..Z31`, `P0..P15`, `FFR`.
- SME: `ZA`; SME2 may add `ZT0` and multi-vector operations.
- FP state: `FPCR`, `FPSR`.
- System/MMU/debug/PMU/virtualization/security register families exist at EL0..EL3 as permitted; generic L0 does not assume privilege.

## Priority 5 — RISC-V RV32/RV64

- GPR: `x0..x31` (RV32E/RV64E profiles can reduce the architectural GPR set; the current main L0 profile uses 32).
- Flow: `pc`.
- FP: `f0..f31` when F/D/Q extensions are enabled; `fcsr` state.
- Vector: `v0..v31` when V is enabled. `v0` also participates in mask semantics.
- Vector control: `vl`, `vtype`, `vstart`, `vxrm`, `vxsat`, `vcsr`, `vlenb` as extension-defined CSR state.
- Privileged/virtualization/debug/PMU: m*/s*/h*/vs* CSR families, PMP and counters; accessibility depends on privilege and extension.

## Next metadata priorities

After the five executable families above, the next register maps to add are:

1. POWER ISA / VSX / MMA (`GPR`, `VSR0..63`, `ACC0..7`);
2. LoongArch64 (`r0..31`, FP/LSX/LASX aliases, CSR families);
3. IBM z/Architecture (`GPR`, access registers, FPR/vector aliasing, control registers, PSW);
4. Qualcomm Hexagon/HVX;
5. ARM M-profile + MVE/Helium.

These next maps remain metadata/research until an actual freestanding target compiler + codegen gate exists. File presence is not executor support.