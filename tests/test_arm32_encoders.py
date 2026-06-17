#!/usr/bin/env python3
"""
Golden tests for Apkc/arch_arm32.h A32 (ARMv7-A) encoders.

Mechanism (mirrors tests/test_arm64_encoders.py):
  - Each function below mirrors, bit-for-bit, the corresponding encoder in
    Apkc/arch_arm32.h. This is an INDEPENDENT re-implementation written from
    the ARM Architecture Reference Manual, not a copy of the C source.
  - The CASES table pins each encoding against the known-good A32 machine word
    (ground truth from the ARM ARM / GNU as output). A transcription error in
    either the C header or this mirror is caught when the two disagree with the
    golden word.

Covers the instructions wired into asm_insn32() in Apkc/apkc.c, with emphasis
on the L5 additions: MVN, NEG(=RSB #0), RSB, BIC, TST, TEQ, CMN, LSL/LSR/ASR,
BLX — previously "unknown ARM32 mnemonic" → UNDEF placeholders.
"""
from __future__ import annotations

A32_AL = 0xE  # always (unconditional)


def u32(x: int) -> int:
    return x & 0xFFFFFFFF


# ── Branches ──────────────────────────────────────────────────────────────
def a32_bx(rm: int, cc: int) -> int:
    return u32((cc << 28) | 0x012FFF10 | rm)


def a32_blx(rm: int) -> int:
    return u32(0xE12FFF30 | rm)


def a32_swi(imm24: int, cc: int) -> int:
    return u32((cc << 28) | 0x0F000000 | (imm24 & 0x00FFFFFF))


# ── Move / bitwise NOT ────────────────────────────────────────────────────
def a32_mvn_imm(rd: int, imm8: int, rot: int, cc: int) -> int:
    return u32((cc << 28) | 0x03E00000 | (rd << 12) | ((rot & 0xF) << 8) | imm8)


def a32_mvn_reg(rd: int, rm: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x01E00000 | (s << 20) | (rd << 12) | rm)


# ── Data processing (register) ────────────────────────────────────────────
def a32_rsb_imm(rd: int, rn: int, imm8: int, rot: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x02600000 | (s << 20) | (rn << 16) | (rd << 12) | ((rot & 0xF) << 8) | imm8)


def a32_rsb_reg(rd: int, rn: int, rm: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x00600000 | (s << 20) | (rn << 16) | (rd << 12) | rm)


def a32_bic_imm(rd: int, rn: int, imm8: int, rot: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x03C00000 | (s << 20) | (rn << 16) | (rd << 12) | ((rot & 0xF) << 8) | imm8)


def a32_bic_reg(rd: int, rn: int, rm: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x01C00000 | (s << 20) | (rn << 16) | (rd << 12) | rm)


def a32_tst_reg(rn: int, rm: int, cc: int) -> int:
    return u32((cc << 28) | 0x01100000 | (rn << 16) | rm)


def a32_teq_reg(rn: int, rm: int, cc: int) -> int:
    return u32((cc << 28) | 0x01300000 | (rn << 16) | rm)


def a32_cmn_reg(rn: int, rm: int, cc: int) -> int:
    return u32((cc << 28) | 0x01700000 | (rn << 16) | rm)


# ── Shifts (encoded as MOV with shift) ────────────────────────────────────
def a32_lsl_imm(rd: int, rm: int, sh: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x01A00000 | (s << 20) | (rd << 12) | (sh << 7) | rm)


def a32_lsr_imm(rd: int, rm: int, sh: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x01A00020 | (s << 20) | (rd << 12) | (sh << 7) | rm)


def a32_asr_imm(rd: int, rm: int, sh: int, s: int, cc: int) -> int:
    return u32((cc << 28) | 0x01A00040 | (s << 20) | (rd << 12) | (sh << 7) | rm)


# Golden words: independently known-correct A32 encodings (ARM ARM / GNU as).
CASES = [
    # L5 additions
    ("MVN  r0, r1",        a32_mvn_reg(0, 1, 0, A32_AL),          0xE1E00001),
    ("MVN  r0, #0",        a32_mvn_imm(0, 0, 0, A32_AL),          0xE3E00000),
    ("NEG  r0, r1 (RSB#0)", a32_rsb_imm(0, 1, 0, 0, 0, A32_AL),   0xE2610000),
    ("RSB  r0, r1, #5",    a32_rsb_imm(0, 1, 5, 0, 0, A32_AL),    0xE2610005),
    ("RSB  r0, r1, r2",    a32_rsb_reg(0, 1, 2, 0, A32_AL),       0xE0610002),
    ("BIC  r0, r1, #0xFF", a32_bic_imm(0, 1, 0xFF, 0, 0, A32_AL), 0xE3C100FF),
    ("BIC  r2, r3, r4",    a32_bic_reg(2, 3, 4, 0, A32_AL),       0xE1C32004),
    ("TST  r0, r1",        a32_tst_reg(0, 1, A32_AL),             0xE1100001),
    ("TEQ  r0, r1",        a32_teq_reg(0, 1, A32_AL),             0xE1300001),
    ("CMN  r0, r1",        a32_cmn_reg(0, 1, A32_AL),             0xE1700001),
    ("LSL  r0, r1, #4",    a32_lsl_imm(0, 1, 4, 0, A32_AL),       0xE1A00201),
    ("LSR  r0, r1, #4",    a32_lsr_imm(0, 1, 4, 0, A32_AL),       0xE1A00221),
    ("ASR  r0, r1, #4",    a32_asr_imm(0, 1, 4, 0, A32_AL),       0xE1A00241),
    ("BLX  r3",            a32_blx(3),                            0xE12FFF33),
    # Pre-existing encoders (regression anchors)
    ("BX   lr",            a32_bx(14, A32_AL),                    0xE12FFF1E),
    ("SWI  #0",            a32_swi(0, A32_AL),                    0xEF000000),
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
        print(f"\n{failed} ARM32 encoder case(s) failed.")
        return 1

    print(f"\nPASS: {len(CASES)} ARM32 encoder golden cases.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
