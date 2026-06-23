#include "RAF_rafaelia_common.h"

/*
 * Método M060: Bootloader OTA via UART polling
 * Alvo: MCU/AVR
 * Domínio: Bootloader/OTA
 * Ganho estimado: atualização firmware sem programador externo
 * Status: AVR-gated para UART real; selftest simulado em buffer estático
 *
 * Protocolo OTA simples:
 *   [0xAA] [size_lo] [size_hi] [payload...] [CRC-8]
 * CRC-8 = XOR de todos os bytes do payload.
 * Buffer máximo: 128 bytes de payload (sem heap).
 * Usa polling em UCSR0A.RXC0 (bit 7) — dependência de M012.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Protocol constants */
#define M060_OTA_START_BYTE 0xAAu
#define M060_FW_BUF_SIZE    128u

/* Firmware receive buffer — static, no heap */
static uint8_t _m060_fw_buf[M060_FW_BUF_SIZE];

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)

/* ATmega328P UART register addresses */
#define AVR_UCSR0A_ADDR 0xC0u
#define AVR_UCSR0B_ADDR 0xC1u
#define AVR_UBRR0L_ADDR 0xC4u
#define AVR_UBRR0H_ADDR 0xC5u
#define AVR_UDR0_ADDR   0xC6u

/* UCSR0A bits */
#define AVR_RXC0  7u   /* RX Complete */
#define AVR_UDRE0 5u   /* Data Register Empty */

/* UCSR0B bits */
#define AVR_RXEN0 4u
#define AVR_TXEN0 3u

/*
 * rafaelia_m060_uart_recv_byte_timeout:
 * Waits up to timeout_ms milliseconds for a byte to arrive on UART0.
 * Returns received byte (0-255) on success, -1 on timeout.
 * On AVR at 16 MHz with baud 9600 each iteration is ~1 µs; timeout_ms
 * is approximated by 1000 spin loops per ms (conservative).
 */
int rafaelia_m060_uart_recv_byte_timeout(uint16_t timeout_ms) {
    uint32_t deadline = (uint32_t)timeout_ms * 1000u; /* rough spin count */
    while (!(RAFA_MMIO8(AVR_UCSR0A_ADDR) & (uint8_t)(1u << AVR_RXC0))) {
        if (deadline == 0u) {
            return -1; /* timeout */
        }
        deadline--;
    }
    return (int)(uint8_t)RAFA_MMIO8(AVR_UDR0_ADDR);
}

/*
 * rafaelia_m060_bootloader_ota_uart:
 * Main bootloader OTA state machine (AVR path):
 *   1. Wait for start byte 0xAA (timeout 5000 ms).
 *   2. Read 2-byte little-endian payload size.
 *   3. Read payload bytes into _m060_fw_buf (capped at M060_FW_BUF_SIZE).
 *   4. Read CRC-8 byte.
 *   5. Verify CRC-8 = XOR of all payload bytes.
 * Returns 0 on successful reception and CRC pass, -1 on any error.
 */
int rafaelia_m060_bootloader_ota_uart(void) {
    int b;
    uint16_t size;
    uint16_t i;
    uint8_t  crc_calc = 0u;
    uint8_t  crc_recv;

    /* Step 1: wait for start byte */
    b = rafaelia_m060_uart_recv_byte_timeout(5000u);
    if (b != (int)M060_OTA_START_BYTE) {
        return -1;
    }

    /* Step 2: read payload size (little-endian, 2 bytes) */
    b = rafaelia_m060_uart_recv_byte_timeout(1000u);
    if (b < 0) { return -1; }
    size = (uint16_t)(uint8_t)b;

    b = rafaelia_m060_uart_recv_byte_timeout(1000u);
    if (b < 0) { return -1; }
    size |= (uint16_t)((uint8_t)b << 8u);

    if (size == 0u || size > (uint16_t)M060_FW_BUF_SIZE) {
        return -1;
    }

    /* Step 3: read payload */
    for (i = 0u; i < size; i++) {
        b = rafaelia_m060_uart_recv_byte_timeout(1000u);
        if (b < 0) { return -1; }
        _m060_fw_buf[i] = (uint8_t)b;
        crc_calc ^= (uint8_t)b;
    }

    /* Step 4: read CRC-8 */
    b = rafaelia_m060_uart_recv_byte_timeout(1000u);
    if (b < 0) { return -1; }
    crc_recv = (uint8_t)b;

    /* Step 5: verify */
    return (crc_calc == crc_recv) ? 0 : -1;
}

#else /* Host / non-AVR */

/*
 * rafaelia_m060_uart_recv_byte_timeout:
 * Host stub: no UART hardware — always returns -1 (timeout).
 */
int rafaelia_m060_uart_recv_byte_timeout(uint16_t timeout_ms) {
    (void)timeout_ms;
    return -1; /* no hardware */
}

/*
 * rafaelia_m060_bootloader_ota_uart:
 * Host stub: TOKEN_VAZIO — no UART hardware present.
 * Returns 0 (no-op, not an error).
 */
int rafaelia_m060_bootloader_ota_uart(void) {
    return 0; /* TOKEN_VAZIO */
}

#endif /* AVR gate */

/* ------------------------------------------------------------------ */
/* CRC-8 helper (always compiled — used by selftest)                   */
/* ------------------------------------------------------------------ */

static uint8_t rafaelia_m060_crc8_buf(const uint8_t *buf, uint16_t len) {
    uint8_t  crc = 0u;
    uint16_t i;
    for (i = 0u; i < len; i++) {
        crc ^= buf[i];
    }
    return crc;
}

/* ------------------------------------------------------------------ */
/* Selftest — simulates the OTA protocol in a static buffer            */
/* ------------------------------------------------------------------ */

/*
 * rafaelia_m060_selftest:
 * Builds an OTA packet in a static buffer:
 *   [0xAA] [size_lo] [size_hi] [payload...] [CRC-8]
 * Then parses it manually (without UART), verifying:
 *   - start byte == 0xAA
 *   - size decoded correctly
 *   - CRC-8 (XOR) matches
 * Returns 0=PASS, -1=FAIL.
 */
int rafaelia_m060_selftest(void) {
    /* Build a test packet: 8-byte payload */
    static const uint8_t payload[8] = {
        0x01u, 0x02u, 0x03u, 0x04u,
        0x05u, 0x06u, 0x07u, 0x08u
    };
    static uint8_t pkt[8u + 4u]; /* start + size_lo + size_hi + payload + crc */

    uint16_t payload_len = 8u;
    uint8_t  crc;
    uint16_t i;
    uint16_t idx = 0u;

    /* Build packet */
    pkt[idx++] = M060_OTA_START_BYTE;
    pkt[idx++] = (uint8_t)(payload_len & 0xFFu);
    pkt[idx++] = (uint8_t)((payload_len >> 8u) & 0xFFu);
    for (i = 0u; i < payload_len; i++) {
        pkt[idx++] = payload[i];
    }
    crc = rafaelia_m060_crc8_buf(payload, payload_len);
    pkt[idx++] = crc;

    /* Parse and verify packet */
    idx = 0u;

    /* Check start byte */
    if (pkt[idx++] != M060_OTA_START_BYTE) { return -1; }

    /* Decode size */
    uint16_t recv_size = (uint16_t)pkt[idx++];
    recv_size |= (uint16_t)((uint16_t)pkt[idx++] << 8u);
    if (recv_size != payload_len) { return -1; }
    if (recv_size > (uint16_t)M060_FW_BUF_SIZE) { return -1; }

    /* Read payload into fw_buf */
    uint8_t crc_calc = 0u;
    for (i = 0u; i < recv_size; i++) {
        _m060_fw_buf[i] = pkt[idx++];
        crc_calc ^= _m060_fw_buf[i];
    }

    /* Check CRC */
    uint8_t crc_recv = pkt[idx];
    if (crc_calc != crc_recv) { return -1; }

    /* Verify payload content */
    for (i = 0u; i < payload_len; i++) {
        if (_m060_fw_buf[i] != payload[i]) { return -1; }
    }

    return 0;
}
