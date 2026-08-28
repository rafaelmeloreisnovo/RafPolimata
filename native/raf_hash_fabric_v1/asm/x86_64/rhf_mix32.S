.text
.globl rhf_x64_mix32
.type rhf_x64_mix32,@function
/* SysV AMD64: u32 a=edi, b=esi, c=edx; return eax */
rhf_x64_mix32:
    movl    %edi, %eax
    addl    %esi, %eax
    addl    $0x9e3779b9, %eax
    xorl    %eax, %edx
    roll    $16, %edx
    addl    %edx, %esi
    xorl    %esi, %eax
    roll    $12, %eax
    roll    $7, %esi
    xorl    %edx, %eax
    xorl    %esi, %eax
    ret
.size rhf_x64_mix32, .-rhf_x64_mix32

.globl rhf_x64_lanes32
.type rhf_x64_lanes32,@function
rhf_x64_lanes32:
    movl    $1, %eax
    movl    $4, %ecx
    cmpl    $128, %edi
    cmove   %ecx, %eax
    movl    $8, %ecx
    cmpl    $256, %edi
    cmove   %ecx, %eax
    movl    $16, %ecx
    cmpl    $512, %edi
    cmove   %ecx, %eax
    ret
.size rhf_x64_lanes32, .-rhf_x64_lanes32
