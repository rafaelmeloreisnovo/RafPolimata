#include "../include/raf_fs_jmv.h"
/* Hosted synthetic harness only. The include/ kernel stays freestanding.
 * CLOSURE_L11: synthetic evidence is scoped; CLOSURE_L12: physical proof stays open.
 */
#define CHECK(c, n) if (!(c)) return (n)
#define STEP() raf_fs_jmv_step(t, v, o, x, n, &used, state, &tick, &idx, &voice, &value, &mix)

int main(void)
{
    const raf_u32 t[7] = {4u, 1u, 2u, 1u, 3u, 3u, 4u};
    const raf_u8 v[7] = {0u, 1u, 0u, 0u, 0u, 1u, 0u};
    const raf_u8 o[7] = {RAF_FS_JMV_GAIN, RAF_FS_JMV_NOTE,
        RAF_FS_JMV_NOTE, RAF_FS_JMV_NOTE, RAF_FS_JMV_DELTA,
        RAF_FS_JMV_REST, RAF_FS_JMV_DELTA};
    const raf_i32 x[7] = {2, 60, 10, 20, 3, 0, 7};
    raf_u32 n = 7u, used = 0u, tick = 0u, idx = 0u, voice = 0u;
    raf_i32 value = 0, mix = 0, state[8] = {0,0,0,0,0,0,0,0};
    const raf_u32 expected_order[7] = {3u, 1u, 2u, 4u, 5u, 0u, 6u};
    raf_u32 k;
    for (k=0u; k<7u; ++k) {
        CHECK(STEP() == RAF_FS_JMV_STEP, 1);
        CHECK(idx == expected_order[k], 2);
        CHECK((used & ((raf_u32)1u << idx)) != 0u, 3);
    }
    CHECK(STEP() == RAF_FS_JMV_DONE, 4);
    CHECK(used == 127u && state[0] == 33 && state[1] == 0 && mix == 33, 5);
    CHECK(tick == 4u && voice == 0u && value == 33, 6);
    CHECK(raf_fs_jmv_step((const raf_u32*)0,(const raf_u8*)0,
        (const raf_u8*)0,(const raf_i32*)0,0u,&used,state,
        &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_BAD_INPUT, 7);
    used = 0; n = 33;
    CHECK(STEP() == RAF_FS_JMV_BAD_INPUT && used == 0, 8);
    n = 0;
    CHECK(STEP() == RAF_FS_JMV_DONE && used == 0, 9);
    n = 7; used = 1u << 10;
    CHECK(STEP() == RAF_FS_JMV_BAD_INPUT && used == (1u << 10), 10);
    used = 0;
    {
        const raf_u8 bad_op[1] = {255};
        const raf_u32 one_tick[1] = {0};
        const raf_u8 one_v[1] = {0};
        const raf_i32 one_x[1] = {0};
        CHECK(raf_fs_jmv_step(one_tick,one_v,bad_op,one_x,1,&used,state,
            &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_BAD_TAPE, 11);
        CHECK(used == 0 && state[0] == 33, 12);
    }
    {
        const raf_u32 one_tick[1] = {0};
        const raf_u8 one_v[1] = {0};
        const raf_u8 one_op[1] = {RAF_FS_JMV_DELTA};
        const raf_i32 one_x[1] = {2147483647};
        CHECK(raf_fs_jmv_step(one_tick,one_v,one_op,one_x,1,&used,state,
            &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_STEP, 13);
        CHECK(state[0] == 2147483647 && mix == 2147483647, 14);
        used = 0; state[0] = (-2147483647 - 1);
        {
            const raf_i32 negative[1] = {-2147483647};
            CHECK(raf_fs_jmv_step(one_tick,one_v,one_op,negative,1,&used,state,
                &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_STEP, 15);
            CHECK(state[0] == (-2147483647 - 1), 16);
        }
    }
    {
        const raf_u32 ticks2[2] = {0,0};
        const raf_u8 voices2[2] = {1,0};
        const raf_u8 ops2[2] = {RAF_FS_JMV_NOTE,RAF_FS_JMV_NOTE};
        const raf_i32 vals2[2] = {70,12};
        used = 0;
        CHECK(raf_fs_jmv_step(ticks2,voices2,ops2,vals2,2,&used,state,
            &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_STEP && idx==1,17);
        CHECK(raf_fs_jmv_step(ticks2,voices2,ops2,vals2,2,&used,state,
            &tick,&idx,&voice,&value,&mix) == RAF_FS_JMV_STEP && idx==0,18);
    }
    CHECK(RAF_FS_JMV_TOKEN_VAZIO != 0 && RAF_FS_JMV_TOKEN_VAZIO != RAF_FS_JMV_DONE,19);
    /* Falsify the 32nd bit boundary and validate all tape entries before commit. */
    {
        raf_u32 tt[32], j;
        raf_u8 vv[32], oo[32];
        raf_i32 xx[32];
        for (j=0u; j<32u; ++j) {
            tt[j]=1u; vv[j]=0u; oo[j]=RAF_FS_JMV_NOTE; xx[j]=0;
        }
        used=0x7fffffffu;
        CHECK(raf_fs_jmv_step(tt,vv,oo,xx,32u,&used,state,
            &tick,&idx,&voice,&value,&mix)==RAF_FS_JMV_STEP && idx==31u,20);
        CHECK(used==0xffffffffu,21);
        CHECK(raf_fs_jmv_step(tt,vv,oo,xx,32u,&used,state,
            &tick,&idx,&voice,&value,&mix)==RAF_FS_JMV_DONE,22);
        used=0u; oo[31]=255u;
        CHECK(raf_fs_jmv_step(tt,vv,oo,xx,32u,&used,state,
            &tick,&idx,&voice,&value,&mix)==RAF_FS_JMV_BAD_TAPE,23);
        CHECK(used==0u,24);
    }
    return 0;
}
