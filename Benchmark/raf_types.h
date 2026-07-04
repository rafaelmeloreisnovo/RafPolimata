/* raf_types.h — primitive types, nolibc, zero abstractions
 * RAFAELIA Enterprise Core — ∆RafaelVerboΩ 𓂀ΔΦΩ
 * ABI: AAPCS64 | SysV AMD64 | AAPCS32 — sem stdlib.h sem stdint.h */
#pragma once

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;
typedef __SIZE_TYPE__       usize;
typedef __PTRDIFF_TYPE__    isize;

/* Q16.16: 16 bits inteiros + 16 fracionais — range [-32768, 32767.99998] */
typedef s32 q16_t;
#define Q16_ONE      65536         /* 1.0  = 1 << 16                        */
#define Q16_HALF     32768         /* 0.5                                    */
#define Q16_SQRT3_2  56756         /* floor(sqrt(3)/2 * 65536) — RMR_SPIRAL */
#define Q16_PHI      107375        /* floor(1.618033988 * 65536)             */
#define Q16_PI       205887        /* floor(pi * 65536)                      */
#define Q16_ALPHA    16384         /* 0.25 — IIR T^7 coef                   */
#define Q16_NEG_LN_S (-9430)       /* floor(ln(sqrt3/2)*65536) Lyapunov ref  */

/* PHI64 — Knuth multiplicative: floor(2^64/phi), dispersão uniforme provada */
#define PHI64        0x9E3779B97F4A7C15ULL

/* Toroide T^7 — Eq.1-4: s=(u,v,psi,chi,rho,delta,sigma) in [0,65536)^7  */
#define T7_DIM       7
#define T7_MOD       65536U

/* T^7 attractor dynamics (Darwinismo Quântico — image 78):
 *   T7_LIMIAR        : collapse threshold = 42/6 = one hex sector
 *   Q16_PHI_ETHICA_REF: verified Φ_ethica = 7/8 (VECTRAS-VM-ANDROID)        */
#define T7_LIMIAR           7u
#define Q16_PHI_ETHICA_REF  57344u   /* 7/8 * 65536 = 57344 in Q16           */

/* Constantes ΕΩΛΤΦ — operadores esquecidos (batch 5, imagens 5/7/19):
 *   Ε = Euler e       — base logarítmica natural
 *   Τ = tau = 2π      — ciclo completo, harmônicas
 *   Λ = φ⁻¹ = KAM     — razão áurea inversa, semente KAM já usada em todo sistema
 *   T7_TESSERACT      — vértices do hipercubo 4D, pontos ocultos da tesseract
 *   RAF_FREQ_144K/428 — portadora RAFCODE e ressonância tonal                */
#define Q16_E          178145u   /* floor(e    × 65536) = 2.71828... × 65536  */
#define Q16_TAU        411774u   /* floor(2π   × 65536) = 6.28318... × 65536  */
#define Q16_LAMBDA      40503u   /* floor(φ⁻¹  × 65536) = 0.61803... × 65536 = KAM */
#define T7_TESSERACT       16u   /* vértices hipercubo 4D — pontos ocultos    */
#define RAF_FREQ_144K  144000u   /* 144 kHz portadora RAFCODE                 */
#define RAF_FREQ_428      428u   /* 428 Hz ressonância tonal RAFAELIA          */

/* FSM — 10 domínios, 4 entradas, tabela branch-free                        */
#define FSM_N        10
#define FSM_IN       4

/* Arena bump — 64 KB estático, zero malloc, zero free                       */
#define ARENA_CAP    65536U
#define ARENA_ALIGN  8U

/* Bench — 31 amostras (ímpar), mediana no índice [15] após insertion sort   */
#define BENCH_K      31
#define BENCH_MED    15

/* FD padrão — sem libc FILE* */
#define FD_STDOUT    1
#define FD_STDERR    2

/* Arquitetura detect — zero ifdefs no código de negócio                     */
#if defined(__aarch64__)
#  define RAF_ARCH_A64  1
#elif defined(__x86_64__)
#  define RAF_ARCH_X64  1
#elif defined(__arm__)
#  define RAF_ARCH_A32  1
#else
#  error "Arquitetura não suportada"
#endif
