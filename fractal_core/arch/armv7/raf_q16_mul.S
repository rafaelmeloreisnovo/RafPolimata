/* AAPCS32 leaf: q16=(a*b)>>16. No stack, no syscall, no data access. */
.syntax unified
.arm
.section .text.raf.q16mul,"ax",%progbits
.global raf_q16_mul_arch
.hidden raf_q16_mul_arch
.type raf_q16_mul_arch,%function
raf_q16_mul_arch:
    smull r2, r3, r0, r1
    mov   r0, r2, lsr #16
    orr   r0, r0, r3, lsl #16
    bx    lr
.size raf_q16_mul_arch, .-raf_q16_mul_arch
.section .note.GNU-stack,"",%progbits
