#ifndef RAF_CI_GATE_V4_H
#define RAF_CI_GATE_V4_H
/* RAFAELIA-CI-GATE-V4 — additive conservative source-topology observer.
 * Scope: C11/C++17 freestanding, byte slices, no include except authorial V3.
 * Caller owns all memory. No malloc, libc, syscalls, callbacks or GC.
 * Not a YAML parser, not a GitHub Actions emulator, not a security attestation.
 * Source identity, actual workflow graph, runner execution and approvals live
 * OUTSIDE this module and remain explicitly typed missing evidence.
 * NEVER lowers a V2 HOLD to REVIEW. Never promotes REVIEW to CI PASS.
 * No scalar tail, hidden fallback, retries, or externally mutable state.
 */
#include "raf_ci_z0_receipt_v1.h"
#define RAF_CI_G4_REVIEW 1u
#define RAF_CI_G4_HOLD 2u
#define RAF_CI_G4_INVALID 3u
#define RAF_CI_G4_RISK_P0 0u
#define RAF_CI_G4_RISK_P1 1u
#define RAF_CI_G4_RISK_P2 2u
#define RAF_CI_G4_VOID_PROVIDER_CI   (1u<<0)
#define RAF_CI_G4_VOID_DEVICE        (1u<<1)
#define RAF_CI_G4_VOID_YAML_AST      (1u<<2)
#define RAF_CI_G4_VOID_PROVENANCE    (1u<<3)
#define RAF_CI_G4_VOID_LICENSE       (1u<<4)
#define RAF_CI_G4_VOID_PERFORMANCE   (1u<<5)
#define RAF_CI_G4_VOID_LOCKFILE      (1u<<6)
#define RAF_CI_G4_VOID_ALL 127u
#define RAF_CI_G4_GAP_ON            (1u<<0)
#define RAF_CI_G4_GAP_JOBS          (1u<<1)
#define RAF_CI_G4_GAP_JOB_DECL      (1u<<2)
#define RAF_CI_G4_GAP_UNSUPPORTED   (1u<<3)
#define RAF_CI_G4_GAP_INPUT         (1u<<4)
#define RAF_CI_G4_GAP_V2            (1u<<5)
#define RAF_CI_G4_GAP_V2_UNKNOWN    (1u<<6)
#define RAF_CI_G4_GAP_EMPTY_BODY    (1u<<7)
typedef struct raf_ci_gate_v4 {
    unsigned int schema;
    unsigned int state;   /* never PASS: REVIEW/HOLD/INVALID */
    unsigned int urgency; /* P0/P1/P2; not a runtime measurement */
    unsigned int gaps;    /* structural and source warning bits */
    unsigned int voids;   /* all evidence boundaries missing by default */
    unsigned int source_fingerprint; /* V2 FNV1a32: NOT cryptographic */
} raf_ci_gate_v4;
RAF_CI_COMPILE_ASSERT(sizeof(raf_ci_gate_v4)==24u,"v4 ABI must not drift");
/* Compare only ASCII exact characters at column 0/2. YAML quotes, aliases,
 * folded keys and flow maps intentionally remain unsupported and HOLD. */
RAF_CI_INLINE int raf_ci_g4_colkey(const unsigned char *p,unsigned int n,const char *key,unsigned int k){
    unsigned int i;
    if(n<=k || p[k]!=':') return 0;
    for(i=0u;i<k;++i) if(p[i]!=(unsigned char)key[i]) return 0;
    return 1;
}
RAF_CI_INLINE int raf_ci_g4_job_key(const unsigned char *p,unsigned int n){
    unsigned int i=2u;
    if(n<4u || p[0]!=32u || p[1]!=32u) return 0;
    if(p[i]==32u || p[i]=='#' || p[i]=='-' || p[i]=='\t') return 0;
    while(i<n && p[i]!=':') {
        unsigned char x=p[i];
        if(!((x>='A'&&x<='Z')||(x>='a'&&x<='z')||
             (x>='0'&&x<='9')||x=='_'||x=='-')) return 0;
        ++i;
    }
    return i<n && i>2u && p[i]==':';
}
RAF_CI_INLINE raf_ci_gate_v4 raf_ci_gate_v4_check(const unsigned char *p,unsigned int n){
    raf_ci_gate_v4 g={4u,RAF_CI_G4_HOLD,RAF_CI_G4_RISK_P1,0u,RAF_CI_G4_VOID_ALL,0u};
    unsigned char raw[RAF_CI_WIRE_SIZE];
    unsigned int i=0u,seen_on=0u,seen_jobs=0u,seen_job=0u,jobs_scope=0u,job_pending=0u;
    unsigned int typ=raf_ci_z0_classify(p,n);
    if(typ!=RAF_CI_Z0_TEXT){
        g.state=RAF_CI_G4_INVALID;
        g.urgency=RAF_CI_G4_RISK_P0;
        g.gaps|=RAF_CI_G4_GAP_INPUT;
        /* Avoid reading p when absent/overlength; still encode deterministic
         * v2 HOLD for governance, never ABI changes or extra I/O. */
        if(n<=RAF_CI_MAX_INPUT) {
            raf_ci_wire_v2_encode(p,n,raw);
            g.source_fingerprint=((unsigned int)raw[20])|((unsigned int)raw[21]<<8u)|
                ((unsigned int)raw[22]<<16u)|((unsigned int)raw[23]<<24u);
        }
        return g;
    }
    /* Valid text with no YAML topological evidence is never REVIEW. */
    while(i<n){
        unsigned int e=i, b=i;
        while(e<n && p[e]!='\n') ++e;
        while(b<e && (p[b]==' '||p[b]=='\t')) ++b;
        if(b<e && p[b]!='#') {
            if(b==i) {
                if(job_pending) { g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY; job_pending=0u; }
                if(raf_ci_g4_colkey(p+i,e-i,"on",2u)) seen_on=1u;
                if(raf_ci_g4_colkey(p+i,e-i,"jobs",4u)) {
                    seen_jobs=1u;
                    jobs_scope=1u;
                } else jobs_scope=0u;
            } else if(jobs_scope && b==i+2u && raf_ci_g4_job_key(p+i,e-i)) {
                if(job_pending) g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                seen_job=1u;job_pending=1u;
            } else if(jobs_scope && b==i+4u &&
                   (raf_ci_g4_colkey(p+b,e-b,"runs-on",7u) ||
                    raf_ci_g4_colkey(p+b,e-b,"uses",4u))) {
                /* A runner or reusable workflow route is only a structural
                 * hint; NEVER actual proof that a job can execute. */
                job_pending=0u;
            }
        }
        i=e<n?e+1u:n;
    }
    if(job_pending)g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
    if(!seen_on)g.gaps|=RAF_CI_G4_GAP_ON;
    if(!seen_jobs)g.gaps|=RAF_CI_G4_GAP_JOBS;
    if(!seen_job)g.gaps|=RAF_CI_G4_GAP_JOB_DECL;
    /* Retain V2 wire as an independent historical contract, no new layout. */
    raf_ci_wire_v2_encode(p,n,raw);
    g.source_fingerprint=((unsigned int)raw[20])|((unsigned int)raw[21]<<8u)|
        ((unsigned int)raw[22]<<16u)|((unsigned int)raw[23]<<24u);
    if(raw[24]==RAF_CI_WIRE_STATE_HOLD){g.gaps|=RAF_CI_G4_GAP_V2;g.urgency=RAF_CI_G4_RISK_P0;}
    if(raw[24]!=RAF_CI_WIRE_STATE_HOLD && raw[24]!=RAF_CI_WIRE_STATE_STATIC_REVIEW){
        g.gaps|=RAF_CI_G4_GAP_V2_UNKNOWN;g.urgency=RAF_CI_G4_RISK_P0;
    }
    if(!g.gaps)g.state=RAF_CI_G4_REVIEW;
    return g;
}
#endif
