/* System V x86-64: rdi=current, rsi=value, rdx=mask, rax=result */
.text
.globl zipraf_patch_u64_asm
.type zipraf_patch_u64_asm,@function
zipraf_patch_u64_asm:
    mov %rdi, %rax
    xor %rsi, %rax
    and %rdx, %rax
    xor %rdi, %rax
    ret
.size zipraf_patch_u64_asm, .-zipraf_patch_u64_asm
