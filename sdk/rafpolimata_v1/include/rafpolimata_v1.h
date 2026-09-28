#ifndef RAFPOLIMATA_V1_H
#define RAFPOLIMATA_V1_H

/*
 * RafPolimata SDK reference surface v1.
 * Boundary: freestanding C API; no libc, heap, syscall, TLS or hosted runtime.
 * ABI version changes on symbol/signature/layout breakage.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define RAFP_V1_ABI_VERSION 1u
#define RAFP_V1_API_VERSION_MAJOR 0u
#define RAFP_V1_API_VERSION_MINOR 1u
#define RAFP_V1_API_VERSION_PATCH 0u
#define RAFP_V1_ARENA_ALIGN 8u

typedef unsigned char      rafp_u8;
typedef unsigned int       rafp_u32;
typedef unsigned long long rafp_u64;
typedef signed int         rafp_s32;
typedef signed long long   rafp_s64;
typedef __SIZE_TYPE__      rafp_size;
typedef __UINTPTR_TYPE__   rafp_uptr;
typedef rafp_s32           rafp_q16;

typedef struct rafp_arena_v1 {
    rafp_u8  *base;
    rafp_u32 capacity;
    rafp_u32 top;
    rafp_u32 peak;
} rafp_arena_v1;

rafp_u32 rafp_v1_crc32c(const void *data, rafp_size len, rafp_u32 seed);
rafp_q16 rafp_v1_q16_mul(rafp_q16 a, rafp_q16 b);
rafp_q16 rafp_v1_q16_iir(rafp_q16 state, rafp_q16 input);
rafp_q16 rafp_v1_q16_abs_sat(rafp_q16 value);

void      rafp_v1_arena_init(rafp_arena_v1 *arena, void *storage, rafp_u32 capacity);
void     *rafp_v1_arena_alloc(rafp_arena_v1 *arena, rafp_u32 size);
void      rafp_v1_arena_reset(rafp_arena_v1 *arena);
rafp_u32  rafp_v1_arena_used(const rafp_arena_v1 *arena);
rafp_u32  rafp_v1_arena_peak(const rafp_arena_v1 *arena);

#ifdef __cplusplus
}
#endif
#endif
