#include "RAF_rafaelia_common.h"

/*
 * Método M029: SPI por registrador BCM
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: SPI
 * Ganho estimado: 5x-50x vs userspace alto
 *
 * Controla SPI0 diretamente via registradores mapeados em memória.
 * Gated em RASPBERRYPI — retorna TOKEN_VAZIO (0) em outros hosts.
 */

/* SPI0 register offsets (in uint32_t words from spi_base) */
#define SPI0_CS_OFFSET    (0x00u / 4u)   /* Control and Status */
#define SPI0_FIFO_OFFSET  (0x04u / 4u)   /* TX/RX FIFO */
#define SPI0_CLK_OFFSET   (0x08u / 4u)   /* Clock Divider */

/* SPI0_CS bit positions */
#define SPI0_CS_TA       (1u << 7)   /* Transfer Active */
#define SPI0_CS_TXD      (1u << 18)  /* TX FIFO has space */
#define SPI0_CS_RXD      (1u << 17)  /* RX FIFO has data */
#define SPI0_CS_DONE     (1u << 16)  /* Transfer done */
#define SPI0_CS_CLEAR_RX (1u << 5)   /* Clear RX FIFO */
#define SPI0_CS_CLEAR_TX (1u << 4)   /* Clear TX FIFO */

/*
 * rafaelia_m029_spi_xfer — single-byte full-duplex SPI transfer.
 * spi_base: mapped SPI0 register base.
 * tx:       byte to transmit.
 * Returns:  received byte.
 */
static uint8_t rafaelia_m029_spi_xfer(volatile uint32_t *spi_base, uint8_t tx)
{
    /* Clear FIFOs then assert Transfer Active */
    spi_base[SPI0_CS_OFFSET] = SPI0_CS_CLEAR_RX | SPI0_CS_CLEAR_TX;
    spi_base[SPI0_CS_OFFSET] = SPI0_CS_TA;

    /* Wait for TX FIFO to accept data (TXD=1) */
    while (!(spi_base[SPI0_CS_OFFSET] & SPI0_CS_TXD))
        ;

    /* Write byte into TX FIFO */
    spi_base[SPI0_FIFO_OFFSET] = (uint32_t)tx;

    /* Wait for transfer to complete (DONE=1) */
    while (!(spi_base[SPI0_CS_OFFSET] & SPI0_CS_DONE))
        ;

    /* Read received byte from RX FIFO */
    uint8_t rx = (uint8_t)(spi_base[SPI0_FIFO_OFFSET] & 0xFFu);

    /* De-assert Transfer Active */
    spi_base[SPI0_CS_OFFSET] &= ~(uint32_t)SPI0_CS_TA;

    return rx;
}

int rafaelia_m029_spi_por_registrador_bcm(volatile uint32_t *mmio_base)
{
    if (!mmio_base) return 0; /* TOKEN_VAZIO — no BCM hardware present */

#if defined(RASPBERRYPI)
    /*
     * Clock divider: 250 MHz OSC / 256 ≈ 976 kHz SPI clock.
     * Perform a loopback transfer (bridge MOSI→MISO for test).
     */
    mmio_base[SPI0_CLK_OFFSET] = 256u;
    uint8_t echo = rafaelia_m029_spi_xfer(mmio_base, 0xA5u);
    (void)echo;
    return 0;
#else
    /* Logic complete; hardware not present on this host. */
    (void)rafaelia_m029_spi_xfer;
    return 0;
#endif
}
