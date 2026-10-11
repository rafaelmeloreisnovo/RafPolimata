#include "../include/raf_fs_virtual_bit_fabric.h"
#define CHECK(expr,code) if (!(expr)) return (code)
static raf_u32 next(raf_u32 *s) { *s=*s*1664525u+1013904223u; return *s; }
int main(void)
{
    raf_fs_vbf f;
    raf_u32 p,i,j,seed=0x20261011u,from,to,src,dst,bit,op,status,expected;
    const raf_u32 profiles[11]={1u,2u,3u,4u,5u,6u,7u,8u,9u,10u,11u};
    CHECK(raf_fs_vbf_init((raf_fs_vbf*)0,1u)==RAF_FS_VBF_BAD_INPUT,1);
    CHECK(raf_fs_vbf_init(&f,0u)==RAF_FS_VBF_BAD_INPUT,2);
    for(p=0u;p<11u;++p) {
        raf_u32 model[8][2]={{0u}};
        CHECK(raf_fs_vbf_init(&f,profiles[p])==RAF_FS_VBF_OK,3);
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,4);
        CHECK(raf_fs_vbf_step(&f,RAF_FS_VBF_INPUT,0u,0u,0u,f.width,1u)==RAF_FS_VBF_BAD_INPUT,5);
        CHECK(f.count==0u,6);
        for(i=0u;i<16u;++i) {
            raf_u32 r=next(&seed);
            to=(r>>9)&7u; from=(r>>5)&7u;
            src=(r>>15)%f.width; dst=(r>>23)%f.width;
            op=(i%3u==0u)?RAF_FS_VBF_INPUT:RAF_FS_VBF_ROUTE;
            bit=(op==RAF_FS_VBF_INPUT)?((r>>30)&1u):0u;
            expected=(op==RAF_FS_VBF_INPUT)?bit:((model[from][src>>5]>>(src&31u))&1u);
            CHECK(raf_fs_vbf_step(&f,op,from,to,src,dst,bit)==RAF_FS_VBF_OK,7);
            model[to][dst>>5]=(model[to][dst>>5]&~((raf_u32)1u<<(dst&31u)))|
                                    (expected?((raf_u32)1u<<(dst&31u)):0u);
            CHECK(f.count==i+1u && f.trace[i].source_value==expected,8);
            CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,9);
            for(j=0u;j<8u;++j) {
                CHECK(f.words[j][0]==model[j][0] && f.words[j][1]==model[j][1],10);
            }
        }
        CHECK(raf_fs_vbf_step(&f,RAF_FS_VBF_ROUTE,0u,1u,0u,0u,0u)==RAF_FS_VBF_TRACE_FULL,11);
        CHECK(f.count==16u,12);
        f.words[0][0]^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_BAD_PARITY,13);
        CHECK(raf_fs_vbf_step(&f,RAF_FS_VBF_ROUTE,0u,1u,0u,0u,0u)==RAF_FS_VBF_BAD_PARITY,14);
        f.words[0][0]^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,15);
        f.words[0][0]^=3u; /* parity cannot detect even flips; state digest must */
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_BAD_DIGEST,16);
        f.words[0][0]^=3u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,17);
        f.trace[0].prior_value^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_BAD_CHAIN,18);
        f.trace[0].prior_value^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,19);
        f.watchdog^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_BAD_WATCHDOG,20);
        f.watchdog^=1u;
        CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,21);
    }
    /* Model-only 8/16/32/64-bit boundary and 63rd-bit path. */
    CHECK(raf_fs_vbf_init(&f,RAF_FS_VBF_PROFILE_X86_64)==RAF_FS_VBF_OK,22);
    CHECK(raf_fs_vbf_step(&f,RAF_FS_VBF_INPUT,0u,1u,0u,63u,1u)==RAF_FS_VBF_OK,23);
    CHECK(raf_fs_vbf_step(&f,RAF_FS_VBF_ROUTE,1u,2u,63u,0u,0u)==RAF_FS_VBF_OK,24);
    CHECK(f.words[2][0]==1u && f.words[1][1]==0x80000000u,25);
    CHECK(raf_fs_vbf_check(&f)==RAF_FS_VBF_OK,26);
    CHECK(raf_fs_vbf_step(&f,99u,1u,2u,0u,0u,0u)==RAF_FS_VBF_BAD_INPUT,27);
    CHECK(f.tick==2u,28);
    status=raf_fs_vbf_check(&f);
    CHECK(status==RAF_FS_VBF_OK,29);
    return 0;
}
