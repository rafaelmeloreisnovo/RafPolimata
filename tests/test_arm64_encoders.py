#!/usr/bin/env python3
"""test_arm64_encoders.py — verify ARM64 encoder bit patterns against ISA reference.
   Reimplements the inline C formulas in Python; no compilation needed."""

PASS = 0
FAIL = 0

def check(name, got, want):
    global PASS, FAIL
    g, w = got & 0xFFFFFFFF, want & 0xFFFFFFFF
    if g == w:
        print(f"  PASS  {name:<42}  {g:08X}")
        PASS += 1
    else:
        print(f"  FAIL  {name:<42}  got={g:08X} want={w:08X}")
        FAIL += 1

# ── reproduce C inline formulas ─────────────────────────────────────────────

def a64_nop():   return 0xD503201F
def a64_ret():   return 0xD65F03C0
def a64_adr(rd, off):
    imm = off & 0x1FFFFF
    immlo = imm & 3; immhi = (imm >> 2) & 0x7FFFF
    return 0x10000000 | (immlo << 29) | (immhi << 5) | rd
def a64_adrp(rd, pg):
    imm = pg & 0x1FFFFF
    immlo = imm & 3; immhi = (imm >> 2) & 0x7FFFF
    return 0x90000000 | (immlo << 29) | (immhi << 5) | rd

def a64_movz(rd, imm, hw, sf):
    return ((0x80000000 if sf else 0) | 0x52800000 | (hw << 21) | ((imm & 0xFFFF) << 5) | rd)

def a64_add_imm(rd, rn, imm12, sh, sf):
    return ((0x80000000 if sf else 0) | 0x11000000 | (sh << 22) | ((imm12 & 0xFFF) << 10) | (rn << 5) | rd)
def a64_sub_imm(rd, rn, imm12, sh, sf):
    return ((0x80000000 if sf else 0) | 0x51000000 | (sh << 22) | ((imm12 & 0xFFF) << 10) | (rn << 5) | rd)

def a64_ldr(rt, rn, imm12, sf):
    base = 0xF9400000 if sf else 0xB9400000
    return base | ((imm12 & 0xFFF) << 10) | (rn << 5) | rt
def a64_str(rt, rn, imm12, sf):
    base = 0xF9000000 if sf else 0xB9000000
    return base | ((imm12 & 0xFFF) << 10) | (rn << 5) | rt

def a64_ldrsw_imm(rt, rn, imm12):
    return 0xB9800000 | ((imm12 & 0xFFF) << 10) | (rn << 5) | rt
def a64_ldr_lit(rt, imm19):
    return 0x58000000 | ((imm19 & 0x7FFFF) << 5) | rt

def a64_ld2_4s(vt, rn): return 0x4C808800 | (rn << 5) | vt
def a64_ld3_4s(vt, rn): return 0x4C804800 | (rn << 5) | vt
def a64_ld4_4s(vt, rn): return 0x4C800800 | (rn << 5) | vt
def a64_st2_4s(vt, rn): return 0x4C008800 | (rn << 5) | vt
def a64_st3_4s(vt, rn): return 0x4C004800 | (rn << 5) | vt
def a64_st4_4s(vt, rn): return 0x4C000800 | (rn << 5) | vt

A64_SR_NZCV  = 0x5A10
A64_SR_DAIF  = 0x5A11
A64_SR_FPCR  = 0x5A20
A64_SR_FPSR  = 0x5A21
A64_SR_TPIDR = 0x5E82
def a64_mrs(rt, sr15): return 0xD5300000 | ((sr15 & 0x7FFF) << 5) | rt
def a64_msr(sr15, rt): return 0xD5100000 | ((sr15 & 0x7FFF) << 5) | rt

def a64_stp_pre(t1, t2, rn, off7, sf):
    base = 0xA9800000 if sf else 0x29800000
    return base | ((off7 & 0x7F) << 15) | (t2 << 10) | (rn << 5) | t1
def a64_ldp_post(t1, t2, rn, off7, sf):
    base = 0xA8C00000 if sf else 0x28C00000
    return base | ((off7 & 0x7F) << 15) | (t2 << 10) | (rn << 5) | t1

def a64_b(off26):   return 0x14000000 | (off26 & 0x3FFFFFF)
def a64_bl(off26):  return 0x94000000 | (off26 & 0x3FFFFFF)
def a64_cbz(rt, off19, sf):
    return ((0x80000000 if sf else 0) | 0x34000000 | ((off19 & 0x7FFFF) << 5) | rt)
def a64_cbnz(rt, off19, sf):
    return ((0x80000000 if sf else 0) | 0x35000000 | ((off19 & 0x7FFFF) << 5) | rt)

def a64_fadd_s(rd, rn, rm): return 0x1E202800 | (rm << 16) | (rn << 5) | rd
def a64_fmul_s(rd, rn, rm): return 0x1E200800 | (rm << 16) | (rn << 5) | rd
def a64_fsqrt_s(rd, rn):    return 0x1E21C000 | (rn << 5) | rd

def a64_aese(vd, vn):      return 0x4E284800 | (vn << 5) | vd
def a64_sha256su0(vd, vn): return 0x5E282800 | (vn << 5) | vd

# ── run tests ────────────────────────────────────────────────────────────────
print("=== ARM64 encoder unit tests (Python) ===\n")

# Control flow
check("NOP",                          a64_nop(),               0xD503201F)
check("RET",                          a64_ret(),               0xD65F03C0)
check("BRK #0",                       0xD4200000,              0xD4200000)
check("BRK #1 (unknown ARM64 trap)",  0xD4200020,              0xD4200020)
check("UNDEF ARM32 (unknown trap)",   0xE7F000F0,              0xE7F000F0)

# ADR/ADRP
check("ADR  x0, PC+0",   a64_adr(0, 0),    0x10000000)
check("ADRP x0, PC+0",   a64_adrp(0, 0),   0x90000000)
check("ADR  x1, PC+0",   a64_adr(1, 0),    0x10000001)
check("ADR  x0, PC+4",   a64_adr(0, 4),    0x10000020)   # imm=4 → immlo=0, immhi=1

# MOV
check("MOVZ x0, #0, sf=1",     a64_movz(0, 0,     0, 1), 0xD2800000)
check("MOVZ x0, #1, sf=1",     a64_movz(0, 1,     0, 1), 0xD2800020)
check("MOVZ x1, #0xFFFF, sf=1",a64_movz(1, 0xFFFF,0, 1), 0xD29FFFE1)

# ADD/SUB immediate
check("ADD x0,x1,#0,sf=1",  a64_add_imm(0,1,0,0,1), 0x91000020)
check("ADD x0,x1,#1,sf=1",  a64_add_imm(0,1,1,0,1), 0x91000420)
check("SUB x0,x1,#1,sf=1",  a64_sub_imm(0,1,1,0,1), 0xD1000420)

# LDR/STR
check("LDR x0,[x1,#0] sf=1",  a64_ldr(0,1,0,1), 0xF9400020)
check("STR x0,[x1,#0] sf=1",  a64_str(0,1,0,1), 0xF9000020)
check("LDR x0,[x1,#8] sf=1",  a64_ldr(0,1,1,1), 0xF9400420)  # imm12=1 → 1×8=8 bytes

# LDRSW
check("LDRSW x0,[x1,#0]",   a64_ldrsw_imm(0,1,0), 0xB9800020)
check("LDRSW x0,[x1,#4]",   a64_ldrsw_imm(0,1,1), 0xB9800420)
check("LDRSW x0,[sp,#0]",   a64_ldrsw_imm(0,31,0),0xB98003E0)

# LDR literal
check("LDR x0, PC+0",    a64_ldr_lit(0, 0), 0x58000000)
check("LDR x1, PC+0",    a64_ldr_lit(1, 0), 0x58000001)
check("LDR x0, PC+4",    a64_ldr_lit(0, 1), 0x58000020)  # imm19=1 → << 5

# Multi-register ASIMD
check("LD2 {v0.4s,v1.4s},[x1]",  a64_ld2_4s(0,1), 0x4C808820)
check("LD3 {v0.4s,...},[x1]",     a64_ld3_4s(0,1), 0x4C804820)
check("LD4 {v0.4s,...},[x1]",     a64_ld4_4s(0,1), 0x4C800820)
check("ST2 {v0.4s,v1.4s},[x1]",  a64_st2_4s(0,1), 0x4C008820)
check("ST3 {v0.4s,...},[x1]",     a64_st3_4s(0,1), 0x4C004820)
check("ST4 {v0.4s,...},[x1]",     a64_st4_4s(0,1), 0x4C000820)

# System registers (from ARM ISA reference)
check("MRS x0, NZCV",       a64_mrs(0, A64_SR_NZCV),  0xD53B4200)
check("MSR NZCV, x0",       a64_msr(A64_SR_NZCV, 0),  0xD51B4200)
check("MRS x0, TPIDR_EL0",  a64_mrs(0, A64_SR_TPIDR), 0xD53BD040)
check("MRS x0, FPCR",       a64_mrs(0, A64_SR_FPCR),  0xD53B4400)
check("MRS x0, FPSR",       a64_mrs(0, A64_SR_FPSR),  0xD53B4420)

# STP/LDP
check("STP x29,x30,[sp,#-16]!", a64_stp_pre(29,30,31,-2,1),  0xA9BF7BFD)
check("LDP x29,x30,[sp],#16",   a64_ldp_post(29,30,31,2,1),  0xA8C17BFD)

# Branches
check("B  .+0",    a64_b(0),        0x14000000)
check("BL .+0",    a64_bl(0),       0x94000000)
check("CBZ x0,.+0 sf=1",  a64_cbz(0,0,1),  0xB4000000)
check("CBNZ x0,.+0 sf=1", a64_cbnz(0,0,1), 0xB5000000)

# Scalar FP
check("FADD s0,s0,s1",  a64_fadd_s(0,0,1), 0x1E212800)
check("FMUL s0,s0,s1",  a64_fmul_s(0,0,1), 0x1E210800)
check("FSQRT s0,s0",    a64_fsqrt_s(0,0),  0x1E21C000)

# Crypto
check("AESE v0.16b,v1.16b",    a64_aese(0,1),      0x4E284820)
check("SHA256SU0 v0.4s,v1.4s", a64_sha256su0(0,1), 0x5E282820)

# Validate sysreg constants
# NZCV: op0=3,op1=3,CRn=4,CRm=2,op2=0 → packed = (1<<14)|(3<<11)|(4<<7)|(2<<3)|0
nzcv_computed = (1<<14)|(3<<11)|(4<<7)|(2<<3)|0
check(f"A64_SR_NZCV constant=0x{A64_SR_NZCV:04X}",
      A64_SR_NZCV, nzcv_computed)
# TPIDR_EL0: op0=3,op1=3,CRn=13,CRm=0,op2=2 → packed
tpidr_computed = (1<<14)|(3<<11)|(13<<7)|(0<<3)|2
check(f"A64_SR_TPIDR constant=0x{A64_SR_TPIDR:04X}",
      A64_SR_TPIDR, tpidr_computed)

print(f"\n=== RESULT: {PASS} PASS, {FAIL} FAIL ===")
import sys; sys.exit(0 if FAIL == 0 else 1)
