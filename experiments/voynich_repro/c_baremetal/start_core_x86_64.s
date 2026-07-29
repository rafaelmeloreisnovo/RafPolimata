.text
.global _start
.extern voy_core
_start:
    xor %rbp, %rbp
    mov (%rsp), %rdi
    lea 8(%rsp), %rsi
    call voy_core
    mov %rax, %rdi
    mov $60, %rax
    syscall
.section .note.GNU-stack,"",@progbits
