#include "RAF_rafaelia_common.h"

/*
 * Método M045: Detecção de ABI em runtime
 * Alvo: Android
 * Domínio: Runtime
 * Ganho estimado: robustez
 *
 * Detecta ABI em tempo de compilação e expõe como valor de runtime.
 * Em código freestanding/embedded a ABI é sempre conhecida na compilação.
 *
 * Status: implementação real via macros de pré-processador — retorna enum ABI.
 */

typedef enum {
    ABI_UNKNOWN = 0,
    ABI_ARM64   = 1,
    ABI_ARM32   = 2,
    ABI_X86_64  = 3,
    ABI_X86     = 4
} rafaelia_abi_t;

static rafaelia_abi_t rafaelia_m045_detect_abi(void) {
#if defined(__aarch64__)
    return ABI_ARM64;
#elif defined(__arm__)
    return ABI_ARM32;
#elif defined(__x86_64__)
    return ABI_X86_64;
#elif defined(__i386__)
    return ABI_X86;
#else
    return ABI_UNKNOWN;
#endif
}

int rafaelia_m045_deteccao_de_abi_em_runtime(void) {
    rafaelia_abi_t abi = rafaelia_m045_detect_abi();
    /* We are always running on a known ABI — ABI_UNKNOWN means
     * genuinely unsupported target, which is a configuration failure. */
    return (abi != ABI_UNKNOWN) ? 0 : -1;
}
