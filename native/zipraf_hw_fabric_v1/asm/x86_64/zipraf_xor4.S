/* RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: provenance notice only; applicable law controls.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */
.text
.global zipraf_xor4_u32_x86_64
.type zipraf_xor4_u32_x86_64, @function
zipraf_xor4_u32_x86_64:
    movl %edi, %eax
    xorl %esi, %eax
    xorl %edx, %eax
    xorl %ecx, %eax
    ret
.size zipraf_xor4_u32_x86_64, .-zipraf_xor4_u32_x86_64
