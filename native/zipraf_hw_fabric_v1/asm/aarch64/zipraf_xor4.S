/* RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: provenance notice only; applicable law controls.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */
.text
.align 2
.global zipraf_xor4_u32_aarch64
.type zipraf_xor4_u32_aarch64, %function
zipraf_xor4_u32_aarch64:
    eor w0, w0, w1
    eor w0, w0, w2
    eor w0, w0, w3
    ret
.size zipraf_xor4_u32_aarch64, .-zipraf_xor4_u32_aarch64
