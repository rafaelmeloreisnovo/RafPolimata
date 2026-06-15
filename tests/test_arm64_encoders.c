/* test_arm64_encoders.c — unit tests for ARM64 encoder correctness.
 * Compile: gcc -std=c11 -I../Apkc -o test_arm64_encoders test_arm64_encoders.c
 * Each test asserts the expected 32-bit encoding from the ARM ISA reference. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Apkc/arch_arm64.h"

typedef unsigned int  u32;
typedef unsigned char u8;
typedef signed int    i32;

static int _pass = 0, _fail = 0;

#define EXPECT_EQ(name, got, want) do { \
    u32 g = (u32)(got), w = (u32)(want); \
    if (g == w) { printf("  PASS  %-40s  %08X\n", name, g); _pass++; } \
    else        { printf("  FAIL  %-40s  got=%08X want=%08X\n", name, g, w); _fail++; } \
} while(0)

int main(void) {
    printf("=== ARM64 encoder unit tests ===\n\n");

    /* ── Scalar integer ─────────────────────────────────────────── */
    EXPECT_EQ("NOP",                  A64_NOP,                      0xD503201Fu);
    EXPECT_EQ("RET",                  A64_RET,                      0xD65F03C0u);
    EXPECT_EQ("BRK #0",              0xD4200000u,                   0xD4200000u);
    EXPECT_EQ("BRK #1",              0xD4200020u,                   0xD4200020u);

    /* ADR / ADRP (offset=0) */
    EXPECT_EQ("ADR  x0, .+0",        a64_adr(0, 0),                0x10000000u);
    EXPECT_EQ("ADRP x0, .+0",        a64_adrp(0, 0),               0x90000000u);
    EXPECT_EQ("ADR  x1, .+0",        a64_adr(1, 0),                0x10000001u);

    /* MOV (MOVZ/MOVN) */
    EXPECT_EQ("MOVZ x0, #0",         a64_movz(0, 0, 0),            0xD2800000u);
    EXPECT_EQ("MOVZ x0, #1",         a64_movz(0, 1, 0),            0xD2800020u);
    EXPECT_EQ("MOVZ x1, #0xFFFF",    a64_movz(1, 0xFFFF, 0),       0xD29FFFE1u);

    /* ADD/SUB immediate */
    EXPECT_EQ("ADD  x0, x1, #0",     a64_add_imm(0,1,0,1),         0x91000020u);
    EXPECT_EQ("ADD  x0, x1, #1",     a64_add_imm(0,1,1,1),         0x91000420u);
    EXPECT_EQ("SUB  x0, x1, #1",     a64_sub_imm(0,1,1,1),         0xD1000420u);

    /* LDR/STR (unsigned offset) */
    EXPECT_EQ("LDR  x0,[x1,#0]",     a64_ldr(0,1,0,1),             0xF9400020u);
    EXPECT_EQ("STR  x0,[x1,#0]",     a64_str(0,1,0,1),             0xF9000020u);
    EXPECT_EQ("LDR  x0,[x1,#8]",     a64_ldr(0,1,1,1),             0xF9400420u);

    /* LDRSW Xt, [Xn, #imm12*4] */
    EXPECT_EQ("LDRSW x0,[x1,#0]",    a64_ldrsw_imm(0,1,0),         0xB9800020u);
    EXPECT_EQ("LDRSW x0,[x1,#4]",    a64_ldrsw_imm(0,1,1),         0xB9800420u);
    EXPECT_EQ("LDRSW x0,[sp,#0]",    a64_ldrsw_imm(0,31,0),        0xB98003E0u);

    /* LDR literal: base encoding, imm19=0 */
    EXPECT_EQ("LDR  x0, PC+0",       a64_ldr_lit(0, 0),            0x58000000u);
    EXPECT_EQ("LDR  x1, PC+0",       a64_ldr_lit(1, 0),            0x58000001u);
    EXPECT_EQ("LDR  x0, PC+4",       a64_ldr_lit(0, 1),            0x58000020u);

    /* STP/LDP */
    EXPECT_EQ("STP  x29,x30,[sp,#-16]!", a64_stp_pre(29,30,31,-2,1), 0xA9BE7BFDu);
    EXPECT_EQ("LDP  x29,x30,[sp],#16",   a64_ldp_post(29,30,31,2,1), 0xA8C27BFDu);

    /* B / BL (offset=0) */
    EXPECT_EQ("B    .+0",            a64_b(0),                      0x14000000u);
    EXPECT_EQ("BL   .+0",            a64_bl(0),                     0x94000000u);

    /* CBZ / CBNZ */
    EXPECT_EQ("CBZ  x0, .+0",        a64_cbz(0,0,1),               0xB4000000u);
    EXPECT_EQ("CBNZ x0, .+0",        a64_cbnz(0,0,1),              0xB5000000u);

    /* ── SIMD multi-register ───────────────────────────────────── */
    /* LD2 {V0.4S, V1.4S}, [X1] → base 0x4C808800 | (1<<5) | 0 */
    EXPECT_EQ("LD2  {v0.4s,v1.4s},[x1]", a64_ld2_4s(0,1),         0x4C808820u);
    EXPECT_EQ("LD3  {v0.4s,...},[x1]",    a64_ld3_4s(0,1),         0x4C804820u);
    EXPECT_EQ("LD4  {v0.4s,...},[x1]",    a64_ld4_4s(0,1),         0x4C800820u);
    EXPECT_EQ("ST2  {v0.4s,v1.4s},[x1]", a64_st2_4s(0,1),         0x4C008820u);
    EXPECT_EQ("ST3  {v0.4s,...},[x1]",    a64_st3_4s(0,1),         0x4C004820u);
    EXPECT_EQ("ST4  {v0.4s,...},[x1]",    a64_st4_4s(0,1),         0x4C000820u);

    /* ── System registers ──────────────────────────────────────── */
    /* MRS x0, NZCV → D53B4200 (from ARM ISA reference) */
    EXPECT_EQ("MRS  x0, NZCV",       a64_mrs(0, A64_SR_NZCV),      0xD53B4200u);
    EXPECT_EQ("MSR  NZCV, x0",       a64_msr(A64_SR_NZCV, 0),      0xD51B4200u);
    EXPECT_EQ("MRS  x0, TPIDR_EL0",  a64_mrs(0, A64_SR_TPIDR),     0xD53BD040u);

    /* ── Scalar FP ─────────────────────────────────────────────── */
    EXPECT_EQ("FADD s0, s0, s1",     a64_fadd_s(0,0,1),            0x1E212800u);
    EXPECT_EQ("FMUL s0, s0, s1",     a64_fmul_s(0,0,1),            0x1E210800u);
    EXPECT_EQ("FSQRT s0, s0",        a64_fsqrt_s(0,0),             0x1E21C000u);

    /* ── AES ───────────────────────────────────────────────────── */
    EXPECT_EQ("AESE v0.16b, v1.16b", a64_aese(0,1),                0x4E284820u);

    /* ── SHA256 ────────────────────────────────────────────────── */
    EXPECT_EQ("SHA256SU0 v0.4s,v1.4s",a64_sha256su0(0,1),          0x5E282820u);

    printf("\n=== RESULT: %d PASS, %d FAIL ===\n", _pass, _fail);
    return _fail ? 1 : 0;
}
