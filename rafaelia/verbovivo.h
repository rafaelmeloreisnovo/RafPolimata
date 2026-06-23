/* rafaelia/verbovivo.h
 * VERBOVIVO — unified convergence of Fiber-H, Trinity Control, Trinity Core.
 *
 * Three layers, one scan:
 *   1. Fiber-H     : structural hash + Hamming distance (bit-level diversity)
 *   2. Trinity Ctrl: stream metrics, audit report, SVG snapshot
 *   3. Trinity Core: HDC hypervectors + synaptic attention + engram memory
 *
 * Convergence rule: Hamming diversity drives retention.
 *   New chunks are compared to all stored engrams via Hamming distance.
 *   High diversity (new information) boosts attention score, increasing
 *   the probability of retention in the engram ring buffer.
 *
 * Design invariants:
 *   - No heap: all state is caller-allocated.
 *   - Deterministic seed (0x524146 = "RAF").
 *   - Bounded memory: VV_MEM_SIZE engrams, ring buffer.
 *   - Hosted API (vv_scan/vv_audit/vv_svg/verbovivo_main): requires stdio/math.
 *   - Freestanding API (vv_scan_buf/vv_recall/vv_init): compile with
 *     -DVERBOVIVO_NO_HEAP; requires only <stdint.h>, <stddef.h>, <string.h>.
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef VERBOVIVO_NO_HEAP
#include <math.h>
#include <stdio.h>
#include <time.h>
#endif

/* ── sub-layer headers (B4: separated for independent use) ─────────── */
#include "fiber_h.h"       /* FiberHash, fiber_hash_init/update/distance  */
#include "trinity_core.h"  /* VVHyperVec, VVEngram, VVCtrl, TrinityState  */
#include "t7_toroid.h"     /* T7State, T7_DIM, t7_step, t7_hdc_expand     */

typedef uint8_t  vv_u8;
typedef uint16_t vv_u16;
typedef uint32_t vv_u32;
typedef uint64_t vv_u64;
typedef float    vv_f32;
typedef size_t   vv_sz;

/* ── unified VerbVivo state ──────────────────────────────────────── */
typedef struct {
    char       signature[32];
    vv_u32     compliance_flags;
    vv_sz      total_bytes;
    vv_u32     chunk_count;
    vv_u32     mem_head;
    VVEngram   memory[VV_MEM_SIZE];
    VVHyperVec context_vec;     /* running context (bound vectors)    */
    vv_f32     W_proj[VV_DIM];  /* synaptic projection weights        */
    VVCtrl     ctrl;            /* stream audit metrics               */
} VerbVivoState;

/* ── public API ──────────────────────────────────────────────────── */
/* relmat is optional (NULL = disabled, zero cost): pass a FiberRelMat*
 * (see fiber_relmat.h) to additionally accumulate the bit relational
 * matrix while scanning. Opt-in — existing callers passing NULL are
 * unaffected. */
void  vv_init(VerbVivoState *vv);
void  vv_scan_buf(VerbVivoState *vv, const vv_u8 *buf, vv_sz len, void *relmat);
int   vv_hamming_256(const vv_u8 a[32], const vv_u8 b[32]);
int   vv_monobit(const vv_u8 hash[32]);

/* Hosted-only API — requires stdio/math (not available with VERBOVIVO_NO_HEAP) */
#ifndef VERBOVIVO_NO_HEAP
void  vv_scan(VerbVivoState *vv, FILE *stream, void *relmat);
void  vv_audit(const VerbVivoState *vv);
void  vv_svg(const VerbVivoState *vv);
#endif
/* Recupera os top_n engrams mais ressonantes com a query.
 * phi_weight: importância da coerência semântica vs. diversidade de Hamming [0..1].
 *   phi_weight → 1.0 : prioriza similaridade vetorial (cosine) — regime coerente.
 *   phi_weight → 0.0 : prioriza diversidade estrutural (Hamming) — regime caótico.
 * phi_weight recomendado:
 *   Layer 2 (T^7):   leia t7.phi após t7_step(); use (float)t7.phi / 65536.0f
 *   Layer 1 (Fiber-H): use vv->total_bytes ? 0.5f : 0.0f, ou passe phi_fst() resultado
 *   Padrão conservador: 0.5f (pesos iguais entre semântica e estrutura)
 * Retorna: número de engrams preenchidos em out[] (0 se buffer vazio).
 * out[] deve ter capacidade >= top_n.
 * Sem malloc — usa stack interno de VV_MEM_SIZE=64 scores. */
int   vv_recall(const VerbVivoState *vv,
                const VVHyperVec    *query_vec,
                const FiberHash     *query_hash,
                vv_f32               phi_weight,
                int                  top_n,
                VVEngram            *out);
