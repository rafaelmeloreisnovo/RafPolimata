/* HOSTED falsifier for exact-source HEADER on a GitHub checkout.
 * Compile with: cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic
 *   tests/bitraf_evidence_planes_v1_test.c -o /tmp/raf_ev3_test && /tmp/raf_ev3_test
 * NOT a physical ARM32 runtime or product certification. */
#include <stdio.h>
#include "../rafaelia/bitraf_evidence_planes_v1.h"
#include "../rafaelia/bitraf_w20_bridge_v1.h"
static int fail(const char *s,unsigned x) {fprintf(stderr,"FAIL:%s:%u\n",s,x);return 1;}
int main(void) {
    static raf_ev3_planes db;
    bitraf_u32 i,k,core=0u,shell=0u,records=0u,flips=0u;
    bitraf_u8 d0,d1,d2,mask;
    bitraf_u16 j,h;
    bitraf_u32 word;
    raf_w20_decoded w;
    if (sizeof(db)!=RAF_EV3_PAGES_BYTES) return fail("bytes",sizeof(db));
    if (raf_ev3_zero(&db)) return fail("zero",0u);
    for(i=0u;i<BITRAF_STATES_TOTAL;i++) {
        bitraf_state s=bitraf_from_index8000((bitraf_u16)i);
        bitraf_state r=s;
        if(!bitraf_state_valid(s) || bitraf_index8000(s)!=(bitraf_u16)i)
            return fail("index8000",i);
        if(raf_ev3_index_to_digits20((bitraf_u16)i,&d0,&d1,&d2) ||
           raf_ev3_digits20_to_index(d0,d1,d2,&j) || j!=i)
            return fail("digits20",i);
        if (bitraf_is_core(s)) {
            if(raf_ev3_historical_core_index(s,&h) ||
               h>=BITRAF_STATES_CORE ||
               !bitraf_equal(bitraf_from_core_index4096(h),s))
                return fail("core_index",i);
            core++;
        } else {
            h=0xAAAAu;
            if(!raf_ev3_historical_core_index(s,&h) || h!=0xAAAAu)
                return fail("shell_fail_closed",i);
            shell++;
        }
        for(k=0u;k<4u;k++) r=bitraf_rotate_z90(r);
        if (!bitraf_equal(s,r) ||
            !bitraf_equal(s,bitraf_opposite(bitraf_opposite(s))))
            return fail("geometry_identity",i);
        if (raf_ev3_read(&db,(bitraf_u16)i,&mask) || mask!=0u)
            return fail("unknown_initial",i);
        /* Distinct 8 masks are materialized; 000 remains a deliberate unknown. */
        if ((i%8u)!=0u) {
            if (raf_ev3_observe(&db,(bitraf_u16)i,(bitraf_u8)(i%8u)))
                return fail("observe",i);
        }
        if (raf_ev3_read(&db,(bitraf_u16)i,&mask) || mask!=(i%8u))
            return fail("mask_read",i);
        if (mask!=0u) {
            if (raf_ev3_observe(&db,(bitraf_u16)i,mask) ||
                raf_ev3_read(&db,(bitraf_u16)i,&mask) || mask!=(i%8u))
                return fail("idempotent",i);
        }
        for(k=0u;k<RAF_W20_SUBVALUES;k++) {
            if (raf_w20_pack(s,(bitraf_u8)k,&word) ||
                raf_w20_unpack(word,&w) || !bitraf_equal(s,w.state) ||
                w.payload5!=k)
                return fail("w20_compat",i);
            records++;
        }
    }
    if(core!=BITRAF_STATES_CORE || shell!=BITRAF_STATES_SHELL ||
       records!=RAF_W20_STATES) return fail("counts",core);
    /* Negative: invalid address, no mutation, 0 mask != explicitly observed absence */
    mask=0xAAu;
    if(!raf_ev3_read(&db,8000u,&mask) || mask!=0xAAu ||
       !raf_ev3_observe(&db,8000u,RAF_EV3_POSITIVE) ||
       !raf_ev3_observe(&db,0u,0u) ||
       !raf_ev3_digits20_to_index(20u,0u,0u,&j))
        return fail("bad_input_rejection",0u);
    if (raf_ev3_read(&db,0u,&mask) || mask!=0u)
        return fail("unknown_not_absence",0u);
    /* Negative: a valid original word with two flips in V8 can be accepted. */
    {
        bitraf_state s={0u,0u,0u,0u,0u};
        if (raf_w20_pack(s,0u,&word)) return fail("w20_origin",0u);
        if (raf_w20_unpack(word^(1u<<2u)^(1u<<3u),&w)!=0u ||
            (bitraf_equal(w.state,s) && w.payload5==0u))
            return fail("two_bit_counterexample_missing",0u);
        for(i=0u;i<20u;i++) {
            if(raf_w20_unpack(word^(1u<<i),&w)==0u)
                return fail("one_bit_not_rejected",i);
            flips++;
        }
    }
    /* Monotonic evidence: append opposite evidence; never overwrite/clear. */
    if(raf_ev3_observe(&db,1u,RAF_EV3_POSITIVE|RAF_EV3_NEGATIVE) ||
       raf_ev3_read(&db,1u,&mask) || mask!=7u)
        return fail("contradiction_preserved",mask);
    printf("RAFAELIA_EV3=PASS_LOCAL_SCOPED\n");
    printf("BASE=%u CORE=%u SHELL=%u W20_ROUNDTRIP=%u SINGLE_FLIPS=%u\n",
           (unsigned)BITRAF_STATES_TOTAL,(unsigned)core,(unsigned)shell,
           (unsigned)records,(unsigned)flips);
    puts("PNA_STATES=8 DOUBLE_FLIP_COUNTEREXAMPLE=OBSERVED");
    puts("CLAIM_ALLOWED=false CI=TOKEN_VAZIO ARM32_DEVICE=TOKEN_VAZIO");
    return 0;
}
