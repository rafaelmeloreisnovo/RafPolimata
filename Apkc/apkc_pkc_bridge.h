/*
 * Apkc/apkc_pkc_bridge.h — PKC wordcode -> existing RAFIR_TINY_V1 bridge.
 *
 * This bridge does not replace ApkC. It lets the new word-oriented frontend
 * reuse the existing audited Stage0 ARM32 encoder.
 *
 * No libc, no heap, no syscall.
 */
#ifndef APKC_PKC_BRIDGE_H
#define APKC_PKC_BRIDGE_H 1

#include "../Pkc/pkc_wordcode.h"
#include "raf_stage0_arm32.h"

static inline i32 apkc_pkc_to_rafir_v1(
    const PkcInsnV1 *src, u32 count,
    RafIr32InsnV1 *dst, u32 cap)
{
    u32 i;
    if (!src || !dst || !count) return -1;
    if (count > cap) return -2;

    for (i = 0u; i < count; ++i) {
        RafIr32InsnV1 out = {0u, 0u, 0u, 0u, 0u};
        const PkcInsnV1 *in = &src[i];

        switch (in->op) {
            case PKC_OP_MOVE_IMM:
                out.op = RAF_IR32_MOV_IMM;
                out.rd = in->rd;
                out.imm = in->imm;
                break;
            case PKC_OP_MOVE_REG:
                out.op = RAF_IR32_MOV_REG;
                out.rd = in->rd;
                out.rm = in->rm;
                break;
            case PKC_OP_ADD_REG:
                out.op = RAF_IR32_ADD_REG;
                out.rd = in->rd;
                out.rn = in->rn;
                out.rm = in->rm;
                break;
            case PKC_OP_SUB_REG:
                out.op = RAF_IR32_SUB_REG;
                out.rd = in->rd;
                out.rn = in->rn;
                out.rm = in->rm;
                break;
            case PKC_OP_MUL_REG:
                out.op = RAF_IR32_MUL_REG;
                out.rd = in->rd;
                out.rn = in->rn;
                out.rm = in->rm;
                break;
            case PKC_OP_RETURN:
                out.op = RAF_IR32_RET;
                break;
            default:
                return -3;
        }
        dst[i] = out;
    }
    return 0;
}

#endif /* APKC_PKC_BRIDGE_H */
