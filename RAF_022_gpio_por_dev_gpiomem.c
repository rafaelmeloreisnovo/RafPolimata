#include "RAF_rafaelia_common.h"

/*
 * Método M022: GPIO por /dev/gpiomem
 * Alvo: Raspberry/Linux
 * Domínio: GPIO
 * Ganho estimado: segurança (sem root)
 *
 * Acessa GPIO via /dev/gpiomem (sem privilégio root no RPi).
 * Base offset é 0 (arquivo já mapeia o bloco GPIO).
 *
 * Status: implementação real Linux mmap via /dev/gpiomem — TOKEN_VAZIO se ausente.
 */

#if defined(__linux__) && !defined(__ANDROID__)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_BLOCK_SIZE 0x1000

int rafaelia_m022_gpio_por_dev_gpiomem(void) {
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) {
        /* Not a Raspberry Pi or no /dev/gpiomem — TOKEN_VAZIO */
        return 0;
    }

    void *gpio_map = mmap(
        NULL,
        GPIO_BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0  /* /dev/gpiomem is pre-mapped to the GPIO block, offset 0 */
    );
    close(fd);

    if (gpio_map == MAP_FAILED) {
        return 0;
    }

    /* GPLEV0 is at word offset 13 (byte offset 0x34) */
    volatile uint32_t *gpio = (volatile uint32_t *)gpio_map;
    volatile uint32_t level = gpio[13]; /* GPLEV0 */
    (void)level;

    munmap(gpio_map, GPIO_BLOCK_SIZE);
    return 0;
}

#else /* non-Linux stub */

int rafaelia_m022_gpio_por_dev_gpiomem(void) {
    return 0;
}

#endif
