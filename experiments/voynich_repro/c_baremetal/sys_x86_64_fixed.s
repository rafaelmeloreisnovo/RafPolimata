.text
.global sys_read
.global sys_write
.global sys_openat
.global sys_close
.global sys_exit
sys_read:
    mov $0, %rax
    syscall
    ret
sys_write:
    mov $1, %rax
    syscall
    ret
sys_openat:
    mov %rcx, %r10
    mov $257, %rax
    syscall
    ret
sys_close:
    mov $3, %rax
    syscall
    ret
sys_exit:
    mov $60, %rax
    syscall
    ud2
.section .note.GNU-stack,"",@progbits
