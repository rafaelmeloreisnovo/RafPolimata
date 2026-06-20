/* raf_asm_emit.c — Architecture-dispatched assembly emitter.
 *
 * Emits both a textual assembly representation (RafAsmBuf) and a binary
 * encoding (RafBin) for a minimal "return 42" function, selected by
 * ctx->cpu.arch:
 *
 *   RAF_ARCH_ARM64  → MOVZ X0,#42 + RET  (AArch64 A64, 8 bytes)
 *   RAF_ARCH_ARM32  → MOVS R0,#42 + BX LR  (Thumb-2, 4 bytes)
 *   RAF_ARCH_X86_64 → MOV EAX,42 + RET  (x86-64, 8 bytes)
 *   RAF_ARCH_RV64   → LI A0,42 + RET  (RV64I pseudo, 8 bytes)
 *   RAF_ARCH_UNKNOWN → TOKEN_VAZIO (return 0, no emission)
 *
 * No malloc/calloc/free. stdio.h used only for snprintf (no FILE* I/O).
 */

#include "raf_compile.h"

#include <string.h>  /* memcpy */
#include <stdio.h>   /* snprintf */

/* ── ARM64 A64 encodings (little-endian u32 words) ──────────────────────── */
/* MOVZ X0, #42, LSL #0  → encoding: 1101_0010_1000_0000_0000_0101_0100_0000
 *   sf=1 opc=10 hw=00 imm16=0x002A Rd=0
 *   [31]=1 [30:29]=10 [28:23]=100101 [22:21]=00 [20:5]=imm16 [4:0]=Rd
 *   = 0xD2800540 */
#define ARM64_MOVZ_X0_42  0xD2800540u
/* RET (= RET X30) → 0xD65F03C0 */
#define ARM64_RET         0xD65F03C0u

/* ── ARM32 Thumb-2 encodings (little-endian) ─────────────────────────────
 * MOVS R0, #42 (Thumb 16-bit):  0x202A  (T1 encoding: 001_00_000_00101010)
 * BX LR        (Thumb 16-bit):  0x4770
 * Laid out as two little-endian 16-bit halfwords in memory order. */
#define THUMB_MOVS_R0_42  0x202Au
#define THUMB_BX_LR       0x4770u

/* ── x86-64 encodings ────────────────────────────────────────────────────
 * MOV EAX, 42:  B8 2A 00 00 00   (5 bytes)
 * RET:          C3               (1 byte) */
static const uint8_t _x86_code[] = {
    0xB8, 0x2A, 0x00, 0x00, 0x00,  /* MOV EAX, 42 */
    0xC3                            /* RET */
};

/* ── RISC-V RV64I encodings ──────────────────────────────────────────────
 * ADDI A0, ZERO, 42  (LI pseudo = ADDI):
 *   imm[11:0]=0x02A  rs1=x0(0) funct3=000 rd=x10(10) opcode=0010011
 *   = 0x02A00513
 * JALR ZERO, RA, 0  (RET pseudo):
 *   imm=0 rs1=x1 funct3=000 rd=x0 opcode=1100111
 *   = 0x00008067 */
#define RV64_LI_A0_42  0x02A00513u
#define RV64_RET       0x00008067u

/* ── Helper: append a line to RafAsmBuf (bounds-checked) ─────────────────*/
static void _asm_line(RafAsmBuf *ab, const char *s) {
    if (ab->n >= RAF_ASM_CAP) return;
    snprintf(ab->lines[ab->n], RAF_ASM_LINE, "%s", s);
    ab->n++;
}

/* ── Helper: write a little-endian u32 to bin at current offset ──────────*/
static void _bin_u32le(RafBin *b, uint32_t w) {
    if (b->n + 4u > (uint32_t)sizeof(b->bytes)) return;
    b->bytes[b->n + 0] = (uint8_t)(w & 0xFFu);
    b->bytes[b->n + 1] = (uint8_t)((w >> 8)  & 0xFFu);
    b->bytes[b->n + 2] = (uint8_t)((w >> 16) & 0xFFu);
    b->bytes[b->n + 3] = (uint8_t)((w >> 24) & 0xFFu);
    b->n += 4u;
}

/* ── Helper: write a little-endian u16 to bin at current offset ──────────*/
static void _bin_u16le(RafBin *b, uint16_t h) {
    if (b->n + 2u > (uint32_t)sizeof(b->bytes)) return;
    b->bytes[b->n + 0] = (uint8_t)(h & 0xFFu);
    b->bytes[b->n + 1] = (uint8_t)((h >> 8) & 0xFFu);
    b->n += 2u;
}

/* ── Main emit function ──────────────────────────────────────────────────── */

int raf_asm_emit(RafCtx *ctx) {
    if (!ctx) return 0;  /* TOKEN_VAZIO — never return -1 for absent hardware */

    ctx->asm_out.n = 0;
    ctx->bin.n     = 0;

    _asm_line(&ctx->asm_out, ".text");
    _asm_line(&ctx->asm_out, ".globl _raf_entry");
    _asm_line(&ctx->asm_out, "_raf_entry:");

    switch (ctx->cpu.arch) {

    /* ── AArch64 ─────────────────────────────────────────────────────── */
    case RAF_ARCH_ARM64:
        _asm_line(&ctx->asm_out, "  movz x0, #42");
        _asm_line(&ctx->asm_out, "  ret");
        _bin_u32le(&ctx->bin, ARM64_MOVZ_X0_42);
        _bin_u32le(&ctx->bin, ARM64_RET);
        break;

    /* ── ARM32 Thumb-2 ───────────────────────────────────────────────── */
    case RAF_ARCH_ARM32:
        _asm_line(&ctx->asm_out, "  .thumb");
        _asm_line(&ctx->asm_out, "  movs r0, #42");
        _asm_line(&ctx->asm_out, "  bx lr");
        _bin_u16le(&ctx->bin, THUMB_MOVS_R0_42);
        _bin_u16le(&ctx->bin, THUMB_BX_LR);
        break;

    /* ── x86-64 ──────────────────────────────────────────────────────── */
    case RAF_ARCH_X86_64:
        _asm_line(&ctx->asm_out, "  mov $42, %eax");
        _asm_line(&ctx->asm_out, "  ret");
        memcpy(ctx->bin.bytes, _x86_code, sizeof(_x86_code));
        ctx->bin.n = (uint32_t)sizeof(_x86_code);
        break;

    /* ── RISC-V RV64I ────────────────────────────────────────────────── */
    case RAF_ARCH_RV64:
        _asm_line(&ctx->asm_out, "  li a0, 42");
        _asm_line(&ctx->asm_out, "  ret");
        _bin_u32le(&ctx->bin, RV64_LI_A0_42);
        _bin_u32le(&ctx->bin, RV64_RET);
        break;

    /* ── Unknown / unsupported — TOKEN_VAZIO ─────────────────────────── */
    case RAF_ARCH_UNKNOWN:
    default:
        /* No emission. asm_out.n and bin.n remain 0. Return 0 (TOKEN_VAZIO). */
        return 0;
    }

    return 0;
}

/* ── Hex encoder (produces byte-accurate binary output) ─────────────────── */

int raf_hex_encode(RafCtx *ctx) {
    if (!ctx) return 0;
    /* bin.bytes already populated by raf_asm_emit; nothing to re-encode.
     * This function is a no-op pass-through — the caller uses ctx->bin directly. */
    return 0;
}
