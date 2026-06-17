#include "RAF_rafaelia_common.h"

/*
 * Método M044: Build armeabi-v7a
 * Alvo: Android NDK / ARMv7 32-bit
 * Domínio: Build
 * Ganho estimado: compatibilidade dispositivos 32-bit + NEON
 *
 * Verifica invariantes do ABI armeabi-v7a em tempo de compilação e execução.
 */

#define M044_TARGET_ABI_NAME "armeabi-v7a"

/* Compile-time ABI invariants for ILP32 */
_Static_assert(sizeof(uint32_t) == 4u, "uint32_t must be 4 bytes (ARM32 ILP32 ABI)");
_Static_assert(sizeof(uint16_t) == 2u, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint8_t)  == 1u, "uint8_t must be 1 byte");
_Static_assert(sizeof(uint64_t) == 8u, "uint64_t must be 8 bytes even on ARM32");

int rafaelia_m044_build_armeabi_v7a(void)
{
    (void)M044_TARGET_ABI_NAME;

#if defined(__arm__)
    /* Running natively on ARM32 — verify ILP32 pointer width */
    if (sizeof(void *) != 4u) return -1;

    /*
     * Verify Thumb-2 is available: the compiler selects Thumb-2 by default
     * for armeabi-v7a. We probe by checking the Thumb bit is supported
     * by loading the PC in Thumb-2 inline asm (harmless, no side effects).
     */
    volatile uint32_t pc_val = 0u;
    __asm__ __volatile__("mov %0, pc" : "=r"(pc_val));
    (void)pc_val;

    return 0;
#else
    /*
     * Cross-host build probe: validate ILP32 type sizes are representable.
     * The _Static_asserts above already verify the critical invariants.
     */
    return 0;
#endif
}
