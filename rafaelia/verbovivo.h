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
 *   - Freestanding-compatible: only <string.h>, <math.h>, <stdio.h>.
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

typedef uint8_t  vv_u8;
typedef uint16_t vv_u16;
typedef uint32_t vv_u32;
typedef uint64_t vv_u64;
typedef float    vv_f32;
typedef size_t   vv_sz;

/* ── dimensions ────────────────────────────────────────────────────── */
#define VV_DIM       1024   /* HDC hypervector dimensionality            */
#define VV_MEM_SIZE  64     /* max engrams in ring buffer                */
#define VV_CHUNK     4096   /* stream chunk size                         */
#define VV_SEED      0x524146u  /* "RAF"                                 */

/* ── compliance flags (from trinity_core.c) ──────────────────────── */
#define VV_ISO_27001    0x01u
#define VV_ISO_25010    0x02u
#define VV_NIST_800_53  0x04u
#define VV_IEEE_12207   0x08u

/* ── Fiber-H 256-bit hash state ───────────────────────────────────── */
typedef struct {
    vv_u64 a, b, c, d;   /* 4 × 64-bit = 256 bits */
} FiberHash;

/* ── HDC hypervector ──────────────────────────────────────────────── */
typedef struct {
    vv_f32 values[VV_DIM];
} VVHyperVec;

/* ── engram: one stored memory unit ──────────────────────────────── */
typedef struct {
    vv_u32    id;
    vv_u32    content_hash;   /* djb2 hash of chunk                   */
    FiberHash fiber_hash;     /* structural 256-bit fingerprint        */
    VVHyperVec vec;           /* HDC vector for this chunk            */
    vv_f32    attention;      /* combined attention score [0,1]       */
    vv_f32    hamming_div;    /* Hamming diversity vs existing engrams */
    vv_u8     type_flag;      /* 1=binary/image, 0=text               */
} VVEngram;

/* ── Trinity Control metrics ─────────────────────────────────────── */
typedef struct {
    vv_sz  ingested_messages;
    vv_sz  ingested_bytes;
    vv_sz  svg_requests;
    clock_t start_time;
} VVCtrl;

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
void  vv_init(VerbVivoState *vv);
void  vv_scan(VerbVivoState *vv, FILE *stream);
void  vv_scan_buf(VerbVivoState *vv, const vv_u8 *buf, vv_sz len);
void  vv_audit(const VerbVivoState *vv);
void  vv_svg(const VerbVivoState *vv);
int   vv_hamming_256(const vv_u8 a[32], const vv_u8 b[32]);
int   vv_monobit(const vv_u8 hash[32]);

/* Recupera os top_n engrams mais ressonantes com a query.
 * phi_weight: importância da coerência semântica vs. diversidade de Hamming [0..1].
 *   phi_weight → 1.0 : prioriza similaridade vetorial (cosine) — regime coerente.
 *   phi_weight → 0.0 : prioriza diversidade estrutural (Hamming) — regime caótico.
 * Recomendado: phi_weight = (vv_recall phi from T^7 pipeline) / 65536.0
 * Retorna: número de engrams preenchidos em out[] (0 se buffer vazio).
 * out[] deve ter capacidade >= top_n.
 * Sem malloc — usa stack interno de VV_MEM_SIZE=64 scores. */
int   vv_recall(const VerbVivoState *vv,
                const VVHyperVec    *query_vec,
                const FiberHash     *query_hash,
                vv_f32               phi_weight,
                int                  top_n,
                VVEngram            *out);
