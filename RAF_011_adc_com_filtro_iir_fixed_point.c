#include "RAF_rafaelia_common.h"

/*
 * Método M011: ADC com filtro IIR fixed-point
 * Alvo: MCU/AVR
 * Domínio: ADC/DSP
 * Ganho estimado: filtro suave sem float
 *
 * IIR de primeira ordem com alpha=1/8 implementado com deslocamento:
 *   y[n] = y[n-1] - (y[n-1] >> 3) + (x[n] >> 3)
 * Equivalente a: y[n] = (7/8)*y[n-1] + (1/8)*x[n]
 *
 * Self-test: após 64 chamadas com x=1024, y deve convergir para >= 900.
 * Retorno int: 0=pass, -1=fail.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* IIR state — always compiled for self-test on any host */
static uint16_t _m011_y = 0u;

/*
 * rafaelia_m011_iir_step:
 * Single IIR step: y = y - (y>>3) + (x>>3)
 * Returns current filtered output.
 */
static uint16_t rafaelia_m011_iir_step(uint16_t x) {
    _m011_y = (uint16_t)(_m011_y - (_m011_y >> 3u) + (x >> 3u));
    return _m011_y;
}

int rafaelia_m011_adc_com_filtro_iir_fixed_point(uint16_t x) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    return (int)rafaelia_m011_iir_step(x);
#else
    return (int)rafaelia_m011_iir_step(x);
#endif
}

/*
 * Self-test: reset state, feed x=1024 for 64 iterations,
 * verify output >= 900 (convergence toward 1024).
 * Returns 0=pass, -1=fail.
 */
int rafaelia_m011_selftest(void) {
    uint8_t  i;
    uint16_t result = 0u;

    /* Reset IIR state */
    _m011_y = 0u;

    for (i = 0u; i < 64u; i++) {
        result = (uint16_t)rafaelia_m011_adc_com_filtro_iir_fixed_point((uint16_t)1024u);
    }

    /* After 64 steps with alpha=1/8, output converges to (1-0.875^64)*1024 ≈ 1024*(1-very_small) */
    if (result < (uint16_t)900u) {
        return -1;
    }
    return 0;
}
