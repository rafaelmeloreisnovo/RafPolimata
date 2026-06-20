#include "RAF_rafaelia_common.h"

/*
 * Método M023: GPIO por /dev/mem controlado
 * Alvo: Raspberry/Linux
 * Domínio: MMIO
 * Ganho estimado: baixo nível
 *
 * Demonstra controle de direção e nível de GPIO4 via /dev/mem (BCM2835/Pi3).
 * Escreve GPFSEL0, GPSET0, GPCLR0 para exercitar o pipeline completo.
 *
 * Status: implementação real Linux mmap — TOKEN_VAZIO se sem /dev/mem (não-RPi).
 */

#if defined(__linux__) && !defined(__ANDROID__)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM_GPIO_BASE   0x3F200000UL
#define GPIO_BLOCK_SIZE 0x1000

/* BCM2835 GPIO register word offsets */
#define GPFSEL0_OFF  0   /* Function select 0 (GPIO 0-9) */
#define GPSET0_OFF   7   /* Output set 0 (GPIO 0-31) */
#define GPCLR0_OFF  10   /* Output clear 0 (GPIO 0-31) */
#define GPLEV0_OFF  13   /* Level 0 (GPIO 0-31) */

int rafaelia_m023_gpio_por_dev_mem_controlado(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        /* No /dev/mem or no permission — TOKEN_VAZIO, not a failure */
        return 0;
    }

    void *gpio_map = mmap(
        NULL,
        GPIO_BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        (off_t)BCM_GPIO_BASE
    );
    close(fd);

    if (gpio_map == MAP_FAILED) {
        return 0;
    }

    volatile uint32_t *gpio = (volatile uint32_t *)gpio_map;

    /* Set GPIO4 to output: clear bits 14-12 in GPFSEL0, then set bit 12 */
    uint32_t sel = gpio[GPFSEL0_OFF];
    sel &= ~(7u << 12);  /* clear FSEL4 (bits 14:12) */
    sel |=  (1u << 12);  /* set as output (001) */
    gpio[GPFSEL0_OFF] = sel;

    /* Set GPIO4 high */
    gpio[GPSET0_OFF] = (1u << 4);

    /* Read back level (just to exercise the path) */
    volatile uint32_t level = gpio[GPLEV0_OFF];
    (void)level;

    /* Clear GPIO4 (set low) */
    gpio[GPCLR0_OFF] = (1u << 4);

    munmap(gpio_map, GPIO_BLOCK_SIZE);
    return 0;
}

#else /* non-Linux stub */

int rafaelia_m023_gpio_por_dev_mem_controlado(void) {
    return 0;
}

#endif
