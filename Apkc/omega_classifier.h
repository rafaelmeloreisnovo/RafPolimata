/* raf_omega_classifier.h — deterministic byte-stream classification bridge.
 *
 * Derived as a compiler-safe bridge between the RMR-CTI deterministic fields,
 * Omega five-path taxonomy, and the freestanding LayersBit design principles.
 *
 * Properties:
 *   - header-only, freestanding, no libc, no heap, no floating point;
 *   - fixed 64-byte local state (seen[32] + fold[32]);
 *   - deterministic for the same byte stream on every host;
 *   - classification is an engineering routing label, not a scientific claim.
 */
#ifndef RAF_OMEGA_CLASSIFIER_H
#define RAF_OMEGA_CLASSIFIER_H

typedef unsigned char      raf_omega_u8;
typedef unsigned int       raf_omega_u32;
typedef unsigned long long raf_omega_u64;

typedef enum {
    RAF_OMEGA_PROCESSUAL   = 0,
    RAF_OMEGA_VOID         = 1,
    RAF_OMEGA_FORGOTTEN    = 2,
    RAF_OMEGA_MENOSPREZADO = 3,
    RAF_OMEGA_URGENT       = 4
} RafOmegaPath;

#define RAF_OMEGA_F_TEXTUAL         0x01u
#define RAF_OMEGA_F_BINARY          0x02u
#define RAF_OMEGA_F_LOW_SIGNAL      0x04u
#define RAF_OMEGA_F_HIGH_COHERENCE  0x08u
#define RAF_OMEGA_F_TRANSITION_RICH 0x10u

typedef struct {
    raf_omega_u32 bytes;
    raf_omega_u32 unique;
    raf_omega_u32 transitions;
    raf_omega_u32 printable;
    raf_omega_u32 controls;
    raf_omega_u32 zeros;
    raf_omega_u32 fold_ones;
    raf_omega_u32 entropy_milli; /* deterministic proxy in [0,8000] */
    raf_omega_u32 coherence_q16; /* structural balance in [0,65536] */
    raf_omega_u32 phi_q16;       /* (1-H)*C in Q16 */
    raf_omega_u32 attractor;     /* [0,41] */
    raf_omega_u32 flags;
    RafOmegaPath  path;
} RafOmegaMetrics;

static __attribute__((always_inline)) inline
raf_omega_u32 raf_omega_pop8(raf_omega_u8 v) {
    v = (raf_omega_u8)(v - ((v >> 1u) & 0x55u));
    v = (raf_omega_u8)((v & 0x33u) + ((v >> 2u) & 0x33u));
    return (raf_omega_u32)((v + (v >> 4u)) & 0x0Fu);
}

static __attribute__((always_inline)) inline
raf_omega_u32 raf_omega_absdiff(raf_omega_u32 a, raf_omega_u32 b) {
    return a > b ? a - b : b - a;
}

static __attribute__((always_inline)) inline
raf_omega_u32 raf_omega_balance_q16(raf_omega_u32 q16) {
    raf_omega_u32 d = raf_omega_absdiff(q16, 0x8000u);
    raf_omega_u32 twice = d << 1u;
    return twice >= 0x10000u ? 0u : 0x10000u - twice;
}

static __attribute__((always_inline)) inline
const char *raf_omega_path_name(RafOmegaPath path) {
    switch (path) {
    case RAF_OMEGA_VOID:         return "VOID";
    case RAF_OMEGA_FORGOTTEN:    return "FORGOTTEN";
    case RAF_OMEGA_MENOSPREZADO: return "MENOSPREZADO";
    case RAF_OMEGA_URGENT:       return "URGENT";
    case RAF_OMEGA_PROCESSUAL:
    default:                     return "PROCESSUAL";
    }
}

static inline RafOmegaMetrics
raf_omega_classify(const raf_omega_u8 *buf, raf_omega_u32 n) {
    RafOmegaMetrics m;
    raf_omega_u8 seen[32];
    raf_omega_u8 fold[32];
    raf_omega_u8 prev = 0u;
    raf_omega_u32 i;

    m.bytes = n;
    m.unique = 0u;
    m.transitions = 0u;
    m.printable = 0u;
    m.controls = 0u;
    m.zeros = 0u;
    m.fold_ones = 0u;
    m.entropy_milli = 0u;
    m.coherence_q16 = 0u;
    m.phi_q16 = 0u;
    m.attractor = 0u;
    m.flags = 0u;
    m.path = RAF_OMEGA_VOID;

    for (i = 0u; i < 32u; ++i) {
        seen[i] = 0u;
        fold[i] = 0u;
    }
    if (!buf || n == 0u) return m;

    for (i = 0u; i < n; ++i) {
        raf_omega_u8 b = buf[i];
        raf_omega_u32 route;
        seen[b >> 3u] |= (raf_omega_u8)(1u << (b & 7u));
        route = (((raf_omega_u32)b * 257u) ^ (i * 131u) ^
                 ((raf_omega_u32)b << 5u) ^ (i >> 3u)) & 255u;
        fold[route >> 3u] ^= (raf_omega_u8)(1u << (route & 7u));

        if (i && b != prev) m.transitions++;
        prev = b;
        if (b == 0u) m.zeros++;
        if ((b >= 32u && b <= 126u) || b == 9u || b == 10u || b == 13u)
            m.printable++;
        else
            m.controls++;
    }

    for (i = 0u; i < 32u; ++i) {
        m.unique += raf_omega_pop8(seen[i]);
        m.fold_ones += raf_omega_pop8(fold[i]);
    }

    {
        raf_omega_u32 unique_term = (m.unique * 6000u) / 256u;
        raf_omega_u32 trans_term = n > 1u
            ? (m.transitions * 2000u) / (n - 1u) : 0u;
        raf_omega_u32 h_q16;
        raf_omega_u32 trans_q16 = n > 1u
            ? (raf_omega_u32)(((raf_omega_u64)m.transitions << 16u) /
                              (raf_omega_u64)(n - 1u)) : 0u;
        raf_omega_u32 occ_q16 = m.fold_ones << 8u;
        raf_omega_u32 c0 = raf_omega_balance_q16(trans_q16);
        raf_omega_u32 c1 = raf_omega_balance_q16(occ_q16);

        m.entropy_milli = unique_term + trans_term;
        if (m.entropy_milli > 8000u) m.entropy_milli = 8000u;
        m.coherence_q16 = (c0 + c1) >> 1u;
        h_q16 = (raf_omega_u32)(((raf_omega_u64)m.entropy_milli << 16u) / 8000u);
        m.phi_q16 = (raf_omega_u32)(((raf_omega_u64)(0x10000u - h_q16) *
                                     m.coherence_q16) >> 16u);
    }

    if (m.printable * 10u >= n * 8u) m.flags |= RAF_OMEGA_F_TEXTUAL;
    if (m.zeros || m.controls * 8u > n) m.flags |= RAF_OMEGA_F_BINARY;
    if (n < 16u || m.unique <= 2u) m.flags |= RAF_OMEGA_F_LOW_SIGNAL;
    if (m.phi_q16 >= 0x4000u) m.flags |= RAF_OMEGA_F_HIGH_COHERENCE;
    if (n > 1u && m.transitions * 4u >= (n - 1u) * 3u)
        m.flags |= RAF_OMEGA_F_TRANSITION_RICH;

    m.attractor = (m.fold_ones + m.unique + (m.phi_q16 >> 10u) +
                   (m.transitions % 42u)) % 42u;

    if (m.flags & RAF_OMEGA_F_LOW_SIGNAL)
        m.path = RAF_OMEGA_FORGOTTEN;
    else if ((m.flags & RAF_OMEGA_F_HIGH_COHERENCE) &&
             (m.flags & RAF_OMEGA_F_TEXTUAL) && n >= 64u)
        m.path = RAF_OMEGA_URGENT;
    else if ((m.flags & RAF_OMEGA_F_HIGH_COHERENCE) ||
             ((m.flags & RAF_OMEGA_F_TEXTUAL) && n >= 32u))
        m.path = RAF_OMEGA_MENOSPREZADO;
    else
        m.path = RAF_OMEGA_PROCESSUAL;

    return m;
}

/* Deterministic selector for verified-equivalent encodings only. */
static __attribute__((always_inline)) inline
raf_omega_u32 raf_omega_codegen_index(const raf_omega_u8 *buf,
                                       raf_omega_u32 n,
                                       raf_omega_u32 variants) {
    RafOmegaMetrics m;
    raf_omega_u32 key;
    if (variants <= 1u) return 0u;
    m = raf_omega_classify(buf, n);
    key = m.attractor ^ ((raf_omega_u32)m.path * 7u) ^
          (m.entropy_milli >> 4u) ^ (m.phi_q16 >> 8u) ^ m.flags;
    return key % variants;
}

#endif /* RAF_OMEGA_CLASSIFIER_H */
