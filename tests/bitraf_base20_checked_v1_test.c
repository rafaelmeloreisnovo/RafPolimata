/* Exact-source hosted C test; physical ARM and FIAT64/base60 NOT_RUN. */
#include <stdio.h>
#include "../rafaelia/bitraf_base20_checked_v1.h"
#include "../rafaelia/bitraf_evidence_planes_v1.h"
static int fail(const char *what,unsigned i){
    fprintf(stderr,"B20C_FAIL:%s:%u\n",what,i);return 1;
}
int main(void){
    bitraf_u32 i;
    for(i=0u;i<BITRAF_STATES_TOTAL;++i){
        char out[4]={'?','?','?','?'};
        char s_out[4]={'?','?','?','?'};
        bitraf_u16 back=0xFFFFu;
        bitraf_u8 d0=0u,d1=0u,d2=0u;
        bitraf_state s=bitraf_from_index8000((bitraf_u16)i);
        if(raf_b20c_encode(i,out)!=RAF_B20C_OK || out[3]!='\0')
            return fail("encode",i);
        if(raf_b20c_decode(out,3u,&back)!=RAF_B20C_OK || back!=i)
            return fail("roundtrip",i);
        if(raf_b20c_from_state(s,s_out)!=RAF_B20C_OK)
            return fail("state",i);
        if(out[0]!=s_out[0] || out[1]!=s_out[1] ||
           out[2]!=s_out[2] || out[3]!=s_out[3])
            return fail("state_match",i);
        if(raf_ev3_index_to_digits20((bitraf_u16)i,&d0,&d1,&d2)!=RAF_EV3_OK ||
           out[0]!=bitraf_base20_digit(d0) ||
           out[1]!=bitraf_base20_digit(d1) ||
           out[2]!=bitraf_base20_digit(d2))
            return fail("ev3_digits",i);
    }
    {
        char keep[4]={'X','X','X','X'};
        bitraf_u16 back=0xA55Au,legacy=0u;
        bitraf_state invalid={10u,0u,0u,0u,0u};
        if(raf_b20c_encode(8000u,keep)!=RAF_B20C_ERR_INPUT ||
           keep[0]!='X' || keep[3]!='X')
            return fail("invalid8000",0u);
        if(raf_b20c_encode(65535u,keep)!=RAF_B20C_ERR_INPUT ||
           keep[1]!='X')return fail("invalid65535",0u);
        if(raf_b20c_encode(0u,(char *)0)!=RAF_B20C_ERR_OUTPUT)
            return fail("null_encode",0u);
        if(raf_b20c_from_state(invalid,keep)!=RAF_B20C_ERR_INPUT ||
           keep[2]!='X')return fail("invalid_state",0u);
        if(raf_b20c_decode("a00",3u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("alias",0u);
        if(bitraf_from_base20_3("a00",&legacy)!=0u || legacy!=4000u)
            return fail("legacy_unchanged",0u);
        if(raf_b20c_decode("K00",3u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("invalid_glyph",0u);
        if(raf_b20c_decode("000",2u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("short",0u);
        if(raf_b20c_decode("000",4u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("long",0u);
        if(raf_b20c_decode("00/",3u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("punctuation",0u);
        if(raf_b20c_decode((const char *)0,3u,&back)!=RAF_B20C_ERR_INPUT ||
           back!=0xA55Au)return fail("null_decode",0u);
        if(raf_b20c_decode("000",3u,(bitraf_u16 *)0)!=RAF_B20C_ERR_OUTPUT)
            return fail("null_out",0u);
        if(raf_b20c_decode("000",3u,&back)!=RAF_B20C_OK || back!=0u)
            return fail("zero",0u);
        if(raf_b20c_decode("JJJ",3u,&back)!=RAF_B20C_OK || back!=7999u)
            return fail("max",0u);
    }
    puts("BITRAF_B20C=PASS_SCOPED");
    puts("EXHAUSTIVE=8000 STRICT_CANONICAL=YES INVALID=REJECTED");
    puts("FIAT64_BASE60=TOKEN_VAZIO ORIGINAL_TERMUX=NOT_RUN");
    puts("CLAIM_ALLOWED=false PHYSICAL_ARM32=TOKEN_VAZIO");
    return 0;
}
