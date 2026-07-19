.syntax unified
.arm
.text
.align 2
.global _start
.hidden _start
.type _start, %function
_start:
    sub sp, sp, #4096
    mov r0, sp
    movw r1, #0x3280
    movt r1, #0x0002
    bl raf_omega_init
    mov r0, sp
    mov r1, #3
    mov r2, #64
    bl raf_omega_set_friction
    mov r0, sp
    movw r1, #0x79b9
    movt r1, #0x9e37
    mov r2, #3
    bl raf_omega_step
    mov r0, sp
    bl raf_omega_validate
    cmp r0, #1
    movne r0, #1
    moveq r0, #0
    mov r7, #1
    svc #0
.size _start, .-_start
