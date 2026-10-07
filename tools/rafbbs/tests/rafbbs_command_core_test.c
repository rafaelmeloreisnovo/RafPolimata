/* Governance: CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.
 * RAF_TOKEN_VAZIO is status vocabulary in this bounded core/test,
 * not a claim promotion or evidence substitute.
 */
#include "rafbbs_command_core.h"

int rafbbs_command_core_test(void)
{
    RafCommandDecision d;

    d = raf_command_decide(0u, 0, 0u);
    if (d.status != RAF_TOKEN_VAZIO || d.limited != 1u || d.failed != 0u)
        return 1;

    d = raf_command_decide(1u, 0, 0u);
    if (d.status != RAF_PASS || d.limited != 0u || d.failed != 0u)
        return 2;

    d = raf_command_decide(1u, 7, 1u);
    if (d.status != RAF_SKIP || d.limited != 1u || d.failed != 0u)
        return 3;

    d = raf_command_decide(1u, 7, 0u);
    if (d.status != RAF_FAIL || d.limited != 0u || d.failed != 1u)
        return 4;

    if (raf_status_name(d.status)[0] != 'F')
        return 5;

    return 0;
}

#ifdef RAFBBS_COMMAND_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_command_core_test();
}
#endif
