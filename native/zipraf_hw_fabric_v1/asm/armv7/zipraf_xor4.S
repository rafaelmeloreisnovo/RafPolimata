/* RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: provenance notice only; applicable law controls.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */
.syntax unified
.text
.align 2
.global zipraf_xor4_u32_armv7
.type zipraf_xor4_u32_armv7, %function
zipraf_xor4_u32_armv7:
    eor r0, r0, r1
    eor r0, r0, r2
    eor r0, r0, r3
    bx lr
.size zipraf_xor4_u32_armv7, .-zipraf_xor4_u32_armv7
