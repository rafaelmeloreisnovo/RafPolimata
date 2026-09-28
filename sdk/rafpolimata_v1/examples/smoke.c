#include "rafpolimata_v1.h"

_Alignas(RAFP_V1_ARENA_ALIGN) static rafp_u8 storage[128];

int main(void) {
    static const rafp_u8 v[] = "123456789";
    if (RAFP_V1_ABI_VERSION != 1u) return 1;
    if (rafp_v1_crc32c(v, 9u, 0u) != 0xe3069283u) return 2;
    if (rafp_v1_q16_mul(65536, 32768) != 32768) return 3;
    if (rafp_v1_q16_iir(65536, 0) != 49152) return 4;
    if (rafp_v1_q16_abs_sat((rafp_q16)0x80000000u) != 0x7fffffff) return 5;

    rafp_arena_v1 a;
    rafp_v1_arena_init(&a, storage, (rafp_u32)sizeof(storage));
    if (!rafp_v1_arena_alloc(&a, 1u)) return 6;
    if (rafp_v1_arena_used(&a) != 8u) return 7;
    if (rafp_v1_arena_alloc(&a, 0xffffffffu) != (void *)0) return 8;
    rafp_v1_arena_reset(&a);
    if (rafp_v1_arena_used(&a) != 0u || rafp_v1_arena_peak(&a) != 8u) return 9;
    return 0;
}
