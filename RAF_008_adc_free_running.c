#include "RAF_rafaelia_common.h"

/*
 * Método M008: ADC free-running
 * Alvo: MCU/AVR
 * Domínio: ADC
 * Ganho estimado: conversão contínua sem polling de CPU
 *
 * ADMUX = 0x60: REFS0=1 (AVcc), MUX=0 (ADC0/PC0).
 * ADCSRB = 0:   trigger source = free-running.
 * ADCSRA: ADEN=1, ADATE=1, ADSC=1, ADIE=1, ADPS=7 (/128 prescaler).
 * Conversão contínua com interrupção a cada resultado.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_ADCSRA_ADDR  0x7Au
#define AVR_ADCSRB_ADDR  0x7Bu
#define AVR_ADMUX_ADDR   0x7Cu
#define AVR_ADCL_ADDR    0x78u
#define AVR_ADCH_ADDR    0x79u

/* ADCSRA bit positions */
#define M008_ADEN   7u   /* ADC Enable */
#define M008_ADSC   6u   /* ADC Start Conversion */
#define M008_ADATE  5u   /* ADC Auto Trigger Enable */
#define M008_ADIF   4u   /* ADC Interrupt Flag (write 1 to clear) */
#define M008_ADIE   3u   /* ADC Interrupt Enable */
#define M008_ADPS2  2u   /* ADC Prescaler Select bit 2 */
#define M008_ADPS1  1u   /* ADC Prescaler Select bit 1 */
#define M008_ADPS0  0u   /* ADC Prescaler Select bit 0 */

/* ADMUX: REFS0=1 (bit6), MUX=0b0000 (ADC0) */
#define M008_ADMUX_VAL  ((uint8_t)(1u << 6u))

/* ADCSRB = 0 => free-running trigger source */
#define M008_ADCSRB_FREERUN ((uint8_t)0u)

/* ADCSRA value: ADEN|ADSC|ADATE|ADIE|ADPS2|ADPS1|ADPS0 (/128 prescaler) */
#define M008_ADCSRA_VAL ((uint8_t)((1u << M008_ADEN)  | \
                                   (1u << M008_ADSC)  | \
                                   (1u << M008_ADATE) | \
                                   (1u << M008_ADIE)  | \
                                   (1u << M008_ADPS2) | \
                                   (1u << M008_ADPS1) | \
                                   (1u << M008_ADPS0)))
#endif

void rafaelia_m008_adc_free_running(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Select AVcc reference, ADC0 channel */
    RAFA_MMIO8(AVR_ADMUX_ADDR) = M008_ADMUX_VAL;

    /* Free-running trigger: ADCSRB = 0 */
    RAFA_MMIO8(AVR_ADCSRB_ADDR) = M008_ADCSRB_FREERUN;

    /*
     * ADCSRA: enable ADC, auto-trigger, start conversion, enable interrupt,
     * /128 prescaler (16MHz/128 = 125kHz ADC clock — within 50-200kHz range).
     */
    RAFA_MMIO8(AVR_ADCSRA_ADDR) = M008_ADCSRA_VAL;
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
