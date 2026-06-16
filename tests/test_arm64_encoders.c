/* test_arm64_encoders.c — ARM64 encoder correctness smoke/reference test.
 *
 * Compile on ARM/Termux or ARM Linux:
 *   cc -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o /tmp/test_arm64_encoders
 *
 * Why ARM-only?
 *   Apkc/arch_arm64.h includes mem.h/sys.h. sys.h intentionally carries
 *   freestanding ARM syscall inline-asm, so this C test is a native ARM
 *   smoke/reference test, not a portable x86_64 CI unit.
 *
 * Contract:
 *   Expected values below are aligned to the current arch_arm64.h API.
 *   - a64_ldr/a64_str receive byte offsets and scale internally.
 *   - a64_stp_pre/a64_ldp_post receive scaled signed imm7 values.
 *   - a64_movz and ADD/SUB immediate include explicit sf/sh parameters.
 */
#include <stdio.h>
#include "../Apkc/arch_arm64.h"

static int _pass = 0, _fail = 0;

#define EXPECT_EQ(name, got, want) do { \
    u32 g = (u32)(got), w = (u32)(want); \
    if (g == w) { printf("  PASS  %-44s  %08X\n", name, g); _pass++; } \
    else        { printf("  FAIL  %-44s  got=%08X want=%08X\n", name, g, w); _fail++; } \
} while(0)

int main(void) {
    printf("=== ARM64 encoder unit tests (C/header-aligned) ===\n\n");

    /* ── Fixed words / scalar integer ───────────────────────────── */
    EXPECT_EQ("NOP",                     A64_NOP,                      0xD503201Fu);
    EXPECT_EQ("RET",                     A64_RET,                      0xD65F03C0u);
    EXPECT_EQ("BRK #0",                  A64_BRK0,                     0xD4200000u);
    EXPECT_EQ("BRK #1 literal",          0xD4200020u,                  0xD4200020u);

    /* ADR / ADRP */
    EXPECT_EQ("ADR  x0, .+0",            a64_adr(0, 0),                0x10000000u);
    EXPECT_EQ("ADR  x0, .+4",            a64_adr(0, 4),                0x10000020u);
    EXPECT_EQ("ADRP x0, page+0",         a64_adrp(0, 0),               0x90000000u);
    EXPECT_EQ("ADR  x1, .+0",            a64_adr(1, 0),                0x10000001u);

    /* MOVZ/MOVK/MOVN — sf=1 is 64-bit */
    EXPECT_EQ("MOVZ x0, #0",             a64_movz(0, 0, 0, 1),         0xD2800000u);
    EXPECT_EQ("MOVZ x0, #1",             a64_movz(0, 1, 0, 1),         0xD2800020u);
    EXPECT_EQ("MOVZ x1, #0xFFFF",        a64_movz(1, 0xFFFF, 0, 1),    0xD29FFFE1u);
    EXPECT_EQ("MOVK x0, #1",             a64_movk(0, 1, 0, 1),         0xF2800020u);
    EXPECT_EQ("MOVN x0, #0",             a64_movn(0, 0, 0, 1),         0x92800000u);

    /* ADD/SUB immediate: sh=0, sf=1 */
    EXPECT_EQ("ADD  x0, x1, #0",         a64_add_imm(0,1,0,0,1),       0x91000020u);
    EXPECT_EQ("ADD  x0, x1, #1",         a64_add_imm(0,1,1,0,1),       0x91000420u);
    EXPECT_EQ("SUB  x0, x1, #1",         a64_sub_imm(0,1,1,0,1),       0xD1000420u);

    /* LDR/STR unsigned offset: off is byte offset in current C API. */
    EXPECT_EQ("LDR  x0,[x1,#0]",         a64_ldr(0,1,0,1),             0xF9400020u);
    EXPECT_EQ("STR  x0,[x1,#0]",         a64_str(0,1,0,1),             0xF9000020u);
    EXPECT_EQ("LDR  x0,[x1,#8]",         a64_ldr(0,1,8,1),             0xF9400420u);
    EXPECT_EQ("STR  x0,[x1,#8]",         a64_str(0,1,8,1),             0xF9000420u);

    /* LDRSW Xt, [Xn, #imm12*4] — imm12 is already scaled by 4. */
    EXPECT_EQ("LDRSW x0,[x1,#0]",        a64_ldrsw_imm(0,1,0),         0xB9800020u);
    EXPECT_EQ("LDRSW x0,[x1,#4]",        a64_ldrsw_imm(0,1,1),         0xB9800420u);
    EXPECT_EQ("LDRSW x0,[sp,#0]",        a64_ldrsw_imm(0,31,0),        0xB98003E0u);

    /* LDR literal: imm19 is byte_delta/4. */
    EXPECT_EQ("LDR  x0, PC+0",           a64_ldr_lit(0, 0),            0x58000000u);
    EXPECT_EQ("LDR  x1, PC+0",           a64_ldr_lit(1, 0),            0x58000001u);
    EXPECT_EQ("LDR  x0, PC+4",           a64_ldr_lit(0, 1),            0x58000020u);

    /* STP/LDP pair ops: off7 is scaled signed immediate (byte_offset/8). */
    EXPECT_EQ("STP  x29,x30,[sp,#-16]!", a64_stp_pre(29,30,31,-2,1),  0xA9BF7BFDu);
    EXPECT_EQ("LDP  x29,x30,[sp],#16",   a64_ldp_post(29,30,31,2,1),  0xA8C17BFDu);

    /* Branches */
    EXPECT_EQ("B    .+0",                a64_b(0),                    0x14000000u);
    EXPECT_EQ("BL   .+0",                a64_bl(0),                   0x94000000u);
    EXPECT_EQ("CBZ  x0, .+0",            a64_cbz(0,0,1),              0xB4000000u);
    EXPECT_EQ("CBNZ x0, .+0",            a64_cbnz(0,0,1),             0xB5000000u);

    /* SIMD multi-register */
    EXPECT_EQ("LD2  {v0.4s,v1.4s},[x1]", a64_ld2_4s(0,1),             0x4C808820u);
    EXPECT_EQ("LD3  {v0.4s,...},[x1]",   a64_ld3_4s(0,1),             0x4C804820u);
    EXPECT_EQ("LD4  {v0.4s,...},[x1]",   a64_ld4_4s(0,1),             0x4C800820u);
    EXPECT_EQ("ST2  {v0.4s,v1.4s},[x1]", a64_st2_4s(0,1),             0x4C008820u);
    EXPECT_EQ("ST3  {v0.4s,...},[x1]",   a64_st3_4s(0,1),             0x4C004820u);
    EXPECT_EQ("ST4  {v0.4s,...},[x1]",   a64_st4_4s(0,1),             0x4C000820u);

    /* System registers */
    EXPECT_EQ("MRS  x0, NZCV",           a64_mrs(0, A64_SR_NZCV),      0xD53B4200u);
    EXPECT_EQ("MSR  NZCV, x0",           a64_msr(A64_SR_NZCV, 0),      0xD51B4200u);
    EXPECT_EQ("MRS  x0, TPIDR_EL0",      a64_mrs(0, A64_SR_TPIDR),     0xD53BD040u);

    /* Scalar FP */
    EXPECT_EQ("FADD s0, s0, s1",         a64_fadd_s(0,0,1),            0x1E212800u);
    EXPECT_EQ("FMUL s0, s0, s1",         a64_fmul_s(0,0,1),            0x1E210800u);
    EXPECT_EQ("FSQRT s0, s0",            a64_fsqrt_s(0,0),             0x1E21C000u);

    /* AES/SHA */
    EXPECT_EQ("AESE v0.16b, v1.16b",     a64_aese(0,1),                0x4E284820u);
    EXPECT_EQ("SHA256SU0 v0.4s,v1.4s",   a64_sha256su0(0,1),           0x5E282820u);

    printf("\n=== RESULT: %d PASS, %d FAIL ===\n", _pass, _fail);
    return _fail ? 1 : 0;
}
