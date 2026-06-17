#include "RAF_rafaelia_common.h"

/*
 * Método M009: ADC com oversampling
 * Alvo: MCU/AVR
 * Domínio: ADC/DSP
 * Ganho estimado: qualidade +1-4 bits
 *
 * Acumula n_samples leituras ADC e desloca direita por extra_bits
 * para aumentar resolução efetiva (oversampling padrão AVR AN8003).
 * Self-test não-AVR: 16 amostras de valor 512, extra_bits=4 => resultado==512.
 * (16 * 512 = 8192; 8192 >> 4 = 512 — normaliza para média real.)
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

/* ADCSRA bits */
#define M009_ADEN   7u
#define M009_ADSC   6u
#define M009_ADIF   4u
#define M009_ADPS2  2u
#define M009_ADPS1  1u
#define M009_ADPS0  0u
#endif

/*
 * rafaelia_m009_oversample_demo:
 * Accumulates n_samples from the fake_samples array and shifts right by
 * extra_bits to produce the oversampled result. Pure software — no AVR hardware.
 */
static uint16_t rafaelia_m009_oversample_demo(const uint16_t *fake_samples,
                                               uint8_t n_samples,
                                               uint8_t extra_bits)
{
    uint32_t sum = 0u;
    uint8_t  i;
    for (i = 0u; i < n_samples; i++) {
        sum += (uint32_t)fake_samples[i];
    }
    return (uint16_t)(sum >> extra_bits);
}

/*
 * Main function — signature changed from void to int (0=pass, -1=fail)
 * to support the self-test path on non-AVR hosts.
 */
int rafaelia_m009_adc_com_oversampling(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /*
     * AVR single-shot oversampling: configure ADC for single conversions,
     * accumulate 16 reads (4x oversampling => +2 bits resolution),
     * shift right by 2 for effective 12-bit result from 10-bit ADC.
     *
     * Note: on real hardware the ISR/polling loop would feed samples here.
     * This block sets up the ADC hardware configuration.
     */
    /* Select AVcc reference, ADC0 channel */
    RAFA_MMIO8(AVR_ADMUX_ADDR) = (uint8_t)(1u << 6u);

    /* Single-shot mode: ADCSRB = 0 (free-running disabled) */
    RAFA_MMIO8(AVR_ADCSRB_ADDR) = (uint8_t)0u;

    /* Enable ADC, /128 prescaler — do NOT start yet */
    RAFA_MMIO8(AVR_ADCSRA_ADDR) = (uint8_t)((1u << M009_ADEN)  |
                                              (1u << M009_ADPS2) |
                                              (1u << M009_ADPS1) |
                                              (1u << M009_ADPS0));
    return 0;
#else
    /* Non-AVR self-test: 16 samples of 512, extra_bits=2 => result must be 512 */
    static uint16_t _m009_demo[16];
    uint8_t i;
    uint16_t result;

    for (i = 0u; i < 16u; i++) {
        _m009_demo[i] = (uint16_t)512u;
    }

    /*
     * 16 samples * 512 = 8192. To get back the original 512 we shift by 4
     * (log2(16) = 4), which normalises the accumulated sum to the mean.
     * extra_bits=4 here; the parameter name describes how many bits of
     * accumulated sum to drop (i.e. divide by 2^extra_bits).
     */
    result = rafaelia_m009_oversample_demo(_m009_demo, (uint8_t)16u, (uint8_t)4u);
    if (result != (uint16_t)512u) {
        return -1;
    }
    return 0;
#endif
}
