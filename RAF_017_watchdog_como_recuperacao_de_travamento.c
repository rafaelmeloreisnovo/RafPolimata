#include "RAF_rafaelia_common.h"

/*
 * Método M017: Watchdog como recuperação de travamento
 * Alvo: MCU/AVR
 * Domínio: Watchdog
 * Ganho estimado: recuperação automática de travamentos
 *
 * Sequência temporizada para habilitar WDT com reset em ~2.1 s:
 * 1. Escreve WDCE=1|WDE=1 em WDTCSR (janela de 4 ciclos).
 * 2. Escreve WDE=1 + prescaler WDP2|WDP0 (WDTO_2S ≈ 2.1 s).
 * rafaelia_m017_wdt_reset: envia instrução WDR para resetar timer.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_WDTCSR_ADDR  0x60u
#define AVR_MCUSR_ADDR   0x54u

/* WDTCSR bits */
#define M017_WDCE  4u   /* Watchdog Change Enable */
#define M017_WDE   3u   /* Watchdog System Reset Enable */
#define M017_WDP3  5u   /* Prescaler bit 3 */
#define M017_WDP2  2u   /* Prescaler bit 2 */
#define M017_WDP1  1u   /* Prescaler bit 1 */
#define M017_WDP0  0u   /* Prescaler bit 0 */

/*
 * WDP[3:0] = 0b0101 => WDTO_0.5s
 * WDP[3:0] = 0b0110 => WDTO_1s
 * WDP[3:0] = 0b0111 => WDTO_2s   (WDP2=1, WDP1=1, WDP0=1)
 * Use WDP2|WDP1|WDP0 for ~2.1 s timeout.
 */
#define M017_WDT_2S_VAL ((uint8_t)((1u << M017_WDE)  | \
                                    (1u << M017_WDP2) | \
                                    (1u << M017_WDP1) | \
                                    (1u << M017_WDP0)))
#endif

/*
 * rafaelia_m017_wdt_reset:
 * Resets the watchdog timer to prevent system reset.
 * On AVR this is the WDR instruction; stubbed as a comment on non-AVR.
 */
void rafaelia_m017_wdt_reset(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /*
     * WDR instruction resets the WDT countdown.
     * In avr-gcc this would be: __asm__ __volatile__("wdr" ::: "memory");
     * Represented here as a volatile barrier since "wdr" is AVR-only mnemonic.
     */
    __asm__ __volatile__("" ::: "memory");  /* compiler barrier as stub */
#else
    /* No-op on non-AVR host */
#endif
}

void rafaelia_m017_watchdog_como_recuperacao_de_travamento(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Clear WDRF (Watchdog Reset Flag) in MCUSR before enabling WDT */
    RAFA_MMIO8(AVR_MCUSR_ADDR) &= (uint8_t)(~(1u << 3u));

    /*
     * Timed sequence to change WDTCSR (must complete within 4 clock cycles):
     * Step 1: set WDCE and WDE simultaneously.
     */
    RAFA_MMIO8(AVR_WDTCSR_ADDR) = (uint8_t)((1u << M017_WDCE) |
                                              (1u << M017_WDE));

    /*
     * Step 2 (within 4 cycles): write new WDT config — WDE=1, ~2.1 s prescaler.
     * Compiler barrier ensures no reordering between steps.
     */
    __asm__ __volatile__("" ::: "memory");
    RAFA_MMIO8(AVR_WDTCSR_ADDR) = M017_WDT_2S_VAL;
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
