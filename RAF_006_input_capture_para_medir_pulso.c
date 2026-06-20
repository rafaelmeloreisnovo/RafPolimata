#include "RAF_rafaelia_common.h"

/*
 * Método M006: Input Capture para medir pulso
 * Alvo: MCU/AVR
 * Domínio: Timer/Medição
 * Ganho estimado: precisão de 62.5 ns (a 16 MHz, /8 prescaler)
 *
 * Configura Timer1 em modo Input Capture:
 * - ICP1 (PB0) como entrada (DDRB bit0 = 0)
 * - TCCR1B: ICES1=1 (rising edge), ICNC1=1 (noise cancel), /8 prescaler
 * - TIMSK1: ICIE1=1 (input capture interrupt enable)
 * Variável estática _m006_capture armazena ICR1 na ISR.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_DDRB_ADDR    0x24u
#define AVR_TCCR1A_ADDR  0x80u
#define AVR_TCCR1B_ADDR  0x81u
#define AVR_TCNT1L_ADDR  0x84u
#define AVR_TCNT1H_ADDR  0x85u
#define AVR_ICR1L_ADDR   0x86u
#define AVR_ICR1H_ADDR   0x87u
#define AVR_TIMSK1_ADDR  0x6Fu

/* TCCR1B bits */
#define M006_ICNC1   7u   /* Input Capture Noise Canceler */
#define M006_ICES1   6u   /* Input Capture Edge Select: 1=rising */
#define M006_CS11    1u   /* /8 prescaler: CS11=1, CS10=0 */

/* TIMSK1 bits */
#define M006_ICIE1   5u   /* Input Capture Interrupt Enable */

/* ICP1 = PB0, bit 0 */
#define M006_ICP1_BIT  0u
#endif

/* Storage for captured value — updated in ISR */
static volatile uint16_t _m006_capture = 0u;

/*
 * ISR stub (comment only — real AVR ISR would be:
 *   ISR(TIMER1_CAPT_vect) {
 *       _m006_capture = (uint16_t)(
 *           ((uint16_t)RAFA_MMIO8(AVR_ICR1H_ADDR) << 8u) |
 *           (uint16_t)RAFA_MMIO8(AVR_ICR1L_ADDR));
 *   }
 * Cannot register real vectors in hosted/non-avr-gcc compilation.
 */

void rafaelia_m006_input_capture_para_medir_pulso(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Clear ICP1 (PB0) direction bit — make it an input */
    RAFA_MMIO8(AVR_DDRB_ADDR) &= (uint8_t)(~(1u << M006_ICP1_BIT));

    /* Stop timer, clear config */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* Clear counter */
    RAFA_MMIO8(AVR_TCNT1H_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCNT1L_ADDR) = (uint8_t)0u;

    /* Enable Input Capture interrupt */
    RAFA_MMIO8(AVR_TIMSK1_ADDR) |= (uint8_t)(1u << M006_ICIE1);

    /* TCCR1A = 0 (normal mode, no OC pin output) */
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* TCCR1B: ICNC1=1 (noise cancel), ICES1=1 (rising edge), CS11=1 (/8) */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)((1u << M006_ICNC1) |
                                              (1u << M006_ICES1) |
                                              (1u << M006_CS11));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
    (void)_m006_capture; /* suppress unused-variable warning on non-AVR */
#endif
}
