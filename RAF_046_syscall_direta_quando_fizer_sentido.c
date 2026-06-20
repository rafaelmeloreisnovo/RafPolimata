#include "RAF_rafaelia_common.h"

/*
 * Método M046: Syscall direta quando fizer sentido
 * Alvo: Linux/Android
 * Domínio: Syscall
 * Ganho estimado: 1.2x-5x pontual
 *
 * Chama syscall(SYS_gettid) diretamente, sem wrapper libc.
 * Demonstra o caminho direto para syscalls críticas de latência.
 *
 * Status: implementação real syscall(SYS_gettid) + verificação TID > 0.
 */

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/syscall.h>
#include <unistd.h>

int rafaelia_m046_syscall_direta_quando_fizer_sentido(void) {
    long tid = syscall(SYS_gettid);
    return (tid > 0) ? 0 : -1;
}

#else

int rafaelia_m046_syscall_direta_quando_fizer_sentido(void) {
    return 0;
}

#endif
