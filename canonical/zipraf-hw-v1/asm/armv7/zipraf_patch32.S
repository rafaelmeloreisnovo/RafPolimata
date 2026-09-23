/* ARMv7 AAPCS: r0=current, r1=value, r2=mask, r0=result */
.syntax unified
.text
.align 2
.global zipraf_patch_u32_asm
.type zipraf_patch_u32_asm, %function
zipraf_patch_u32_asm:
    eor r3, r0, r1
    and r3, r3, r2
    eor r0, r0, r3
    bx lr
.size zipraf_patch_u32_asm, .-zipraf_patch_u32_asm
