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
#if defined(__GNUC__) || defined(__clang__)
#define RAF_CI_G4_NOINLINE static __attribute__((noinline))
#else
#define RAF_CI_G4_NOINLINE static
#endif
RAF_CI_G4_NOINLINE int raf_ci_g4_colkey(const unsigned char *p,unsigned int n,const char *key,unsigned int k){
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
/* YAML subset only: a byte slice after a physical key. A scalar consisting
 * solely of whitespace or comment is NOT evidence of a configured value.
 * Deliberately no YAML expression evaluator or command execution. */
RAF_CI_G4_NOINLINE int raf_ci_g4_value(const unsigned char *p,unsigned int n,unsigned int col){
    unsigned int i=col;
    if(i>=n || p[i]!=':') return 0;
    ++i;
    while(i<n && (p[i]==32u || p[i]==9u || p[i]==13u))++i;
    return i<n && p[i]!='#';
}
RAF_CI_INLINE void raf_ci_g4_close_job(unsigned int active,unsigned int runner,
    unsigned int reuse,unsigned int steps,unsigned int executable,unsigned int *gaps){
    if(active && !((reuse && !runner && !steps) ||
                   (!reuse && runner && steps && executable)))
        *gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
    if(active && reuse && (runner || steps)) *gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
}
RAF_CI_INLINE raf_ci_gate_v4 raf_ci_gate_v4_check(const unsigned char *p,unsigned int n){
    raf_ci_gate_v4 g={4u,RAF_CI_G4_HOLD,RAF_CI_G4_RISK_P1,0u,RAF_CI_G4_VOID_ALL,0u};
    unsigned char raw[RAF_CI_WIRE_SIZE];
    unsigned int i=0u,seen_on=0u,on_valid=0u,seen_jobs=0u,seen_job=0u;
    unsigned int scope=0u,active=0u,runner=0u,reuse=0u,steps=0u,exe=0u,step=0u;
    unsigned int typ=raf_ci_z0_classify(p,n);
    if(typ!=RAF_CI_Z0_TEXT){
        g.state=RAF_CI_G4_INVALID;
        g.urgency=RAF_CI_G4_RISK_P0;
        g.gaps|=RAF_CI_G4_GAP_INPUT;
        if(n<=RAF_CI_MAX_INPUT){
            raf_ci_wire_v2_encode(p,n,raw);
            g.source_fingerprint=((unsigned int)raw[20])|((unsigned int)raw[21]<<8u)|
                ((unsigned int)raw[22]<<16u)|((unsigned int)raw[23]<<24u);
        }
        return g;
    }
    while(i<n){
        unsigned int e=i,b=i,indent;
        while(e<n && p[e]!='\n')++e;
        while(b<e && (p[b]==32u || p[b]==9u))++b;
        indent=b-i;
        if(b<e && p[b]!='#'){
            /* Tabs in indentation and block scalars outside a YAML AST are
             * not safely interpretable: review without positive claims. */
            if(b!=i && p[b-1u]==9u) g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
            if(indent==0u){
                raf_ci_g4_close_job(active,runner,reuse,steps,exe,&g.gaps);
                active=runner=reuse=steps=exe=step=0u;
                scope=0u;
                if(raf_ci_g4_colkey(p+b,e-b,"on",2u)){
                    if(seen_on)g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED; /* duplicate root event */
                    ++seen_on;scope=1u;
                    if(raf_ci_g4_value(p+b,e-b,2u))on_valid=1u;
                }else if(raf_ci_g4_colkey(p+b,e-b,"jobs",4u)){
                    if(seen_jobs)g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
                    ++seen_jobs;scope=2u;
                    if(raf_ci_g4_value(p+b,e-b,4u))g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
                }
            }else if(scope==1u && indent==2u){
                /* Nested event mapping, e.g. on:\n  push:. No scalar is
                 * required for the event key itself. */
                unsigned int k=0u;
                while(b+k<e && p[b+k]!=':' && p[b+k]!=' ' && p[b+k]!='#')++k;
                if(k && b+k<e && p[b+k]==':')on_valid=1u;
            }else if(scope==2u){
                if(indent==2u && raf_ci_g4_job_key(p+i,e-i)){
                    raf_ci_g4_close_job(active,runner,reuse,steps,exe,&g.gaps);
                    active=1u;runner=reuse=steps=exe=step=0u;
                    seen_job=1u;
                    /* Inline jobs require YAML interpretation (not this subset). */
                    {unsigned int c=b;
                     while(c<e && p[c]!=':')++c;
                     if(raf_ci_g4_value(p+b,e-b,c-b))g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
                    }
                }else if(active && indent==4u){
                    step=0u;
                    if(raf_ci_g4_colkey(p+b,e-b,"runs-on",7u)){
                        if(raf_ci_g4_value(p+b,e-b,7u))runner=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }else if(raf_ci_g4_colkey(p+b,e-b,"uses",4u)){
                        if(raf_ci_g4_value(p+b,e-b,4u))reuse=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }else if(raf_ci_g4_colkey(p+b,e-b,"steps",5u)){
                        if(raf_ci_g4_value(p+b,e-b,5u))g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
                        else steps=1u;
                    }
                }else if(active && steps && indent==6u && p[b]=='-' && b+1u<e && p[b+1u]==' '){
                    unsigned int v=b+2u;
                    step=1u;
                    if(raf_ci_g4_colkey(p+v,e-v,"run",3u)){
                        if(raf_ci_g4_value(p+v,e-v,3u))exe=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }else if(raf_ci_g4_colkey(p+v,e-v,"uses",4u)){
                        if(raf_ci_g4_value(p+v,e-v,4u))exe=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }
                }else if(active && steps && step && indent==8u){
                    if(raf_ci_g4_colkey(p+b,e-b,"run",3u)){
                        if(raf_ci_g4_value(p+b,e-b,3u))exe=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }else if(raf_ci_g4_colkey(p+b,e-b,"uses",4u)){
                        if(raf_ci_g4_value(p+b,e-b,4u))exe=1u;
                        else g.gaps|=RAF_CI_G4_GAP_EMPTY_BODY;
                    }
                }
            }
        }
        i=e<n?e+1u:n;
    }
    raf_ci_g4_close_job(active,runner,reuse,steps,exe,&g.gaps);
    if(!seen_on || !on_valid)g.gaps|=RAF_CI_G4_GAP_ON;
    if(!seen_jobs)g.gaps|=RAF_CI_G4_GAP_JOBS;
    if(!seen_job)g.gaps|=RAF_CI_G4_GAP_JOB_DECL;
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
