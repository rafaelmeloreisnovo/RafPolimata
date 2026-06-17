# ARM64 (AArch64 / ARMv8-A) Architecture Reference

This document describes the ARM64 architecture as used in the RafPolimata project.
It covers the instruction encoders in `Apkc/arch_arm64.h`, the execve bootstrap in
`Apkc/lang_script.h`, and the system-register access patterns in the RAF_024–RAF_028
benchmark files. Encoding constants below are taken directly from the source headers.

---

## 1. Register File

### 1.1 General-Purpose Registers (GPRs)

| Macro | Register | Role in AAPCS64 |
|-------|----------|-----------------|
| R0    | X0       | Argument 1 / return value |
| R1    | X1       | Argument 2 |
| R2    | X2       | Argument 3 |
| R3    | X3       | Argument 4 |
| R4    | X4       | Argument 5 |
| R5    | X5       | Argument 6 |
| R6    | X6       | Argument 7 |
| R7    | X7       | Argument 8 |
| R8    | X8       | Indirect result location / syscall number |
| R9    | X9       | Caller-saved scratch |
| R10   | X10      | Caller-saved scratch |
| R16   | X16      | Intra-procedure-call scratch (IP0) |
| R29   | X29      | Frame pointer (FP) |
| R30   | X30      | Link register (LR) |
| RSP   | SP       | Stack pointer (load/store context) |
| RZR   | XZR      | Zero register (data-processing context) |

All macros are defined in `Apkc/arch_arm64.h`. The `RSP` and `RZR` aliases both
encode as register 31; the distinction is instruction-class-dependent.

### 1.2 Condition Codes

| Macro   | Value | Meaning |
|---------|-------|---------|
| CC_EQ   | 0x0   | Equal (Z=1) |
| CC_NE   | 0x1   | Not equal (Z=0) |
| CC_CS   | 0x2   | Carry set / unsigned higher or same |
| CC_CC   | 0x3   | Carry clear / unsigned lower |
| CC_MI   | 0x4   | Minus / negative (N=1) |
| CC_PL   | 0x5   | Plus / positive or zero (N=0) |
| CC_VS   | 0x6   | Overflow (V=1) |
| CC_VC   | 0x7   | No overflow (V=0) |
| CC_HI   | 0x8   | Unsigned higher |
| CC_LS   | 0x9   | Unsigned lower or same |
| CC_GE   | 0xA   | Signed greater or equal |
| CC_LT   | 0xB   | Signed less than |
| CC_GT   | 0xC   | Signed greater than |
| CC_LE   | 0xD   | Signed less than or equal |
| CC_AL   | 0xE   | Always |

---

## 2. Instruction Set Coverage

### 2.1 Implemented Instructions (arch_arm64.h)

The table below documents every instruction family currently in `Apkc/arch_arm64.h`.
Encoding constants (opcode base values) are from the source.

#### Fixed Words

| Mnemonic   | Encoding       | Notes |
|------------|----------------|-------|
| NOP        | 0xD503201F     | No-op |
| RET x30    | 0xD65F03C0     | Return via LR |
| BRK #0     | 0xD4200000     | Software breakpoint |

#### Branches

| Function          | Mnemonic         | Opcode base   |
|-------------------|------------------|---------------|
| `a64_b`           | B #off26         | 0x14000000    |
| `a64_bl`          | BL #off26        | 0x94000000    |
| `a64_br`          | BR Xn            | 0xD61F0000    |
| `a64_blr`         | BLR Xn           | 0xD63F0000    |
| `a64_ret`         | RET Xn           | 0xD65F0000    |
| `a64_bcond`       | B.cond #imm19    | 0x54000000    |
| `a64_cbz`         | CBZ Xn, #imm19   | 0x34000000    |
| `a64_cbnz`        | CBNZ Xn, #imm19  | 0x35000000    |
| `a64_tbz`         | TBZ Rt, #bit, off14 | 0x36000000 |
| `a64_tbnz`        | TBNZ Rt, #bit, off14 | 0x37000000 |
| `a64_svc`         | SVC #imm16       | 0xD4000001    |

#### Move Immediate

| Function      | Mnemonic          | Notes                          |
|---------------|-------------------|-------------------------------|
| `a64_movz`    | MOVZ Rd, #imm16   | Zero-extends; sf selects width |
| `a64_movk`    | MOVK Rd, #imm16   | Keep other bits                |
| `a64_movn`    | MOVN Rd, #imm16   | Inverted immediate             |
| `a64_mov64`   | (sequence)        | MOVZ + up to 3x MOVK for 64-bit constant |

#### Data Processing — Register

| Function         | Mnemonic         | Opcode base  |
|------------------|------------------|--------------|
| `a64_orr_reg`    | ORR Xd, Xn, Xm   | 0x2B000000   |
| `a64_and_reg`    | AND Xd, Xn, Xm   | 0x0A000000   |
| `a64_eor_reg`    | EOR Xd, Xn, Xm   | 0x4A000000   |
| `a64_add_reg`    | ADD Xd, Xn, Xm   | 0x0B000000   |
| `a64_sub_reg`    | SUB Xd, Xn, Xm   | 0x4B000000   |
| `a64_mov_reg`    | MOV Xd, Xm       | alias: ORR Xd, XZR, Xm |

#### Data Processing — Immediate

| Function       | Mnemonic            |
|----------------|---------------------|
| `a64_add_imm`  | ADD Xd, Xn, #imm12  |
| `a64_sub_imm`  | SUB Xd, Xn, #imm12  |
| `a64_cmp_imm`  | CMP Xn, #imm12 (SUBS XZR) |
| `a64_cmp_reg`  | CMP Xn, Xm         |

#### Branchless Select

| Function      | Mnemonic                | Notes |
|---------------|-------------------------|-------|
| `a64_csel`    | CSEL Xd, Xn, Xm, cond  | Xd = (cond) ? Xn : Xm |
| `a64_csinc`   | CSINC Xd, Xn, Xm, cond | Used to synthesize CSET |

#### Shifts

| Function      | Mnemonic             |
|---------------|----------------------|
| `a64_lsl_imm` | LSL Xd, Xn, #sh (UBFM) |
| `a64_lsr_imm` | LSR Xd, Xn, #sh (UBFM) |
| `a64_asr_imm` | ASR Xd, Xn, #sh (SBFM) |
| `a64_lslv`    | LSLV Xd, Xn, Xm (register shift) |
| `a64_lsrv`    | LSRV Xd, Xn, Xm |
| `a64_asrv`    | ASRV Xd, Xn, Xm |

#### Multiply / Divide

| Function   | Mnemonic              | Notes |
|------------|-----------------------|-------|
| `a64_mul`  | MUL Xd, Xn, Xm       | MADD with Ra=XZR |
| `a64_sdiv` | SDIV Xd, Xn, Xm      | Signed divide |
| `a64_udiv` | UDIV Xd, Xn, Xm      | Unsigned divide |

#### Load / Store

| Function          | Mnemonic                         |
|-------------------|----------------------------------|
| `a64_ldr`         | LDR Xt, [Xn, #off] (unsigned offset, scaled) |
| `a64_str`         | STR Xt, [Xn, #off] |
| `a64_ldrb`        | LDRB Wt, [Xn, #off] |
| `a64_strb`        | STRB Wt, [Xn, #off] |
| `a64_ldrh`        | LDRH Wt, [Xn, #off] (scaled /2) |
| `a64_strh`        | STRH Wt, [Xn, #off] |
| `a64_stp`         | STP Xt1, Xt2, [Xn, #off7*8] signed-offset |
| `a64_stp_pre`     | STP ... [Xn, #off]! pre-index |
| `a64_stp_post`    | STP ... [Xn], #off post-index |
| `a64_ldp`         | LDP Xt1, Xt2, [Xn, #off7*8] |
| `a64_ldp_pre`     | LDP ... pre-index |
| `a64_ldp_post`    | LDP ... post-index |
| `a64_adr`         | ADR Xd, #byte_off (PC-relative) |
| `a64_adrp`        | ADRP Xd, #page_off (4 KiB page) |
| `a64_ldrsw_imm`   | LDRSW Xt, [Xn, #imm12*4] |
| `a64_ldr_lit`     | LDR Xt, label (PC-relative literal) |

#### NEON / Advanced SIMD

| Function           | Mnemonic                     | Notes |
|--------------------|------------------------------|-------|
| `a64_ld1_16b`      | LD1 {Vt.16B}, [Xn]           | Load one 128-bit vector |
| `a64_st1_16b`      | ST1 {Vt.16B}, [Xn]           | Store one 128-bit vector |
| `a64_add_4s`       | ADD Vd.4S, Vn.4S, Vm.4S      | 0x4EA08400 |
| `a64_sub_4s`       | SUB Vd.4S, Vn.4S, Vm.4S      | 0x6EA08400 |
| `a64_mul_4s`       | MUL Vd.4S, Vn.4S, Vm.4S      | 0x4E209C00 |
| `a64_fmul_4s`      | FMUL Vd.4S, Vn.4S, Vm.4S     | 0x6E20DC00 |
| `a64_fadd_4s`      | FADD Vd.4S, Vn.4S, Vm.4S     | 0x4E20D400 |
| `a64_cnt_16b`      | CNT Vd.16B, Vn.16B            | Popcount per byte; 0x4E205800 |
| `a64_addv_4s`      | ADDV Sd, Vn.4S                | Horizontal add; 0x4EB1B800 |
| `a64_eor_16b`      | EOR Vd.16B, Vn.16B, Vm.16B   | 0x6E201C00 |
| `a64_and_16b`      | AND Vd.16B, Vn.16B, Vm.16B   | 0x4E201C00 |
| `a64_orr_16b`      | ORR Vd.16B, Vn.16B, Vm.16B   | 0x4EA01C00 |
| `a64_zip1_16b`     | ZIP1 Vd.16B, Vn.16B, Vm.16B  | Interleave low halves |
| `a64_rev64_16b`    | REV64 Vd.16B, Vn.16B         | 0x4E200800 |
| `a64_ld2_4s`       | LD2 {Vt.4S, Vt2.4S}, [Xn]    | Two 128-bit registers |
| `a64_ld3_4s`       | LD3 {Vt.4S, Vt2.4S, Vt3.4S}, [Xn] | Three registers |
| `a64_ld4_4s`       | LD4 {Vt.4S...Vt4.4S}, [Xn]   | Four registers |
| `a64_st2_4s`       | ST2 {Vt.4S, Vt2.4S}, [Xn]    | |
| `a64_st3_4s`       | ST3 {Vt.4S...Vt3.4S}, [Xn]   | |
| `a64_st4_4s`       | ST4 {Vt.4S...Vt4.4S}, [Xn]   | |

#### FMA (Fused Multiply-Accumulate)

| Function       | Mnemonic                  | Notes |
|----------------|---------------------------|-------|
| `a64_fmla_4s`  | FMLA Vd.4S, Vn.4S, Vm.4S | Vd += Vn*Vm (float32x4); 0x4E20CC00 |
| `a64_fmls_4s`  | FMLS Vd.4S, Vn.4S, Vm.4S | Vd -= Vn*Vm (float32x4); 0x4EA0CC00 |
| `a64_fmla_2d`  | FMLA Vd.2D, Vn.2D, Vm.2D | float64x2; 0x4E60CC00 |

#### Widening Multiply

| Function        | Mnemonic                    | Notes |
|-----------------|-----------------------------|-------|
| `a64_umull_2d`  | UMULL Vd.2D, Vn.2S, Vm.2S  | Unsigned 32->64; 0x2E60C000 |
| `a64_smull_2d`  | SMULL Vd.2D, Vn.2S, Vm.2S  | Signed 32->64; 0x0E60C000 |
| `a64_umlal_2d`  | UMLAL Vd.2D, Vn.2S, Vm.2S  | Unsigned widening accumulate |
| `a64_smlal_2d`  | SMLAL Vd.2D, Vn.2S, Vm.2S  | Signed widening accumulate |

#### Polynomial Multiply (PMULL — GF(2^128))

| Function      | Mnemonic                    | Notes |
|---------------|-----------------------------|-------|
| `a64_pmull`   | PMULL Vd.1Q, Vn.1D, Vm.1D  | Lower-half GF(2^64) product; 0x0EE0E000 |
| `a64_pmull2`  | PMULL2 Vd.1Q, Vn.2D, Vm.2D | Upper-half GF(2^64) product; 0x4EE0E000 |

PMULL/PMULL2 are the carry-less multiply primitives used for GCM authentication
and other GF(2^128) computations. They require the ARMv8.0 Crypto extension.

#### Integer Dot Product (SDOT/UDOT — ML inference)

| Function        | Mnemonic                       | Notes |
|-----------------|--------------------------------|-------|
| `a64_sdot_4s`   | SDOT Vd.4S, Vn.16B, Vm.16B    | Signed 4x int8 -> int32; 0x4E809400 |
| `a64_udot_4s`   | UDOT Vd.4S, Vn.16B, Vm.16B    | Unsigned 4x int8 -> uint32; 0x6E809400 |

SDOT/UDOT require the ARMv8.2 DotProd extension (present on Cortex-A55/A76+).
Each instruction accumulates four 8-bit products into one 32-bit lane, producing
four 32-bit results per instruction.

#### Vector Broadcast

| Function            | Mnemonic               |
|---------------------|------------------------|
| `a64_dup_4s_gpr`    | DUP Vd.4S, Xn          |
| `a64_dup_4s_lane0`  | DUP Vd.4S, Vn.S[0]     |
| `a64_dup_16b_gpr`   | DUP Vd.16B, Xn         |

#### Vector Permute / Interleave

| Function       | Mnemonic                  |
|----------------|---------------------------|
| `a64_zip1_16b` | ZIP1 Vd.16B, Vn.16B, Vm.16B |
| `a64_zip2_4s`  | ZIP2 Vd.4S, Vn.4S, Vm.4S  |
| `a64_uzp1_4s`  | UZP1 Vd.4S, Vn.4S, Vm.4S  |
| `a64_uzp2_4s`  | UZP2 Vd.4S, Vn.4S, Vm.4S  |
| `a64_trn1_4s`  | TRN1 Vd.4S, Vn.4S, Vm.4S  |
| `a64_trn2_4s`  | TRN2 Vd.4S, Vn.4S, Vm.4S  |
| `a64_tbl_16b`  | TBL Vd.16B, {Vn.16B}, Vm.16B (OOB->0) |
| `a64_tbx_16b`  | TBX Vd.16B, {Vn.16B}, Vm.16B (OOB->unchanged) |
| `a64_ext_16b`  | EXT Vd.16B, Vn.16B, Vm.16B, #imm |

#### Bit Manipulation (Scalar 64-bit)

| Function    | Mnemonic       | Encoding     |
|-------------|----------------|--------------|
| `a64_clz`   | CLZ Xd, Xn     | 0xDAC01000   |
| `a64_cls`   | CLS Xd, Xn     | 0xDAC01400   |
| `a64_rbit`  | RBIT Xd, Xn    | 0xDAC00000   |
| `a64_rev`   | REV Xd, Xn     | 0xDAC00C00   |
| `a64_rev32` | REV32 Xd, Xn   | 0xDAC00800   |
| `a64_extr`  | EXTR Xd, Xn, Xm, #lsb | 0x93C00000 |

#### Scalar Floating-Point (S/D registers)

| Function              | Mnemonic              |
|-----------------------|-----------------------|
| `a64_fadd_s`          | FADD Sd, Sn, Sm       |
| `a64_fsub_s`          | FSUB Sd, Sn, Sm       |
| `a64_fmul_s`          | FMUL Sd, Sn, Sm       |
| `a64_fdiv_s`          | FDIV Sd, Sn, Sm       |
| `a64_fsqrt_s`         | FSQRT Sd, Sn          |
| `a64_fabs_s`          | FABS Sd, Sn           |
| `a64_fneg_s`          | FNEG Sd, Sn           |
| `a64_fcvtzs`          | FCVTZS Xd, Sn (float32->int64 truncate) |
| `a64_scvtf`           | SCVTF Sd, Xn (int64->float32) |
| `a64_fcmp_s`          | FCMP Sn, Sm (set FP condition flags) |
| `a64_fmov_gpr_to_s`   | FMOV Sd, Xn (move GPR bits into FP) |
| `a64_fmov_s_to_gpr`   | FMOV Xd, Sn (move FP bits to GPR) |

#### CRC32 (ARMv8.0 CRC32 extension)

| Function       | Polynomial   | Width |
|----------------|--------------|-------|
| `a64_crc32b`   | ISO 3309     | byte  |
| `a64_crc32h`   | ISO 3309     | halfword |
| `a64_crc32w`   | ISO 3309     | word  |
| `a64_crc32x`   | ISO 3309     | doubleword |
| `a64_crc32cb`  | Castagnoli   | byte  |
| `a64_crc32ch`  | Castagnoli   | halfword |
| `a64_crc32cw`  | Castagnoli   | word  |
| `a64_crc32cx`  | Castagnoli   | doubleword |

The CRC32C (Castagnoli) variant is used by `fmt_zip.h` for ZIP/APK CRC computation
with hardware fallback.

#### SHA-256 (ARMv8.0 Crypto extension)

| Function         | Mnemonic     |
|------------------|--------------|
| `a64_sha256h`    | SHA256H Qd, Qn, Vm |
| `a64_sha256h2`   | SHA256H2 Qd, Qn, Vm |
| `a64_sha256su0`  | SHA256SU0 Vd, Vn |
| `a64_sha256su1`  | SHA256SU1 Vd, Vn, Vm |

#### AES (ARMv8.0 Crypto extension)

| Function      | Mnemonic          |
|---------------|-------------------|
| `a64_aese`    | AESE Vd, Vn       |
| `a64_aesd`    | AESD Vd, Vn       |
| `a64_aesmc`   | AESMC Vd, Vn      |
| `a64_aesimc`  | AESIMC Vd, Vn     |

#### Acquire-Release / Exclusive Atomics (ARMv8.0)

| Function      | Mnemonic            | Notes |
|---------------|---------------------|-------|
| `a64_ldar`    | LDAR Xt, [Xn]       | Load-acquire |
| `a64_stlr`    | STLR Xt, [Xn]       | Store-release |
| `a64_ldaxr`   | LDAXR Xt, [Xn]      | Load-acquire exclusive |
| `a64_stlxr`   | STLXR Ws, Xt, [Xn] | Store-release exclusive |
| `a64_ldxr`    | LDXR Xt, [Xn]       | Load exclusive |
| `a64_stxr`    | STXR Ws, Xt, [Xn]  | Store exclusive |
| `a64_cas`     | CAS Xs, Xt, [Xn]   | Compare-and-swap (ARMv8.1 LSE) |

#### Cache / Prefetch

| Function          | Mnemonic                   |
|-------------------|----------------------------|
| `a64_prfm_l1`     | PRFM PLDL1KEEP, [Xn, #off] |
| `a64_prfm_l2`     | PRFM PLDL2KEEP, [Xn, #off] |
| `a64_dc_civac`    | DC CIVAC, Xt (clean+invalidate to PoC) |
| `a64_dc_cvac`     | DC CVAC, Xt (clean to PoC) |

### 2.2 Instructions Not Yet Implemented

The following A64 instruction families are absent from `arch_arm64.h` and are marked
as PENDING coverage. The header comment states approximately 65% ISA coverage.

| Category | Examples of missing instructions |
|----------|----------------------------------|
| SVE / SVE2 | FMLA (predicated), WHILELT, PTRUE |
| BFloat16 | BFMMLA, BFDOT, BFMLALB |
| SHA-3 / SHA-512 | SHA512H, RAXQX, EOR3 |
| SM3 / SM4 | SM3SS1, SM4E |
| System ops | AT, TLBI, DC ZVA (cache zero) |
| Load-Store pairs (LDPSW) | LDPSW Xt1, Xt2, [Xn, #off] |
| Atomic memory ops (ARMv8.1) | LDADD, STADD, SWP |
| FP double-precision vector | FADD Vd.2D full set |
| Integer min/max NEON | SMIN, UMIN, SMAX, UMAX |
| Conversion | FCVTN, FCVTL, FCVTXN |

---

## 3. System Registers

### 3.1 Registers Defined in arch_arm64.h

The 15-bit packed sysreg encoding used by `a64_mrs` / `a64_msr`:
`[14]=op0 [13:11]=op1 [10:7]=CRn [6:3]=CRm [2:0]=op2`

| Macro        | Value  | Full name | Usage |
|--------------|--------|-----------|-------|
| A64_SR_NZCV  | 0x5A10 | NZCV      | Condition flag register |
| A64_SR_DAIF  | 0x5A11 | DAIF      | Interrupt mask bits |
| A64_SR_FPCR  | 0x5A20 | FPCR      | FP control (rounding, exception masks) |
| A64_SR_FPSR  | 0x5A21 | FPSR      | FP status (exception flags) |
| A64_SR_TPIDR | 0x5E82 | TPIDR_EL0 | EL0 thread pointer (TLS base) |

### 3.2 System Registers Used in Benchmark Files

These registers are accessed via inline `__asm__ __volatile__("mrs %0, <reg>")` in
the RAF_024/RAF_025 benchmark files and are not encoded by the project's own
`a64_mrs` function (which targets freestanding code generation, not host probing).

| Register       | Source file | Purpose |
|----------------|-------------|---------|
| `cntvct_el0`   | RAF_024_leitura_de_contador_arm64_cntvct_el0.c | Read virtual counter (monotonic, no syscall) |
| `cntfrq_el0`   | RAF_025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo.c | Counter frequency in Hz; divides delta into nanoseconds |
| `MDSCR_EL1`    | AUDIT (referenced in CLAUDE.md) | Debug/single-step control register |

Conversion formula used in RAF_025:
```
elapsed_ns = (cntvct_el0_delta * 1000000000ULL) / cntfrq_el0
```
Fallback when `cntfrq_el0` returns zero: use 1,000,000,000 (assume 1 GHz counter).

---

## 4. Memory Barriers

Three barrier instructions are implemented as named macros and documented in
RAF_026, RAF_027, and RAF_028.

| Macro        | Encoding   | ARM mnemonic | ISB/DSB/DMB class | Scope |
|--------------|------------|--------------|-------------------|-------|
| A64_DMB_ISH  | 0xD5033BBF | DMB ISH      | Data Memory Barrier | Inner Shareable |
| A64_DSB_ISH  | 0xD5033B9F | DSB ISH      | Data Synchronization Barrier | Inner Shareable |
| A64_ISB      | 0xD5033FDF | ISB          | Instruction Synchronization Barrier | — |

### 4.1 When to Use Each Barrier

**DMB ISH** (Data Memory Barrier — Inner Shareable domain):
- Use between a memory write and a subsequent memory read/write where ordering is
  required across cores sharing the same coherence domain.
- Does NOT prevent the CPU from issuing further instructions before the barrier
  completes; only orders memory accesses.
- Pattern: `dmb ish` between MMIO write and read-back, or between releasing a lock
  and writing shared data (file: RAF_026).

**DSB ISH** (Data Synchronization Barrier):
- Stronger than DMB: completion of the DSB instruction is guaranteed only after all
  preceding memory accesses complete to the point of coherence.
- Required before cache maintenance operations (DC, IC) take effect, and before any
  TLB invalidation instruction.
- Pattern: `dsb ish` after writing page tables, before issuing TLBI (file: RAF_027).

**ISB** (Instruction Synchronization Barrier):
- Flushes the pipeline and instruction prefetch buffers. Subsequent instructions are
  fetched from the cache or memory.
- Required after writing to system registers that affect instruction fetch (e.g.,
  SCTLR_EL1), after self-modifying code, and after changing exception levels.
- Does not directly order data memory accesses (file: RAF_028).
- No ISB equivalent on x86; the project falls back to a compiler barrier (`""` with
  `"memory"` clobber) on non-ARM hosts.

### 4.2 Inline ASM Patterns

```c
// DMB
__asm__ __volatile__("dmb ish" ::: "memory");

// DSB
__asm__ __volatile__("dsb ish" ::: "memory");

// ISB
__asm__ __volatile__("isb" ::: "memory");
```

The `"memory"` clobber prevents the compiler from reordering memory accesses
across the barrier at the C level.

---

## 5. Calling Convention (AAPCS64)

### 5.1 Register Allocation Summary

| Role | Registers | Notes |
|------|-----------|-------|
| Arguments / return | X0-X7 | Integer and pointer arguments; X0 = return value |
| Caller-saved scratch | X9-X15 | May be destroyed by called function |
| Callee-saved | X19-X28 | Must be preserved across calls |
| Frame pointer | X29 (FP) | Required for stack-unwinding on Android |
| Link register | X30 (LR) | Return address; saved/restored with STP/LDP |
| Stack pointer | SP       | Must be 16-byte aligned at all call sites |
| Intra-call scratch | X16, X17 (IP0, IP1) | Used by linker veneers; not callee-saved |
| Zero register | XZR      | Reads as 0; writes discarded |

### 5.2 Frame Pointer Protocol (X29/X30)

On Android, the frame pointer must be maintained to support stack unwinding in
debuggers and crash reporters. The canonical prologue/epilogue uses STP/LDP:

```asm
// Prologue
stp  x29, x30, [sp, #-16]!   // save FP and LR, decrement SP
mov  x29, sp                   // FP points to saved-FP slot

// Epilogue
ldp  x29, x30, [sp], #16      // restore FP and LR, increment SP
ret
```

In the project's codegen this is produced by `a64_stp_pre(R29, R30, RSP, -2, 1)`
followed by `a64_mov_reg(R29, RSP, 1)`.

### 5.3 Syscall Convention (Linux/Android)

| Register | Role |
|----------|------|
| X8       | Syscall number |
| X0-X5    | Arguments (up to 6) |
| X0       | Return value |

In the lang_script.h bootstrap, `x8 = 221` is the `__NR_execve` syscall number on
AArch64 Linux. The sequence is:
```
movz x8, #221   // x8 = __NR_execve
svc  #0         // invoke kernel
```

---

## 6. execve Bootstrap (lang_script.h)

The 18-instruction bootstrap generated by `gen_script_code64` is the only code path
that does not use the `CodeBuf` emitter context. It writes directly into a flat byte
buffer, appending a string pool after the instruction region.

### 6.1 Instruction Sequence

| Position | Instruction | Purpose |
|----------|-------------|---------|
| 0        | SUB SP, SP, #48   | Allocate argv array on stack |
| 1        | ADR X19, pool_start-4 | X19 = pointer to interp string |
| 2        | ADD X20, X19, interp_len+1 | X20 = pointer to arg1 (or script) |
| 3        | ADD X21, X20, arg1_len+1 | X21 = pointer to script (when arg1 present) |
| 4        | STR X19, [SP, #0]  | argv[0] = interp |
| 5        | STR X20, [SP, #8]  | argv[1] = arg1 |
| 6        | STR X21, [SP, #16] | argv[2] = script |
| 7        | MOVZ X22, #0       | X22 = NULL |
| 8        | STR X22, [SP, #24] | argv[3] = NULL |
| 9        | STR X22, [SP, #32] | envp[0] = NULL |
| 10       | ORR X0, XZR, X19   | X0 = interp (execve path arg) |
| 11       | ADD X1, SP, #0     | X1 = argv |
| 12       | ADD X2, SP, #32    | X2 = envp (points to NULL) |
| 13       | MOVZ X8, #221      | X8 = __NR_execve |
| 14       | SVC #0             | execve() |
| 15       | ADD SP, SP, #48    | Fallback: restore stack |
| 16       | MOVZ X0, #0        | Return 0 |
| 17       | RET                | Return to caller (execve failed) |

Scratch registers X19–X22 are callee-saved per AAPCS64, so this code is safe to
call from C without saving them first (though in practice execve replaces the
process image on success).

The constant `SCRIP_INSN_COUNT = 18` is defined in the source and must match the
instruction count exactly for the pool-start offset calculation to be correct.

### 6.2 Languages Using the Bootstrap

| Language | Interpreter path | arg1 |
|----------|-----------------|------|
| Python   | /usr/bin/python3 | -c |
| Shell    | /bin/sh          | -c |
| Perl     | /usr/bin/perl    | -e |
| Node.js  | /usr/bin/node    | -e |
| PHP      | /usr/bin/php     | -r |

JSX uses a two-stage pipeline: Babel transpiles to JS first, then the JS text
is embedded into a Node.js bootstrap using the same `gen_script_code64` function.

---

## 7. Performance Invariants

### 7.1 System Register Read Latency

Reading `cntvct_el0` via `mrs` is a low-latency, non-privileged instruction on
all ARMv8-A implementations visible to EL0. On Cortex-A55 the latency is typically
3-5 cycles. No privilege escalation or syscall overhead is incurred, unlike
`clock_gettime(CLOCK_MONOTONIC)`.

The benchmark pattern (RAF_024) reads the counter twice in sequence to measure the
overhead of the read itself:
```c
uint64_t t0, t1;
__asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(t0));
__asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(t1));
// t1 - t0 = raw read overhead in counter ticks
```

### 7.2 NEON Throughput

NEON instructions (`a64_fmla_4s`, `a64_mul_4s`, etc.) operate on 128-bit V
registers. On Cortex-A55 and Cortex-A76 class cores:
- `FMLA Vd.4S`: throughput 1 per cycle, latency 4 cycles
- `MUL Vd.4S`: throughput 1 per cycle, latency 4 cycles
- `PMULL`: latency 3 cycles (varies by implementation)
- `SDOT Vd.4S`: latency 4 cycles; 4 multiplies + 4 accumulates per instruction

### 7.3 Codegen Buffer Limits

The `_code64` buffer used by the compiler is statically allocated at 64 KiB. Each
ARM64 instruction is exactly 4 bytes, so the maximum instruction count before
overflow is 16,384 instructions per compilation unit.

---

## 8. Key Source Files

| File | Description |
|------|-------------|
| `Apkc/arch_arm64.h` | All instruction encoders (~65% ISA coverage, approximately 110 functions) |
| `Apkc/lang_script.h` | 18-instruction execve bootstrap for interpreter-based languages |
| `Apkc/apkc.c` | Main compiler; dispatches to `asm_insn64()` for each instruction case |
| `Apkc/coherence.h` | `phi_fst()` coherence metric; uses system entropy from emitted bytes |
| `RAF_024_leitura_de_contador_arm64_cntvct_el0.c` | cntvct_el0 read pattern |
| `RAF_025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo.c` | cntfrq_el0 + ns conversion |
| `RAF_026_memory_barrier_dmb.c` | DMB ISH barrier |
| `RAF_027_memory_barrier_dsb.c` | DSB ISH barrier |
| `RAF_028_memory_barrier_isb.c` | ISB barrier |
