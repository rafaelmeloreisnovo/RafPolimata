#include "rafbbs_result_core.h"

int rafbbs_result_core_test(void)
{
    RafResultDecision d;

    d = raf_result_decide(RAF_PASS, 0u, 0u, 0u);
    if (d.final_status != RAF_PASS) return 1;
    if ((d.hash_state & RAFBBS_HASH_TOKEN_VAZIO) == 0u) return 2;

    /* Numeric CRC32=0 is intentionally irrelevant here: validity is separate. */
    d = raf_result_decide(RAF_PASS_LIMITED, 0u, 0u, 1u);
    if (d.final_status != RAF_PASS_LIMITED) return 3;
    if ((d.hash_state & RAFBBS_HASH_CRC32_OK) == 0u) return 4;
    if ((d.hash_state & RAFBBS_HASH_TOKEN_VAZIO) != 0u) return 5;

    d = raf_result_decide(RAF_PASS, 0u, 1u, 0u);
    if ((d.hash_state & RAFBBS_HASH_SHA256_OK) == 0u) return 6;
    if ((d.hash_state & RAFBBS_HASH_TOKEN_VAZIO) != 0u) return 7;

    d = raf_result_decide(RAF_PASS_LIMITED, 1u, 1u, 1u);
    if (d.final_status != RAF_FAIL) return 8;

    return 0;
}

#ifdef RAFBBS_RESULT_CORE_TEST_MAIN
int main(void) { return rafbbs_result_core_test(); }
#endif
