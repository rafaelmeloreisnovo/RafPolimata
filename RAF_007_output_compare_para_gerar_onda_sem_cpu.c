#include "RAF_rafaelia_common.h"

/*
 * Método M007: Output Compare para gerar onda sem CPU
 * Alvo: MCU/AVR
 * Domínio: Timer/Áudio
 * Ganho estimado: 0% CPU para geração de frequência contínua
 *
 * Timer1 CTC com toggle OC1A a cada compare match (COM1A0=1, COM1A1=0).
 * WGM12=1 (CTC mode). Prescaler /256 (CS12=1).
 * OCR1A = F_CPU / prescaler / 2 / 440 - 1 para nota 440 Hz.
 * DDRB bit1 (OC1A/PB1) como saída. CPU-free após configuração.
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
#define M007_COM1A0  6u   /* Toggle OC1A on compare match */

/* TCCR1B bits */
#define M007_WGM12   3u   /* CTC mode: TOP = OCR1A */
#define M007_CS12    2u   /* /256 prescaler: CS12=1, CS11=0, CS10=0 */

/* OC1A = PB1 */
#define M007_OC1A_BIT  1u

/*
 * OCR1A for 440 Hz toggle (half-period) with /256 prescaler:
 * OCR1A = F_CPU / 256 / 2 / 440 - 1
 *       = 16000000 / 256 / 880 - 1
 *       = 62500 / 880 - 1 ≈ 71 - 1 = 70
 */
#define M007_OCR1A_440HZ ((uint16_t)((F_CPU / 256UL / 2UL / 440UL) - 1UL))
#endif

void rafaelia_m007_output_compare_para_gerar_onda_sem_cpu(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Set OC1A pin (PB1) as output */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)(1u << M007_OC1A_BIT);

    /* Stop timer, clear config */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)0u;

    /* Clear counter */
    RAFA_MMIO8(AVR_TCNT1H_ADDR) = (uint8_t)0u;
    RAFA_MMIO8(AVR_TCNT1L_ADDR) = (uint8_t)0u;

    /* Set OCR1A for 440 Hz */
    RAFA_MMIO8(AVR_OCR1AH_ADDR) = (uint8_t)(M007_OCR1A_440HZ >> 8u);
    RAFA_MMIO8(AVR_OCR1AL_ADDR) = (uint8_t)(M007_OCR1A_440HZ & 0xFFu);

    /* TCCR1A: COM1A0=1, COM1A1=0 => toggle OC1A on compare match */
    RAFA_MMIO8(AVR_TCCR1A_ADDR) = (uint8_t)(1u << M007_COM1A0);

    /* TCCR1B: WGM12=1 (CTC), CS12=1 (/256 prescaler) — starts timer */
    RAFA_MMIO8(AVR_TCCR1B_ADDR) = (uint8_t)((1u << M007_WGM12) |
                                              (1u << M007_CS12));
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
