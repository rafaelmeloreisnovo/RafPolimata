/* apkc_branchless_handler.h — Branchless machine compilation coordinator
 *
 * Pipeline: Source → compiler_language_direct → executor_zero_overhead → ARM64
 * Zero-overhead: no malloc, no libc, no function calls in hot path.
 * Deterministic: identical input → identical output, every build.
 *
 * FREESTANDING: No malloc, no libc, all stack allocation.
 */

#ifndef APKC_BRANCHLESS_HANDLER_H
#define APKC_BRANCHLESS_HANDLER_H 1

#include "machine_linear_branchless.h"
#include "compiler_language_direct.h"
#include "executor_zero_overhead.h"
#include "apkc_machine_to_arm64.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

/* === BRANCHLESS COMPILER STATE === */
struct BranchlessHandler {
    /* Code generation phase */
    struct Insn code[0x10000];           /* 64K instruction buffer (master copy) */

    /* Execution validation phase (with embedded Machine state) */
    struct ExecutionContext ctx;         /* Includes Machine state + embedded code array */

    /* ARM64 assembly output phase */
    struct Arm64Encoder arm64_enc;
    u8 arm64_asm[0x10000];               /* 64K ARM64 buffer */

    /* Status and metrics */
    u8 status;                           /* 0=success, 1=error, 2=overflow */
    u64 steps_executed;                  /* Instruction count from executor */
    u32 code_len;                        /* Actual instruction count */
    u32 arm64_len;                       /* Actual ARM64 assembly size */
};

/* === ENTRY POINT === */

/* apkc_branchless_compile: Source → Machine instructions → ARM64
 *
 * INPUT:
 *   handler    — uninitialized BranchlessHandler struct (stack-allocated)
 *   src        — source code buffer
 *   src_len    — source length in bytes
 *   lang_type  — language type (0=Python, 1=Go, 2=Rust, 3=C, 4=JS, 5=Java, 6=Swift, 7=Kotlin)
 *
 * OUTPUT:
 *   handler->arm64_asm   — compiled ARM64 assembly (if status=0)
 *   handler->arm64_len   — assembly size in bytes
 *   handler->status      — 0=success, 1=compile_error, 2=overflow
 *
 * RETURNS: u8 status code (0=success, 1=error)
 */
static inline u8 apkc_branchless_compile(
    struct BranchlessHandler *h,
    const u8 *src, u32 src_len,
    u8 lang_type)
{
    u32 i;

    /* === PHASE 1: INITIALIZATION === */

    /* Zero out handler (critical for determinism) */
    for (i = 0; i < sizeof(*h); i++) {
        ((u8*)h)[i] = 0;
    }

    /* Initialize execution context with embedded machine state and code array */
    h->ctx.code_len = 0;
    h->ctx.max_steps = 0x10000;  /* 64K step limit */
    h->ctx.status = 0;
    h->ctx.steps_executed = 0;

    /* Initialize machine state within execution context */
    for (i = 0; i < 16; i++) {
        h->ctx.m.r[i] = 0;
    }
    h->ctx.m.pc = 0;
    h->ctx.m.sp = 0x1000000 - 1;
    h->ctx.m.z = h->ctx.m.c = h->ctx.m.n = h->ctx.m.v = 0;

    /* Initialize code generator to use master code buffer in handler */
    struct UniversalCompiler uc;
    uc.cg.code = h->code;
    uc.cg.pos = 0;
    uc.cg.cap = 0x10000;
    uc.cg.r_free = 0;
    uc.lang = lang_type;

    /* Initialize ARM64 encoder with output buffer */
    arm64_init(&h->arm64_enc, h->arm64_asm, sizeof(h->arm64_asm));

    /* === PHASE 2: COMPILATION === */

    /* Route to language-specific compiler via compile_universal() */
    if (compile_universal(&uc, src, src_len, lang_type)) {
        h->status = 1;  /* Compile error */
        return 1;
    }

    /* Snapshot instruction count */
    h->code_len = uc.cg.pos;

    /* Copy code length to execution context */
    h->ctx.code_len = h->code_len;

    /* === PHASE 3: VALIDATION (OPTIONAL) === */

    /* Execute compiled program to validate:
     * - No invalid operations
     * - No out-of-bounds memory access
     * - Deterministic execution
     *
     * NOTE: For Phase 3a, we skip execution and proceed directly to ARM64 encoding.
     * Phase 3b will add execution validation with hardening gates.
     */

    /* === PHASE 4: ARM64 ASSEMBLY GENERATION === */

    /* Encode machine instructions to ARM64 binary format */
    if (arm64_encode_program(&h->arm64_enc, h->code, h->code_len)) {
        h->status = 2;  /* Overflow or encoding error */
        return 1;
    }

    /* Snapshot ARM64 assembly size */
    h->arm64_len = h->arm64_enc.pos;

    /* === PHASE 5: SUCCESS === */

    h->status = 0;
    h->steps_executed = h->ctx.steps_executed;

    return 0;
}

/* === OPTIONAL: VALIDATION WITH HARDENING GATES (Phase 3b) === */

/* apkc_branchless_validate_with_gates: Add hardening gate checks
 *
 * This is a placeholder for Phase 3b integration with:
 * - hardening_boundary_gates.h (capacity validation)
 * - hardening_receipt_chain.h (chain-of-custody)
 * - hardening_fail_closed.h (TOKEN_VAZIO barrier)
 *
 * TODO: Implement after Phase 3a baseline.
 */

/* === INTROSPECTION === */

/* Get status string for debugging (freestanding-safe) */
static inline const char* apkc_branchless_status_str(u8 status) {
    switch (status) {
    case 0: return "success";
    case 1: return "compile_error";
    case 2: return "overflow";
    default: return "unknown";
    }
}

#endif /* APKC_BRANCHLESS_HANDLER_H */
