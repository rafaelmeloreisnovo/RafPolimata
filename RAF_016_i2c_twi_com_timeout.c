#include "RAF_rafaelia_common.h"

/*
 * Método M016: I2C/TWI com timeout
 * Alvo: MCU/AVR
 * Domínio: I2C/TWI
 * Ganho estimado: robustez contra barramento travado
 *
 * TWBR = ((F_CPU/100000)-16)/2 para SCL=100 kHz.
 * rafaelia_m016_twi_start: aguarda TWINT com timeout de 1000 iterações.
 * rafaelia_m016_twi_stop: envia condição STOP.
 * Retorno int: 0=ok, -1=timeout (AVR) ou 0 direto (non-AVR).
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_TWBR_ADDR   0xB8u
#define AVR_TWSR_ADDR   0xB9u
#define AVR_TWDR_ADDR   0xBBu
#define AVR_TWCR_ADDR   0xBCu
#define AVR_TWAMR_ADDR  0xBDu

/* TWCR bits */
#define M016_TWINT  7u   /* TWI Interrupt Flag (write 1 to clear) */
#define M016_TWEA   6u   /* TWI Enable Acknowledge Bit */
#define M016_TWSTA  5u   /* TWI START Condition Bit */
#define M016_TWSTO  4u   /* TWI STOP Condition Bit */
#define M016_TWEN   2u   /* TWI Enable Bit */

/* TWBR for 100 kHz SCL: ((F_CPU/SCL_FREQ) - 16) / 2
 * = ((16000000/100000) - 16) / 2 = (160 - 16) / 2 = 72
 */
#define M016_TWBR_100KHZ ((uint8_t)(((F_CPU / 100000UL) - 16UL) / 2UL))

#define M016_TWI_TIMEOUT 1000u

static int rafaelia_m016_twi_start(void) {
    uint16_t timeout = (uint16_t)M016_TWI_TIMEOUT;

    RAFA_MMIO8(AVR_TWCR_ADDR) = (uint8_t)((1u << M016_TWINT) |
                                            (1u << M016_TWSTA) |
                                            (1u << M016_TWEN));
    while (!(RAFA_MMIO8(AVR_TWCR_ADDR) & (uint8_t)(1u << M016_TWINT))) {
        if (timeout == 0u) {
            return -1;
        }
        timeout--;
    }
    return 0;
}

static void rafaelia_m016_twi_stop(void) {
    RAFA_MMIO8(AVR_TWCR_ADDR) = (uint8_t)((1u << M016_TWINT) |
                                            (1u << M016_TWSTO) |
                                            (1u << M016_TWEN));
}
#endif

int rafaelia_m016_i2c_twi_com_timeout(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    int rc;

    /* Set TWI bit rate for 100 kHz SCL (TWSR prescaler bits = 0 => /1) */
    RAFA_MMIO8(AVR_TWSR_ADDR) = (uint8_t)0u;   /* prescaler = 1 */
    RAFA_MMIO8(AVR_TWBR_ADDR) = M016_TWBR_100KHZ;

    /* Send START, wait with timeout */
    rc = rafaelia_m016_twi_start();
    if (rc != 0) {
        return -1;
    }

    /* Send STOP to release bus */
    rafaelia_m016_twi_stop();
    return 0;
#else
    /* Non-AVR: no hardware — return 0 to indicate pass */
    return 0;
#endif
}
