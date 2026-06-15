/* arch_arm64.h — AArch64 (ARMv8-A) A64 instruction encoder.
 * Every function returns the 32-bit machine word (little-endian).
 * No heap. No imports beyond mem.h. Pure bit arithmetic. */
#pragma once
#include "mem.h"

/* ── Register numbers ────────────────────────────────────────────────── */
#define R0  0u  /* also XZR in some contexts */
#define R1  1u
#define R2  2u
#define R3  3u
#define R4  4u
#define R5  5u
#define R6  6u
#define R7  7u
#define R8  8u
#define R9  9u
#define R10 10u
#define R16 16u
#define R29 29u  /* FP */
#define R30 30u  /* LR */
#define RSP 31u  /* SP in load/store */
#define RZR 31u  /* XZR in data proc */

/* ── Condition codes ─────────────────────────────────────────────────── */
#define CC_EQ 0x0u
#define CC_NE 0x1u
#define CC_CS 0x2u
#define CC_CC 0x3u
#define CC_MI 0x4u
#define CC_PL 0x5u
#define CC_VS 0x6u
#define CC_VC 0x7u
#define CC_HI 0x8u
#define CC_LS 0x9u
#define CC_GE 0xAu
#define CC_LT 0xBu
#define CC_GT 0xCu
#define CC_LE 0xDu
#define CC_AL 0xEu

/* ── Emitter context ─────────────────────────────────────────────────── */
typedef struct { u8 *p; sz cap; sz pos; } CodeBuf;
static inline void cb_emit(CodeBuf *b, u32 insn) {
    w32(b->p + b->pos, insn); b->pos += 4;
}
static inline u32 cb_mark(CodeBuf *b) { return (u32)b->pos; }

/* ── Fixed words ─────────────────────────────────────────────────────── */
#define A64_NOP  0xD503201Fu
#define A64_RET  0xD65F03C0u  /* ret x30 */
#define A64_BRK0 0xD4200000u  /* brk #0 */

/* ── Branches ────────────────────────────────────────────────────────── */
/* B  #imm26  (PC-relative, unit = 4 bytes) */
static inline u32 a64_b  (i32 off4) { return 0x14000000u|((u32)off4&0x3FFFFFFu); }
/* BL #imm26 */
static inline u32 a64_bl (i32 off4) { return 0x94000000u|((u32)off4&0x3FFFFFFu); }
/* BR  Xn */
static inline u32 a64_br (u8 rn)    { return 0xD61F0000u|((u32)rn<<5); }
/* BLR Xn */
static inline u32 a64_blr(u8 rn)    { return 0xD63F0000u|((u32)rn<<5); }
/* RET Xn (default LR=x30) */
static inline u32 a64_ret(u8 rn)    { return 0xD65F0000u|((u32)rn<<5); }
/* B.cond #imm19 */
static inline u32 a64_bcond(u8 cc, i32 off4) {
    return 0x54000000u|(((u32)off4&0x7FFFFu)<<5)|(u32)cc;
}
/* CBZ/CBNZ  Xn, #imm19 */
static inline u32 a64_cbz (u8 rn, i32 off4, u8 sf) {
    return ((u32)sf<<31)|0x34000000u|((((u32)off4)&0x7FFFFu)<<5)|(u32)rn;
}
static inline u32 a64_cbnz(u8 rn, i32 off4, u8 sf) {
    return ((u32)sf<<31)|0x35000000u|((((u32)off4)&0x7FFFFu)<<5)|(u32)rn;
}
/* TBZ/TBNZ Rt, #bit, #imm14 */
static inline u32 a64_tbz(u8 rt, u8 bit, i16 off14) {
    u32 b5=(bit>>5)&1u, b40=bit&0x1Fu;
    return (b5<<31)|0x36000000u|(b40<<19)|(((u32)(u16)off14&0x3FFFu)<<5)|(u32)rt;
}
static inline u32 a64_tbnz(u8 rt, u8 bit, i16 off14) {
    u32 b5=(bit>>5)&1u, b40=bit&0x1Fu;
    return (b5<<31)|0x37000000u|(b40<<19)|(((u32)(u16)off14&0x3FFFu)<<5)|(u32)rt;
}
/* SVC #imm16 */
static inline u32 a64_svc(u16 imm) { return 0xD4000001u|((u32)imm<<5); }

/* ── Move immediate ──────────────────────────────────────────────────── */
/* MOVZ  Rd, #imm16 LSL (hw*16)  — sf=1 → 64-bit, sf=0 → 32-bit */
static inline u32 a64_movz(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x2u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* MOVK  Rd, #imm16 LSL (hw*16)  — keep other bits */
static inline u32 a64_movk(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x3u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* MOVN  Rd, #imm16 LSL (hw*16)  — inverted */
static inline u32 a64_movn(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x0u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* Emit 64-bit constant into Xd using MOVZ + up to 3× MOVK */
static inline void a64_mov64(CodeBuf *b, u8 rd, u64 v) {
    u8 first = 1;
    for (u8 hw = 0; hw < 4; hw++) {
        u16 part = (u16)(v >> (hw * 16));
        if (!part && !first) continue;
        if (first) { cb_emit(b, a64_movz(rd, part, hw, 1)); first = 0; }
        else         cb_emit(b, a64_movk(rd, part, hw, 1));
    }
    if (first) cb_emit(b, a64_movz(rd, 0, 0, 1)); /* v == 0 */
}

/* ── Data processing (register) ──────────────────────────────────────── */
/* ORR Xd, Xn, Xm  →  MOV Xd,Xm when Xn=XZR */
static inline u32 a64_orr_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x2Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_and_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x0Au<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_eor_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x4Au<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_add_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x0Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_sub_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x4Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
/* MOV Xd, Xm  (alias: ORR Xd, XZR, Xm) */
static inline u32 a64_mov_reg(u8 rd, u8 rm, u8 sf) {
    return a64_orr_reg(rd, RZR, rm, sf);
}

/* ── Data processing (immediate) ─────────────────────────────────────── */
/* ADD/SUB Xd, Xn, #imm12 [shift=0 or shift=1 (LSL#12)] */
static inline u32 a64_add_imm(u8 rd, u8 rn, u16 imm12, u8 sh, u8 sf) {
    return ((u32)sf<<31)|(0x11u<<24)|((u32)sh<<22)|((u32)imm12<<10)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_sub_imm(u8 rd, u8 rn, u16 imm12, u8 sh, u8 sf) {
    return ((u32)sf<<31)|(0x51u<<24)|((u32)sh<<22)|((u32)imm12<<10)|((u32)rn<<5)|(u32)rd;
}
/* CMP Xn, #imm12  (SUBS XZR, Xn, #imm) */
static inline u32 a64_cmp_imm(u8 rn, u16 imm12, u8 sf) {
    return ((u32)sf<<31)|(0x71u<<24)|((u32)imm12<<10)|((u32)rn<<5)|(u32)RZR;
}
/* CMP Xn, Xm */
static inline u32 a64_cmp_reg(u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x6Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)RZR;
}

/* ── Branchless select ────────────────────────────────────────────────── */
/* CSEL  Xd, Xn, Xm, cond  — Xd = (cond) ? Xn : Xm */
static inline u32 a64_csel(u8 rd, u8 rn, u8 rm, u8 cc, u8 sf) {
    return ((u32)sf<<31)|(0x1Au<<24)|(1u<<23)|((u32)rm<<16)|((u32)cc<<12)|((u32)rn<<5)|(u32)rd;
}
/* CSINC Xd, Xn, Xm, cond  — used for CSET (rd,xzr,xzr,inv_cond) */
static inline u32 a64_csinc(u8 rd, u8 rn, u8 rm, u8 cc, u8 sf) {
    return ((u32)sf<<31)|(0x1Au<<24)|(1u<<23)|((u32)rm<<16)|((u32)cc<<12)|(1u<<10)|((u32)rn<<5)|(u32)rd;
}

/* ── Shifts ──────────────────────────────────────────────────────────── */
/* LSL Xd, Xn, #sh  (UBFM) */
static inline u32 a64_lsl_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 64u : 32u;
    return ((u32)sf<<31)|(0x53u<<23)|((u32)sf<<22)|
           ((u32)(bsz-sh)<<16)|((u32)(bsz-sh-1)<<10)|((u32)rn<<5)|(u32)rd;
}
/* LSR Xd, Xn, #sh  (UBFM) */
static inline u32 a64_lsr_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 63u : 31u;
    return ((u32)sf<<31)|(0x53u<<23)|((u32)sf<<22)|
           ((u32)sh<<16)|((u32)bsz<<10)|((u32)rn<<5)|(u32)rd;
}
/* ASR Xd, Xn, #sh  (SBFM) */
static inline u32 a64_asr_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 63u : 31u;
    return ((u32)sf<<31)|(0x13u<<23)|((u32)sf<<22)|
           ((u32)sh<<16)|((u32)bsz<<10)|((u32)rn<<5)|(u32)rd;
}
/* Variable shifts (register form) */
static inline u32 a64_lslv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02000u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_lsrv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02400u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_asrv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}

/* ── Multiply / Divide ───────────────────────────────────────────────── */
/* MUL Xd, Xn, Xm  (MADD with Ra=XZR) */
static inline u32 a64_mul(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1B007C00u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
/* SDIV / UDIV */
static inline u32 a64_sdiv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC00C00u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_udiv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC00800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}

/* ── Load / Store ────────────────────────────────────────────────────── */
/* LDR Xt, [Xn, #off]  unsigned offset (multiple of 8 for 64-bit) */
static inline u32 a64_ldr(u8 rt, u8 rn, u16 off, u8 sf) {
    u8 sz = sf ? 3u : 2u;
    u16 sc = sf ? (off>>3) : (off>>2);
    return ((u32)sz<<30)|(0x39u<<24)|(1u<<22)|((u32)sc<<10)|((u32)rn<<5)|(u32)rt;
}
/* STR Xt, [Xn, #off] */
static inline u32 a64_str(u8 rt, u8 rn, u16 off, u8 sf) {
    u8 sz = sf ? 3u : 2u;
    u16 sc = sf ? (off>>3) : (off>>2);
    return ((u32)sz<<30)|(0x39u<<24)|((u32)sc<<10)|((u32)rn<<5)|(u32)rt;
}
/* LDRB Wt, [Xn, #off]  — byte load (off not scaled) */
static inline u32 a64_ldrb(u8 rt, u8 rn, u16 off) {
    return 0x39400000u|((u32)(off&0xFFFu)<<10)|((u32)rn<<5)|(u32)rt;
}
/* STRB Wt, [Xn, #off] */
static inline u32 a64_strb(u8 rt, u8 rn, u16 off) {
    return 0x39000000u|((u32)(off&0xFFFu)<<10)|((u32)rn<<5)|(u32)rt;
}
/* LDRH Wt, [Xn, #off]  — halfword (off scaled /2) */
static inline u32 a64_ldrh(u8 rt, u8 rn, u16 off) {
    return 0x79400000u|((u32)((off>>1)&0xFFFu)<<10)|((u32)rn<<5)|(u32)rt;
}
/* STRH Wt, [Xn, #off] */
static inline u32 a64_strh(u8 rt, u8 rn, u16 off) {
    return 0x79000000u|((u32)((off>>1)&0xFFFu)<<10)|((u32)rn<<5)|(u32)rt;
}
/* STP Xt1, Xt2, [Xn, #off7*8]  signed-offset — base 0xA9000000 */
static inline u32 a64_stp(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA9000000u : 0x29000000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* STP pre-index [Xn, #off7*8]! — base 0xA9800000 */
static inline u32 a64_stp_pre(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA9800000u : 0x29800000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* STP post-index [Xn], #off7*8 — base 0xA8800000 */
static inline u32 a64_stp_post(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA8800000u : 0x28800000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* LDP signed-offset — base 0xA9400000 */
static inline u32 a64_ldp(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA9400000u : 0x29400000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* LDP pre-index — base 0xA9C00000 */
static inline u32 a64_ldp_pre(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA9C00000u : 0x29C00000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* LDP post-index — base 0xA8C00000 */
static inline u32 a64_ldp_post(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u32 base = sf ? 0xA8C00000u : 0x28C00000u;
    return base|(((u32)(u8)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* UDIV Xd, Xn, Xm */
static inline u32 a64_udiv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC00800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
/* LSL/LSR/ASR register forms (variable shift) */
static inline u32 a64_lslv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02000u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_lsrv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02400u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_asrv(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|0x1AC02800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}

/* ── Test-and-branch ────────────────────────────────────────────────── */
/* TBZ  Rt, #bit, #imm14  (off14 in instruction units from PC) */
static inline u32 a64_tbz(u8 rt, u8 bit, i16 off14) {
    u32 b5=(bit>>5)&1u, b40=bit&0x1Fu;
    return (b5<<31)|0x36000000u|(b40<<19)|(((u32)(u16)off14&0x3FFFu)<<5)|(u32)rt;
}
/* TBNZ Rt, #bit, #imm14 */
static inline u32 a64_tbnz(u8 rt, u8 bit, i16 off14) {
    u32 b5=(bit>>5)&1u, b40=bit&0x1Fu;
    return (b5<<31)|0x37000000u|(b40<<19)|(((u32)(u16)off14&0x3FFFu)<<5)|(u32)rt;
}

/* ADR  Xd, #byte_offset (PC-relative) */
static inline u32 a64_adr(u8 rd, i32 off) {
    return 0x10000000u|(((u32)off&3u)<<29)|(((u32)off>>2)&0x7FFFFu)<<5|(u32)rd;
}
/* ADRP Xd, #page_offset (page = 4 KiB) */
static inline u32 a64_adrp(u8 rd, i32 pg) {
    return 0x90000000u|(((u32)pg&3u)<<29)|(((u32)(pg>>2))&0x7FFFFu)<<5|(u32)rd;
}

/* ── NEON / Advanced SIMD (V-registers 0-31, 128-bit) ────────────────── */
/* LD1 {Vt.16B}, [Xn] */
static inline u32 a64_ld1_16b(u8 vt, u8 rn) { return 0x4C407000u|((u32)rn<<5)|(u32)vt; }
/* ST1 {Vt.16B}, [Xn] */
static inline u32 a64_st1_16b(u8 vt, u8 rn) { return 0x4C007000u|((u32)rn<<5)|(u32)vt; }
/* ADD Vd.4S, Vn.4S, Vm.4S */
static inline u32 a64_add_4s(u8 vd, u8 vn, u8 vm) {
    return 0x4EA08400u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* SUB Vd.4S, Vn.4S, Vm.4S */
static inline u32 a64_sub_4s(u8 vd, u8 vn, u8 vm) {
    return 0x6EA08400u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* MUL Vd.4S, Vn.4S, Vm.4S */
static inline u32 a64_mul_4s(u8 vd, u8 vn, u8 vm) {
    return 0x4E209C00u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* FMUL Vd.4S, Vn.4S, Vm.4S */
static inline u32 a64_fmul_4s(u8 vd, u8 vn, u8 vm) {
    return 0x6E20DC00u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* FADD Vd.4S, Vn.4S, Vm.4S */
static inline u32 a64_fadd_4s(u8 vd, u8 vn, u8 vm) {
    return 0x4E20D400u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* CNT Vd.16B, Vn.16B — popcount per byte */
static inline u32 a64_cnt_16b(u8 vd, u8 vn) { return 0x4E205800u|((u32)vn<<5)|(u32)vd; }
/* ADDV Sd, Vn.4S — horizontal add, result in scalar */
static inline u32 a64_addv_4s(u8 vd, u8 vn) { return 0x4EB1B800u|((u32)vn<<5)|(u32)vd; }
/* EOR Vd.16B, Vn.16B, Vm.16B */
static inline u32 a64_eor_16b(u8 vd, u8 vn, u8 vm) {
    return 0x6E201C00u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* AND Vd.16B, Vn.16B, Vm.16B */
static inline u32 a64_and_16b(u8 vd, u8 vn, u8 vm) {
    return 0x4E201C00u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* ORR Vd.16B, Vn.16B, Vm.16B */
static inline u32 a64_orr_16b(u8 vd, u8 vn, u8 vm) {
    return 0x4EA01C00u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* ZIP1 Vd.16B, Vn.16B, Vm.16B — interleave low halves */
static inline u32 a64_zip1_16b(u8 vd, u8 vn, u8 vm) {
    return 0x4E003800u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}
/* REV64 Vd.16B, Vn.16B */
static inline u32 a64_rev64_16b(u8 vd, u8 vn) { return 0x4E200800u|((u32)vn<<5)|(u32)vd; }

/* ── CRC32 (ARMv8.0 CRC32 extension) ────────────────────────────────── */
/* ISO 3309 poly: CRC32B/H/W/X */
static inline u32 a64_crc32b(u8 rd, u8 rn, u8 rm) {
    return 0x1AC04000u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32h(u8 rd, u8 rn, u8 rm) {
    return 0x1AC04400u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32w(u8 rd, u8 rn, u8 rm) {
    return 0x1AC04800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32x(u8 rd, u8 rn, u8 rm) {
    return 0x9AC04C00u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
/* Castagnoli poly: CRC32CB/CH/CW/CX */
static inline u32 a64_crc32cb(u8 rd, u8 rn, u8 rm) {
    return 0x1AC05000u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32ch(u8 rd, u8 rn, u8 rm) {
    return 0x1AC05400u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32cw(u8 rd, u8 rn, u8 rm) {
    return 0x1AC05800u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_crc32cx(u8 rd, u8 rn, u8 rm) {
    return 0x9AC05C00u|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}

/* ── SHA-256 (ARMv8.0 Crypto extension) ─────────────────────────────── */
static inline u32 a64_sha256h  (u8 qd, u8 qn, u8 vm) {
    return 0x5E004000u|((u32)vm<<16)|((u32)qn<<5)|(u32)qd;
}
static inline u32 a64_sha256h2 (u8 qd, u8 qn, u8 vm) {
    return 0x5E005000u|((u32)vm<<16)|((u32)qn<<5)|(u32)qd;
}
static inline u32 a64_sha256su0(u8 vd, u8 vn) {
    return 0x5E282800u|((u32)vn<<5)|(u32)vd;
}
static inline u32 a64_sha256su1(u8 vd, u8 vn, u8 vm) {
    return 0x5E006000u|((u32)vm<<16)|((u32)vn<<5)|(u32)vd;
}

/* ── AES (ARMv8.0 Crypto extension) ─────────────────────────────────── */
static inline u32 a64_aese  (u8 vd, u8 vn) { return 0x4E284800u|((u32)vn<<5)|(u32)vd; }
static inline u32 a64_aesd  (u8 vd, u8 vn) { return 0x4E285800u|((u32)vn<<5)|(u32)vd; }
static inline u32 a64_aesmc (u8 vd, u8 vn) { return 0x4E286800u|((u32)vn<<5)|(u32)vd; }
static inline u32 a64_aesimc(u8 vd, u8 vn) { return 0x4E287800u|((u32)vn<<5)|(u32)vd; }

/* ── Acquire-release / Exclusive atomics (ARMv8.0) ───────────────────── */
/* LDAR Xt, [Xn]  — load-acquire */
static inline u32 a64_ldar (u8 rt, u8 rn) { return 0xC8DFFC00u|((u32)rn<<5)|(u32)rt; }
/* STLR Xt, [Xn]  — store-release */
static inline u32 a64_stlr (u8 rt, u8 rn) { return 0xC89FFC00u|((u32)rn<<5)|(u32)rt; }
/* LDAXR Xt, [Xn] — load-acquire exclusive */
static inline u32 a64_ldaxr(u8 rt, u8 rn) { return 0xC85FFC00u|((u32)rn<<5)|(u32)rt; }
/* STLXR Ws, Xt, [Xn] — store-release exclusive */
static inline u32 a64_stlxr(u8 rs, u8 rt, u8 rn) {
    return 0xC8008000u|((u32)rs<<16)|((u32)rn<<5)|(u32)rt;
}
/* LDXR Xt, [Xn] — load exclusive */
static inline u32 a64_ldxr (u8 rt, u8 rn) { return 0xC85F7C00u|((u32)rn<<5)|(u32)rt; }
/* STXR Ws, Xt, [Xn] — store exclusive */
static inline u32 a64_stxr (u8 rs, u8 rt, u8 rn) {
    return 0xC8007C00u|((u32)rs<<16)|((u32)rn<<5)|(u32)rt;
}
/* CAS Xs, Xt, [Xn]  — compare-and-swap (ARMv8.1 LSE) */
static inline u32 a64_cas(u8 rs, u8 rt, u8 rn) {
    return 0xC8A07C00u|((u32)rs<<16)|((u32)rn<<5)|(u32)rt;
}
/* Memory barriers */
#define A64_DMB_ISH  0xD5033BBFu
#define A64_DSB_ISH  0xD5033B9Fu
#define A64_ISB      0xD5033FDFu

/* ── Cache / prefetch hints ──────────────────────────────────────────── */
/* PRFM PLDL1KEEP, [Xn, #off*8] */
static inline u32 a64_prfm_l1(u8 rn, u16 off) {
    return 0xF9800000u|((u32)((off>>3)&0xFFFu)<<10)|((u32)rn<<5)|0u;
}
/* PRFM PLDL2KEEP, [Xn, #off*8] */
static inline u32 a64_prfm_l2(u8 rn, u16 off) {
    return 0xF9800000u|((u32)((off>>3)&0xFFFu)<<10)|((u32)rn<<5)|2u;
}
/* DC CIVAC, Xt  — clean+invalidate cache line */
static inline u32 a64_dc_civac(u8 rt) { return 0xD50B7E20u|(u32)rt; }
/* DC CVAC, Xt   — clean to PoC */
static inline u32 a64_dc_cvac (u8 rt) { return 0xD50B7C20u|(u32)rt; }
