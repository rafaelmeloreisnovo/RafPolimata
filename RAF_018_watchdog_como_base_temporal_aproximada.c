#include "RAF_rafaelia_common.h"

/*
 * Método M018: Watchdog como base temporal aproximada
 * Alvo: MCU/AVR
 * Domínio: Watchdog/Timer
 * Ganho estimado: base de tempo sem Timer dedicado
 *
 * Configura WDT para interrupt a cada ~16 ms (prescaler mínimo, WDIE=1).
 * ISR incrementa _m018_wdt_ticks (stub comentário).
 * Não emite system reset (WDE=0, WDIE=1).
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_WDTCSR_ADDR  0x60u
#define AVR_MCUSR_ADDR   0x54u

/* WDTCSR bits */
#define M018_WDCE  4u   /* Watchdog Change Enable */
#define M018_WDE   3u   /* Watchdog System Reset Enable */
#define M018_WDIE  6u   /* Watchdog Interrupt Enable */
/* WDP[3:0] = 0b0000 => ~16 ms */
#endif

/* Tick counter — incremented by WDT ISR */
static volatile uint16_t _m018_wdt_ticks = 0u;

/*
 * ISR stub (comment only — real AVR ISR would be:
 *   ISR(WDT_vect) {
 *       _m018_wdt_ticks++;
 *   }
 * Cannot register real vectors in hosted/non-avr-gcc compilation.
 */

void rafaelia_m018_watchdog_como_base_temporal_aproximada(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Clear WDRF to avoid unintended reset */
    RAFA_MMIO8(AVR_MCUSR_ADDR) &= (uint8_t)(~(1u << 3u));

    /*
     * Timed sequence: enable change enable + clear WDE in same write,
     * then configure WDIE=1, WDE=0, WDP=0000 (16 ms interval).
     */
    RAFA_MMIO8(AVR_WDTCSR_ADDR) = (uint8_t)((1u << M018_WDCE) |
                                              (1u << M018_WDE));
    __asm__ __volatile__("" ::: "memory");

    /* WDIE=1, WDE=0, WDP[3:0]=0000 => interrupt only, ~16 ms */
    RAFA_MMIO8(AVR_WDTCSR_ADDR) = (uint8_t)(1u << M018_WDIE);
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
    (void)_m018_wdt_ticks; /* suppress unused warning on non-AVR */
#endif
}
