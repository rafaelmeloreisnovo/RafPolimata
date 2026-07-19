.text
.align 2
.global _start
.hidden _start
.type _start, %function
_start:
    sub sp, sp, #1, lsl #12
    mov x0, sp
    mov w1, #0x3280
    movk w1, #0x2, lsl #16
    bl raf_omega_init
    mov x0, sp
    mov w1, #3
    mov w2, #64
    bl raf_omega_set_friction
    mov x0, sp
    mov w1, #0x79b9
    movk w1, #0x9e37, lsl #16
    mov w2, #3
    bl raf_omega_step
    mov x0, sp
    bl raf_omega_validate
    cmp w0, #1
    cset w0, ne
    mov x8, #93
    svc #0
.size _start, .-_start
