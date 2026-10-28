/* Deterministic hostile bytes, invariant-only — not comprehensive fuzz. */
#include "raf_ci_gate_v4.h"
#define CHECK(x) if(!(x))return __LINE__
static unsigned char buf[4096];
static int check(void){
    unsigned int x=0x126a73efu,i,j,len;
    raf_ci_gate_v4 g;
    for(i=0u;i<10000u;++i){
        x^=x<<13u;x^=x>>17u;x^=x<<5u;
        len=x%4096u;
        for(j=0u;j<len;++j){x^=x<<13u;x^=x>>17u;x^=x<<5u;buf[j]=(unsigned char)(x>>24u);}
        g=raf_ci_gate_v4_check(buf,len);
        CHECK(g.schema==4u && g.voids==127u);
        CHECK(g.state==RAF_CI_G4_REVIEW || g.state==RAF_CI_G4_HOLD || g.state==RAF_CI_G4_INVALID);
        CHECK(g.state!=RAF_CI_G4_REVIEW || g.gaps==0u);
    }
    g=raf_ci_gate_v4_check(0u,0xffffffffu);
    CHECK(g.state==RAF_CI_G4_INVALID && (g.gaps&RAF_CI_G4_GAP_INPUT));
    return 0;
}
int main(void){return check();}
