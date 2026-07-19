#include "../../freestanding/omega/omega_core.h"

int main(void)
{
    raf_omega_state a;
    raf_omega_state b;
    raf_omega_u32 i;

    raf_omega_init(&a, 144000u);
    raf_omega_init(&b, 144000u);
    for (i = 0u; i < 8u; ++i) {
        raf_omega_set_friction(&a, i, 11u + i * 7u);
        raf_omega_set_friction(&b, i, 11u + i * 7u);
    }
    for (i = 0u; i < 96u; ++i) {
        const raf_omega_u32 token = 0x9e3779b9u * (i + 1u);
        if (raf_omega_step(&a, token, i) != raf_omega_step(&b, token, i)) {
            return 10;
        }
    }
    if (raf_omega_validate(&a) != 1u || raf_omega_validate(&b) != 1u) {
        return 11;
    }
    if (raf_omega_digest(&a) != raf_omega_digest(&b)) {
        return 12;
    }
    return 0;
}
