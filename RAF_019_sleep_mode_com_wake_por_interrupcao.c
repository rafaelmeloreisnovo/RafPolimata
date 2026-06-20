#include "RAF_rafaelia_common.h"

/*
 * Método M019: Sleep mode com wake por interrupção
 * Alvo: MCU/AVR
 * Domínio: Power Management
 * Ganho estimado: redução drástica de consumo
 *
 * SMCR: SM1=0, SM0=0 => Idle sleep mode. SE=1 habilita sleep.
 * Sequência: sei (habilita interrupções globais), set SE,
 *            executa instrução SLEEP, limpa SE ao acordar.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_SMCR_ADDR  0x53u

/* SMCR bits */
#define M019_SE   0u   /* Sleep Enable */
#define M019_SM0  1u   /* Sleep Mode Select bit 0 */
#define M019_SM1  2u   /* Sleep Mode Select bit 1 */
#define M019_SM2  3u   /* Sleep Mode Select bit 2 */
/* SM2:SM1:SM0 = 000 => Idle */
#endif

void rafaelia_m019_sleep_mode_com_wake_por_interrupcao(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Ensure Idle mode: SM2:SM1:SM0 = 000 */
    RAFA_MMIO8(AVR_SMCR_ADDR) &= (uint8_t)(~((1u << M019_SM2) |
                                               (1u << M019_SM1) |
                                               (1u << M019_SM0)));

    /* Enable global interrupts so the MCU can wake on interrupt */
    __asm__ __volatile__("sei" ::: "memory");

    /* Set Sleep Enable bit */
    RAFA_MMIO8(AVR_SMCR_ADDR) |= (uint8_t)(1u << M019_SE);

    /* Execute SLEEP instruction — MCU halts here until interrupt fires */
    __asm__ __volatile__("sleep" ::: "memory");

    /* Clear Sleep Enable immediately after waking (good practice) */
    RAFA_MMIO8(AVR_SMCR_ADDR) &= (uint8_t)(~(1u << M019_SE));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
