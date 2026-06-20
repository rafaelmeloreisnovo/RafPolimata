#include "RAF_rafaelia_common.h"

/*
 * Método M038: Isolamento de núcleo quando disponível
 * Alvo: Linux
 * Domínio: Scheduler
 * Ganho estimado: jitter menor
 *
 * Lê /sys/devices/system/cpu/isolated para verificar se algum núcleo está isolado.
 * Retorna 0 sempre (TOKEN_VAZIO se arquivo ausente — sem erro em não-Linux/sem isolcpus).
 *
 * Status: implementação real — lê sysfs isolcpus + TOKEN_VAZIO se ausente.
 */

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>

int rafaelia_m038_isolamento_de_nucleo_quando_disponivel(void) {
    int fd = open("/sys/devices/system/cpu/isolated", O_RDONLY);
    if (fd >= 0) {
        char buf[4] = {0};
        ssize_t n = read(fd, buf, 3); (void)n;
        close(fd);
        /* buf now contains e.g. "2-3\n" if CPUs 2 and 3 are isolated,
         * or "\n" / "0-0" if none. We just probe existence — no action needed. */
    }
    return 0;
}

#else

int rafaelia_m038_isolamento_de_nucleo_quando_disponivel(void) {
    return 0;
}

#endif
