#ifndef RAF_CI_GATE_V43_H
#define RAF_CI_GATE_V43_H
/* RAFAELIA V4.3 conservative additive veto over V4.2.
 * The V4.2 receipt ABI is unchanged (24-byte struct, RCG4 32-byte wire).
 * This code does NOT parse full YAML or execute GitHub Actions.
 * L0: C11 / C++17, no libc, heap, GC, syscalls, runtime, global state.
 * Complexity O(n + jobs*64), bounded 64 job IDs, caller-owned bytes.
 * Any malformed/unsupported structure causes HOLD, NEVER CI PASS.
 */
#include "raf_ci_gate_v4.h"
#define RAF_CI_G43_STEP_MISSING   (1u<<0)
#define RAF_CI_G43_STEP_CONFLICT  (1u<<1)
#define RAF_CI_G43_DUPLICATE_JOB (1u<<2)
#define RAF_CI_G43_BAD_EVENT     (1u<<3)
#define RAF_CI_G43_BAD_RUNNER    (1u<<4)
#define RAF_CI_G43_DUP_STEPS     (1u<<5)
#define RAF_CI_G43_UNSUPPORTED   (1u<<6)
#define RAF_CI_G43_BAD_ACTION    (1u<<7)
#define RAF_CI_G43_MAX_JOBS 64u
/* Boundary: exact plain YAML subset. Cannot validate aliases/anchors,
 * expressions, quote escaping, action trust or GitHub provider state. */
RAF_CI_INLINE unsigned int raf_ci_g43_end(const unsigned char *p,unsigned int n){
    while(n && (p[n-1u]==' '||p[n-1u]=='\t'||p[n-1u]=='\r'))--n;
    return n;
}
RAF_CI_INLINE int raf_ci_g43_bad_scalar(const unsigned char *p,unsigned int n,unsigned int colon){
    unsigned int i=colon+1u,b,e;
    if(colon>=n||p[colon]!=':')return 1;
    while(i<n && (p[i]==' '||p[i]=='\t'))++i;
    b=i;
    while(i<n && p[i]!='#')++i;
    e=raf_ci_g43_end(p,i);
    if(e<=b)return 1;
    n=e-b;p+=b;
    if((n==1u && (p[0]=='~'||p[0]=='['||p[0]=='{')) ||
       (n==2u && ((p[0]=='['&&p[1]==']')||(p[0]=='{'&&p[1]=='}')||
                   (p[0]=='"'&&p[1]=='"')||(p[0]=='\''&&p[1]=='\''))))return 1;
    if(n==4u && p[0]=='n'&&p[1]=='u'&&p[2]=='l'&&p[3]=='l')return 1;
    if(n==5u && p[0]=='f'&&p[1]=='a'&&p[2]=='l'&&p[3]=='s'&&p[4]=='e')return 1;
    if(n>=2u && ((p[0]=='['&&p[n-1u]==']')||(p[0]=='{'&&p[n-1u]=='}'))){
        unsigned int j,nonempty=0u;
        for(j=1u;j<n-1u;++j)if(p[j]!=' '&&p[j]!='\t')nonempty=1u;
        if(!nonempty)return 1;
    }
    return 0;
}
/* Caller-supplied findings bitmask, optional; never pretends to authenticate.
 * All CHECK/REVIEW states are *static observations only*. */
RAF_CI_INLINE raf_ci_gate_v4 raf_ci_gate_v43_check(const unsigned char *p,unsigned int n,unsigned int *findings){
    raf_ci_gate_v4 g=raf_ci_gate_v4_check(p,n);
    unsigned int why=0u,off=0u,scope=0u,active=0u,steps=0u,step=0u,acts=0u,seen_steps=0u;
    unsigned int js=0u,job_off[RAF_CI_G43_MAX_JOBS],job_len[RAF_CI_G43_MAX_JOBS];
    if(findings)*findings=0u;
    /* Do not read uncertain caller input; V4.2 is authoritative here. */
    if(g.state!=RAF_CI_G4_REVIEW)return g;
    while(off<n){
        unsigned int e=off,b=off,indent;
        while(e<n && p[e]!='\n')++e;
        while(b<e && (p[b]==' '||p[b]=='\t'))++b;
        indent=b-off;
        if(b<e && p[b]!='#'){
            if(indent==0u){
                if(step && acts!=1u && acts!=2u)
                    why|=(acts==3u?RAF_CI_G43_STEP_CONFLICT:RAF_CI_G43_STEP_MISSING);
                step=acts=steps=seen_steps=active=0u;
                scope=raf_ci_g4_colkey(p+b,e-b,"jobs",4u)?2u:
                      raf_ci_g4_colkey(p+b,e-b,"on",2u)?1u:0u;
                if(scope==1u && raf_ci_g4_value(p+b,e-b,2u) &&
                   raf_ci_g43_bad_scalar(p+b,e-b,2u))why|=RAF_CI_G43_BAD_EVENT;
            }else if(scope==2u){
                if(indent==2u && raf_ci_g4_job_key(p+off,e-off)){
                    unsigned int j,k=b+2u;
                    if(step && acts!=1u && acts!=2u)
                        why|=(acts==3u?RAF_CI_G43_STEP_CONFLICT:RAF_CI_G43_STEP_MISSING);
                    step=acts=steps=seen_steps=0u;active=1u;
                    while(k<e && p[k]!=':')++k;
                    if(js>=RAF_CI_G43_MAX_JOBS)why|=RAF_CI_G43_UNSUPPORTED;
                    else {
                        for(j=0u;j<js;++j){
                            unsigned int x=0u,len=k-(b+2u);
                            if(job_len[j]!=len)continue;
                            while(x<len && p[job_off[j]+x]==p[b+2u+x])++x;
                            if(x==len){why|=RAF_CI_G43_DUPLICATE_JOB;break;}
                        }
                        job_off[js]=b+2u;job_len[js]=k-(b+2u);++js;
                    }
                }else if(active && indent==4u){
                    if(step && acts!=1u && acts!=2u)
                        why|=(acts==3u?RAF_CI_G43_STEP_CONFLICT:RAF_CI_G43_STEP_MISSING);
                    step=acts=0u;
                    if(raf_ci_g4_colkey(p+b,e-b,"steps",5u)){
                        if(seen_steps)why|=RAF_CI_G43_DUP_STEPS;
                        seen_steps=1u;steps=1u;
                        if(raf_ci_g43_bad_scalar(p+b,e-b,5u)==0)
                            why|=RAF_CI_G43_UNSUPPORTED; /* inline YAML list */
                    }else if(raf_ci_g4_colkey(p+b,e-b,"runs-on",7u)){
                        if(raf_ci_g43_bad_scalar(p+b,e-b,7u))why|=RAF_CI_G43_BAD_RUNNER;
                    }else if(raf_ci_g4_colkey(p+b,e-b,"uses",4u)){
                        if(raf_ci_g43_bad_scalar(p+b,e-b,4u))why|=RAF_CI_G43_BAD_ACTION;
                    }
                }else if(active && steps && indent==6u && p[b]=='-' && b+1u<e && p[b+1u]==' '){
                    if(step && acts!=1u && acts!=2u)
                        why|=(acts==3u?RAF_CI_G43_STEP_CONFLICT:RAF_CI_G43_STEP_MISSING);
                    step=1u;acts=0u;
                    if(raf_ci_g4_colkey(p+b+2u,e-b-2u,"run",3u)){
                        if(raf_ci_g43_bad_scalar(p+b+2u,e-b-2u,3u))why|=RAF_CI_G43_BAD_ACTION;
                        else acts|=1u;
                    }else if(raf_ci_g4_colkey(p+b+2u,e-b-2u,"uses",4u)){
                        if(raf_ci_g43_bad_scalar(p+b+2u,e-b-2u,4u))why|=RAF_CI_G43_BAD_ACTION;
                        else acts|=2u;
                    }
                }else if(active && steps && step && indent==8u){
                    if(raf_ci_g4_colkey(p+b,e-b,"run",3u)){
                        if(raf_ci_g43_bad_scalar(p+b,e-b,3u))why|=RAF_CI_G43_BAD_ACTION;
                        else acts|=1u;
                    }else if(raf_ci_g4_colkey(p+b,e-b,"uses",4u)){
                        if(raf_ci_g43_bad_scalar(p+b,e-b,4u))why|=RAF_CI_G43_BAD_ACTION;
                        else acts|=2u;
                    }
                }
            }
        }
        off=e<n?e+1u:n;
    }
    if(step && acts!=1u && acts!=2u)
        why|=(acts==3u?RAF_CI_G43_STEP_CONFLICT:RAF_CI_G43_STEP_MISSING);
    if(why){
        /* Existing bit: all unknown topology is UNSUPPORTED; RCG4 ABI intact. */
        g.gaps|=RAF_CI_G4_GAP_UNSUPPORTED;
        g.state=RAF_CI_G4_HOLD;
    }
    if(findings)*findings=why;
    return g;
}
#endif
