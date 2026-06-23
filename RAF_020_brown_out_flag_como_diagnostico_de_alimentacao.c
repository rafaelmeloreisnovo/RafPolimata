#include "RAF_rafaelia_common.h"

/*
 * Método M020: Brown-out flag como diagnóstico de alimentação
 * Alvo: MCU/AVR
 * Domínio: Power/Diagnóstico
 * Ganho estimado: detecção de reset por queda de tensão
 *
 * Lê BORF (bit 2) de MCUSR para detectar brown-out reset.
 * Limpa MCUSR após leitura (necessário para WDT funcionar corretamente).
 * Non-AVR: retorna 0 (sem diagnóstico real disponível em x86).
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_MCUSR_ADDR   0x54u

/* MCUSR bits */
#define M020_BORF  2u   /* Brown-out Reset Flag */
#define M020_EXTRF 1u   /* External Reset Flag */
#define M020_PORF  0u   /* Power-on Reset Flag */
#define M020_WDRF  3u   /* Watchdog Reset Flag */
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
/*
 * rafaelia_m020_was_brown_out:
 * Returns 1 if the last reset was caused by a brown-out, 0 otherwise.
 * Clears MCUSR after reading (required before enabling WDT).
 */
uint8_t rafaelia_m020_was_brown_out(void) {
    uint8_t borf_set;

    borf_set = (uint8_t)((RAFA_MMIO8(AVR_MCUSR_ADDR) >> M020_BORF) & 1u);

    /* Clear all reset flags in MCUSR */
    RAFA_MMIO8(AVR_MCUSR_ADDR) &= (uint8_t)(~((1u << M020_BORF)  |
                                                (1u << M020_WDRF)  |
                                                (1u << M020_EXTRF) |
                                                (1u << M020_PORF)));
    return borf_set;
}
#endif

int rafaelia_m020_brown_out_flag_como_diagnostico_de_alimentacao(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    uint8_t was_bor = rafaelia_m020_was_brown_out();
    /*
     * Caller can inspect was_bor:
     *   1 => last reset was a brown-out (power supply dip below BOD level)
     *   0 => normal POR, external reset, or WDT reset
     */
    return (int)was_bor;
#else
    /* Non-AVR: no BOD hardware available — return 0 (no brown-out detected) */
    return 0;
#endif
}
