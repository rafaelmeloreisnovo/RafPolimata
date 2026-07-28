/* x86-64 SysV leaf: q16=(a*b)>>16. No stack, no syscall, no data access. */
.section .text.raf.q16mul,"ax",@progbits
.globl raf_q16_mul_arch
.hidden raf_q16_mul_arch
.type raf_q16_mul_arch,@function
raf_q16_mul_arch:
    movslq %edi, %rax
    movslq %esi, %rcx
    imulq  %rcx, %rax
    sarq   $16, %rax
    ret
.size raf_q16_mul_arch, .-raf_q16_mul_arch
.section .note.GNU-stack,"",@progbits
