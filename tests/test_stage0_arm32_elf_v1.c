#include "../Apkc/raf_stage0_arm32.h"

int main(void) {
    /* fmt_elf.h intentionally owns both target writers. Referencing the sibling
     * keeps this one-target smoke translation unit warning-clean under -Werror
     * without weakening diagnostics or changing the writer implementation. */
    (void)elf64_build_so;

    static const RafIr32InsnV1 ir[] = {
        { RAF_IR32_MOV_IMM, A32_R0, 0u, 0u, 42u },
        { RAF_IR32_RET,     0u,     0u, 0u, 0u  }
    };

    u8 text[64];
    u8 elf[4096];
    sz elf_n = 0;

    i32 rc = raf_stage0_ir32_to_elf32(
        ir, (u32)(sizeof(ir) / sizeof(ir[0])),
        text, (u32)sizeof(text),
        elf, sizeof(elf),
        "raf_entry", &elf_n);
    if (rc != 0) return 1;
    if (elf_n <= 52u) return 2;

    /* ELF32 little-endian, ET_DYN, EM_ARM. */
    if (elf[0] != 0x7Fu || elf[1] != 'E' || elf[2] != 'L' || elf[3] != 'F') return 3;
    if (elf[4] != 1u || elf[5] != 1u) return 4;
    if (elf[18] != 0x28u || elf[19] != 0x00u) return 5;

    /* Writer declares exactly PT_LOAD then PT_DYNAMIC; no PT_INTERP. */
    if (elf[0x34u] != 0x01u || elf[0x35u] != 0x00u || elf[0x36u] != 0x00u || elf[0x37u] != 0x00u) return 6;
    if (elf[0x54u] != 0x02u || elf[0x55u] != 0x00u || elf[0x56u] != 0x00u || elf[0x57u] != 0x00u) return 7;

    /* .text begins with MOV r0,#42 then BX lr, little-endian A32 words. */
    if (elf[A32SO_TEXT_BASE + 0u] != 0x2Au ||
        elf[A32SO_TEXT_BASE + 1u] != 0x00u ||
        elf[A32SO_TEXT_BASE + 2u] != 0xA0u ||
        elf[A32SO_TEXT_BASE + 3u] != 0xE3u) return 8;
    if (elf[A32SO_TEXT_BASE + 4u] != 0x1Eu ||
        elf[A32SO_TEXT_BASE + 5u] != 0xFFu ||
        elf[A32SO_TEXT_BASE + 6u] != 0x2Fu ||
        elf[A32SO_TEXT_BASE + 7u] != 0xE1u) return 9;

    /* Fail closed on insufficient output capacity. */
    u8 tiny[256];
    sz tiny_n = 0;
    if (raf_stage0_ir32_to_elf32(
            ir, (u32)(sizeof(ir) / sizeof(ir[0])),
            text, (u32)sizeof(text),
            tiny, sizeof(tiny),
            "raf_entry", &tiny_n) != -32) return 10;

    return 0;
}
