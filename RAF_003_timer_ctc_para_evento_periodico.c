#include "RAF_rafaelia_common.h"

/*
 * Método M003: Timer CTC para evento periódico
 * Alvo: MCU/AVR
 * Domínio: Timer
 * Ganho estimado: 5x-30x
 *
 * Gera evento preciso sem delay bloqueante.
 * Configura Timer1 em modo CTC: WGM12 em TCCR1B, prescaler /64,
 * OCR1A = (F_CPU/64/1000)-1 para 1 kHz, pino OC1A (DDRB bit1) como saída.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_DDRB_ADDR    0x24u
#define AVR_PINB_ADDR    0x23u
#define AVR_PORTB_ADDR   0x25u
#define AVR_TCCR1A_ADDR  0x80u
#define AVR_TCCR1B_ADDR  0x81u
#define AVR_TCNT1L_ADDR  0x84u
#define AVR_TCNT1H_ADDR  0x85u
#define AVR_OCR1AL_ADDR  0x88u
#define AVR_OCR1AH_ADDR  0x89u
#define AVR_TIMSK1_ADDR  0x6Fu

/* TCCR1B bits */
#define M003_WGM12   3u   /* CTC mode: TOP = OCR1A */
#define M003_CS11    1u   /* prescaler bit: /8  */
#define M003_CS10    0u   /* prescaler bit: /1  */
/* CS11=1, CS10=1 => /64 */

/* TIMSK1 bits */
#define M003_OCIE1A  1u   /* Output Compare A Match Interrupt Enable */

/* DDRB bit for OC1A = PB1 */
#define M003_OC1A_BIT 1u

/* OCR1A value for 1 kHz with /64 prescaler:
 * OCR1A = F_CPU / prescaler / target_hz - 1
 *       = 16000000 / 64 / 1000 - 1 = 249
 */
#define M003_OCR1A_1KHZ ((uint16_t)((F_CPU / 64UL / 1000UL) - 1UL))
#endif

void rafaelia_m003_timer_ctc_para_evento_periodico(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Set OC1A pin (PB1) as output */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)(1u << M003_OC1A_BIT);

    /* Stop timer before configuring */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* Clear counter */
    RAFA_MMIO8(AVR_TCNT1H_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCNT1L_ADDR) = (uint8_t)0u;

    /* Set OCR1A for 1 kHz */
    RAFA_MMIO8(AVR_OCR1AH_ADDR) = (uint8_t)(M003_OCR1A_1KHZ >> 8u);
    RAFA_MMIO8(AVR_OCR1AL_ADDR) = (uint8_t)(M003_OCR1A_1KHZ & 0xFFu);

    /* Enable Output Compare A Match interrupt */
    RAFA_MMIO8(AVR_TIMSK1_ADDR) |= (uint8_t)(1u << M003_OCIE1A);

    /* TCCR1A: normal port operation (no OC pin toggling needed for CTC ISR mode) */
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* TCCR1B: WGM12=1 (CTC), CS11=1, CS10=1 (/64 prescaler) — starts timer */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)((1u << M003_WGM12) |
                                              (1u << M003_CS11)  |
                                              (1u << M003_CS10));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
