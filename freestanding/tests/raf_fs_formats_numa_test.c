#include "../include/raf_fs_elf_view.h"
#include "../include/raf_fs_dex_view.h"
#include "../include/raf_fs_numa_plan.h"
#define CHECK(expr, id) if (!(expr)) return (id)
static void w16(raf_u8 *p, raf_u32 v, raf_u32 be) {
    p[0]=(raf_u8)(be?(v>>8):v); p[1]=(raf_u8)(be?v:(v>>8));
}
static void w32(raf_u8 *p, raf_u32 v, raf_u32 be) {
    p[0]=(raf_u8)(be?(v>>24):v);
    p[1]=(raf_u8)(be?(v>>16):(v>>8));
    p[2]=(raf_u8)(be?(v>>8):(v>>16));
    p[3]=(raf_u8)(be?v:(v>>24));
}
static void w64(raf_u8 *p, raf_u64 v, raf_u32 be) {
    if (be) { w32(p,(raf_u32)(v>>32),be); w32(p+4,(raf_u32)v,be); }
    else { w32(p,(raf_u32)v,be); w32(p+4,(raf_u32)(v>>32),be); }
}
static void elf_fixture(raf_u8 *b,raf_u32 is64,raf_u32 be) {
    raf_u32 i;
    for(i=0;i<192u;++i) b[i]=0;
    b[0]=127;b[1]='E';b[2]='L';b[3]='F';b[4]=(raf_u8)(is64?2:1);b[5]=(raf_u8)(be?2:1);b[6]=1;
    w16(b+16,2u,be);w16(b+18,183u,be);w32(b+20,1u,be);
    if(is64){
        w64(b+24,4096u,be);w64(b+32,64u,be);w16(b+52,64u,be);
        w16(b+54,56u,be);w16(b+56,1u,be);
        w32(b+64,1u,be);w32(b+68,5u,be);w64(b+72,128u,be);
        w64(b+80,4096u,be);w64(b+96,16u,be);w64(b+104,32u,be);w64(b+112,16u,be);
    } else {
        w32(b+24,4096u,be);w32(b+28,52u,be);w16(b+40,52u,be);
        w16(b+42,32u,be);w16(b+44,1u,be);
        w32(b+52,1u,be);w32(b+56,128u,be);w32(b+60,4096u,be);
        w32(b+68,16u,be);w32(b+72,32u,be);w32(b+80,16u,be);
    }
}
static void dex_fixture(raf_u8 *d) {
    raf_u32 i;
    for(i=0;i<160u;++i) d[i]=0;
    d[0]='d';d[1]='e';d[2]='x';d[3]='\n';d[4]='0';d[5]='3';d[6]='9';
    w32(d+32,140u,0);w32(d+36,112u,0);w32(d+40,0x12345678u,0);
    w32(d+52,112u,0);w32(d+104,28u,0);w32(d+108,112u,0);
    w32(d+112,2u,0); /* two map items: HEADER_ITEM and MAP_LIST */
    w16(d+116,0u,0);w32(d+120,1u,0);w32(d+124,0u,0);
    w16(d+128,0x1000u,0);w32(d+132,1u,0);w32(d+136,112u,0);
}
static void numa_fixture(raf_fs_numa_snapshot *s) {
    raf_u32 i,j;
    s->node_count=3u;s->allowed_mask=7u;s->origin=0u;
    for(i=0;i<8u;++i) {s->capacity_pages[i]=0u;s->used_pages[i]=0u;}
    for(i=0;i<64u;++i) s->distance[i]=0u;
    for(i=0;i<3u;++i) for(j=0;j<3u;++j) s->distance[i*8u+j]=(raf_u8)(i==j?10u:(i==0u&&j==1u?20u:30u));
    s->capacity_pages[0]=20u;s->capacity_pages[1]=100u;s->capacity_pages[2]=100u;
    s->used_pages[0]=11u;s->used_pages[1]=5u;s->used_pages[2]=10u;
}

/* Adversarial model-property suite: fixed seed, no external corpus.
 * CLOSURE_L11: rights/provenance remain typed unknown.
 * CLOSURE_L12: structural view != ELF loader, ART VM, or NUMA placement.
 * Metamorphic checks reject corruptions and verify outputs are commit-on-success.
 */
static raf_u32 doom_formats_rng(raf_u32 *seed)
{
    *seed=(*seed * 1664525u)+1013904223u;
    return *seed;
}
static int doom_formats_properties(void)
{
    raf_u32 trial, seed=0x454C4644u;
    for(trial=0u;trial<256u;++trial) {
        raf_u32 rnd=doom_formats_rng(&seed);
        raf_u32 cls=trial&1u, be=(trial>>1)&1u, selected=77u;
        raf_u8 b[192], d[160];
        raf_fs_elf_view ev;
        raf_fs_dex_view dv;
        raf_fs_numa_snapshot topo;
        /* Valid synthetic ELF remains structurally accepted in both endian classes. */
        elf_fixture(b,cls,be);
        if(raf_fs_elf_inspect(b,144u,&ev)!=RAF_FS_ELF_OK ||
           ev.word_class!=(cls?64u:32u) || ev.big_endian!=be ||
           ev.machine!=183u || ev.load_count!=1u) return 101;
        /* Malformed magic cannot mutate a previously validated output. */
        b[(raf_usize)(rnd&3u)]=(raf_u8)(rnd>>16);
        /* Guaranteed corruption, independent of randomized byte coincidence. */
        b[0]=0u;
        if(raf_fs_elf_inspect(b,144u,&ev)!=RAF_FS_ELF_BAD ||
           ev.entry!=4096u || ev.load_count!=1u) return 102;
        elf_fixture(b,cls,be);
        if(cls) w64(b+72,0xfffffffffffffff0ULL,be);
        else w32(b+56,0xfffffff0u,be);
        if(raf_fs_elf_inspect(b,144u,&ev)!=RAF_FS_ELF_BAD ||
           ev.entry!=4096u) return 103;
        elf_fixture(b,cls,be);
        if(cls) {
            w64(b+80,0xfffffffffffffff0ULL,be);
            if(raf_fs_elf_inspect(b,144u,&ev)!=RAF_FS_ELF_BAD ||
               ev.entry!=4096u) return 104;
        }

        dex_fixture(d);
        if(raf_fs_dex_inspect(d,140u,&dv)!=RAF_FS_DEX_OK ||
           dv.version!=39u || dv.map_items!=2u) return 105;
        w32(d+112,3u+(rnd&1023u),0u);
        if(raf_fs_dex_inspect(d,140u,&dv)!=RAF_FS_DEX_BAD ||
           dv.version!=39u || dv.map_items!=2u) return 106;
        dex_fixture(d);
        w16(d+116,0x1000u,0u); /* duplicate forged map_list */
        if(raf_fs_dex_inspect(d,140u,&dv)!=RAF_FS_DEX_BAD ||
           dv.version!=39u || dv.map_items!=2u) return 107;
        dex_fixture(d);
        d[6]='6'; /* 036 unsupported; never silently accepted as 035 */
        if(raf_fs_dex_inspect(d,140u,&dv)!=RAF_FS_DEX_UNSUPPORTED ||
           dv.version!=39u) return 108;

        numa_fixture(&topo);
        if(raf_fs_numa_choose(&topo,1u+(rnd%9u),&selected)!=RAF_FS_NUMA_SELECTED ||
           selected!=0u) return 109;
        topo.used_pages[2]=101u; /* invalid unused node must still fail */
        if(raf_fs_numa_choose(&topo,1u+(rnd%9u),&selected)!=RAF_FS_NUMA_BAD_INPUT ||
           selected!=0u) return 110;
        numa_fixture(&topo);
        topo.allowed_mask=0u;
        if(raf_fs_numa_choose(&topo,1u+(rnd%9u),&selected)!=RAF_FS_NUMA_NO_ALLOWED ||
           selected!=0u) return 111;
    }
    return 0;
}

int main(void) {
    raf_u8 b[192], d[160];
    raf_fs_elf_view e={0,0,0,0,0,0};
    raf_fs_dex_view dv={0,0,0,0,0};
    raf_fs_numa_snapshot topo;
    raf_u32 i, v=77u;
    for(i=0;i<2u;++i) {
        raf_u32 be;
        for(be=0u;be<2u;++be) {
            elf_fixture(b,i,be);
            CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_OK,1);
            CHECK(e.word_class==(i?64u:32u)&&e.big_endian==be&&e.load_count==1u,2);
            CHECK(e.machine==183u&&e.entry==4096u,3);
            CHECK(raf_fs_elf_inspect(b,50u,&e)==RAF_FS_ELF_BAD,4);
            b[0]=0u;CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,5);
            elf_fixture(b,i,be);
            if(i) { w64(b+96,1000u,be); }
            else { w32(b+68,1000u,be); }
            CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,6);
            elf_fixture(b,i,be);
            if(i) { w64(b+112,3u,be); }
            else { w32(b+80,3u,be); }
            CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,7);
            elf_fixture(b,i,be);
            if(i) { w64(b+32,0xfffffffffffffff0ULL,be); }
            else { w32(b+28,0xfffffff0u,be); }
            CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,8);
            elf_fixture(b,i,be);
            if(i) {w64(b+40,120u,be);w16(b+58,64u,be);w16(b+60,1u,be);}
            else {w32(b+32,120u,be);w16(b+46,40u,be);w16(b+48,1u,be);}
            CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_UNSUPPORTED ||
                  raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,9);
        }
    }
    CHECK(raf_fs_elf_inspect((const raf_u8*)0,144u,&e)==RAF_FS_ELF_BAD,10);
    dex_fixture(d);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_OK,11);
    CHECK(dv.version==39u&&dv.map_items==2u&&dv.data_offset==112u,12);
    CHECK(raf_fs_dex_inspect(d,139u,&dv)==RAF_FS_DEX_BAD,13);
    d[5]='4';d[6]='1';
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_UNSUPPORTED,14);
    dex_fixture(d);w32(d+40,0x78563412u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,15);
    dex_fixture(d);w32(d+112,0xffffffffu,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,16);
    dex_fixture(d);w32(d+56,1u,0);w32(d+60,136u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,17);
    dex_fixture(d);w32(d+136,141u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,18);
    CHECK(raf_fs_dex_inspect((const raf_u8*)0,140u,&dv)==RAF_FS_DEX_BAD,19);
    dex_fixture(d); w16(d+128,0x2000u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,31);
    dex_fixture(d); w32(d+136,116u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,32);
    dex_fixture(d); w16(d+130,1u,0);
    CHECK(raf_fs_dex_inspect(d,140u,&dv)==RAF_FS_DEX_BAD,33);
    elf_fixture(b,1u,0u);
    w64(b+80,0xfffffffffffffff0ULL,0u);
    CHECK(raf_fs_elf_inspect(b,144u,&e)==RAF_FS_ELF_BAD,34);
    numa_fixture(&topo);
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_SELECTED&&v==0u,20);
    CHECK(raf_fs_numa_choose(&topo,15u,&v)==RAF_FS_NUMA_SELECTED&&v==1u,21);
    CHECK(raf_fs_numa_choose(&topo,200u,&v)==RAF_FS_NUMA_NO_FIT&&v==1u,22);
    topo.allowed_mask=0u;
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_NO_ALLOWED&&v==1u,23);
    topo.allowed_mask=0x100u;
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_BAD_INPUT&&v==1u,24);
    numa_fixture(&topo);topo.used_pages[1]=101u;
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_BAD_INPUT&&v==1u,25);
    numa_fixture(&topo);topo.distance[0]=9u;
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_BAD_INPUT&&v==1u,26);
    numa_fixture(&topo);
    CHECK(raf_fs_numa_choose(&topo,0u,&v)==RAF_FS_NUMA_BAD_INPUT&&v==1u,27);
    CHECK(raf_fs_numa_choose((const raf_fs_numa_snapshot*)0,4u,&v)==RAF_FS_NUMA_BAD_INPUT,28);
    topo.node_count=0u;
    CHECK(raf_fs_numa_choose(&topo,5u,&v)==RAF_FS_NUMA_TOPOLOGY_UNKNOWN&&v==1u,29);
    CHECK(RAF_FS_NUMA_TOPOLOGY_UNKNOWN!=0u&&RAF_FS_NUMA_NO_ALLOWED!=RAF_FS_NUMA_NO_FIT,30);
    { int doom=doom_formats_properties(); if(doom) return doom; }
    return 0;
}
