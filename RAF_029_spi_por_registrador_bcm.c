#include "RAF_rafaelia_common.h"
#include "RAF_hw_status.h"

/*
 * Método M029: SPI por registrador BCM
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: SPI
 * Ganho estimado: 5x-50x vs userspace alto
 *
 * Controla SPI0 diretamente via registradores mapeados em memória.
 * Gated em RASPBERRYPI. Sem hardware retorna erro tipado, nao sucesso.
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
static int rafaelia_m029_spi_xfer(volatile uint32_t *spi_base, uint8_t tx,
                                  uint8_t *rx)
{
    uint32_t n;
    if (!spi_base || !rx) return RAF_HW_ERR_INVALID;
    /* Caller must supply a valid, independently mapped SPI0 register block. */
    spi_base[SPI0_CS_OFFSET] = SPI0_CS_CLEAR_RX | SPI0_CS_CLEAR_TX;
    spi_base[SPI0_CS_OFFSET] = SPI0_CS_TA;

    for (n = 0; n < RAF_HW_POLL_LIMIT; ++n)
        if (spi_base[SPI0_CS_OFFSET] & SPI0_CS_TXD) break;
    if (n == RAF_HW_POLL_LIMIT) {
        spi_base[SPI0_CS_OFFSET] = 0u;
        return RAF_HW_ERR_TIMEOUT;
    }

    spi_base[SPI0_FIFO_OFFSET] = (uint32_t)tx;
    for (n = 0; n < RAF_HW_POLL_LIMIT; ++n)
        if (spi_base[SPI0_CS_OFFSET] & SPI0_CS_DONE) break;
    if (n == RAF_HW_POLL_LIMIT) {
        spi_base[SPI0_CS_OFFSET] = 0u;
        return RAF_HW_ERR_TIMEOUT;
    }

    *rx = (uint8_t)(spi_base[SPI0_FIFO_OFFSET] & 0xffu);
    spi_base[SPI0_CS_OFFSET] = 0u;
    return RAF_HW_OK;
}

int rafaelia_m029_spi_por_registrador_bcm(volatile uint32_t *mmio_base)
{
    if (!mmio_base) return RAF_HW_ERR_NO_DEVICE;
#if defined(RASPBERRYPI)
    uint8_t echo = 0u;
    mmio_base[SPI0_CLK_OFFSET] = 256u;
    return rafaelia_m029_spi_xfer(mmio_base, 0xa5u, &echo);
#else
    (void)rafaelia_m029_spi_xfer;
    return RAF_HW_ERR_UNSUPPORTED;
#endif
}
