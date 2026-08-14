/* apkc_machine_to_arm64.h — Machine-to-ARM64 instruction encoder
 *
 * Zero-overhead conversion: Machine::Insn → ARM64 assembly
 * No function calls, direct register mapping, inline macros only.
 * Register mapping: r[0-15] → X[0-15] (machine regs to ARM64 GPRs)
 *
 * FREESTANDING: No malloc, no libc, no syscalls.
 */

#ifndef APKC_MACHINE_TO_ARM64_H
#define APKC_MACHINE_TO_ARM64_H 1

#include "machine_linear_branchless.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

/* === OUTPUT BUFFER & ENCODER STATE === */
struct Arm64Encoder {
    u8 *buf;                /* Output buffer */
    u32 pos;                /* Current position */
    u32 cap;                /* Capacity */
    u64 pc;                 /* Program counter (for branches) */
};

static inline void arm64_init(struct Arm64Encoder *enc, u8 *buf, u32 cap) {
    enc->buf = buf;
    enc->pos = 0;
    enc->cap = cap;
    enc->pc = 0;
}

static inline u8 arm64_emit(struct Arm64Encoder *enc, u32 insn) {
    if (enc->pos + 4 > enc->cap) return 1;
    *(u32*)&enc->buf[enc->pos] = insn;
    enc->pos += 4;
    enc->pc += 4;
    return 0;
}

/* === ARM64 INSTRUCTION ENCODING (Little-Endian, ARMv8 ISA) === */

/* ARM64 register encoding: Xn = n (0-31) */
#define ARM64_X(n) (n)

/* Operand shifter: logical shift left by amount */
#define ARM64_LSL(n) ((n) << 6)

/* Condition codes */
#define ARM64_EQ 0x0  /* Z set */
#define ARM64_NE 0x1  /* Z clear */
#define ARM64_CS 0x2  /* C set (unsigned >=) */
#define ARM64_CC 0x3  /* C clear (unsigned <) */

/* === DATA PROCESSING (3-operand register) === */

/* ADD Xd, Xn, Xm — d = n + m */
#define ARM64_ADD(d, n, m) \
    (0x8b000000 | ((m) << 16) | ((n) << 5) | (d))

/* SUB Xd, Xn, Xm — d = n - m */
#define ARM64_SUB(d, n, m) \
    (0xcb000000 | ((m) << 16) | ((n) << 5) | (d))

/* AND Xd, Xn, Xm — d = n & m */
#define ARM64_AND(d, n, m) \
    (0x8a000000 | ((m) << 16) | ((n) << 5) | (d))

/* ORR Xd, Xn, Xm — d = n | m */
#define ARM64_ORR(d, n, m) \
    (0xaa000000 | ((m) << 16) | ((n) << 5) | (d))

/* EOR Xd, Xn, Xm — d = n ^ m */
#define ARM64_EOR(d, n, m) \
    (0xca000000 | ((m) << 16) | ((n) << 5) | (d))

/* === SHIFTS (3-operand register) === */

/* LSL Xd, Xn, Xm — d = n << (m & 0x3F) */
#define ARM64_LSL_REG(d, n, m) \
    (0x9ac02000 | ((m) << 16) | ((n) << 5) | (d))

/* LSR Xd, Xn, Xm — d = n >> (m & 0x3F) */
#define ARM64_LSR_REG(d, n, m) \
    (0x9ac02400 | ((m) << 16) | ((n) << 5) | (d))

/* === MULTIPLY === */

/* MUL Xd, Xn, Xm — d = n * m */
#define ARM64_MUL(d, n, m) \
    (0x9b007c00 | ((m) << 16) | ((n) << 5) | (d))

/* === CONDITIONAL SELECT === */

/* CSEL Xd, Xn, Xm, cond — d = (cond) ? n : m */
#define ARM64_CSEL(d, n, m, cond) \
    (0x9a800000 | ((m) << 16) | ((cond) << 12) | ((n) << 5) | (d))

/* === IMMEDIATE MOVES === */

/* MOVZ Xd, imm16 — d = imm16 (zero extend) */
#define ARM64_MOVZ(d, imm) \
    (0xd2800000 | (((imm) & 0xffff) << 5) | (d))

/* MOVK Xd, imm16, lsl #(shift*16) — d = d | (imm16 << shift*16) */
#define ARM64_MOVK(d, imm, shift) \
    (0xf2800000 | (((shift) & 0x3) << 21) | (((imm) & 0xffff) << 5) | (d))

/* === COMPARE === */

/* CMP Xn, Xm — set flags based on n - m */
#define ARM64_CMP(n, m) \
    (0xeb000000 | ((m) << 16) | ((n) << 5) | 31)  /* XZR as destination */

/* === LOAD/STORE === */

/* LDR Xd, [Xn, imm] — d = mem[Xn + imm] (imm in bytes) */
#define ARM64_LDR(d, n, imm) \
    (0xf9400000 | ((((imm) >> 3) & 0x1fff) << 10) | ((n) << 5) | (d))

/* STR Xs, [Xn, imm] — mem[Xn + imm] = s */
#define ARM64_STR(s, n, imm) \
    (0xf9000000 | ((((imm) >> 3) & 0x1fff) << 10) | ((n) << 5) | (s))

/* === UNCONDITIONAL BRANCHES === */

/* B imm26 — jump to pc + (imm26 << 2) */
#define ARM64_B(imm26) \
    (0x14000000 | ((imm26) & 0x3ffffff))

/* B.cond imm19 — conditional branch */
#define ARM64_B_COND(imm19, cond) \
    (0x54000000 | (((imm19) & 0x7ffff) << 5) | (cond))

/* === RETURN === */

/* RET Xn — return to address in Xn (typically X30, link register) */
#define ARM64_RET(n) \
    (0xd65f0000 | ((n) << 5))

/* === ENCODING FUNCTION === */

/* Convert machine instruction to ARM64 encoding */
static inline u8 arm64_encode_insn(
    struct Arm64Encoder *enc,
    const struct Insn *insn)
{
    u32 arm64;
    u8 rd, rs1, rs2;

    if (enc->pos + 4 > enc->cap) return 1;

    /* Register mapping: r[0-15] → X[0-15] */
    rd = insn->rd & 0x0f;
    rs1 = insn->rs1 & 0x0f;
    rs2 = insn->rs2 & 0x0f;

    switch (insn->op) {
    /* ALU 3-operand */
    case OP_ADD:
        arm64 = ARM64_ADD(rd, rs1, rs2);
        break;
    case OP_SUB:
        arm64 = ARM64_SUB(rd, rs1, rs2);
        break;
    case OP_AND:
        arm64 = ARM64_AND(rd, rs1, rs2);
        break;
    case OP_OR:
        arm64 = ARM64_ORR(rd, rs1, rs2);
        break;
    case OP_XOR:
        arm64 = ARM64_EOR(rd, rs1, rs2);
        break;

    /* Shifts */
    case OP_SHL:
        arm64 = ARM64_LSL_REG(rd, rs1, rs2);
        break;
    case OP_SHR:
        arm64 = ARM64_LSR_REG(rd, rs1, rs2);
        break;

    /* Multiply */
    case OP_MUL:
        arm64 = ARM64_MUL(rd, rs1, rs2);
        break;

    /* Compare */
    case OP_CMP:
        arm64 = ARM64_CMP(rs1, rs2);
        break;

    /* Move immediate */
    case OP_MOVI:
        /* Load 16-bit immediate into Xd (zero-extend via MOVZ) */
        if (insn->imm > 0xffff) {
            /* If imm > 16 bits, use MOVZ + MOVK */
            if (arm64_emit(enc, ARM64_MOVZ(rd, insn->imm & 0xffff))) return 1;
            arm64 = ARM64_MOVK(rd, (insn->imm >> 16) & 0xffff, 1);
        } else {
            arm64 = ARM64_MOVZ(rd, insn->imm);
        }
        break;

    /* Move register */
    case OP_MOV:
        /* Use ORR Xd, Xn, XZR (same as mov in ARM64 semantics) */
        arm64 = ARM64_ORR(rd, rs1, 31);  /* 31 = XZR */
        break;

    /* Memory operations (pseudo: load/store assume X29 as base for now) */
    case OP_LOAD:
        /* LDR Xd, [Xrs1, imm] */
        arm64 = ARM64_LDR(rd, rs1, insn->imm);
        break;
    case OP_STORE:
        /* STR Xrs2, [Xrs1, imm] */
        arm64 = ARM64_STR(rs2, rs1, insn->imm);
        break;

    /* Conditional moves (using CSEL) */
    case OP_CMOV_Z:
        /* if Z: Xd = Xrs1 else Xd = Xd (use CSEL) */
        arm64 = ARM64_CSEL(rd, rs1, rd, ARM64_EQ);
        break;
    case OP_CMOV_NZ:
        /* if !Z: Xd = Xrs1 else Xd = Xd */
        arm64 = ARM64_CSEL(rd, rs1, rd, ARM64_NE);
        break;

    /* Control flow: translate jumps to branches */
    case OP_JMP:
        /* B imm — unconditional jump */
        /* imm26 = (target_offset >> 2) */
        arm64 = ARM64_B((insn->imm >> 2) & 0x3ffffff);
        break;

    case OP_JZ:
        /* B.eq imm — jump if zero flag set */
        arm64 = ARM64_B_COND((insn->imm >> 2) & 0x7ffff, ARM64_EQ);
        break;

    case OP_JNZ:
        /* B.ne imm — jump if zero flag clear */
        arm64 = ARM64_B_COND((insn->imm >> 2) & 0x7ffff, ARM64_NE);
        break;

    /* Function call: save return address in X30 (LR) */
    case OP_CALL:
        /* We need two instructions:
         * 1. ADR X30, pc + 8  (save return address)
         * 2. B target         (jump)
         * For now, use B only and assume LR is pre-set */
        arm64 = ARM64_B((insn->imm >> 2) & 0x3ffffff);
        break;

    /* Return: use RET X30 */
    case OP_RET:
        arm64 = ARM64_RET(30);  /* X30 = LR (link register) */
        break;

    /* No-op: use NOOP encoding (ORR X0, X0, X0) */
    case OP_NOOP:
        arm64 = ARM64_ORR(0, 0, 0);
        break;

    /* Default: unknown opcode */
    default:
        return 1;  /* error: unknown opcode */
    }

    return arm64_emit(enc, arm64);
}

/* === BATCH ENCODING === */

/* Encode all machine instructions to ARM64 assembly buffer */
static inline u8 arm64_encode_program(
    struct Arm64Encoder *enc,
    const struct Insn *code, u32 code_len)
{
    u32 i;

    if (!code || !code_len) return 1;

    for (i = 0; i < code_len; i++) {
        if (arm64_encode_insn(enc, &code[i])) {
            return 1;  /* overflow or unknown opcode */
        }
    }

    return 0;
}

#endif /* APKC_MACHINE_TO_ARM64_H */
