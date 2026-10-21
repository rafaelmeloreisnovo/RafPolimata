#include "RAF_rafaelia_common.h"
#include "RAF_hw_status.h"

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

#if defined(RASPBERRYPI) && defined(__linux__) && !defined(__ANDROID__)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_BLOCK_SIZE 0x1000

int rafaelia_m022_gpio_por_dev_gpiomem(void) {
    int fd = open("/dev/gpiomem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        /* Not a Raspberry Pi or no /dev/gpiomem — TOKEN_VAZIO */
        return RAF_HW_ERR_NO_DEVICE;
    }

    void *gpio_map = mmap(
        NULL,
        GPIO_BLOCK_SIZE,
        PROT_READ,
        MAP_SHARED,
        fd,
        0  /* /dev/gpiomem is pre-mapped to the GPIO block, offset 0 */
    );
    close(fd);

    if (gpio_map == MAP_FAILED) {
        return RAF_HW_ERR_MAPPING;
    }

    /* GPLEV0 is at word offset 13 (byte offset 0x34) */
    volatile uint32_t *gpio = (volatile uint32_t *)gpio_map;
    volatile uint32_t level = gpio[13]; /* GPLEV0 */
    (void)level;

    if (munmap(gpio_map, GPIO_BLOCK_SIZE) != 0) return RAF_HW_ERR_IO;
    return RAF_HW_OK;
}

#else /* non-Linux stub */

int rafaelia_m022_gpio_por_dev_gpiomem(void) {
    return RAF_HW_ERR_UNSUPPORTED;
}

#endif
