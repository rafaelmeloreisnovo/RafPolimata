#include "RAF_rafaelia_common.h"

/*
 * Método M005: Timer Phase Correct PWM para controle motor
 * Alvo: MCU/AVR
 * Domínio: PWM/Motor
 * Ganho estimado: redução de ripple de corrente
 *
 * Timer1 Phase Correct PWM mode 10 (WGM13=1,WGM11=1, WGM12=0,WGM10=0).
 * TOP=ICR1=1023 (10-bit equivalent). OCR1A=512 para ~50% duty.
 * Prescaler /8 (CS11=1). DDRB bit1 (OC1A/PB1) como saída.
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
#define AVR_OCR1AL_ADDR  0x88u
#define AVR_OCR1AH_ADDR  0x89u

/* TCCR1A bits */
#define M005_COM1A1  7u   /* non-inverting OC1A */
#define M005_WGM11   1u   /* mode 10: WGM11=1, WGM10=0 */

/* TCCR1B bits */
#define M005_WGM13   4u   /* mode 10: WGM13=1, WGM12=0 */
#define M005_CS11    1u   /* /8 prescaler: CS11=1, CS10=0 */

/* OC1A = PB1 */
#define M005_OC1A_BIT  1u

/* ICR1 TOP for 10-bit equivalent */
#define M005_ICR1_TOP    ((uint16_t)1023u)

/* OCR1A = 512 => ~50% duty on 10-bit scale */
#define M005_OCR1A_50PCT ((uint16_t)512u)
#endif

void rafaelia_m005_timer_phase_correct_pwm_para_controle_motor(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Set OC1A pin (PB1) as output */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)(1u << M005_OC1A_BIT);

    /* Stop timer, clear config */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* Clear counter */
    RAFA_MMIO8(AVR_TCNT1H_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCNT1L_ADDR) = (uint8_t)0u;

    /* Set ICR1 (TOP) = 1023 — write high byte first per AVR data sheet */
    RAFA_MMIO8(AVR_ICR1H_ADDR) = (uint8_t)(M005_ICR1_TOP >> 8u);
    RAFA_MMIO8(AVR_ICR1L_ADDR) = (uint8_t)(M005_ICR1_TOP & 0xFFu);

    /* Set OCR1A for ~50% duty */
    RAFA_MMIO8(AVR_OCR1AH_ADDR) = (uint8_t)(M005_OCR1A_50PCT >> 8u);
    RAFA_MMIO8(AVR_OCR1AL_ADDR) = (uint8_t)(M005_OCR1A_50PCT & 0xFFu);

    /* TCCR1A: COM1A1=1 (non-inverting OC1A), WGM11=1 (mode 10 low bits) */
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)((1u << M005_COM1A1) |
                                              (1u << M005_WGM11));

    /* TCCR1B: WGM13=1 (mode 10 high bit), CS11=1 (/8 prescaler) — starts timer */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)((1u << M005_WGM13) |
                                              (1u << M005_CS11));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
