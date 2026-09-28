/* raf_toroid.h — bounded T^7-inspired Q16 state machine.
 * Eq.1-2 provide coordinate notation; the implemented 42-value field is a
 * deterministic bounded state index, NOT proof of 42 dynamical attractors.
 * Golden-ratio/KAM language is a construction reference, not a theorem about
 * this implementation's stability. Strong convergence claims remain gated by
 * the repository T^7 closure/falsifier.
 * Coordinates: u,v,psi,chi,rho,delta,sigma.                                */
#pragma once
#include "raf_types.h"
#include "raf_q16.h"

typedef struct {
    u32 s[T7_DIM];   /* coordenadas em [0, 65536) = [0,1) em Q16            */
    q16_t H;         /* entropia IIR acumulada                               */
    q16_t C;         /* coerência IIR acumulada                              */
    q16_t phi;       /* phi_ethica = (1-H)*C — Eq.8                         */
    u32  step;       /* contador de passos                                   */
    u32  attractor;  /* historical field name: bounded state index [0..41] */
    /* ── campos de topologia evolutiva (Darwinismo Quântico) ─────────────── */
    u32 phase_acc;    /* Φ: fase acumulada, Φ_{t+1}=Φ_t+Obs_t, nunca reseta */
    u32 delta;        /* Δ=dist circular(atrator, fase%42): incoerência struct */
    u32 omega_inv[3]; /* Ω: I₁=ψ(s,φ) I₂=ψ(1,t) I₃=ψ(2,t) — checksums traj */
    /* ── operadores esquecidos (batch 5): lateral, antiderivada, harmônica ── */
    u32 delta_dir;   /* 1=atrator à frente da fase, 0=atrás — CHAVE DE REVERSÃO */
    u32 perm_class;  /* [7:4]=setor hex S₆ [0..5], [3:0]=nível Fibonacci C₇ [0..6] */
} T7State;

/* Indices das dimensões — sem enumeração, trabalha com matriz de índices    */
/* 0=u 1=v 2=psi 3=chi 4=rho 5=delta 6=sigma                                */

/* Classe de permutação lateral S₆×C₇ — setor hexagonal e nível Fibonacci
 * High nibble: attractor/7 ∈ [0..5] — 6 sectores do hexágono toroidal
 * Low  nibble: attractor%7 ∈ [0..6] — 7 níveis Fibonacci por sector        */
static inline u32 t7_perm_class(u32 attractor) {
    return ((attractor / 7u) << 4) | (attractor % 7u);
}

/* Entrada para o toroide — Eq.4: x=(dados,entropia,hash,estado)             */
typedef struct {
    u32 data_hash;   /* hash dos dados                                       */
    q16_t entropy;   /* entropia milli em Q16                               */
    u32 hw_state;    /* estado do hardware (CPUID flags, etc.)               */
} T7Input;

static void t7_init(T7State *t) {
    u32 seed = 40503U; /* phi^-1 reference seed; no stability claim */
    for (u32 i = 0; i < T7_DIM; i++) t->s[i] = (seed * (i + 1)) & 0xFFFFU;
    t->H = Q16_HALF;
    t->C = Q16_HALF;
    t->phi = q16_phi_ethica(Q16_HALF, Q16_HALF);
    t->step = 0;
    t->attractor = 0;
    t->phase_acc = 0;
    t->delta = 0;
    for (u32 i = 0; i < 3; i++) t->omega_inv[i] = 0;
    t->delta_dir = 0;
    t->perm_class = 0;
}

/* ToroidalMap — Eq.3: s = ToroidalMap(x)
 * Mapa determinístico de input → coordenadas, branch-free                   */
static void t7_map_input(T7State *t, const T7Input *x) {
    /* Deriva 7 coordenadas do input via rotações e XOR — sem hash heap      */
    u32 h = x->data_hash;
    u32 coords[T7_DIM];
    coords[0] = (h                   ) & 0xFFFFU; /* u: bits 0-15           */
    coords[1] = (h >> 16             ) & 0xFFFFU; /* v: bits 16-31          */
    coords[2] = (u32)(x->entropy >> 0) & 0xFFFFU; /* psi                   */
    coords[3] = (u32)(x->entropy >>16) & 0xFFFFU; /* chi                   */
    coords[4] = (x->hw_state         ) & 0xFFFFU; /* rho                   */
    coords[5] = (x->hw_state >> 8    ) & 0xFFFFU; /* delta                 */
    coords[6] = ((h ^ x->hw_state)   ) & 0xFFFFU; /* sigma                 */
    /* IIR update: s[i] = s[i] - s[i]/4 + in[i]/4 — Eq.5 com alpha=0.25  */
    for (u32 i = 0; i < T7_DIM; i++) {
        t->s[i] = (t->s[i] - (t->s[i] >> 2) + (coords[i] >> 2)) & 0xFFFFU;
    }
}

/* Step: aplica spiral decay + update phi_ethica + topologia evolutiva       */
static void t7_step(T7State *t, q16_t H_in, q16_t C_in) {
    /* H e C via IIR alpha=0.25 — Eq.5-6                                    */
    t->H   = q16_iir(t->H, H_in);
    t->C   = q16_iir(t->C, C_in);
    t->phi = q16_phi_ethica(t->H, t->C);
    /* Spiral decay nas dimensões de ruído (rho=4, delta=5)                 */
    t->s[4] = (u32)q16_spiral((q16_t)t->s[4]) & 0xFFFFU;
    t->s[5] = (u32)q16_spiral((q16_t)t->s[5]) & 0xFFFFU;
    /* Psi (intenção=2) cresce com coerência — integração ética              */
    t->s[2] = (t->s[2] + (u32)(t->phi >> 8)) & 0xFFFFU;
    /* Sigma (memória=6): IIR logarítmica — acumulação longa / antiderivada  */
    t->s[6] = (u32)q16_log_iir((q16_t)t->s[6], (q16_t)(t->s[0] ^ t->s[2])) & 0xFFFFu;

    /* ── Topologia evolutiva (Darwinismo Quântico) ───────────────────────── */
    /* ω: phi_ethica → frequência angular [0,6]; ω=6 em máxima coerência   */
    u32 omega = (u32)((u64)(u32)t->phi * 6u >> 16);
    /* u_t: perturbação de entropia IIR [0,6]                               */
    u32 u_t   = ((u32)t->H >> 13) % 7u;
    /* φ(t+1) = (φ_t + ω + u_t) mod 42 — Eq.EVO: atrator evolui, não salta */
    t->attractor = (t->attractor + omega + u_t) % 42u;
    /* phase_acc is an explicit modulo-2^16 accumulator, not unbounded time. */
    t->phase_acc = (t->phase_acc + (u32)(u16)t->H + (u32)(u16)t->C) & 0xFFFFu;
    /* Δ = dist circular(atrator, fase%42) — incoerência estrutural          */
    u32 phi42 = t->phase_acc % 42u;
    /* Direção do Δ: 1=atrator à frente da fase, 0=atrás — CHAVE DE REVERSÃO */
    t->delta_dir = (t->attractor >= phi42) ? 1u : 0u;
    u32 raw_d = (t->attractor > phi42) ? (t->attractor - phi42)
                                        : (phi42 - t->attractor);
    if (raw_d > 21u) raw_d = 42u - raw_d; /* distância circular wrap        */
    t->delta = raw_d;
    /* Colapso: |Δ| > T7_LIMIAR → salto (fase fica, só lente atrator muda)  */
    if (raw_d > T7_LIMIAR) {
        t->attractor = phi42;
        t->delta     = 0u;
        t->delta_dir = 0u;                  /* pós-colapso: sem direção      */
    }
    /* Classe de permutação S₆×C₇ — lateral, pós-colapso                    */
    t->perm_class = t7_perm_class(t->attractor);
    /* Ω-invariantes: checksums XOR da trajetória I₁,I₂,I₃                  */
    t->omega_inv[0] ^= t->s[2] ^ (u32)(u16)t->phi; /* I₁ = ψ(s,φ)         */
    t->omega_inv[1] ^= t->s[1];                      /* I₂ = ψ(1,t) coer.   */
    t->omega_inv[2] ^= t->s[3];                      /* I₃ = ψ(2,t) obs.    */
    t->step++;
}

/* Squared cosine-like coherence against the uniform reference direction:
 * R^2 = (sum(s)^2) / (T7_DIM * sum(s^2)).
 * This form is bounded in [0,1] by Cauchy-Schwarz and avoids the previous
 * overflowing product ns*nk. It is explicitly R^2, not R.                  */
static q16_t t7_coherence_sq(const T7State *t) {
    u64 sum = 0, ns = 0;
    for (u32 i = 0; i < T7_DIM; i++) {
        sum += t->s[i];
        ns += (u64)t->s[i] * t->s[i];
    }
    u64 denom = (u64)T7_DIM * ns;
    if (!denom) return 0;
    u64 numer = sum * sum * (u64)Q16_ONE;
#ifdef RAF_ARCH_A32
    return (q16_t)raf_udiv64_bounded(numer, denom);
#else
    return (q16_t)(numer / denom);
#endif
}
