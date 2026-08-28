.text
.align 2
.global rhf_a64_mix32
.type rhf_a64_mix32,%function
/* u32 rhf_a64_mix32(u32 a=w0,u32 b=w1,u32 c=w2) */
rhf_a64_mix32:
    add     w0, w0, w1
    movz    w3, #0x79b9
    movk    w3, #0x9e37, lsl #16
    add     w0, w0, w3
    eor     w2, w2, w0
    ror     w2, w2, #16
    add     w1, w1, w2
    eor     w0, w0, w1
    ror     w0, w0, #20      /* rotl32(...,12) */
    ror     w1, w1, #25      /* rotl32(...,7)  */
    eor     w0, w0, w2
    eor     w0, w0, w1
    ret
.size rhf_a64_mix32, .-rhf_a64_mix32

.global rhf_a64_lanes32
.type rhf_a64_lanes32,%function
/* u8 lanes32(u16 vector_bits) */
rhf_a64_lanes32:
    mov     w1, #1
    mov     w2, #4
    cmp     w0, #128
    csel    w1, w2, w1, eq
    mov     w2, #8
    cmp     w0, #256
    csel    w1, w2, w1, eq
    mov     w2, #16
    cmp     w0, #512
    csel    w0, w2, w1, eq
    ret
.size rhf_a64_lanes32, .-rhf_a64_lanes32
