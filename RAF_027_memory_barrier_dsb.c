#include "RAF_rafaelia_common.h"

/*
 * Método M027: Memory barrier dsb
 * Alvo: ARM
 * Domínio: MMIO
 * Ganho estimado: correção
 *
 * Emite Data Synchronization Barrier (dsb ish) em ARM64, dsb em ARM32,
 * ou __sync_synchronize() em outras arquiteturas.
 * Self-test: escreve variável volátil, barreira, verifica inalterada.
 *
 * Status: implementação real ARM64/ARM32 inline asm + fallback GCC built-in.
 */

static volatile uint32_t _m027_shared = 0x12345678u;

int rafaelia_m027_memory_barrier_dsb(void) {
    _m027_shared = 0xDEADBEEFu;
#if defined(__aarch64__)
    __asm__ __volatile__("dsb ish" ::: "memory");
#elif defined(__arm__)
    __asm__ __volatile__("dsb" ::: "memory");
#else
    __sync_synchronize();
#endif
    return (_m027_shared == 0xDEADBEEFu) ? 0 : -1;
}
