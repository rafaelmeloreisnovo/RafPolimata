#include "RAF_rafaelia_common.h"

/*
 * Método M015: SPI burst transfer
 * Alvo: MCU/AVR
 * Domínio: SPI
 * Ganho estimado: throughput máximo em transferências longas
 *
 * Extensão do M014: rafaelia_m015_spi_burst envia n bytes de tx[] e
 * armazena respostas em rx[], iniciando o próximo byte imediatamente após
 * SPIF sem delay extra.
 * Self-test não-AVR: conta bytes transferidos, verifica n==transferido.
 * Retorno int: 0=pass, -1=fail.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_DDRB_ADDR   0x24u
#define AVR_SPCR_ADDR   0x4Cu
#define AVR_SPSR_ADDR   0x4Du
#define AVR_SPDR_ADDR   0x4Eu

#define M015_SPE    6u
#define M015_MSTR   4u
#define M015_SPIF   7u
#define M015_SS_BIT   2u
#define M015_MOSI_BIT 3u
#define M015_MISO_BIT 4u
#define M015_SCK_BIT  5u

#define M015_SPCR_VAL ((uint8_t)((1u << M015_SPE) | (1u << M015_MSTR)))
#endif

/*
 * rafaelia_m015_spi_burst:
 * Transfer n bytes: write tx[i], wait SPIF, read rx[i].
 * Returns number of bytes transferred.
 */
static uint8_t rafaelia_m015_spi_burst_avr(const uint8_t *tx, uint8_t *rx,
                                             uint8_t n)
{
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    uint8_t i;
    for (i = 0u; i < n; i++) {
        RAFA_MMIO8(AVR_SPDR_ADDR) = tx[i];
        while (!(RAFA_MMIO8(AVR_SPSR_ADDR) & (uint8_t)(1u << M015_SPIF))) {
            /* spin — next byte sent immediately after flag clears */
        }
        rx[i] = RAFA_MMIO8(AVR_SPDR_ADDR);
    }
    return n;
#else
    /*
     * Non-AVR simulation: loopback — rx mirrors tx, count bytes.
     * (No real SPI hardware; used only for self-test counting.)
     */
    uint8_t i;
    for (i = 0u; i < n; i++) {
        rx[i] = tx[i];   /* loopback */
    }
    return n;
#endif
}

int rafaelia_m015_spi_burst_transfer(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Configure SPI pins and enable master mode (same as M014) */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)((1u << M015_SS_BIT)   |
                                             (1u << M015_MOSI_BIT) |
                                             (1u << M015_SCK_BIT));
    RAFA_MMIO8(AVR_DDRB_ADDR) &= (uint8_t)(~(1u << M015_MISO_BIT));
    RAFA_MMIO8(AVR_SPCR_ADDR)  = M015_SPCR_VAL;
    return 0;
#else
    /* Non-AVR self-test: send [0xAA, 0x55], verify transfer count == 2 */
    static const uint8_t test_tx[2] = {0xAAu, 0x55u};
    static uint8_t       test_rx[2] = {0u, 0u};
    uint8_t transferred;

    transferred = rafaelia_m015_spi_burst_avr(test_tx, test_rx, (uint8_t)2u);
    if (transferred != (uint8_t)2u) {
        return -1;
    }
    /* Verify loopback */
    if (test_rx[0] != test_tx[0] || test_rx[1] != test_tx[1]) {
        return -1;
    }
    return 0;
#endif
}
