/* AAPCS64 leaf: q16=(a*b)>>16. No stack, no syscall, no data access. */
.section .text.raf.q16mul,"ax",%progbits
.global raf_q16_mul_arch
.hidden raf_q16_mul_arch
.type raf_q16_mul_arch,%function
raf_q16_mul_arch:
    smull x2, w0, w1
    asr   x0, x2, #16
    ret
.size raf_q16_mul_arch, .-raf_q16_mul_arch
.section .note.GNU-stack,"",%progbits
