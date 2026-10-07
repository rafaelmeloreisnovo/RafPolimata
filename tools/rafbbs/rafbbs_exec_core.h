#ifndef RAFBBS_EXEC_CORE_H
#define RAFBBS_EXEC_CORE_H

#include "rafbbs_log_core.h"

#define RAFBBS_EXEC_ARG_LIMIT 16u

typedef struct {
    const char *const *argv;
    RafU32 argc;
} RafExecSpec;

static inline RafU32 raf_exec_spec_valid(const RafExecSpec *spec)
{
    RafU32 i;
    if (spec == (const RafExecSpec *)0 ||
        spec->argv == (const char *const *)0 ||
        spec->argc == 0u ||
        spec->argc > RAFBBS_EXEC_ARG_LIMIT)
        return 0u;
    for (i = 0u; i < spec->argc; i++)
        if (spec->argv[i] == (const char *)0 || spec->argv[i][0] == 0)
            return 0u;
    if (spec->argv[spec->argc] != (const char *)0)
        return 0u;
    return 1u;
}

static inline void raf_exec_render(
    RafLogText *out,
    const RafExecSpec *spec
) {
    RafU32 i;
    if (raf_exec_spec_valid(spec) == 0u) return;
    for (i = 0u; i < spec->argc; i++) {
        if (i != 0u) raf_log_text_putc(out, ' ');
        raf_log_text_puts(out, spec->argv[i]);
    }
}

#endif
