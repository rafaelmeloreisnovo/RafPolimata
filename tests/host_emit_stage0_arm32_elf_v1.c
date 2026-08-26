#include <stdio.h>
#include "../Apkc/raf_stage0_arm32.h"

int main(int argc, char **argv) {
    if (argc != 2 || !argv[1]) return 64;

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
    if (rc != 0 || !elf_n) return 65;

    FILE *f = fopen(argv[1], "wb");
    if (!f) return 66;
    if (fwrite(elf, 1u, (size_t)elf_n, f) != (size_t)elf_n) {
        fclose(f);
        return 67;
    }
    if (fflush(f) != 0) {
        fclose(f);
        return 68;
    }
    if (fclose(f) != 0) return 69;
    return 0;
}
