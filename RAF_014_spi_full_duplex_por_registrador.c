#include "RAF_rafaelia_common.h"

/*
 * Método M014: SPI full-duplex por registrador
 * Alvo: MCU/AVR
 * Domínio: SPI
 * Ganho estimado: transferência sem HAL
 *
 * DDRB: SS=PB2, MOSI=PB3, SCK=PB5 como saídas; MISO=PB4 como entrada.
 * SPCR = 0x50: SPE=1 (bit6), MSTR=1 (bit4), modo 0, /4 prescaler.
 * rafaelia_m014_spi_xfer: escreve SPDR, espera SPIF, lê SPDR.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_DDRB_ADDR   0x24u
#define AVR_SPCR_ADDR   0x4Cu
#define AVR_SPSR_ADDR   0x4Du
#define AVR_SPDR_ADDR   0x4Eu

/* SPCR bits */
#define M014_SPE    6u   /* SPI Enable */
#define M014_MSTR   4u   /* Master/Slave Select */
/* CPOL=0, CPHA=0 (mode 0), SPR1=0, SPR0=0 => /4 prescaler */

/* SPSR bits */
#define M014_SPIF   7u   /* SPI Interrupt Flag (transfer complete) */

/* DDRB pin assignments */
#define M014_SS_BIT   2u
#define M014_MOSI_BIT 3u
#define M014_MISO_BIT 4u
#define M014_SCK_BIT  5u

/* SPCR value: SPE=1, MSTR=1, mode 0, /4 prescaler = 0x50 */
#define M014_SPCR_VAL ((uint8_t)((1u << M014_SPE) | (1u << M014_MSTR)))
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
uint8_t rafaelia_m014_spi_xfer(uint8_t tx) {
    RAFA_MMIO8(AVR_SPDR_ADDR) = tx;
    /* Wait for transfer complete */
    while (!(RAFA_MMIO8(AVR_SPSR_ADDR) & (uint8_t)(1u << M014_SPIF))) {
        /* spin */
    }
    return RAFA_MMIO8(AVR_SPDR_ADDR);
}
#endif

void rafaelia_m014_spi_full_duplex_por_registrador(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Configure SPI pins: SS, MOSI, SCK as outputs; MISO as input */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)((1u << M014_SS_BIT)   |
                                             (1u << M014_MOSI_BIT) |
                                             (1u << M014_SCK_BIT));
    RAFA_MMIO8(AVR_DDRB_ADDR) &= (uint8_t)(~(1u << M014_MISO_BIT));

    /* Enable SPI, master mode, mode 0, /4 prescaler */
    RAFA_MMIO8(AVR_SPCR_ADDR) = M014_SPCR_VAL;

    /* SPSR: SPI2X=0 (normal speed) — default after reset */
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
