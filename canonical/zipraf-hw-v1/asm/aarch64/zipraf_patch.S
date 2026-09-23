/* AArch64 PCS: x0=current, x1=value, x2=mask, x0=result */
.text
.global zipraf_patch_u64_asm
.type zipraf_patch_u64_asm, %function
zipraf_patch_u64_asm:
    eor x3, x0, x1
    and x3, x3, x2
    eor x0, x0, x3
    ret
.size zipraf_patch_u64_asm, .-zipraf_patch_u64_asm
