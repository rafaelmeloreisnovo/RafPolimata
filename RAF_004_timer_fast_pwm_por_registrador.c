#include "RAF_rafaelia_common.h"

/*
 * Método M004: Timer Fast PWM por registrador
 * Alvo: MCU/AVR
 * Domínio: PWM
 * Ganho estimado: 2x-10x
 *
 * Configura PWM direto sem analogWrite.
 * Timer1 Fast PWM 8-bit (mode 5): WGM10=1 em TCCR1A, WGM12=1 em TCCR1B.
 * Non-inverting OC1A: COM1A1=1. Prescaler /64. OCR1A=127 para 50% duty.
 * DDRB bit1 (PB1/OC1A) como saída.
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
#define AVR_OCR1AL_ADDR  0x88u
#define AVR_OCR1AH_ADDR  0x89u

/* TCCR1A bits */
#define M004_COM1A1  7u   /* non-inverting OC1A */
#define M004_WGM10   0u   /* Fast PWM 8-bit, part 1 */

/* TCCR1B bits */
#define M004_WGM12   3u   /* Fast PWM 8-bit, part 2 */
#define M004_CS11    1u   /* prescaler /64: CS11=1, CS10=1 */
#define M004_CS10    0u

/* OC1A = PB1 */
#define M004_OC1A_BIT 1u

/* 50% duty cycle on 8-bit TOP (0xFF): OCR1A = 127 */
#define M004_OCR1A_50PCT ((uint8_t)127u)
#endif

void rafaelia_m004_timer_fast_pwm_por_registrador(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Set OC1A pin (PB1) as output */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)(1u << M004_OC1A_BIT);

    /* Stop timer, clear config */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* Clear counter */
    RAFA_MMIO8(AVR_TCNT1H_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCNT1L_ADDR) = (uint8_t)0u;

    /* Set OCR1A for 50% duty (8-bit TOP = 0xFF) */
    RAFA_MMIO8(AVR_OCR1AH_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_OCR1AL_ADDR) = M004_OCR1A_50PCT;

    /* TCCR1A: COM1A1=1 (non-inverting), WGM10=1 (Fast PWM 8-bit mode 5 low bit) */
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)((1u << M004_COM1A1) |
                                              (1u << M004_WGM10));

    /* TCCR1B: WGM12=1 (Fast PWM 8-bit mode 5 high bit), CS11=1, CS10=1 (/64) */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)((1u << M004_WGM12) |
                                              (1u << M004_CS11)  |
                                              (1u << M004_CS10));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
