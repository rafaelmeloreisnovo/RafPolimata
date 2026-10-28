#ifndef RAF_CI_Z0_VIEW_H
#define RAF_CI_Z0_VIEW_H
/* RAFAELIA Z0 / typed byte absence — AUTHORIAL V1, standalone L0.
 * No malloc, libc, runtime, syscalls, GC, codecs or external translations.
 * Bytes are observations, not semantic meanings; UTF-8 is a structural test.
 * ∅ (UTF-8 E2 88 85), pointer-absence, 0x00, empty, "", 0, and spaces
 * MUST NOT collapse into a common integer or the same TOKEN_VAZIO status.
 * Precondition: p addresses n bytes (only read when p != 0); n <= 1MiB.
 * No codepoint normalization or inference about scripts or ancient origins.
 */
#define RAF_CI_Z0_ABSENT 1u
#define RAF_CI_Z0_EMPTY 2u
#define RAF_CI_Z0_NUL 3u
#define RAF_CI_Z0_SPACES 4u
#define RAF_CI_Z0_ASCII_ZERO 5u
#define RAF_CI_Z0_EMPTY_SET 6u
#define RAF_CI_Z0_TOKEN_VAZIO 7u
#define RAF_CI_Z0_JSON_NULL_TEXT 8u
#define RAF_CI_Z0_TEXT 9u
#define RAF_CI_Z0_INVALID_UTF8 10u
#define RAF_CI_Z0_TOO_LARGE 11u
#define RAF_CI_Z0_DOUBLE_QUOTED_EMPTY 12u
#define RAF_CI_Z0_CONTROL 13u
#define RAF_CI_Z0_BLANK_ONLY 14u
#ifndef RAF_CI_MAX_INPUT
#define RAF_CI_MAX_INPUT 1048576u
#endif
#if defined(__GNUC__) || defined(__clang__)
#define RAF_CI_Z0_INLINE static __inline__ __attribute__((always_inline))
#else
#define RAF_CI_Z0_INLINE static inline
#endif
RAF_CI_Z0_INLINE int raf_ci_z0_eq(const unsigned char *p, unsigned int n, const char *s, unsigned int sl) {
    unsigned int i;
    if (n != sl) return 0;
    for (i=0u;i<n;++i) if(p[i]!=(unsigned char)s[i]) return 0;
    return 1;
}
/* Reject truncated, overlong, surrogate and > U+10FFFF sequences; no heap. */
RAF_CI_Z0_INLINE int raf_ci_z0_valid_utf8(const unsigned char *p,unsigned int n) {
    unsigned int i=0u;
    if (!p) return 0;
    while (i<n) {
        unsigned int c=p[i], need=0u,lo=0x80u,hi=0xbfu,j;
        if (c<0x80u) {
            /* CR LF TAB allowed for workflow host reader; all other C0/C1-as-ASCII rejected. */
            if (c<0x20u && c!=9u && c!=10u && c!=13u) return 0;
            if (c==0x7fu) return 0;
            ++i;continue;
        }
        if (c>=0xc2u && c<=0xdfu) need=1u;
        else if (c>=0xe0u && c<=0xefu) {
            need=2u;
            if(c==0xe0u)lo=0xa0u;
            if(c==0xedu)hi=0x9fu;
        }
        else if (c>=0xf0u && c<=0xf4u) {
            need=3u;
            if(c==0xf0u)lo=0x90u;
            if(c==0xf4u)hi=0x8fu;
        }
        else return 0;
        if (need > n-i-1u) return 0;
        if (p[i+1u]<lo || p[i+1u]>hi) return 0;
        for(j=2u;j<=need;++j) if(p[i+j]<0x80u||p[i+j]>0xbfu)return 0;
        i+=need+1u;
    }
    return 1;
}
/* Explicit typed zero/void distinctions. Valid other byte strings remain TEXT. */
RAF_CI_Z0_INLINE unsigned int raf_ci_z0_classify(const unsigned char *p,unsigned int n) {
    unsigned int i, allspace=1u, allblank=1u;
    if (!p) return RAF_CI_Z0_ABSENT;
    if(n>RAF_CI_MAX_INPUT) return RAF_CI_Z0_TOO_LARGE;
    if (n==0u) return RAF_CI_Z0_EMPTY;
    if(n==1u && p[0]==0u) return RAF_CI_Z0_NUL;
    if(n==1u && p[0]=='0') return RAF_CI_Z0_ASCII_ZERO;
    for(i=0u;i<n;++i){
        if(p[i]!=32u)allspace=0u;
        if(p[i]!=32u && p[i]!=9u && p[i]!=10u && p[i]!=13u)allblank=0u;
    }
    if(allspace) return RAF_CI_Z0_SPACES;
    if(allblank) return RAF_CI_Z0_BLANK_ONLY;
    if(n==2u && p[0]=='"' && p[1]=='"') return RAF_CI_Z0_DOUBLE_QUOTED_EMPTY;
    if(n==3u && p[0]==0xe2u && p[1]==0x88u && p[2]==0x85u) return RAF_CI_Z0_EMPTY_SET;
    if(raf_ci_z0_eq(p,n,"TOKEN_VAZIO",11u))return RAF_CI_Z0_TOKEN_VAZIO;
    if(raf_ci_z0_eq(p,n,"null",4u))return RAF_CI_Z0_JSON_NULL_TEXT;
    if(!raf_ci_z0_valid_utf8(p,n))return RAF_CI_Z0_INVALID_UTF8;
    return RAF_CI_Z0_TEXT;
}
/* Full YAML interpretation is intentionally OUTSIDE this layer. */
#endif
