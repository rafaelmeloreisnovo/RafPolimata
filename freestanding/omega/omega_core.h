#ifndef RAFPOLIMATA_OMEGA_CORE_H
#define RAFPOLIMATA_OMEGA_CORE_H

#if !defined(__UINT32_TYPE__)
#error "compiler must provide __UINT32_TYPE__"
#endif

typedef __UINT32_TYPE__ raf_omega_u32;

enum {
    RAF_OMEGA_DIM = 10,
    RAF_OMEGA_CELLS = RAF_OMEGA_DIM * RAF_OMEGA_DIM * RAF_OMEGA_DIM,
    RAF_OMEGA_AXES = 8,
    RAF_OMEGA_HEADER_WORDS = 16,
    RAF_OMEGA_WORDS = RAF_OMEGA_HEADER_WORDS + RAF_OMEGA_CELLS
};

typedef struct {
    raf_omega_u32 q[RAF_OMEGA_WORDS];
} raf_omega_state;

#if defined(__GNUC__) || defined(__clang__)
#define RAF_OMEGA_API __attribute__((visibility("default")))
#else
#define RAF_OMEGA_API
#endif

RAF_OMEGA_API void raf_omega_init(raf_omega_state *s, raf_omega_u32 seed);
RAF_OMEGA_API void raf_omega_set_friction(raf_omega_state *s, raf_omega_u32 axis, raf_omega_u32 q8);
RAF_OMEGA_API raf_omega_u32 raf_omega_step(raf_omega_state *s, raf_omega_u32 token, raf_omega_u32 axis);
RAF_OMEGA_API raf_omega_u32 raf_omega_digest(const raf_omega_state *s);
RAF_OMEGA_API raf_omega_u32 raf_omega_validate(const raf_omega_state *s);

#endif
