/* raf_stage0_arm32.h — bounded RAFAELIA Stage0 ARM32 core v1.
 *
 * Direct artifact route:
 *   RAFIR_TINY_V1 -> A32 machine words -> ELF32 shared object
 *
 * No heap, no external assembler, no external linker in this route.
 * Caller owns all buffers. This is intentionally a tiny bootstrap subset.
 */
#pragma once
#include "arch_arm32.h"
#include "fmt_elf.h"

#define RAF_STAGE0_ARM32_ELF_OVERHEAD_MAX 2048u
#define RAF_STAGE0_ARM32_EXPORT_MAX       63u

typedef enum {
    RAF_IR32_MOV_IMM = 1,
    RAF_IR32_MOV_REG = 2,
    RAF_IR32_ADD_REG = 3,
    RAF_IR32_SUB_REG = 4,
    RAF_IR32_MUL_REG = 5,
    RAF_IR32_BX_REG  = 6,
    RAF_IR32_RET     = 7,
    RAF_IR32_SWI     = 8
} RafIr32OpV1;

typedef struct {
    u8 op;
    u8 rd;
    u8 rn;
    u8 rm;
    u32 imm;
} RafIr32InsnV1;

static inline int raf_stage0_reg32_valid(u8 r) {
    return r <= 15u;
}

static inline sz raf_stage0_strnlen(const char *s, sz max_n) {
    sz n = 0;
    if (!s) return 0;
    while (n < max_n && s[n]) n++;
    return n;
}

/* Returns 0 on success; negative values are fail-closed parse/codegen states. */
static inline i32 raf_stage0_ir32_encode(
    const RafIr32InsnV1 *ir, u32 count,
    u8 *text_out, u32 text_cap, u32 *text_size_out)
{
    if (!ir || !count || !text_out || !text_size_out) return -1;
    if (count > text_cap / 4u) return -2;

    u32 pos = 0;
    for (u32 i = 0; i < count; i++) {
        const RafIr32InsnV1 *in = &ir[i];
        u32 w = 0;

        switch ((RafIr32OpV1)in->op) {
            case RAF_IR32_MOV_IMM:
                if (!raf_stage0_reg32_valid(in->rd) || in->imm > 0xFFu) return -10;
                w = a32_mov_imm(in->rd, (u8)in->imm, 0u, 0u, A32_AL);
                break;
            case RAF_IR32_MOV_REG:
                if (!raf_stage0_reg32_valid(in->rd) || !raf_stage0_reg32_valid(in->rm)) return -11;
                w = a32_mov_reg(in->rd, in->rm, 0u, A32_AL);
                break;
            case RAF_IR32_ADD_REG:
                if (!raf_stage0_reg32_valid(in->rd) || !raf_stage0_reg32_valid(in->rn) ||
                    !raf_stage0_reg32_valid(in->rm)) return -12;
                w = a32_add_reg(in->rd, in->rn, in->rm, 0u, A32_AL);
                break;
            case RAF_IR32_SUB_REG:
                if (!raf_stage0_reg32_valid(in->rd) || !raf_stage0_reg32_valid(in->rn) ||
                    !raf_stage0_reg32_valid(in->rm)) return -13;
                w = a32_sub_reg(in->rd, in->rn, in->rm, 0u, A32_AL);
                break;
            case RAF_IR32_MUL_REG:
                if (!raf_stage0_reg32_valid(in->rd) || !raf_stage0_reg32_valid(in->rn) ||
                    !raf_stage0_reg32_valid(in->rm)) return -14;
                /* A32 MUL syntax: Rd, Rm, Rs. rn is used as Rm and rm as Rs. */
                w = a32_mul(in->rd, in->rn, in->rm, 0u, A32_AL);
                break;
            case RAF_IR32_BX_REG:
                if (!raf_stage0_reg32_valid(in->rm)) return -15;
                w = a32_bx(in->rm, A32_AL);
                break;
            case RAF_IR32_RET:
                w = A32_BXLR;
                break;
            case RAF_IR32_SWI:
                if (in->imm > 0x00FFFFFFu) return -16;
                w = a32_swi(in->imm, A32_AL);
                break;
            default:
                return -20;
        }

        if (pos + 4u > text_cap) return -3;
        w32(text_out + pos, w);
        pos += 4u;
    }

    *text_size_out = pos;
    return 0;
}

/*
 * Builds one ARM32 ELF32 shared object exporting `export_name` at text offset 0.
 * The conservative capacity gate runs before the existing writer, whose API
 * predates explicit capacity passing.
 */
static inline i32 raf_stage0_ir32_to_elf32(
    const RafIr32InsnV1 *ir, u32 count,
    u8 *text_tmp, u32 text_cap,
    u8 *elf_out, sz elf_cap,
    const char *export_name,
    sz *elf_size_out)
{
    if (!elf_out || !elf_size_out || !export_name) return -30;

    sz export_n = raf_stage0_strnlen(export_name, RAF_STAGE0_ARM32_EXPORT_MAX + 1u);
    if (!export_n || export_n > RAF_STAGE0_ARM32_EXPORT_MAX) return -31;

    u32 text_size = 0;
    i32 rc = raf_stage0_ir32_encode(ir, count, text_tmp, text_cap, &text_size);
    if (rc != 0) return rc;

    if (elf_cap < (sz)text_size + (sz)RAF_STAGE0_ARM32_ELF_OVERHEAD_MAX) return -32;

    ElfSym sym;
    sym.name = export_name;
    sym.va = 0u;

    sz elf_size = elf32_build_so(elf_out, text_tmp, text_size, &sym, 1);
    if (!elf_size || elf_size > elf_cap) return -33;

    *elf_size_out = elf_size;
    return 0;
}
