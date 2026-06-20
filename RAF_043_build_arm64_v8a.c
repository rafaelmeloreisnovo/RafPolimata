#include "RAF_rafaelia_common.h"

/*
 * Método M043: Build arm64-v8a
 * Alvo: Android NDK / AArch64 Linux
 * Domínio: Build
 * Ganho estimado: compatibilidade + performance 64-bit
 *
 * Verifica invariantes do ABI arm64-v8a em tempo de compilação e execução.
 */

#define M043_TARGET_ABI_NAME "arm64-v8a"

/* Compile-time ABI invariants (universal — checked on all hosts) */
_Static_assert(sizeof(uint64_t) == 8u, "uint64_t must be 8 bytes (ARM64 LP64 ABI)");
_Static_assert(sizeof(uint32_t) == 4u, "uint32_t must be 4 bytes");
_Static_assert(sizeof(uint16_t) == 2u, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint8_t)  == 1u, "uint8_t must be 1 byte");

int rafaelia_m043_build_arm64_v8a(void)
{
    (void)M043_TARGET_ABI_NAME;

#if defined(__aarch64__)
    /* Running natively on arm64 — verify LP64 pointer width */
    if (sizeof(void *) != 8u) return -1;

    /* Verify NEON is available by attempting a trivial intrinsic-free
     * inline asm that uses a NEON register (v0) without touching memory */
    __asm__ __volatile__("movi v0.16b, #0" : : : "v0");

    return 0;
#else
    /*
     * Cross-host build probe: we are not on arm64 but we are validating
     * that the constants and types this ABI requires are correct.
     * sizeof checks done via _Static_assert above.
     */
    return 0;
#endif
}
