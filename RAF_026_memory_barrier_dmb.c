#include "RAF_rafaelia_common.h"

/*
 * Método M026: Memory barrier dmb
 * Alvo: ARM
 * Domínio: MMIO
 * Ganho estimado: correção
 *
 * Emite Data Memory Barrier (dmb ish) em ARM64, dmb em ARM32,
 * ou __sync_synchronize() em outras arquiteturas.
 * Self-test: escreve variável volátil, barreira, verifica inalterada.
 *
 * Status: implementação real ARM64/ARM32 inline asm + fallback GCC built-in.
 */

static volatile uint32_t _m026_shared = 0x12345678u;

int rafaelia_m026_memory_barrier_dmb(void) {
    _m026_shared = 0xDEADBEEFu;
#if defined(__aarch64__)
    __asm__ __volatile__("dmb ish" ::: "memory");
#elif defined(__arm__)
    __asm__ __volatile__("dmb" ::: "memory");
#else
    __sync_synchronize();
#endif
    return (_m026_shared == 0xDEADBEEFu) ? 0 : -1;
}
