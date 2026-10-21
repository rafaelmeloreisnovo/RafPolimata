#include "RAF_rafaelia_common.h"
#include "RAF_hw_status.h"

/*
 * Método M030: I2C por registrador BCM
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: I2C
 * Ganho estimado: 2x-20x vs userspace
 *
 * Controla BSC1 (I2C bus) diretamente via registradores mapeados.
 * Gated em RASPBERRYPI. Sem hardware retorna erro tipado, nao sucesso.
 */

/* BSC register offsets (in uint32_t words from bsc_base) */
#define BSC_C_OFFSET    0u   /* Control */
#define BSC_S_OFFSET    1u   /* Status */
#define BSC_DLEN_OFFSET 2u   /* Data Length */
#define BSC_A_OFFSET    3u   /* Slave Address */
#define BSC_FIFO_OFFSET 4u   /* Data FIFO */
#define BSC_DIV_OFFSET  5u   /* Clock Divider */

/* BSC_C (Control) bit positions */
#define BSC_C_I2CEN  (1u << 15)  /* I2C Enable */
#define BSC_C_ST     (1u << 7)   /* Start Transfer */
#define BSC_C_CLEAR  (1u << 4)   /* Clear FIFO (bits 4:5) */
#define BSC_C_READ   (1u << 0)   /* Read Transfer (1=read, 0=write) */

/* BSC_S (Status) bit positions */
#define BSC_S_CLKT   (1u << 9)   /* Clock stretch timeout */
#define BSC_S_ERR    (1u << 8)   /* ACK error */
#define BSC_S_RXF    (1u << 7)   /* RX FIFO full */
#define BSC_S_TXE    (1u << 6)   /* TX FIFO empty */
#define BSC_S_RXD    (1u << 5)   /* RX FIFO has data */
#define BSC_S_TXD    (1u << 4)   /* TX FIFO can accept data */
#define BSC_S_RXR    (1u << 3)   /* RX FIFO needs reading */
#define BSC_S_TXW    (1u << 2)   /* TX FIFO needs writing */
#define BSC_S_DONE   (1u << 1)   /* Transfer done */
#define BSC_S_TA     (1u << 0)   /* Transfer Active */

/*
 * rafaelia_m030_i2c_write — write one register byte to an I2C slave.
 * bsc:  mapped BSC1 register base.
 * addr: 7-bit slave address.
 * reg:  register address within slave.
 * val:  value to write.
 */
static int rafaelia_m030_i2c_write(volatile uint32_t *bsc,
                                    uint8_t addr, uint8_t reg, uint8_t val)
{
    uint32_t n, st;
    if (!bsc || addr > 0x7fu) return RAF_HW_ERR_INVALID;
    /* Caller must provide the separately mapped BSC1 register block. */
    bsc[BSC_S_OFFSET] = BSC_S_DONE | BSC_S_ERR | BSC_S_CLKT;
    bsc[BSC_A_OFFSET] = (uint32_t)addr;
    bsc[BSC_DLEN_OFFSET] = 2u;
    bsc[BSC_C_OFFSET] = BSC_C_I2CEN | BSC_C_CLEAR;
    bsc[BSC_FIFO_OFFSET] = (uint32_t)reg;
    bsc[BSC_FIFO_OFFSET] = (uint32_t)val;
    bsc[BSC_C_OFFSET] = BSC_C_I2CEN | BSC_C_ST;

    for (n = 0; n < RAF_HW_POLL_LIMIT; ++n) {
        st = bsc[BSC_S_OFFSET];
        if (st & (BSC_S_ERR | BSC_S_CLKT)) {
            bsc[BSC_S_OFFSET] = BSC_S_ERR | BSC_S_CLKT;
            bsc[BSC_C_OFFSET] = 0u;
            return RAF_HW_ERR_IO;
        }
        if (st & BSC_S_DONE) {
            bsc[BSC_S_OFFSET] = BSC_S_DONE;
            bsc[BSC_C_OFFSET] = 0u;
            return RAF_HW_OK;
        }
    }
    bsc[BSC_C_OFFSET] = 0u;
    return RAF_HW_ERR_TIMEOUT;
}

int rafaelia_m030_i2c_por_registrador_bcm(volatile uint32_t *mmio_base)
{
    if (!mmio_base) return RAF_HW_ERR_NO_DEVICE;
#if defined(RASPBERRYPI)
    mmio_base[BSC_DIV_OFFSET] = 0x05dcu;
    mmio_base[BSC_C_OFFSET] = BSC_C_I2CEN;
    return rafaelia_m030_i2c_write(mmio_base, 0x3cu, 0x00u, 0x01u);
#else
    (void)rafaelia_m030_i2c_write;
    return RAF_HW_ERR_UNSUPPORTED;
#endif
}
