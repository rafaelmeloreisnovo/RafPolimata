#ifndef RAF_CI_WIRE_V2_H
#define RAF_CI_WIRE_V2_H
/* RAFAELIA CI wire/v2: fixed 40-byte little-endian structural observation.
 * NOT a CI result, GitHub-issued token, signature, attestation or YAML parse.
 * Caller owns input/output. No syscalls, libc, heap, GC, ELF load or DEX VM.
 * Source registers and memory aliasing are compiler-controlled (no ASM claim).
 * Layout [0..3]='RCI2', [4..7]=2, [8..11]=flags,
 * [12..15]=uses, [16..19]=lines, [20..23]=FNV32 (noncrypto),
 * [24..27]=STATIC_REVIEW(1) or HOLD(2), [28..31]=0x0f (4 missing gates),
 * [32..35]=0 reserved, [36..39]=CRC32C of bytes [0..35].
 * Additional flags (above v1): bit10 broad write permission,
 * bit11 ambiguous event. Flags preserve original v1 bits.
 */
#include "raf_ci_core.h"
#define RAF_CI_WIRE_SIZE 40u
#define RAF_CI_WIRE_SCHEMA 2u
#define RAF_CI_WRITE_REVIEW 1024u
#define RAF_CI_EVENT_REVIEW 2048u
#define RAF_CI_WIRE_MISSING_BOUNDARIES 15u
#define RAF_CI_WIRE_STATE_STATIC_REVIEW 1u
#define RAF_CI_WIRE_STATE_HOLD 2u
#define RAF_CI_V2_HOLD_BITS (RAF_CI_UNPINNED | RAF_CI_DYNAMIC | RAF_CI_TRIGGER_REVIEW | RAF_CI_TOO_LARGE | RAF_CI_MALFORMED | RAF_CI_WRITE_REVIEW | RAF_CI_EVENT_REVIEW)

RAF_CI_INLINE void raf_ci_le32_put(unsigned char *p, unsigned int x) {
    p[0]=(unsigned char)x; p[1]=(unsigned char)(x>>8);
    p[2]=(unsigned char)(x>>16); p[3]=(unsigned char)(x>>24);
}
/* Bitwise CRC32C Castagnoli (reflected polynomial), not a MAC. */
RAF_CI_INLINE unsigned int raf_ci_crc32c(const unsigned char *p, unsigned int n) {
    unsigned int c=0xffffffffu, i,j;
    for (i=0u;i<n;++i) {
        c^=(unsigned int)p[i];
        for (j=0u;j<8u;++j) c=(c>>1) ^ (0x82f63b78u & (0u-(c&1u)));
    }
    return ~c;
}
RAF_CI_INLINE int raf_ci_ident_char(unsigned char x) {
    return (x>='a' && x<='z') || (x>='A' && x<='Z') ||
           (x>='0' && x<='9') || x=='_' || x=='-';
}
/* Bounded lexical suspicion only. Comments are ignored. Block scalar semantics
 * and YAML aliases are not inferred; conservative review is mandatory. */
RAF_CI_INLINE int raf_ci_match_word(const unsigned char *p, unsigned int n, const char *key, unsigned int k) {
    unsigned int j,i;
    if (n<k) return 0;
    for (i=0;i+k<=n;++i) {
        if (i && raf_ci_ident_char(p[i-1])) continue;
        for (j=0u;j<k && p[i+j]==(unsigned char)key[j];++j) {}
        if (j==k && (i+k==n || !raf_ci_ident_char(p[i+k]))) return 1;
    }
    return 0;
}
RAF_CI_INLINE unsigned int raf_ci_extra_policy(const unsigned char *p,unsigned int n) {
    unsigned int i=0u, flags=0u, in_permissions=0u, permissions_indent=0u;
    if (!p || n>RAF_CI_MAX_INPUT) return 0u;
    while (i<n) {
        unsigned int e=i,b=i,stop,indent;
        while (e<n && p[e]!='\n') ++e;
        while (b<e && raf_ci_space(p[b])) ++b;
        indent=b-i;
        stop=b;
        while (stop<e && p[stop]!='#') ++stop; /* comments ignored; quoted '#' => REVIEW only */
        if (b<stop) {
            if (in_permissions && indent<=permissions_indent) in_permissions=0u;
            if (raf_ci_key(p+b,stop-b,"permissions",11u)) {
                in_permissions=1u;
                permissions_indent=indent;
            }
            if (raf_ci_match_word(p+b,stop-b,"pull_request_target",19u)) flags|=RAF_CI_EVENT_REVIEW;
            if (in_permissions && (raf_ci_match_word(p+b,stop-b,"write-all",9u) ||
                (raf_ci_match_word(p+b,stop-b,"write",5u) &&
                 !raf_ci_match_word(p+b,stop-b,"read",4u)))) flags|=RAF_CI_WRITE_REVIEW;
            /* Inline permissions: write-all (same-level) is covered above. */
        }
        i=(e<n)?e+1u:n;
    }
    return flags;
}
/* Encoding is total: even malformed source emits a HOLD record (never PASS). */
RAF_CI_INLINE void raf_ci_wire_v2_encode(const unsigned char *p,unsigned int n,unsigned char out[RAF_CI_WIRE_SIZE]) {
    raf_ci_receipt_v1 r=raf_ci_probe(p,n);
    unsigned int flags=r.flags | raf_ci_extra_policy(p,n);
    if (n==0u) flags|=RAF_CI_MALFORMED; /* Empty source is evidence absence, not reviewable workflow. */
    unsigned int state= (flags & RAF_CI_V2_HOLD_BITS) ? RAF_CI_WIRE_STATE_HOLD : RAF_CI_WIRE_STATE_STATIC_REVIEW;
    unsigned int i;
    for (i=0u;i<RAF_CI_WIRE_SIZE;++i) out[i]=0u;
    out[0]='R';out[1]='C';out[2]='I';out[3]='2';
    raf_ci_le32_put(out+4,RAF_CI_WIRE_SCHEMA);
    raf_ci_le32_put(out+8,flags);
    raf_ci_le32_put(out+12,r.uses_count);
    raf_ci_le32_put(out+16,r.line_count);
    raf_ci_le32_put(out+20,r.fnv1a32);
    raf_ci_le32_put(out+24,state);
    raf_ci_le32_put(out+28,RAF_CI_WIRE_MISSING_BOUNDARIES);
    raf_ci_le32_put(out+36,raf_ci_crc32c(out,36u));
}
#endif
