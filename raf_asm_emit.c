#include "raf_compile.h"
#include <stdio.h>

static void asm_line(RafAsmBuf *ab, const char *s) {
    if (ab->n >= RAF_ASM_CAP) return;
    (void)snprintf(ab->lines[ab->n], RAF_ASM_LINE, "%s", s);
    ++ab->n;
}

static void asm_line_imm(RafAsmBuf *ab, const char *fmt, uint32_t value) {
    if (ab->n >= RAF_ASM_CAP) return;
    (void)snprintf(ab->lines[ab->n], RAF_ASM_LINE, fmt, value);
    ++ab->n;
}

static int bin_u8(RafBin *b, uint8_t v) {
    if (b->n + 1u > (uint32_t)sizeof(b->bytes)) return -1;
    b->bytes[b->n++] = v;
    return 0;
}

static int bin_u16le(RafBin *b, uint16_t v) {
    if (b->n + 2u > (uint32_t)sizeof(b->bytes)) return -1;
    b->bytes[b->n++] = (uint8_t)v;
    b->bytes[b->n++] = (uint8_t)(v >> 8);
    return 0;
}

static int bin_u32le(RafBin *b, uint32_t v) {
    if (b->n + 4u > (uint32_t)sizeof(b->bytes)) return -1;
    b->bytes[b->n++] = (uint8_t)v;
    b->bytes[b->n++] = (uint8_t)(v >> 8);
    b->bytes[b->n++] = (uint8_t)(v >> 16);
    b->bytes[b->n++] = (uint8_t)(v >> 24);
    return 0;
}

static int ir_return_u32(const RafCtx *ctx, uint32_t *value) {
    if (!ctx || !value || ctx->ir.n != 2u) return -1;
    if ((uint8_t)(ctx->ir.buf[0] >> 56) != (uint8_t)IR_MOVIMM) return -1;
    if ((uint8_t)(ctx->ir.buf[1] >> 56) != (uint8_t)IR_RET) return -1;
    *value = (uint32_t)ctx->ir.buf[0];
    return 0;
}

static int emit_arm64(RafCtx *ctx, uint32_t value) {
    uint32_t lo = value & 0xffffu;
    uint32_t hi = value >> 16;
    asm_line_imm(&ctx->asm_out, "  movz x0, #0x%04x", lo);
    if (bin_u32le(&ctx->bin, 0xD2800000u | (lo << 5)) != 0) return -1;
    if (hi != 0u) {
        asm_line_imm(&ctx->asm_out, "  movk x0, #0x%04x, lsl #16", hi);
        if (bin_u32le(&ctx->bin, 0xF2A00000u | (hi << 5)) != 0) return -1;
    }
    asm_line(&ctx->asm_out, "  ret");
    return bin_u32le(&ctx->bin, 0xD65F03C0u);
}

static int emit_thumb_mov16(RafBin *bin, uint16_t value, int top) {
    uint16_t imm4 = (uint16_t)((value >> 12) & 0x000fu);
    uint16_t i = (uint16_t)((value >> 11) & 0x0001u);
    uint16_t imm3 = (uint16_t)((value >> 8) & 0x0007u);
    uint16_t imm8 = (uint16_t)(value & 0x00ffu);
    uint16_t first = (uint16_t)((top ? 0xF2C0u : 0xF240u) | (uint16_t)(i << 10) | imm4);
    uint16_t second = (uint16_t)((uint16_t)(imm3 << 12) | imm8); /* Rd = r0 */
    if (bin_u16le(bin, first) != 0) return -1;
    return bin_u16le(bin, second);
}

static int emit_arm32(RafCtx *ctx, uint32_t value) {
    uint16_t lo = (uint16_t)value;
    uint16_t hi = (uint16_t)(value >> 16);
    asm_line(&ctx->asm_out, "  .thumb");
    asm_line_imm(&ctx->asm_out, "  movw r0, #0x%04x", lo);
    if (emit_thumb_mov16(&ctx->bin, lo, 0) != 0) return -1;
    if (hi != 0u) {
        asm_line_imm(&ctx->asm_out, "  movt r0, #0x%04x", hi);
        if (emit_thumb_mov16(&ctx->bin, hi, 1) != 0) return -1;
    }
    asm_line(&ctx->asm_out, "  bx lr");
    return bin_u16le(&ctx->bin, 0x4770u);
}

static int emit_x86_64(RafCtx *ctx, uint32_t value) {
    asm_line_imm(&ctx->asm_out, "  mov $0x%08x, %eax", value);
    asm_line(&ctx->asm_out, "  ret");
    if (bin_u8(&ctx->bin, 0xB8u) != 0) return -1;
    if (bin_u32le(&ctx->bin, value) != 0) return -1;
    return bin_u8(&ctx->bin, 0xC3u);
}

static int emit_rv64(RafCtx *ctx, uint32_t value) {
    int64_t signed_value = (int64_t)(int32_t)value;
    int64_t upper = (signed_value + 0x800ll) >> 12;
    int64_t lower = signed_value - (upper << 12);
    asm_line_imm(&ctx->asm_out, "  li a0, 0x%08x", value);
    if (upper != 0) {
        uint32_t lui = ((uint32_t)upper & 0x000fffffu) << 12;
        lui |= (10u << 7) | 0x37u;
        if (bin_u32le(&ctx->bin, lui) != 0) return -1;
        uint32_t addi = (((uint32_t)lower & 0x0fffu) << 20) |
                        (10u << 15) | (10u << 7) | 0x13u;
        if (bin_u32le(&ctx->bin, addi) != 0) return -1;
    } else {
        uint32_t addi = (((uint32_t)lower & 0x0fffu) << 20) | (10u << 7) | 0x13u;
        if (bin_u32le(&ctx->bin, addi) != 0) return -1;
    }
    asm_line(&ctx->asm_out, "  ret");
    return bin_u32le(&ctx->bin, 0x00008067u);
}

int raf_asm_emit(RafCtx *ctx) {
    uint32_t value;
    if (ir_return_u32(ctx, &value) != 0) return -1;
    ctx->asm_out.n = 0u;
    ctx->bin.n = 0u;
    asm_line(&ctx->asm_out, ".text");
    asm_line(&ctx->asm_out, ".globl _raf_entry");
    asm_line(&ctx->asm_out, "_raf_entry:");
    switch (ctx->cpu.arch) {
    case RAF_ARCH_ARM64: return emit_arm64(ctx, value);
    case RAF_ARCH_ARM32: return emit_arm32(ctx, value);
    case RAF_ARCH_X86_64: return emit_x86_64(ctx, value);
    case RAF_ARCH_RV64: return emit_rv64(ctx, value);
    default: return -2;
    }
}

int raf_hex_encode(RafCtx *ctx) {
    return ctx && ctx->bin.n != 0u ? 0 : -1;
}
