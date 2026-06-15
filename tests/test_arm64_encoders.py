#!/usr/bin/env python3
"""
Golden tests for Apkc/arch_arm64.h A64 encoders.

These tests intentionally pin the interface used by the C header:
- a64_adr(off) receives a byte PC-relative offset.
- a64_ldr/a64_str(off) receive a byte offset and scale internally.
- a64_stp_pre/a64_ldp_post(off7) receive the signed scaled imm7
  field, i.e. byte_offset / 8 for 64-bit pair ops.

The four historical failures fixed here are:
  ADR x0, PC+4                         -> 0x10000020
  LDR x0, [x1, #8]                     -> 0xF9400420
  STP x29, x30, [sp, #-16]!            -> 0xA9BF7BFD
  LDP x29, x30, [sp], #16              -> 0xA8C17BFD
"""
from __future__ import annotations

R29 = 29
R30 = 30
RSP = 31
A64_SR_NZCV = 0x5A10


def u32(x: int) -> int:
    return x & 0xFFFFFFFF


def imm7(x: int) -> int:
    return x & 0x7F


def a64_adr(rd: int, off: int) -> int:
    # Mirrors arch_arm64.h:a64_adr().
    # immlo = off[1:0] goes to [30:29], immhi = off[20:2] goes to [23:5].
    return u32(0x10000000 | ((off & 3) << 29) | (((off >> 2) & 0x7FFFF) << 5) | rd)


def a64_ldr(rt: int, rn: int, off: int, sf: int) -> int:
    # Mirrors arch_arm64.h:a64_ldr(): off is a byte offset, not pre-scaled imm12.
    size = 3 if sf else 2
    scaled = (off >> 3) if sf else (off >> 2)
    return u32((size << 30) | (0x39 << 24) | (1 << 22) | (scaled << 10) | (rn << 5) | rt)


def a64_str(rt: int, rn: int, off: int, sf: int) -> int:
    size = 3 if sf else 2
    scaled = (off >> 3) if sf else (off >> 2)
    return u32((size << 30) | (0x39 << 24) | (scaled << 10) | (rn << 5) | rt)


def a64_stp_pre(t1: int, t2: int, rn: int, off7: int, sf: int) -> int:
    base = 0xA9800000 if sf else 0x29800000
    return u32(base | (imm7(off7) << 15) | (t2 << 10) | (rn << 5) | t1)


def a64_ldp_post(t1: int, t2: int, rn: int, off7: int, sf: int) -> int:
    base = 0xA8C00000 if sf else 0x28C00000
    return u32(base | (imm7(off7) << 15) | (t2 << 10) | (rn << 5) | t1)


def a64_ldrsw_imm(rt: int, rn: int, imm12: int) -> int:
    return u32(0xB9800000 | ((imm12 & 0xFFF) << 10) | (rn << 5) | rt)


def a64_ldr_lit(rt: int, imm19: int) -> int:
    return u32(0x58000000 | ((imm19 & 0x7FFFF) << 5) | rt)


def a64_mrs(rt: int, sr15: int) -> int:
    return u32(0xD5300000 | ((sr15 & 0x7FFF) << 5) | rt)


def a64_msr(sr15: int, rt: int) -> int:
    return u32(0xD5100000 | ((sr15 & 0x7FFF) << 5) | rt)


CASES = [
    ("ADR x0, PC+4", a64_adr(0, 4), 0x10000020),
    ("LDR x0, [x1, #8]", a64_ldr(0, 1, 8, 1), 0xF9400420),
    ("STR x0, [x1, #8]", a64_str(0, 1, 8, 1), 0xF9000420),
    ("STP x29, x30, [sp, #-16]!", a64_stp_pre(R29, R30, RSP, -2, 1), 0xA9BF7BFD),
    ("LDP x29, x30, [sp], #16", a64_ldp_post(R29, R30, RSP, 2, 1), 0xA8C17BFD),
    ("LDRSW x0, [x1, #8]", a64_ldrsw_imm(0, 1, 2), 0xB9800820),
    ("LDR x0, literal +4", a64_ldr_lit(0, 1), 0x58000020),
    ("MRS x0, NZCV", a64_mrs(0, A64_SR_NZCV), 0xD53B4200),
    ("MSR NZCV, x0", a64_msr(A64_SR_NZCV, 0), 0xD51B4200),
]


def main() -> int:
    failed = 0
    for name, got, expected in CASES:
        if got != expected:
            failed += 1
            print(f"FAIL {name}: got=0x{got:08X} expected=0x{expected:08X}")
        else:
            print(f"PASS {name}: 0x{got:08X}")

    if failed:
        print(f"\n{failed} ARM64 encoder case(s) failed.")
        return 1

    print(f"\nPASS: {len(CASES)} ARM64 encoder golden cases.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
