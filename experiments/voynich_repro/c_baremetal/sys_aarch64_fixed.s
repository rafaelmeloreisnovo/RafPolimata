.text
.global sys_read
.global sys_write
.global sys_openat
.global sys_close
.global sys_exit
sys_read:
    mov x8, #63
    svc #0
    ret
sys_write:
    mov x8, #64
    svc #0
    ret
sys_openat:
    mov x8, #56
    svc #0
    ret
sys_close:
    mov x8, #57
    svc #0
    ret
sys_exit:
    mov x8, #93
    svc #0
1:  b 1b
.section .note.GNU-stack,"",%progbits
