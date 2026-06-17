#ifndef RAF_LUT_DEMO_H
#define RAF_LUT_DEMO_H

/*
 * S17 — Usar lookup table quando multiplicação/divisão pesar
 *
 * Demonstra substituição de cálculo runtime por LUT:
 * CRC-8/MAXIM (poly=0x31) calculado nibble-a-nibble via tabela de 16 entradas
 * em vez de loop de 8 XOR+shift por byte.
 *
 * Custo runtime por byte: 2 table lookups + 2 shifts + 2 XORs
 * vs. loop alternativo: 8 × (compare + shift + conditional XOR)
 *
 * Tabela gerada offline (poly=0x31, bit-reverse): 16 valores uint8_t.
 * Flash/ROM footprint: 16 bytes. Stack: 0 bytes extras.
 */

#include <stdint.h>
#include <stddef.h>

/* CRC-8/MAXIM nibble LUT (poly 0x31 reflected = 0x8C, nibble table) */
static const uint8_t _raf_crc8_lut16[16] = {
    0x00u, 0x9Du, 0x23u, 0xBEu, 0x46u, 0xDBu, 0x65u, 0xF8u,
    0x8Cu, 0x11u, 0xAFu, 0x32u, 0xCAu, 0x57u, 0xE9u, 0x74u
};

/* Update CRC-8 with one byte using the nibble LUT */
static inline uint8_t raf_crc8_update(uint8_t crc, uint8_t data) {
    crc ^= data;
    crc  = _raf_crc8_lut16[crc & 0x0Fu] ^ (crc >> 4);
    crc  = _raf_crc8_lut16[crc & 0x0Fu] ^ (crc >> 4);
    return crc;
}

/* Compute CRC-8 of a buffer */
static inline uint8_t raf_crc8_buf(const uint8_t *buf, size_t len) {
    uint8_t crc = 0x00u;
    for (size_t i = 0; i < len; i++) crc = raf_crc8_update(crc, buf[i]);
    return crc;
}

/*
 * Autovalidação: CRC-8/MAXIM do vetor [0x01,0x02,0x03] deve ser 0xD8.
 * (Known-good value from reference bitwise implementation with poly=0x31 reflected.)
 */
static inline int raf_lut_demo_selftest(void) {
    static const uint8_t vec[3] = {0x01u, 0x02u, 0x03u};
    uint8_t result = raf_crc8_buf(vec, 3u);
    return (result == 0xD8u) ? 0 : -1;
}

#endif /* RAF_LUT_DEMO_H */
