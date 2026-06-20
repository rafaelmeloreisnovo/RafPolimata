#include "RAF_rafaelia_common.h"

/*
 * Método M035: GPIO event detect por polling leve
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: GPIO/IRQ
 * Ganho estimado: baixa latência sem overhead de IRQ
 *
 * Detecta eventos GPIO pondo-a-pondo o registrador GPEDS0.
 * Self-test usa fake GPIO array — verificável sem hardware.
 */

/* GPIO register offsets (uint32_t words from gpio_base = 0x3F200000) */
#define GPFSEL0_WORD  (0x00u / 4u)  /* Function select 0 (GPIO 0-9) */
#define GPSET0_WORD   (0x1Cu / 4u)  /* Pin output set 0 (GPIO 0-31) */
#define GPCLR0_WORD   (0x28u / 4u)  /* Pin output clear 0 */
#define GPLEV0_WORD   (0x34u / 4u)  /* Pin level 0 */
#define GPEDS0_WORD   (0x40u / 4u)  /* Event detect status 0 */
#define GPREN0_WORD   (0x4Cu / 4u)  /* Rising edge detect enable 0 */
#define GPFEN0_WORD   (0x58u / 4u)  /* Falling edge detect enable 0 */

/*
 * rafaelia_m035_enable_rising_edge — enable rising-edge event detection for a pin.
 * gpio_base: mapped GPIO register base.
 * pin:       GPIO pin number (0-31).
 */
static void rafaelia_m035_enable_rising_edge(volatile uint32_t *gpio_base,
                                              uint32_t pin)
    __attribute__((unused));
static void rafaelia_m035_enable_rising_edge(volatile uint32_t *gpio_base,
                                              uint32_t pin)
{
    gpio_base[GPREN0_WORD] |= (1u << pin);
}

/*
 * rafaelia_m035_enable_falling_edge — enable falling-edge event detection for a pin.
 */
static void rafaelia_m035_enable_falling_edge(volatile uint32_t *gpio_base,
                                               uint32_t pin)
    __attribute__((unused));
static void rafaelia_m035_enable_falling_edge(volatile uint32_t *gpio_base,
                                               uint32_t pin)
{
    gpio_base[GPFEN0_WORD] |= (1u << pin);
}

/*
 * rafaelia_m035_poll_gpio_event — poll GPEDS0 for any bit in pin_mask.
 * gpio_base:  mapped GPIO register base (or fake array for self-test).
 * pin_mask:   bitmask of GPIO pins to watch (e.g. 1u<<17 for GPIO17).
 * max_polls:  iteration limit (prevents infinite loops without hardware).
 *
 * Returns 1 when an event is detected and cleared, 0 on timeout.
 */
static int rafaelia_m035_poll_gpio_event(volatile uint32_t *gpio_base,
                                          uint32_t pin_mask,
                                          uint32_t max_polls)
{
    if (!gpio_base) return 0; /* TOKEN_VAZIO */
    for (uint32_t i = 0u; i < max_polls; i++) {
        if (gpio_base[GPEDS0_WORD] & pin_mask) {
            gpio_base[GPEDS0_WORD] = pin_mask; /* clear by writing 1 */
            return 1; /* event detected */
        }
    }
    return 0; /* timeout — no event within max_polls iterations */
}

int rafaelia_m035_gpio_event_detect_por_polling_leve(volatile uint32_t *mmio_base)
{
    (void)mmio_base;

#if defined(RASPBERRYPI)
    if (mmio_base) {
        /* Enable both edge detectors on GPIO17 */
        rafaelia_m035_enable_rising_edge(mmio_base, 17u);
        rafaelia_m035_enable_falling_edge(mmio_base, 17u);
        /* Poll for up to 1 000 000 iterations (non-blocking on real hardware) */
        int found = rafaelia_m035_poll_gpio_event(mmio_base, (1u << 17), 1000000u);
        (void)found;
        return 0;
    }
#endif

    /*
     * Self-test: simulate an event on pin 17 using a local fake GPIO array.
     *
     * Note on write-1-to-clear (W1C): BCM GPEDS0 clears a bit when the CPU
     * writes 1 to that bit position. A plain volatile array cannot model this
     * hardware behaviour — the write simply stores the value unchanged. We
     * therefore only verify that the polling function detects the planted event
     * (found == 1); we do not check the post-clear value, which would require
     * a real memory-mapped W1C register.
     */
    static volatile uint32_t _fake_gpio[64];
    /* Zero the array first (static, but may retain values across calls) */
    for (int i = 0; i < 64; i++) _fake_gpio[i] = 0u;

    /* Plant a simulated rising-edge event on pin 17 */
    _fake_gpio[GPEDS0_WORD] = (1u << 17);

    int found = rafaelia_m035_poll_gpio_event(_fake_gpio, (1u << 17), 10u);

    /* Verify the polling logic detected the event — W1C semantics not checked */
    return (found == 1) ? 0 : -1;
}
