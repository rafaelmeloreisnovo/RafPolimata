#include "raf_ci_gate_v43.h"
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
static unsigned char buf[2048];
static unsigned int rnd(unsigned int *s){*s^=*s<<13u;*s^=*s>>17u;*s^=*s<<5u;return *s;}
static int run(void){
    unsigned int s=0x2a1e5c17u,i,j,len,why;
    const unsigned char alphabet[]="on: push\njobs\t  build:  steps - uses run\n#q0123456789[]{}\r";
    for(i=0u;i<10000u;++i){
        len=rnd(&s)%sizeof(buf);
        for(j=0u;j<len;++j)buf[j]=alphabet[rnd(&s)%(sizeof(alphabet)-1u)];
        if(i%31u==0u && len>3u)buf[len/2u]=0xffu;
        {raf_ci_gate_v4 a=raf_ci_gate_v4_check(buf,len);
         raf_ci_gate_v4 b=raf_ci_gate_v43_check(buf,len,&why);
         CHECK(a.schema==4u && b.schema==4u && a.voids==b.voids && a.source_fingerprint==b.source_fingerprint);
         CHECK(b.state==RAF_CI_G4_HOLD || b.state==RAF_CI_G4_INVALID || b.state==RAF_CI_G4_REVIEW);
         CHECK(!(a.state==RAF_CI_G4_HOLD && b.state!=RAF_CI_G4_HOLD));
         CHECK(!(a.state==RAF_CI_G4_INVALID && b.state!=RAF_CI_G4_INVALID));
         CHECK(!(b.state==RAF_CI_G4_REVIEW && a.state!=RAF_CI_G4_REVIEW));
         CHECK(!why || b.state==RAF_CI_G4_HOLD);
        }
    }
    return 0;
}
int main(void){return run();}
