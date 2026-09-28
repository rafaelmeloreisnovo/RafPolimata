/* raf_arena.h — bounded static bump arena: no malloc/free/GC dependency.
 * Performance is runtime-receipt scoped; source text does not assert latency.
 * Allocation fails closed on size/alignment/capacity overflow. */
#pragma once
#include "raf_types.h"

typedef struct {
    _Alignas(ARENA_ALIGN) u8 buf[ARENA_CAP]; /* guaranteed aligned arena base */
    u32 top;            /* cursor — único campo mutável                      */
    u32 peak;           /* high watermark para métricas                      */
} Arena;

/* Allocation safety precedes branch-count optimization. */
static __attribute__((always_inline)) inline
void* arena_alloc(Arena * __restrict__ a, u32 sz) {
    if (!a || sz > ARENA_CAP) return (void*)0;
    if (sz > 0xFFFFFFFFU - (ARENA_ALIGN - 1U)) return (void*)0;
    u32 aligned = (sz + (ARENA_ALIGN - 1U)) & ~(ARENA_ALIGN - 1U);
    if (a->top > ARENA_CAP || aligned > ARENA_CAP - a->top) return (void*)0;
    u32 old = a->top;
    a->top = old + aligned;
    if (a->top > a->peak) a->peak = a->top;
    return (void*)(a->buf + old);
}

/* reset: O(1) — simplesmente zera cursor. Sem free() individual            */
static __attribute__((always_inline)) inline
void arena_reset(Arena *a) {
    a->top = 0;
    /* Não zera buf — dados antigos ficam (performance). Zerar quando seguro:
       __builtin_memset(a->buf, 0, a->peak);  */
}

/* alloc e zera — para structs que precisam de estado inicial limpo          */
static __attribute__((always_inline)) inline
void* arena_calloc(Arena * __restrict__ a, u32 sz) {
    void *p = arena_alloc(a, sz);
    if (p) __builtin_memset(p, 0, sz);
    return p;
}

/* Métricas do arena — sem syscall                                           */
static inline u32 arena_used(const Arena *a) { return a->top; }
static inline u32 arena_free_bytes(const Arena *a) { return ARENA_CAP - a->top; }
static inline u32 arena_peak(const Arena *a) { return a->peak; }

/* Arena global estática — segmento .bss, sem heap, sem mmap                */
static Arena G_ARENA;
#define ALLOC(sz)      arena_alloc(&G_ARENA, (sz))
#define CALLOC(sz)     arena_calloc(&G_ARENA, (sz))
#define ARENA_RESET()  arena_reset(&G_ARENA)
