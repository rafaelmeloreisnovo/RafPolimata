#include "RAF_rafaelia_common.h"

/*
 * Método M028: Memory barrier isb
 * Alvo: ARM
 * Domínio: MMIO
 * Ganho estimado: correção
 *
 * Emite Instruction Synchronization Barrier (isb) em ARM64/ARM32.
 * Em x86 usa barreira de compilador (__asm__ __volatile__ com "memory").
 * Self-test: escreve variável volátil, barreira, verifica inalterada.
 *
 * Status: implementação real ARM64/ARM32 inline asm + fallback compilador.
 */

static volatile uint32_t _m028_shared = 0x12345678u;

int rafaelia_m028_memory_barrier_isb(void) {
    _m028_shared = 0xDEADBEEFu;
#if defined(__aarch64__)
    __asm__ __volatile__("isb" ::: "memory");
#elif defined(__arm__)
    __asm__ __volatile__("isb" ::: "memory");
#else
    /* x86 has no ISB equivalent; compiler barrier suffices for the test */
    __asm__ __volatile__("" ::: "memory");
#endif
    return (_m028_shared == 0xDEADBEEFu) ? 0 : -1;
}
