#include "RAF_rafaelia_common.h"

/*
 * Método M010: ADC com média móvel inteira
 * Alvo: MCU/AVR
 * Domínio: ADC/DSP
 * Ganho estimado: estabilidade
 *
 * Filtro de média móvel de 8 amostras (ring buffer estático, sem float).
 * Self-test: 8x value=100, depois 1x value=200 => retorna (7*100+200)/8 = 112.
 * Retorno int: 0=pass, -1=fail.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Ring buffer — always compiled so self-test works on any host */
static uint16_t _m010_buf[8];
static uint8_t  _m010_idx = 0u;
static uint32_t _m010_sum = 0u;

/*
 * rafaelia_m010_moving_avg:
 * Remove oldest sample, add new_sample, return sum/8.
 */
static uint16_t rafaelia_m010_moving_avg(uint16_t new_sample) {
    _m010_sum -= (uint32_t)_m010_buf[_m010_idx];
    _m010_buf[_m010_idx] = new_sample;
    _m010_sum += (uint32_t)new_sample;
    _m010_idx = (uint8_t)((_m010_idx + 1u) & 7u);  /* modulo 8 */
    return (uint16_t)(_m010_sum >> 3u);              /* divide by 8 */
}

int rafaelia_m010_adc_com_media_movel_inteira(uint16_t new_sample) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /*
     * On AVR: update moving average with new ADC sample, return filtered value.
     * Caller feeds this function from ADC ISR or polling loop.
     */
    return (int)rafaelia_m010_moving_avg(new_sample);
#else
    return (int)rafaelia_m010_moving_avg(new_sample);
#endif
}

/*
 * Self-test entry point: resets internal state, feeds known values,
 * verifies correctness. Returns 0=pass, -1=fail.
 */
int rafaelia_m010_selftest(void) {
    uint8_t  i;
    uint16_t result;

    /* Reset state */
    for (i = 0u; i < 8u; i++) { _m010_buf[i] = 0u; }
    _m010_idx = 0u;
    _m010_sum = 0u;

    /* Feed 8 samples of 100 to fill the buffer */
    for (i = 0u; i < 8u; i++) {
        (void)rafaelia_m010_adc_com_media_movel_inteira((uint16_t)100u);
    }

    /* One sample of 200 — oldest (100) is displaced, sum = 7*100+200 = 900 */
    result = (uint16_t)rafaelia_m010_adc_com_media_movel_inteira((uint16_t)200u);

    /* Expected: 900 / 8 = 112 */
    if (result != (uint16_t)112u) {
        return -1;
    }
    return 0;
}
