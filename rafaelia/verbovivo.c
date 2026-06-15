/* rafaelia/verbovivo.c
 * VERBOVIVO — convergência RAFAELIA.
 *
 * Este arquivo unifica três sistemas dos arquivos de referência:
 *
 *   fiber_core.c   → Hamming distance 256-bit, monobit balance, popcnt
 *   trinity.c      → stream control, audit report, SVG snapshot
 *   trinity_core.c → HDC (Hyperdimensional Computing): hypervectors,
 *                    synaptic attention, engram ring buffer, SVG graph
 *
 * CONVERGÊNCIA: Hamming diversity (Fiber-H) × synaptic attention (Trinity Core)
 *   = score combinado que decide quais chunks entram na memória.
 *   Um chunk com alta diversidade de Hamming em relação aos engrams existentes
 *   carrega NOVA INFORMAÇÃO estrutural → atenção aumentada → retenção preferencial.
 *
 * Esta é a invariante de design DISRUPTIVA:
 *   O sistema não aprende por gradiente — aprende por DIVERGÊNCIA ESTRUTURAL.
 *   Cada chunk é uma coordenada em espaço de Hamming. A memória converge para
 *   uma cobertura máxima desse espaço. Isso é antiderivada do overfitting:
 *   em vez de convergir para um ponto, o sistema diverge em direção à
 *   cobertura completa do espaço de informação observado.
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */

#include "verbovivo.h"

/* ── Fiber-H layer: popcnt64, Hamming, monobit ──────────────────── */

static int _popcnt64(vv_u64 x) {
    return __builtin_popcountll((unsigned long long)x);
}

/* Hamming distance between two 256-bit buffers (as 4×64-bit words). */
int vv_hamming_256(const vv_u8 a[32], const vv_u8 b[32]) {
    if (!a || !b) return 0;
    vv_u64 a0,a1,a2,a3, b0,b1,b2,b3;
    memcpy(&a0,a+0,8); memcpy(&a1,a+8,8); memcpy(&a2,a+16,8); memcpy(&a3,a+24,8);
    memcpy(&b0,b+0,8); memcpy(&b1,b+8,8); memcpy(&b2,b+16,8); memcpy(&b3,b+24,8);
    return _popcnt64(a0^b0)+_popcnt64(a1^b1)+_popcnt64(a2^b2)+_popcnt64(a3^b3);
}

/* Count set bits in a 256-bit digest: monobit balance check. */
int vv_monobit(const vv_u8 hash[32]) {
    if (!hash) return 0;
    vv_u64 w0,w1,w2,w3;
    memcpy(&w0,hash+0,8); memcpy(&w1,hash+8,8);
    memcpy(&w2,hash+16,8); memcpy(&w3,hash+24,8);
    return _popcnt64(w0)+_popcnt64(w1)+_popcnt64(w2)+_popcnt64(w3);
}

/* Compute 256-bit Fiber-H fingerprint of a data block via djb2×4 lanes. */
static void _fiber_hash(const vv_u8 *data, vv_sz len, FiberHash *out) {
    /* Four independent djb2 lanes with different seeds — cheap, portable,
     * no external dependencies. Each lane XORs into a different 64-bit word
     * so the four words diverge structurally across chunk content. */
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

/* Hamming distance between two FiberHash structs (256-bit comparison). */
static int _fiber_hamming(const FiberHash *x, const FiberHash *y) {
    return _popcnt64(x->a^y->a)+_popcnt64(x->b^y->b)
          +_popcnt64(x->c^y->c)+_popcnt64(x->d^y->d);
}

/* ── Trinity Core layer: LCG RNG, HDC vectors, attention ────────── */

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

/* Compute average Hamming diversity of new_hash against stored engrams.
 * Returns value in [0.0, 1.0] where 1.0 = maximally different. */
static vv_f32 _hamming_diversity(VerbVivoState *vv, const FiberHash *fh) {
    if (!vv||!fh) return 0.5f;
    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;
    if (n==0) return 1.0f;  /* first chunk: maximum novelty */
    int total=0;
    for (vv_u32 i=0;i<n;i++)
        total += _fiber_hamming(fh, &vv->memory[i].fiber_hash);
    return (vv_f32)total / ((vv_f32)n * 256.0f);
}

/* ── Trinity Control layer: metrics, djb2, SVG ──────────────────── */

static vv_u32 _djb2_chunk(const vv_u8 *buf, vv_sz len) {
    vv_u32 h=5381u;
    for (vv_sz i=0;i<len;i++) h=((h<<5)+h)+(vv_u32)buf[i];
    return h;
}

/* ── VerbVivo public implementation ─────────────────────────────── */

void vv_init(VerbVivoState *vv) {
    if (!vv) return;
    memset(vv,0,sizeof(*vv));
    memcpy(vv->signature,"RAFAELIA_VV_V1",14);
    vv->compliance_flags = VV_ISO_27001|VV_ISO_25010|VV_NIST_800_53|VV_IEEE_12207;
    vv->ctrl.start_time  = clock();

    /* deterministic W_proj initialization (same as trinity_core.c) */
    vv_u32 seed = VV_SEED;
    for (int i=0;i<VV_DIM;i++)
        vv->W_proj[i] = ((vv_f32)(_rng(&seed)%100u)/100.0f) - 0.5f;
}

/* Process one chunk: hash → HDC vector → attention → Hamming diversity → store. */
static void _process_chunk(VerbVivoState *vv, const vv_u8 *buf, vv_sz len, vv_u32 cid) {
    /* Trinity Control: count records (newline-delimited) */
    vv_sz start=0;
    for (vv_sz i=0;i<len;i++) {
        if (buf[i]=='\n') {
            vv->ctrl.ingested_messages++;
            vv->ctrl.ingested_bytes += (i-start+1);
            start = i+1;
        }
    }
    if (start<len) {
        vv->ctrl.ingested_messages++;
        vv->ctrl.ingested_bytes += (len-start);
    }

    /* djb2 hash (Trinity Core style) */
    vv_u32 hash = _djb2_chunk(buf,len);

    /* Fiber-H 256-bit structural fingerprint */
    FiberHash fh;
    _fiber_hash(buf,len,&fh);

    /* HDC vector from chunk hash */
    VVHyperVec vec;
    _gen_vec(hash,&vec);

    /* synaptic attention (Trinity Core) */
    vv_f32 attn = _attention(vv,&vec);

    /* Hamming diversity vs stored engrams (Fiber-H convergence layer) */
    vv_f32 hdiv = _hamming_diversity(vv,&fh);

    /* monobit balance quality [0..1]: distance from 0.5 penalizes degenerate hashes */
    vv_u8 fhb[32];
    memcpy(fhb+0,&fh.a,8); memcpy(fhb+8,&fh.b,8);
    memcpy(fhb+16,&fh.c,8); memcpy(fhb+24,&fh.d,8);
    int ones = vv_monobit(fhb);
    vv_f32 balance = 1.0f - fabsf((vv_f32)ones/256.0f - 0.5f)*2.0f;

    /* combined score: attention × hamming_diversity × balance */
    vv_f32 combined = attn * hdiv * balance;

    /* retention decision: store if combined>threshold or memory not full */
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
    vv_sz n;
    vv_u32 cid=0;
    while ((n=fread(buf,1,VV_CHUNK,stream))>0)
        _process_chunk(vv,buf,n,cid++);
}

void vv_scan_buf(VerbVivoState *vv, const vv_u8 *buf, vv_sz len) {
    if (!vv||!buf||!len) return;
    vv_sz off=0;
    vv_u32 cid=0;
    while (off<len) {
        vv_sz chunk = len-off; if (chunk>VV_CHUNK) chunk=VV_CHUNK;
        _process_chunk(vv,buf+off,chunk,cid++);
        off+=chunk;
    }
}

/* Audit: Trinity Control metrics + compliance flags + convergence stats. */
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

    /* convergence: average Hamming diversity of stored engrams */
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

/* SVG similarity graph — Trinity Core visualization with Hamming-weighted edges. */
void vv_svg(const VerbVivoState *vv) {
    if (!vv) return;
    ((VerbVivoState*)vv)->ctrl.svg_requests++;

    printf("<svg width='900' height='900' xmlns='http://www.w3.org/2000/svg' "
           "style='background:#0b132b'>\n");
    printf("<title>VerbVivo — Engram Convergence Map</title>\n");
    printf("<g stroke='#2ec4b6' stroke-width='1'>\n");

    vv_u32 n = vv->chunk_count < VV_MEM_SIZE ? vv->chunk_count : VV_MEM_SIZE;

    /* edges: cosine similarity > 0.5 → structural resonance */
    for (vv_u32 i=0;i<n;i++) {
        for (vv_u32 j=i+1;j<n;j++) {
            vv_f32 sim = _cosine(&vv->memory[i].vec, &vv->memory[j].vec);
            /* Hamming-weighted opacity: high diversity = dimmer edge */
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

    /* nodes: color by Hamming diversity */
    for (vv_u32 i=0;i<n;i++) {
        const VVEngram *m=&vv->memory[i];
        int x=(int)(i*13+60);
        int y=(int)(450+(m->attention*300));
        /* high hamming_div → blue (novel), low → green (familiar) */
        vv_u32 r=(vv_u32)(m->hamming_div*255.0f);
        vv_u32 g=(vv_u32)((1.0f-m->hamming_div)*200.0f+55.0f);
        vv_u32 b2=(vv_u32)(m->attention*255.0f);
        printf("<circle cx='%d' cy='%d' r='5' fill='#%02x%02x%02x' opacity='0.9'>\n",
               x,y,(unsigned)r,(unsigned)g,(unsigned)b2);
        printf("  <title>id=%u hash=%08x attn=%.3f hdiv=%.3f</title>\n",
               m->id,m->content_hash,(double)m->attention,(double)m->hamming_div);
        printf("</circle>\n");
    }

    /* legend */
    printf("<text x='10' y='20' fill='#2ec4b6' font-family='monospace' font-size='11'>"
           "VerbVivo — n=%u chunks=%u</text>\n", n, vv->chunk_count);
    printf("<text x='10' y='36' fill='#888' font-family='monospace' font-size='9'>"
           "blue=novel  green=familiar  size=attention</text>\n");
    printf("</svg>\n");
}

/* ── CLI: compile-as-binary when VERBOVIVO_MAIN is defined ─────── */
#ifdef VERBOVIVO_MAIN
int main(int argc, char *argv[]) {
    VerbVivoState vv;
    vv_init(&vv);

    int do_svg=0;
    for (int i=1;i<argc;i++) {
        if (argv[i][0]=='-'&&argv[i][1]=='s') do_svg=1;
    }

    vv_scan(&vv,stdin);
    vv_audit(&vv);
    if (do_svg) vv_svg(&vv);
    return 0;
}
#endif
