/* raf_q16.h — Q16.16 fixed-point, todas as operações, zero libm zero float
 * Eq.18: r_n = (sqrt3/2)^n — Spiral RAFAELIA via q16_spiral()
 * Eq.22: F[n+1] = F[n]*sqrt3/2 - pi*sin(279°) — sequência Rafael
 * Eq.5-8: IIR T^7 com alpha=0.25 — filtro estável polo em z=sqrt3/2
 * Lyapunov discreto: lambda = ln(sqrt3/2) = -0.1438 nats/step              */
#pragma once
#include "raf_types.h"

/* Multiply Q16 × Q16 → Q16 : (a*b)>>16  — sem overflow se |a|,|b|<32768  */
static __attribute__((always_inline)) inline
q16_t q16_mul(q16_t a, q16_t b) {
    return (q16_t)(((s64)a * b) >> 16);
}

/* Spiral step: multiply by sqrt(3)/2 in Q16 — decaimento geométrico        */
static __attribute__((always_inline)) inline
q16_t q16_spiral(q16_t v) {
    return (q16_t)(((s64)v * Q16_SQRT3_2) >> 16);
}

/* IIR alpha=0.25: s = s - (s>>2) + (in>>2)  — um shift, um add            */
static __attribute__((always_inline)) inline
q16_t q16_iir(q16_t s, q16_t in) {
    return s - (s >> 2) + (in >> 2);
}

/* Phi_ethica = (1-H)*C em Q16 — Eq.8                                       */
static __attribute__((always_inline)) inline
q16_t q16_phi_ethica(q16_t H, q16_t C) {
    return q16_mul(Q16_ONE - H, C);
}

/* Toroid wrap: mod 65536 — single AND, branch-free                          */
static __attribute__((always_inline)) inline
u32 q16_tmod(s32 v) {
    return (u32)v & 0xFFFFU;
}

/* log2 inteiro: posição do bit mais alto — usado no Lyapunov estimator      */
static __attribute__((always_inline)) inline
s32 q16_log2i(u32 v) {
    return v ? (31 - __builtin_clz(v)) : -1;
}

/* Entropia milli — Eq.43: (unique*6000)/256 + (transitions*2000)/(len-1)   */
static inline q16_t q16_entropy_milli(u32 unique, u32 transitions, u32 len) {
    if (len <= 1) return 0;
    u32 a = (unique * 6000) / 256;
    u32 b = (transitions * 2000) / (len - 1);
    /* retorna em miliunidades, escala Q16 = *65536/1000                     */
    return (q16_t)(((u64)(a + b) * 65536) / 1000);
}

/* Sequência Fibonacci-Rafael: F[n+1] = F[n]*sqrt3/2 - pi*sin(279°)
 * sin(279°) = -sin(81°) ≈ -0.9877 → Q16: -64716
 * pi*sin(279°) ≈ -3.1024 → Q16: -203280                                   */
#define Q16_PI_SIN279  (-203280)
static inline q16_t q16_fraf_next(q16_t fn) {
    return q16_spiral(fn) - Q16_PI_SIN279; /* += pi*|sin279| */
}

/* Normaliza Q16 para [0, Q16_ONE) — usado no toroid                        */
static __attribute__((always_inline)) inline
q16_t q16_norm(q16_t v) {
    /* v & 0xFFFF interpreta como fracional [0,1)                            */
    return (q16_t)((u32)v & 0xFFFFU);
}

/* ABS branch-free: mask trick                                               */
static __attribute__((always_inline)) inline
q16_t q16_abs(q16_t v) {
    s32 mask = v >> 31;
    return (v + mask) ^ mask;
}

/* IIR logarítmica — memória longa / antiderivada (batch 5, sigma dim 6)
 * alpha_fast=1/16, alpha_slow=1/128: acumulação lenta, esquecimento lento
 * Aplica-se à dimensão sigma (memória) para integração de longo prazo       */
static __attribute__((always_inline)) inline
q16_t q16_log_iir(q16_t s, q16_t in) {
    return s - (s >> 4) - (s >> 7) + (in >> 4);
}

/* Normaliza ângulo para [0, Q16_TAU) — wrap de ciclo completo harmônico    */
static __attribute__((always_inline)) inline
q16_t q16_tau_wrap(q16_t v) {
    return (q16_t)((u32)v % Q16_TAU);
}
