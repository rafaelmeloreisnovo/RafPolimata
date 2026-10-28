#ifndef RAF_CI_CORE_H
#define RAF_CI_CORE_H
/* RAFAELIA-CI-L0-V1 — source contract, not a GitHub Actions interpreter.
 * INPUT: caller-owned bytes and length; bounded by RAF_CI_MAX_INPUT.
 * OUTPUT: caller-owned flags + non-cryptographic FNV receipt fingerprint.
 * DEPENDENCIES: zero includes, libc, malloc, runtime, syscalls and GC.
 * BIT LAYOUT: 0 pins | 1 unpinned | 2 dynamic | 3 local ref
 *             4 unsafe trigger review | 5 permission seen | 6 too large
 *             7 malformed | 8 YAML semantics unknown | 9 uses seen.
 * COMPATIBILITY: v1 only scans physical lines, NOT full YAML. Full CI is
 * always TOKEN_VAZIO_NOT_RUN; no green CI claims from this classifier.
 */
#define RAF_CI_MAX_INPUT 1048576u
#define RAF_CI_PINNED 1u
#define RAF_CI_UNPINNED 2u
#define RAF_CI_DYNAMIC 4u
#define RAF_CI_LOCAL 8u
#define RAF_CI_TRIGGER_REVIEW 16u
#define RAF_CI_PERMISSIONS 32u
#define RAF_CI_TOO_LARGE 64u
#define RAF_CI_MALFORMED 128u
#define RAF_CI_YAML_UNVERIFIED 256u
#define RAF_CI_USES_SEEN 512u
#define RAF_CI_ELF32 1u
#define RAF_CI_ELF64 2u
#define RAF_CI_ELF_INVALID 0u
#define RAF_CI_ELF_UNKNOWN 3u

typedef struct raf_ci_receipt_v1 {
    unsigned int schema;
    unsigned int flags;
    unsigned int uses_count;
    unsigned int line_count;
    unsigned int fnv1a32; /* NOT an integrity signature. */
} raf_ci_receipt_v1;
/* ABI equality is checked at compile time, never inferred from tool name. */
#if defined(__cplusplus)
#define RAF_CI_COMPILE_ASSERT(x, msg) static_assert(x, msg)
#else
#define RAF_CI_COMPILE_ASSERT(x, msg) _Static_assert(x, msg)
#endif
RAF_CI_COMPILE_ASSERT(sizeof(unsigned int) == 4u, "CI gate requires a 32-bit unsigned int");
RAF_CI_COMPILE_ASSERT((unsigned char)256u == 0u, "CI gate requires 8-bit bytes");
RAF_CI_COMPILE_ASSERT(sizeof(raf_ci_receipt_v1) == 20u, "CI receipt ABI mismatch");

#if defined(__GNUC__) || defined(__clang__)
#define RAF_CI_INLINE static __inline__ __attribute__((always_inline))
#else
#define RAF_CI_INLINE static inline
#endif

RAF_CI_INLINE int raf_ci_space(unsigned char a) { return a == 32u || a == 9u || a == 13u; }
RAF_CI_INLINE int raf_ci_hex(unsigned char a) {
    return (a >= '0' && a <= '9') || (a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F');
}
static __attribute__((noinline)) int raf_ci_key(const unsigned char *p, unsigned int n, const char *k, unsigned int m) {
    unsigned int i;
    for (i = 0u; i < m; ++i) {
        if (i >= n || p[i] != (unsigned char)k[i]) return 0;
    }
    return i < n && p[i] == ':';
}
RAF_CI_INLINE unsigned int raf_ci_uses(const unsigned char *p, unsigned int n) {
    unsigned int i = 0u, at = n, bytes;
    while (i < n && raf_ci_space(p[i])) ++i;
    if (i == n || p[i] == '#' || p[i] == '|' || p[i] == '>') return RAF_CI_MALFORMED;
    if (p[i] == '\'' || p[i] == '"') {
        unsigned char quote = p[i++];
        if (n <= i || p[n-1u] != quote) return RAF_CI_YAML_UNVERIFIED;
        --n;
    }
    if (i + 1u < n && p[i] == '.' && p[i+1u] == '/') return RAF_CI_LOCAL;
    /* github expressions, Docker references and YAML aliases are NOT parsed. */
    if (p[i] == '$' || p[i] == '*' || (n - i >= 9u && p[i] == 'd' && p[i+1u] == 'o' && p[i+2u] == 'c' && p[i+3u] == 'k'))
        return RAF_CI_DYNAMIC;
    for (unsigned int j = i; j < n; ++j) {
        if (p[j] == '@') { at = j; break; }
        if (raf_ci_space(p[j]) || p[j] == '#') break;
    }
    if (at == n || at == i) return RAF_CI_UNPINNED;
    bytes = n - at - 1u;
    /* An inline comment requires whitespace before '#'; unknown suffixes fail closed. */
    for (unsigned int j = at + 1u; j < n; ++j) {
        if (raf_ci_space(p[j]) || p[j] == '#') { bytes = j - at - 1u; break; }
    }
    if (bytes != 40u) return RAF_CI_UNPINNED;
    for (unsigned int j = 0u; j < bytes; ++j) if (!raf_ci_hex(p[at+1u+j])) return RAF_CI_UNPINNED;
    return RAF_CI_PINNED; /* Syntactic SHA-1 pin ONLY; never provider attestation. */
}
RAF_CI_INLINE raf_ci_receipt_v1 raf_ci_probe(const unsigned char *p, unsigned int n) {
    raf_ci_receipt_v1 r = {1u, RAF_CI_YAML_UNVERIFIED, 0u, 0u, 2166136261u};
    unsigned int i = 0u;
    if (!p || n > RAF_CI_MAX_INPUT) { r.flags |= (n > RAF_CI_MAX_INPUT ? RAF_CI_TOO_LARGE : RAF_CI_MALFORMED); return r; }
    for (unsigned int j = 0u; j < n; ++j) r.fnv1a32 = (r.fnv1a32 ^ p[j]) * 16777619u;
    while (i < n) {
        unsigned int e = i, b;
        while (e < n && p[e] != '\n') ++e;
        b = i;
        while (b < e && raf_ci_space(p[b])) ++b;
        if (b < e && p[b] == '-' && b+1u < e && raf_ci_space(p[b+1u])) {
            b += 2u;
            while (b < e && raf_ci_space(p[b])) ++b;
        }
        if (b < e && p[b] != '#') {
            if (e-b > 5u && raf_ci_key(p+b, e-b, "uses", 4u)) {
                r.flags |= RAF_CI_USES_SEEN | raf_ci_uses(p+b+5u, e-b-5u);
                ++r.uses_count;
            }
            if (e-b >= 12u && raf_ci_key(p+b, e-b, "permissions", 11u)) r.flags |= RAF_CI_PERMISSIONS;
            if (e-b >= 20u && raf_ci_key(p+b, e-b, "pull_request_target", 19u)) r.flags |= RAF_CI_TRIGGER_REVIEW;
        }
        ++r.line_count;
        i = e < n ? e+1u : n;
    }
    return r;
}
/* ELF header view only. No linking, relocation, loading, or execution.
 * e_ident: 0..15, e_machine: 18..19, byte order from EI_DATA. */
RAF_CI_INLINE unsigned int raf_ci_elf_class(const unsigned char *p, unsigned int n, unsigned int machine) {
    unsigned int cls, m;
    if (!p || n < 20u || p[0] != 0x7fu || p[1] != 'E' || p[2] != 'L' || p[3] != 'F') return RAF_CI_ELF_INVALID;
    cls = p[4];
    if ((cls != RAF_CI_ELF32 && cls != RAF_CI_ELF64) || n < (cls == RAF_CI_ELF32 ? 52u : 64u)) return RAF_CI_ELF_INVALID;
    if (p[5] != 1u && p[5] != 2u) return RAF_CI_ELF_INVALID;
    m = p[5] == 1u ? ((unsigned int)p[18] | ((unsigned int)p[19] << 8u)) : (((unsigned int)p[18] << 8u) | p[19]);
    return machine && m != machine ? RAF_CI_ELF_UNKNOWN : cls;
}
#endif
