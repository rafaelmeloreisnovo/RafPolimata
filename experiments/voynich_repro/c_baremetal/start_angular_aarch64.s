.text
.global _start
.extern voy_angular_main
_start:
    mov x29, #0
    mov x30, #0
    ldr x0, [sp]
    add x1, sp, #8
    bl voy_angular_main
    mov x8, #93
    svc #0
.section .note.GNU-stack,"",%progbits
