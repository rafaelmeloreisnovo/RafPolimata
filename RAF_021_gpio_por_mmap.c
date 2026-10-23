#include "RAF_rafaelia_common.h"
#include "RAF_hw_status.h"

/*
 * Método M021: GPIO por mmap
 * Alvo: Raspberry/Linux
 * Domínio: GPIO
 * Ganho estimado: 10x-100x vs libs
 *
 * Mapeia registradores GPIO via /dev/mem no espaço do processo (BCM2835/Pi3).
 *
 * Status: implementação real Linux mmap — TOKEN_VAZIO se sem /dev/mem (não-RPi).
 */

#if defined(RASPBERRYPI) && defined(RAFAELIA_BCM2837) && defined(__linux__) && !defined(__ANDROID__)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM_GPIO_BASE   0x3F200000UL
#define GPIO_BLOCK_SIZE 0x1000

int rafaelia_m021_gpio_por_mmap(void) {
    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        /* No /dev/mem or no permission — TOKEN_VAZIO, not a failure */
        return RAF_HW_ERR_NO_DEVICE;
    }

    void *gpio_map = mmap(
        NULL,
        GPIO_BLOCK_SIZE,
        PROT_READ,
        MAP_SHARED,
        fd,
        (off_t)BCM_GPIO_BASE
    );
    close(fd);

    if (gpio_map == MAP_FAILED) {
        /* mmap failed (e.g., not a Pi) — TOKEN_VAZIO */
        return RAF_HW_ERR_MAPPING;
    }

    /* Read GPIO level register GPLEV0 at byte offset 0x34, GPLEV1 at 0x38 */
    volatile uint32_t *gpio = (volatile uint32_t *)gpio_map;
    volatile uint32_t level = gpio[0x34 / 4];
    (void)level;

    if (munmap(gpio_map, GPIO_BLOCK_SIZE) != 0) return RAF_HW_ERR_IO;
    return RAF_HW_OK;
}

#else /* non-Linux stub */

int rafaelia_m021_gpio_por_mmap(void) {
    return RAF_HW_ERR_UNSUPPORTED;
}

#endif
