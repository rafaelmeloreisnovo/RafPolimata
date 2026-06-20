#include "RAF_rafaelia_common.h"

/*
 * Método M058: CAN bus via SPI + MCP2515
 * Alvo: MCU/AVR
 * Domínio: CAN/Comunicação
 * Ganho estimado: isolamento de barramento CAN sem periférico nativo
 * Status: host-safe (spi_base=NULL → TOKEN_VAZIO, retorna 0)
 *
 * MCP2515 é um controlador CAN externo acessado via SPI.
 * Comandos SPI: RESET(0xC0), READ(0x03), WRITE(0x02), RTS(0x80+),
 *               READ_STATUS(0xA0).
 * Registradores principais: CANSTAT=0x0E, CANCTRL=0x0F,
 *                            CNF1=0x2A, CNF2=0x29, CNF3=0x28.
 */

/* MCP2515 SPI instruction set */
#define MCP2515_CMD_RESET       0xC0u
#define MCP2515_CMD_READ        0x03u
#define MCP2515_CMD_WRITE       0x02u
#define MCP2515_CMD_RTS_BASE    0x80u
#define MCP2515_CMD_READ_STATUS 0xA0u

/* MCP2515 register addresses */
#define MCP2515_CANSTAT 0x0Eu
#define MCP2515_CANCTRL 0x0Fu
#define MCP2515_CNF1    0x2Au
#define MCP2515_CNF2    0x29u
#define MCP2515_CNF3    0x28u

/*
 * Generic SPI register offsets for a memory-mapped SPI peripheral.
 * Exact offsets are platform-specific; these match a common ARM SoC layout.
 * On AVR, SPI is accessed via SPDR/SPSR/SPCR at fixed I/O addresses instead.
 */
#define SPI_REG_CR    0u   /* Control register word index  */
#define SPI_REG_SR    1u   /* Status register word index   */
#define SPI_REG_DR    2u   /* Data register word index     */

#define SPI_SR_TXE    (1u << 1)  /* TX buffer empty */
#define SPI_SR_RXNE   (1u << 0)  /* RX buffer not empty */

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
/* AVR hardware SPI I/O register addresses */
#define AVR_SPCR_ADDR 0x4Cu
#define AVR_SPSR_ADDR 0x4Du
#define AVR_SPDR_ADDR 0x4Eu

#define AVR_SPE  6u   /* SPI Enable bit in SPCR  */
#define AVR_MSTR 4u   /* Master mode bit in SPCR */
#define AVR_SPIF 7u   /* SPI Interrupt Flag in SPSR */

/*
 * rafaelia_m058_avr_spi_byte:
 * Sends one byte over AVR hardware SPI and returns the received byte.
 * Blocking: waits for SPIF.
 */
static uint8_t rafaelia_m058_avr_spi_byte(uint8_t tx) {
    RAFA_MMIO8(AVR_SPDR_ADDR) = tx;
    while (!(RAFA_MMIO8(AVR_SPSR_ADDR) & (uint8_t)(1u << AVR_SPIF))) {
        /* spin */
    }
    return RAFA_MMIO8(AVR_SPDR_ADDR);
}
#endif /* AVR */

/*
 * rafaelia_m058_can_bus_mcp2515_spi:
 * Sends a command (and optional address + data) to the MCP2515 via SPI.
 *
 * spi_base: pointer to memory-mapped SPI peripheral registers (32-bit words).
 *           Pass NULL on host or when no SPI hardware is present (TOKEN_VAZIO).
 * cmd:      MCP2515 SPI instruction byte (e.g. MCP2515_CMD_RESET).
 * addr:     register address for READ/WRITE commands; ignored for RESET.
 * data:     byte to write; ignored for READ/READ_STATUS/RESET commands.
 *
 * Returns:
 *   0  on success or TOKEN_VAZIO (spi_base == NULL).
 *  -1  on error (SPI timeout — host path never reaches here).
 */
int rafaelia_m058_can_bus_mcp2515_spi(volatile uint32_t *spi_base,
                                       uint8_t cmd,
                                       uint8_t addr,
                                       uint8_t data) {
    /* TOKEN_VAZIO: no hardware present — no-op */
    if (spi_base == (volatile uint32_t *)0) {
        return 0;
    }

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    (void)spi_base; /* AVR uses fixed I/O addresses via rafaelia_m058_avr_spi_byte */

    switch (cmd) {
    case MCP2515_CMD_RESET:
        rafaelia_m058_avr_spi_byte(MCP2515_CMD_RESET);
        break;

    case MCP2515_CMD_WRITE:
        rafaelia_m058_avr_spi_byte(MCP2515_CMD_WRITE);
        rafaelia_m058_avr_spi_byte(addr);
        rafaelia_m058_avr_spi_byte(data);
        break;

    case MCP2515_CMD_READ:
        rafaelia_m058_avr_spi_byte(MCP2515_CMD_READ);
        rafaelia_m058_avr_spi_byte(addr);
        rafaelia_m058_avr_spi_byte(0x00u); /* dummy to clock in RX byte */
        break;

    case MCP2515_CMD_READ_STATUS:
        rafaelia_m058_avr_spi_byte(MCP2515_CMD_READ_STATUS);
        rafaelia_m058_avr_spi_byte(0x00u); /* dummy to clock in status */
        break;

    default:
        /* RTS and other commands: send cmd byte only */
        rafaelia_m058_avr_spi_byte(cmd);
        break;
    }
    return 0;

#else
    /*
     * Memory-mapped SPI path (ARM SoC or similar).
     * Wait for TX empty, write byte to DR, repeat for addr/data as needed.
     */
    uint32_t timeout;

    /* Helper lambda via goto not available in C11; inline the wait */
#define M058_SPI_WAIT_TX(base, tout)                            \
    do {                                                         \
        (tout) = 0xFFFFu;                                        \
        while (!((base)[SPI_REG_SR] & SPI_SR_TXE)) {            \
            if (--(tout) == 0u) { return -1; }                   \
        }                                                        \
    } while (0)

    switch (cmd) {
    case MCP2515_CMD_RESET:
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)MCP2515_CMD_RESET;
        break;

    case MCP2515_CMD_WRITE:
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)MCP2515_CMD_WRITE;
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)addr;
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)data;
        break;

    case MCP2515_CMD_READ:
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)MCP2515_CMD_READ;
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)addr;
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = 0x00u; /* dummy byte */
        break;

    case MCP2515_CMD_READ_STATUS:
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)MCP2515_CMD_READ_STATUS;
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = 0x00u; /* dummy byte */
        break;

    default:
        M058_SPI_WAIT_TX(spi_base, timeout);
        spi_base[SPI_REG_DR] = (uint32_t)cmd;
        break;
    }

#undef M058_SPI_WAIT_TX

    return 0;
#endif /* AVR */
}

/*
 * rafaelia_m058_selftest:
 * Verifies that cmd=RESET(0xC0) with spi_base=NULL returns 0 (TOKEN_VAZIO).
 * Also verifies that READ/WRITE/READ_STATUS with NULL also return 0.
 * Returns 0=PASS, -1=FAIL.
 */
int rafaelia_m058_selftest(void) {
    int r;

    /* TOKEN_VAZIO path: all commands with NULL spi_base must return 0 */
    r = rafaelia_m058_can_bus_mcp2515_spi(
            (volatile uint32_t *)0,
            MCP2515_CMD_RESET, 0x00u, 0x00u);
    if (r != 0) { return -1; }

    r = rafaelia_m058_can_bus_mcp2515_spi(
            (volatile uint32_t *)0,
            MCP2515_CMD_WRITE, MCP2515_CNF1, 0x01u);
    if (r != 0) { return -1; }

    r = rafaelia_m058_can_bus_mcp2515_spi(
            (volatile uint32_t *)0,
            MCP2515_CMD_READ, MCP2515_CANSTAT, 0x00u);
    if (r != 0) { return -1; }

    r = rafaelia_m058_can_bus_mcp2515_spi(
            (volatile uint32_t *)0,
            MCP2515_CMD_READ_STATUS, 0x00u, 0x00u);
    if (r != 0) { return -1; }

    return 0;
}
