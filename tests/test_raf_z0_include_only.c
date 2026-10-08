#include "../tools/raf_z0_tokenizer.h"

/* Inclusion-only smoke test: no calls, no unused-function warnings. */
int main(void)
{
    raf_z0_token_kind kind = RAF_Z0_ABSOLUTE_EMPTY;
    return kind == RAF_Z0_ABSOLUTE_EMPTY ? 0 : 1;
}
