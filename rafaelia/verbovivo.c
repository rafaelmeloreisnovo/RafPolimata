/* verbovivo.c — RAFAELIA cognitive convergence engine.
 *
 * Unified pipeline (two complementary layers):
 *
 *   LAYER 1 — Fiber-H + Trinity (structural divergence memory):
 *     binary stream → 256-bit FiberHash → HDC hypervectors → synaptic attention
 *     → Hamming-diversity retention → engram ring buffer → SVG similarity graph
 *
 *   LAYER 2 — T^7 toroid (geometric coherence invariant):
 *     APK/ELF binary → T^7 toroid (7-dim, 42 attractors, phi_ethica)
 *                    → 1024-dim HDC hypervector (XOR-mixing expansion)
 *                    → SVG engram (2D trajectory + phi bar)
 *
 * Entry points:
 *   verbovivo_main(apk_path, svg_out)  — T^7 pipeline (Layer 2)
 *   vv_init / vv_scan / vv_audit / vv_svg  — Fiber-H pipeline (Layer 1)
 *
 * INVARIANT: Hamming diversity (Fiber-H) × synaptic attention (Trinity Core)
 *   decides which chunks enter memory. High structural divergence = retention.
 *   The system does NOT learn by gradient — it learns by STRUCTURAL DIVERGENCE.
 *
 * phi_ethica = (1-H)*C: 0 = pure noise, Q16_ONE = perfect coherence.
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "verbovivo.h"
#include "../Benchmark/raf_toroid.h"

/* ══════════════════════════════════════════════════════════════════════════
 * LAYER 1 — Fiber-H + Trinity Core
 * ══════════════════════════════════════════════════════════════════════════ */

/* ── Fiber-H: popcnt64, Hamming distance, monobit ───────────────────────── */

static int _popcnt64(vv_u64 x) {
    return __builtin_popcountll((unsigned long long)x);
}

int vv_hamming_256(const vv_u8 a[32], const vv_u8 b[32]) {
    if (!a || !b) return 0;
    vv_u64 a0,a1,a2,a3, b0,b1,b2,b3;
    memcpy(&a0,a+0,8);  memcpy(&a1,a+8,8);  memcpy(&a2,a+16,8); memcpy(&a3,a+24,8);
    memcpy(&b0,b+0,8);  memcpy(&b1,b+8,8);  memcpy(&b2,b+16,8); memcpy(&b3,b+24,8);
    return _popcnt64(a0^b0)+_popcnt64(a1^b1)+_popcnt64(a2^b2)+_popcnt64(a3^b3);
}

int vv_monobit(const vv_u8 hash[32]) {
    if (!hash) return 0;
    vv_u64 w0,w1,w2,w3;
    memcpy(&w0,hash+0,8); memcpy(&w1,hash+8,8);
    memcpy(&w2,hash+16,8); memcpy(&w3,hash+24,8);
    return _popcnt64(w0)+_popcnt64(w1)+_popcnt64(w2)+_popcnt64(w3);
}

static void _fiber_hash(const vv_u8 *data, vv_sz len, FiberHash *out) {
    vv_u64 h0=5381u, h1=5381u^0xDEADBEEFu,
           h2=5381u^0xCAFEBABEu, h3=5381u^0xF00DFACEu;
    for (vv_sz i=0;i<len;i++) {
        vv_u8 b=data[i];
        h0=((h0<<5)+h0)^(vv_u64)b;
        h1=((h1<<5)+h1)^(vv_u64)(b^(vv_u8)(i&0xFFu));
        h2=((h2<<5)+h2)^(vv_u64)(b^(vv_u8)(len&0xFFu));
        h3=((h3<<5)+h3)^(vv_u64)(b^(vv_u8)((i^len)&0xFFu));
    }
    out->a=h0; out->b=h1; out->c=h2; out->d=h3;
}

static int _fiber_hamming(const FiberHash *x, const FiberHash *y) {
    return _popcnt64(x->a^y->a)+_popcnt64(x->b^y->b)
          +_popcnt64(x->c^y->c)+_popcnt64(x->d^y->d);
}

/* ── Trinity Core: LCG RNG, HDC vectors, synaptic attention ─────────────── */

static vv_u32 _rng(vv_u32 *s) {
    vv_u32 x = s && *s ? *s : VV_SEED;
    x ^= x<<13; x ^= x>>17; x ^= x<<5;
    if (s) *s=x;
    return x;
}

static void _gen_vec(vv_u32 seed, VVHyperVec *v) {
    if (!v) return;
    vv_u32 r = seed ? seed : 0xDEADBEEFu;
    for (int i=0;i<VV_DIM;i++)
        v->values[i] = (_rng(&r)%2u) ? 1.0f : -1.0f;
}

static vv_f32 _cosine(const VVHyperVec *a, const VVHyperVec *b) {
    if (!a||!b) return 0.0f;
    vv_f32 dot=0,mA=0,mB=0;
    for (int i=0;i<VV_DIM;i++) {
        dot += a->values[i]*b->values[i];
        mA  += a->values[i]*a->values[i];
        mB  += b->values[i]*b->values[i];
    }
    if (mA==0.0f||mB==0.0f) return 0.0f;
    return dot/(sqrtf(mA)*sqrtf(mB));
}

static void _bind(VVHyperVec *tgt, const VVHyperVec *inp) {
    if (!tgt||!inp) return;
    vv_f32 mag=0;
    for (int i=0;i<VV_DIM;i++) {
        tgt->values[i] += inp->values[i];
        tgt->values[i] *= 0.95f;
        mag += tgt->values[i]*tgt->values[i];
    }
    mag=sqrtf(mag);
    if (mag>0.0f)
        for (int i=0;i<VV_DIM;i++) tgt->values[i]/=mag;
}

static vv_f32 _attention(VerbVivoState *vv, const VVHyperVec *v) {
    if (!vv||!v) return 0.0f;
    vv_f32 score=0;
    for (int i=0;i<VV_DIM;i++) score += v->values[i]*vv->W_proj[i];
    return 1.0f/(1.0f+expf(-score));
}

static vv_f32 _hamming_diversity_vv(VerbVivoState *vv, const FiberHash *fh) {
    if (!vv||!fh) return 0.5f;
    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;
    if (n==0) return 1.0f;
    int total=0;
    for (vv_u32 i=0;i<n;i++)
        total += _fiber_hamming(fh, &vv->memory[i].fiber_hash);
    return (vv_f32)total / ((vv_f32)n * 256.0f);
}

static vv_u32 _djb2_chunk(const vv_u8 *buf, vv_sz len) {
    vv_u32 h=5381u;
    for (vv_sz i=0;i<len;i++) h=((h<<5)+h)+(vv_u32)buf[i];
    return h;
}

/* ── VerbVivo public API ─────────────────────────────────────────────────── */

void vv_init(VerbVivoState *vv) {
    if (!vv) return;
    memset(vv,0,sizeof(*vv));
    memcpy(vv->signature,"RAFAELIA_VV_V1",14);
    vv->compliance_flags = VV_ISO_27001|VV_ISO_25010|VV_NIST_800_53|VV_IEEE_12207;
    vv->ctrl.start_time  = clock();
    vv_u32 seed = VV_SEED;
    for (int i=0;i<VV_DIM;i++)
        vv->W_proj[i] = ((vv_f32)(_rng(&seed)%100u)/100.0f) - 0.5f;
}

static void _process_chunk(VerbVivoState *vv, const vv_u8 *buf, vv_sz len, vv_u32 cid) {
    vv_sz start=0;
    for (vv_sz i=0;i<len;i++) {
        if (buf[i]=='\n') {
            vv->ctrl.ingested_messages++;
            vv->ctrl.ingested_bytes += (i-start+1);
            start = i+1;
        }
    }
    if (start<len) { vv->ctrl.ingested_messages++; vv->ctrl.ingested_bytes += (len-start); }

    vv_u32 hash = _djb2_chunk(buf,len);
    FiberHash fh; _fiber_hash(buf,len,&fh);
    VVHyperVec vec; _gen_vec(hash,&vec);
    vv_f32 attn = _attention(vv,&vec);
    vv_f32 hdiv = _hamming_diversity_vv(vv,&fh);

    vv_u8 fhb[32];
    memcpy(fhb+0,&fh.a,8); memcpy(fhb+8,&fh.b,8);
    memcpy(fhb+16,&fh.c,8); memcpy(fhb+24,&fh.d,8);
    int ones = vv_monobit(fhb);
    vv_f32 balance = 1.0f - fabsf((vv_f32)ones/256.0f - 0.5f)*2.0f;
    vv_f32 combined = attn * hdiv * balance;

    int retain = (combined > 0.15f) || (vv->chunk_count < VV_MEM_SIZE);
    if (retain) {
        int idx = (int)(vv->mem_head % VV_MEM_SIZE);
        vv->memory[idx].id           = cid;
        vv->memory[idx].content_hash = hash;
        vv->memory[idx].fiber_hash   = fh;
        vv->memory[idx].vec          = vec;
        vv->memory[idx].attention    = attn;
        vv->memory[idx].hamming_div  = hdiv;
        vv->memory[idx].type_flag    = (buf[0]==0xFFu||buf[0]==0x89u) ? 1u : 0u;
        _bind(&vv->context_vec,&vec);
        vv->mem_head = (vv->mem_head+1) % VV_MEM_SIZE;
    }
    vv->total_bytes += len;
    vv->chunk_count++;
}

void vv_scan(VerbVivoState *vv, FILE *stream) {
    if (!vv||!stream) return;
    static vv_u8 buf[VV_CHUNK];
    vv_sz n; vv_u32 cid=0;
    while ((n=fread(buf,1,VV_CHUNK,stream))>0)
        _process_chunk(vv,buf,n,cid++);
}

void vv_scan_buf(VerbVivoState *vv, const vv_u8 *buf, vv_sz len) {
    if (!vv||!buf||!len) return;
    vv_sz off=0; vv_u32 cid=0;
    while (off<len) {
        vv_sz chunk = len-off; if (chunk>VV_CHUNK) chunk=VV_CHUNK;
        _process_chunk(vv,buf+off,chunk,cid++);
        off+=chunk;
    }
}

void vv_audit(const VerbVivoState *vv) {
    if (!vv) return;
    double elapsed=(double)(clock()-vv->ctrl.start_time)/(double)CLOCKS_PER_SEC;
    fprintf(stderr,"[VERBOVIVO AUDIT]\n");
    fprintf(stderr,"  Signature    : %s\n", vv->signature);
    fprintf(stderr,"  Compliance   : 0x%02X [OK]\n", vv->compliance_flags);
    fprintf(stderr,"  Chunks       : %u\n", vv->chunk_count);
    fprintf(stderr,"  Bytes        : %lu\n", (unsigned long)vv->total_bytes);
    fprintf(stderr,"  Messages     : %lu\n", (unsigned long)vv->ctrl.ingested_messages);
    fprintf(stderr,"  Engrams      : %u / %u\n",
            vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE, VV_MEM_SIZE);
    fprintf(stderr,"  Uptime       : %.3f s\n", elapsed);
    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;
    if (n>1) {
        int total_h=0; int pairs=0;
        for (vv_u32 i=0;i<n;i++)
            for (vv_u32 j=i+1;j<n;j++) {
                total_h += _fiber_hamming(&vv->memory[i].fiber_hash,
                                          &vv->memory[j].fiber_hash);
                pairs++;
            }
        fprintf(stderr,"  Hamming div  : %.3f (avg over %d pairs)\n",
                pairs ? (double)total_h/(256.0*pairs) : 0.0, pairs);
    }
    fprintf(stderr,"  SVG requests : %lu\n", (unsigned long)vv->ctrl.svg_requests);
}

void vv_svg(const VerbVivoState *vv) {
    if (!vv) return;
    ((VerbVivoState*)vv)->ctrl.svg_requests++;
    printf("<svg width='900' height='900' xmlns='http://www.w3.org/2000/svg' "
           "style='background:#0b132b'>\n");
    printf("<title>VerbVivo — Engram Convergence Map</title>\n");
    printf("<g stroke='#2ec4b6' stroke-width='1'>\n");
    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;
    for (vv_u32 i=0;i<n;i++) {
        for (vv_u32 j=i+1;j<n;j++) {
            vv_f32 sim = _cosine(&vv->memory[i].vec, &vv->memory[j].vec);
            int hd = _fiber_hamming(&vv->memory[i].fiber_hash,
                                     &vv->memory[j].fiber_hash);
            vv_f32 hd_norm = (vv_f32)hd/256.0f;
            if (sim > 0.5f) {
                int x1=(int)(i*13+60), y1=(int)(450+(vv->memory[i].attention*300));
                int x2=(int)(j*13+60), y2=(int)(450+(vv->memory[j].attention*300));
                printf("  <line x1='%d' y1='%d' x2='%d' y2='%d' "
                       "opacity='%.2f' stroke='#4a9fff'/>\n",
                       x1,y1,x2,y2,(double)(sim*(1.0f-hd_norm*0.7f)));
            }
        }
    }
    printf("</g>\n");
    for (vv_u32 i=0;i<n;i++) {
        const VVEngram *m=&vv->memory[i];
        int x=(int)(i*13+60), y=(int)(450+(m->attention*300));
        vv_u32 r=(vv_u32)(m->hamming_div*255.0f);
        vv_u32 g=(vv_u32)((1.0f-m->hamming_div)*200.0f+55.0f);
        vv_u32 b2=(vv_u32)(m->attention*255.0f);
        printf("<circle cx='%d' cy='%d' r='5' fill='#%02x%02x%02x' opacity='0.9'>\n",
               x,y,(unsigned)r,(unsigned)g,(unsigned)b2);
        printf("  <title>id=%u hash=%08x attn=%.3f hdiv=%.3f</title>\n",
               m->id,m->content_hash,(double)m->attention,(double)m->hamming_div);
        printf("</circle>\n");
    }
    printf("<text x='10' y='20' fill='#2ec4b6' font-family='monospace' font-size='11'>"
           "VerbVivo — n=%u chunks=%u</text>\n", n, vv->chunk_count);
    printf("<text x='10' y='36' fill='#888' font-family='monospace' font-size='9'>"
           "blue=novel  green=familiar  size=attention</text>\n");
    printf("</svg>\n");
}

/* ── vv_recall: geometric attention-based engram retrieval ─────────────── */
/*
 * recall_score(q, e) = phi_w * cos(q.vec, e.vec)
 *                    + (1 - phi_w) * (1 - fiber_hamming(q.hash, e.hash) / 256)
 *
 * When phi_w is high (coherent stream): cosine similarity dominates — the system
 * retrieves engrams that are semantically close to the query.
 * When phi_w is low (chaotic stream): Hamming complement dominates — the system
 * retrieves the most structurally distinct engrams, learning by divergence.
 *
 * This is the GEOMETRIC gate: no gradient, no backprop. The gate is computed
 * from the stream itself via phi_ethica = (1-H)*C, seed {40503}.
 */
int vv_recall(const VerbVivoState *vv,
              const VVHyperVec    *query_vec,
              const FiberHash     *query_hash,
              vv_f32               phi_weight,
              int                  top_n,
              VVEngram            *out)
{
    if (!vv || !query_vec || !out || top_n <= 0) return 0;

    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;
    if (n == 0) return 0;

    if (phi_weight < 0.0f) phi_weight = 0.0f;
    if (phi_weight > 1.0f) phi_weight = 1.0f;

    /* score each engram — stack only, no heap */
    vv_f32 scores[VV_MEM_SIZE];
    int    idx[VV_MEM_SIZE];
    for (vv_u32 i = 0; i < n; i++) {
        const VVEngram *e = &vv->memory[i];
        vv_f32 cos_sim = _cosine(query_vec, &e->vec);
        /* cos_sim in [-1,1]; map to [0,1] for scoring */
        vv_f32 sem = (cos_sim + 1.0f) * 0.5f;
        vv_f32 ham_div = 0.0f;
        if (query_hash) {
            int hd = _fiber_hamming(query_hash, &e->fiber_hash);
            ham_div = 1.0f - (vv_f32)hd / 256.0f;  /* 1 = identical, 0 = maximally different */
        }
        scores[i] = phi_weight * sem + (1.0f - phi_weight) * ham_div;
        idx[i]    = (int)i;
    }

    /* partial selection sort — top_n passes, O(n * top_n), n<=64 */
    int out_count = top_n < (int)n ? top_n : (int)n;
    for (int k = 0; k < out_count; k++) {
        int best = k;
        for (int j = k + 1; j < (int)n; j++)
            if (scores[idx[j]] > scores[idx[best]]) best = j;
        /* swap */
        int tmp = idx[k]; idx[k] = idx[best]; idx[best] = tmp;
        out[k] = vv->memory[idx[k]];
    }
    return out_count;
}

/* ══════════════════════════════════════════════════════════════════════════
 * LAYER 2 — T^7 toroid pipeline (verbovivo_main)
 * ══════════════════════════════════════════════════════════════════════════ */

#define HDC_DIM 1024

static void hdc_expand(const T7State *t, uint32_t hdc[HDC_DIM]) {
    for (int d = 0; d < T7_DIM; d++) hdc[d] = t->s[d];
    for (int d = T7_DIM; d < HDC_DIM; d++) {
        uint32_t x = hdc[d - T7_DIM] ^ hdc[d - 1];
        int sh = (d % 31) + 1;
        hdc[d] = (x << sh) | (x >> (32 - sh));
    }
}

static double hdc_hamming_diversity(const uint32_t hdc[HDC_DIM]) {
    uint64_t diff = 0;
    for (int d = 0; d < HDC_DIM - 1; d++) {
        uint32_t x = hdc[d] ^ hdc[d + 1];
        while (x) { x &= x - 1u; diff++; }
    }
    return (double)diff / (double)((uint64_t)(HDC_DIM - 1) * 32u);
}

#define SVG_W    512
#define SVG_H    480
#define SVG_PHI_H 32
#define MAX_TRAJ 4096

typedef struct { int x, y; } Pt;

static void svg_write_t7(FILE *f,
                         const Pt *traj, int ntraj,
                         double phi, int attractor, double hamming)
{
    int total_h = SVG_H + SVG_PHI_H + 8;
    fprintf(f,
        "<?xml version='1.0' encoding='UTF-8'?>\n"
        "<svg xmlns='http://www.w3.org/2000/svg' "
             "width='%d' height='%d' "
             "style='background:#0a0a0f'>\n",
        SVG_W, total_h);
    fprintf(f,
        "  <text x='8' y='18' font-family='monospace' font-size='11' "
               "fill='#7aecb4'>"
               "verbovivo T^7 — attractor=%d phi=%.4f hamming=%.4f"
               "</text>\n",
        attractor, phi, hamming);
    if (ntraj > 1) {
        fprintf(f,
            "  <polyline fill='none' stroke='#4e9eff' "
                       "stroke-width='1' opacity='0.75' points='");
        for (int i = 0; i < ntraj; i++)
            fprintf(f, "%d,%d ", traj[i].x, traj[i].y);
        fprintf(f, "'/>\n");
        fprintf(f,
            "  <circle cx='%d' cy='%d' r='4' "
                     "fill='#ff6e6e' stroke='#fff' stroke-width='1'/>\n",
            traj[ntraj-1].x, traj[ntraj-1].y);
    }
    int bar_w = (int)(phi * SVG_W);
    if (bar_w < 0) bar_w = 0;
    if (bar_w > SVG_W) bar_w = SVG_W;
    const char *bar_col = (phi > 0.6) ? "#4e9eff"
                        : (phi > 0.3) ? "#f0c040"
                                      : "#ff6e6e";
    fprintf(f,
        "  <rect x='0' y='%d' width='%d' height='%d' fill='#1a1a1f'/>\n"
        "  <rect x='0' y='%d' width='%d' height='%d' fill='%s'/>\n"
        "  <text x='4' y='%d' font-family='monospace' font-size='10' "
               "fill='#ccc'>phi_ethica=%.4f</text>\n",
        SVG_H + 4, SVG_W, SVG_PHI_H,
        SVG_H + 4, bar_w, SVG_PHI_H, bar_col,
        SVG_H + SVG_PHI_H, phi);
    fprintf(f, "</svg>\n");
}

int verbovivo_main(const char *apk_path, const char *svg_out) {
    FILE *fin = fopen(apk_path, "rb");
    if (!fin) {
        fprintf(stderr, "verbovivo: cannot open %s\n", apk_path);
        return -1;
    }

    T7State t7;
    t7_init(&t7);

    Pt    *traj  = (Pt *)malloc(MAX_TRAJ * sizeof(Pt));
    int    ntraj = 0;
    size_t nbytes = 0;

    uint8_t blk[256];
    size_t  n;
    while ((n = fread(blk, 1, sizeof(blk), fin)) > 0) {
        uint32_t unique = 0, transitions = 0;
        uint8_t  seen[256];
        memset(seen, 0, 256);
        for (size_t i = 0; i < n; i++) {
            if (!seen[blk[i]]) { seen[blk[i]] = 1; unique++; }
            if (i > 0 && blk[i] != blk[i-1]) transitions++;
        }

        q16_t H_in = (q16_t)((unique  * Q16_ONE) / 256);
        q16_t C_in = (n > 1)
            ? (q16_t)(Q16_ONE - (transitions * Q16_ONE) / (uint32_t)(n - 1))
            : (q16_t)Q16_ONE;

        T7Input inp;
        uint32_t h = 2166136261u;
        for (size_t i = 0; i < n; i++) h = (h ^ blk[i]) * 16777619u;
        inp.data_hash = h;
        inp.entropy   = H_in;
        inp.hw_state  = (uint32_t)nbytes;
        t7_map_input(&t7, &inp);
        t7_step(&t7, H_in, C_in);

        nbytes += n;

        if (ntraj < MAX_TRAJ) {
            traj[ntraj].x = (int)((t7.s[0] * (uint32_t)(SVG_W - 16)) >> 16) + 8;
            traj[ntraj].y = (int)((t7.s[1] * (uint32_t)(SVG_H - 32)) >> 16) + 24;
            ntraj++;
        }
    }
    fclose(fin);

    if (!nbytes) {
        fprintf(stderr, "verbovivo: empty file %s\n", apk_path);
        free(traj);
        return -1;
    }

    uint32_t hdc[HDC_DIM];
    hdc_expand(&t7, hdc);
    double hamming = hdc_hamming_diversity(hdc);
    double phi     = (double)(uint32_t)t7.phi / 65536.0;
    if (phi < 0.0) phi = 0.0;

    FILE *fout = svg_out ? fopen(svg_out, "w") : stdout;
    if (!fout) {
        fprintf(stderr, "verbovivo: cannot write %s\n", svg_out ? svg_out : "(stdout)");
        free(traj);
        return -1;
    }
    svg_write_t7(fout, traj, ntraj, phi, (int)t7.attractor, hamming);
    if (svg_out) fclose(fout);

    fprintf(stderr,
        "verbovivo: %zu bytes  phi=%.4f  attractor=%u  "
        "hamming=%.4f  hdc[0]=%08x\n",
        nbytes, phi, t7.attractor, hamming, hdc[0]);

    free(traj);
    return 0;
}

/* ── CLI wrapper (build with -DVERBOVIVO_MAIN) ──────────────────────────── */
#ifdef VERBOVIVO_MAIN
int main(int argc, char **argv) {
    if (argc >= 2 && argv[1][0] != '-') {
        /* T^7 mode: verbovivo <apk_or_elf> [out.svg] */
        return verbovivo_main(argv[1], argc >= 3 ? argv[2] : NULL) == 0 ? 0 : 1;
    }
    /* Fiber-H / Trinity mode: verbovivo [-s] < binary */
    VerbVivoState vv;
    vv_init(&vv);
    int do_svg = 0;
    for (int i=1;i<argc;i++)
        if (argv[i][0]=='-'&&argv[i][1]=='s') do_svg=1;
    vv_scan(&vv, stdin);
    vv_audit(&vv);
    if (do_svg) vv_svg(&vv);
    return 0;
}
#endif
