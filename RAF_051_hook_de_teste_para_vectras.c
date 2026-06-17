#include "RAF_rafaelia_common.h"

/*
 * Método M051: Hook de teste para Vectras
 * Alvo: Vectras (external instrumentation / profiling framework)
 * Domínio: Integração
 * Ganho estimado: validação de hot-paths em ambiente instrumentado
 *
 * Vectras connects to benchmark code via a registered callback pointer.
 * This module exposes the registration API and a self-test that fires
 * a synthetic event and verifies the hook received the correct value.
 */

/* Callback type: receives an event name and a 64-bit payload value */
typedef void (*rafaelia_vectras_hook_fn_t)(const char *event_name, uint64_t value);

/* Global hook slot — NULL means no hook registered */
static rafaelia_vectras_hook_fn_t _m051_hook = (rafaelia_vectras_hook_fn_t)0;

/*
 * rafaelia_m051_register_vectras_hook — install (or clear) the Vectras hook.
 * Pass NULL to deregister.
 */
void rafaelia_m051_register_vectras_hook(rafaelia_vectras_hook_fn_t fn)
{
    _m051_hook = fn;
}

/*
 * rafaelia_m051_emit_vectras_event — fire an event to the registered hook.
 * No-op when no hook is installed.
 */
void rafaelia_m051_emit_vectras_event(const char *name, uint64_t value)
{
    if (_m051_hook) {
        _m051_hook(name, value);
    }
}

/* --- Self-test infrastructure --- */

static uint64_t _m051_last_val = 0u;

static void _m051_noop_hook(const char *n, uint64_t v)
{
    (void)n;
    _m051_last_val = v;
}

int rafaelia_m051_hook_de_teste_para_vectras(void)
{
    /* Reset capture state */
    _m051_last_val = 0u;

    /* Install the no-op capture hook */
    rafaelia_m051_register_vectras_hook(_m051_noop_hook);

    /* Emit a synthetic test event */
    rafaelia_m051_emit_vectras_event("test", 0xBEEFu);

    /* Deregister so subsequent code isn't affected */
    rafaelia_m051_register_vectras_hook((rafaelia_vectras_hook_fn_t)0);

    /* Verify the hook received the correct value */
    return (_m051_last_val == 0xBEEFu) ? 0 : -1;
}
