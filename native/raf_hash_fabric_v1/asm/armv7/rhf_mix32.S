.syntax unified
.text
.align 2
.global rhf_armv7_mix32
.type rhf_armv7_mix32,%function
/* u32 rhf_armv7_mix32(u32 a=r0,u32 b=r1,u32 c=r2) */
rhf_armv7_mix32:
    add     r0, r0, r1
    movw    r3, #0x79b9
    movt    r3, #0x9e37
    add     r0, r0, r3
    eor     r2, r2, r0
    ror     r2, r2, #16
    add     r1, r1, r2
    eor     r0, r0, r1
    ror     r0, r0, #20      /* rotl32(...,12) */
    ror     r1, r1, #25      /* rotl32(...,7)  */
    eor     r0, r0, r2
    eor     r0, r0, r1
    bx      lr
.size rhf_armv7_mix32, .-rhf_armv7_mix32

.global rhf_armv7_lanes32
.type rhf_armv7_lanes32,%function
rhf_armv7_lanes32:
    mov     r1, #1
    cmp     r0, #128
    moveq   r1, #4
    cmp     r0, #256
    moveq   r1, #8
    cmp     r0, #512
    moveq   r1, #16
    mov     r0, r1
    bx      lr
.size rhf_armv7_lanes32, .-rhf_armv7_lanes32
