#include "RAF_rafaelia_common.h"

/*
 * Método M057: EEPROM wear-leveling por endereço circular
 * Alvo: MCU/AVR
 * Domínio: EEPROM/Storage
 * Ganho estimado: vida útil 10-100x mais longa que escrita no mesmo endereço
 * Status: AVR-gated para escrita real; selftest simulado em array estático
 *
 * ATmega328P EEPROM: 1024 bytes, endereços 0x000-0x3FF.
 * Técnica: ponteiro circular avança a cada escrita por N slots de 1 byte.
 * Espalha desgaste uniformemente por toda a memória EEPROM.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ATmega328P EEPROM register addresses (from datasheet) */
#define M057_EECR_ADDR  0x1Fu   /* EEPROM Control Register  */
#define M057_EEDR_ADDR  0x20u   /* EEPROM Data Register     */
#define M057_EEARL_ADDR 0x21u   /* EEPROM Address Low byte  */
#define M057_EEARH_ADDR 0x22u   /* EEPROM Address High byte */

/* EECR bits */
#define M057_EERE   0u  /* EEPROM Read Enable  */
#define M057_EEPE   1u  /* EEPROM Write Enable */
#define M057_EEMPE  2u  /* Master Write Enable */

/* Wear-leveling parameters */
#define M057_EEPROM_SIZE  1024u  /* ATmega328P EEPROM bytes   */
#define M057_SLOT_SIZE    1u     /* bytes per slot            */
#define M057_NUM_SLOTS    (M057_EEPROM_SIZE / M057_SLOT_SIZE)

/* Circular write pointer — persists across calls */
static uint16_t _m057_write_addr = 0u;

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
/*
 * rafaelia_m057_avr_eeprom_write_byte:
 * Blocking EEPROM write on ATmega328P.
 * Waits for any previous write to finish before starting.
 */
static void rafaelia_m057_avr_eeprom_write_byte(uint16_t addr, uint8_t data) {
    /* Wait for previous write to complete */
    while (RAFA_MMIO8(M057_EECR_ADDR) & (uint8_t)(1u << M057_EEPE)) {
        /* spin */
    }
    /* Set address registers */
    RAFA_MMIO8(M057_EEARH_ADDR) = (uint8_t)((addr >> 8u) & 0x03u);
    RAFA_MMIO8(M057_EEARL_ADDR) = (uint8_t)(addr & 0xFFu);
    /* Set data */
    RAFA_MMIO8(M057_EEDR_ADDR) = data;
    /* Master write enable then write enable (must be within 4 cycles) */
    RAFA_MMIO8(M057_EECR_ADDR) |= (uint8_t)(1u << M057_EEMPE);
    RAFA_MMIO8(M057_EECR_ADDR) |= (uint8_t)(1u << M057_EEPE);
}
#endif /* AVR */

/*
 * rafaelia_m057_eeprom_wear_leveling:
 * Writes data to the current circular slot and advances the pointer.
 * On AVR: performs real EEPROM write.
 * On host: updates a shadow array for simulation.
 * Returns 0 on success.
 */

/* Shadow array for host simulation (always compiled) */
static uint8_t _m057_shadow[M057_NUM_SLOTS];
/* Write-count array for selftest verification */
static uint16_t _m057_write_count[M057_NUM_SLOTS];

int rafaelia_m057_eeprom_wear_leveling(uint8_t data) {
    uint16_t slot = _m057_write_addr;

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    rafaelia_m057_avr_eeprom_write_byte(slot * (uint16_t)M057_SLOT_SIZE, data);
#else
    /* Host simulation: write into shadow array */
    _m057_shadow[slot] = data;
    _m057_write_count[slot]++;
#endif

    /* Advance pointer circularly */
    _m057_write_addr = (uint16_t)((_m057_write_addr + 1u) % M057_NUM_SLOTS);
    return 0;
}

/*
 * rafaelia_m057_selftest:
 * Simulates N writes using the shadow array.
 * Verifies that the write pointer circulates through all slots exactly once
 * after NUM_SLOTS writes, and each slot receives exactly 1 write.
 * Returns 0=PASS, -1=FAIL.
 */
int rafaelia_m057_selftest(void) {
    uint16_t i;

    /* Reset state */
    _m057_write_addr = 0u;
    for (i = 0u; i < (uint16_t)M057_NUM_SLOTS; i++) {
        _m057_shadow[i]      = 0u;
        _m057_write_count[i] = 0u;
    }

    /* Write NUM_SLOTS times — each slot should receive exactly one write */
    for (i = 0u; i < (uint16_t)M057_NUM_SLOTS; i++) {
        if (rafaelia_m057_eeprom_wear_leveling((uint8_t)(i & 0xFFu)) != 0) {
            return -1;
        }
    }

    /* Pointer must have wrapped back to 0 */
    if (_m057_write_addr != 0u) {
        return -1;
    }

    /* Every slot must have exactly one write */
    for (i = 0u; i < (uint16_t)M057_NUM_SLOTS; i++) {
        if (_m057_write_count[i] != 1u) {
            return -1;
        }
        /* Shadow must hold the expected data byte */
        if (_m057_shadow[i] != (uint8_t)(i & 0xFFu)) {
            return -1;
        }
    }

    return 0;
}
