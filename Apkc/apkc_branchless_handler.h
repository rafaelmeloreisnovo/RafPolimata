/* apkc_branchless_handler.h — Branchless machine compilation coordinator
 *
 * Pipeline: Source -> canonical LangProfile -> language-specific frontend
 *          -> linear machine -> ARM64
 * Zero-overhead: no malloc, no libc, no function calls in hot path.
 * Deterministic: identical input -> identical output, every build.
 *
 * FREESTANDING: No malloc, no libc, all stack allocation.
 */

#ifndef APKC_BRANCHLESS_HANDLER_H
#define APKC_BRANCHLESS_HANDLER_H 1

#include "machine_linear_branchless.h"
#include "compiler_language_direct.h"
#include "apkc_language_dispatch.h"
#include "executor_zero_overhead.h"
#include "apkc_machine_to_arm64.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

/* Branchless handler status codes. */
#define APKC_BRANCHLESS_OK                  0u
#define APKC_BRANCHLESS_COMPILE_ERROR       1u
#define APKC_BRANCHLESS_ENCODE_ERROR        2u
#define APKC_BRANCHLESS_UNSUPPORTED_LANG    3u

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
    u8 status;                           /* APKC_BRANCHLESS_* */
    u8 lang_profile_id;                  /* canonical LP_* value from lang_profile.h */
    u8 frontend_kind;                    /* APKC_FRONTEND_* */
    u8 semantic_proof;                   /* APKC_SEMANTIC_*; remains UNPROVEN here */
    u64 steps_executed;                  /* Instruction count from executor */
    u32 code_len;                        /* Actual instruction count */
    u32 arm64_len;                       /* Actual ARM64 assembly size */
};

/* === ENTRY POINT === */

/* apkc_branchless_compile: Source -> language-specific frontend -> Machine -> ARM64
 *
 * INPUT:
 *   handler    — uninitialized BranchlessHandler struct (stack-allocated)
 *   src        — source code buffer
 *   src_len    — source length in bytes
 *   lang_type  — canonical LP_* id from lang_profile.h.  No private numbering.
 *
 * OUTPUT:
 *   handler->arm64_asm   — compiled ARM64 bytes (if status=0)
 *   handler->arm64_len   — assembly size in bytes
 *   handler->status      — APKC_BRANCHLESS_* status
 *   handler->lang_profile_id — exact LP_* identity used for routing
 *   handler->frontend_kind   — language-specific frontend selected
 *   handler->semantic_proof  — remains UNPROVEN until semantic/action gates pass
 *
 * RETURNS: 0 on structural compile+encode success, 1 on rejection/error.
 *
 * IMPORTANT:
 *   status=0 means the active frontend emitted machine code and the ARM64 encoder
 *   accepted it.  It does NOT mean source semantics were proved equivalent.
 */
static inline u8 apkc_branchless_compile(
    struct BranchlessHandler *h,
    const u8 *src, u32 src_len,
    u8 lang_type)
{
    u32 i;

    if (!h) return 1;

    /* === PHASE 1: INITIALIZATION === */

    /* Zero out handler (critical for determinism) */
    for (i = 0; i < sizeof(*h); i++) {
        ((u8*)h)[i] = 0;
    }

    if (!src || src_len == 0) {
        h->status = APKC_BRANCHLESS_COMPILE_ERROR;
        return 1;
    }

    /* G-S1: there is one language identity contract: LP_* from lang_profile.h. */
    if (!apkc_branchless_lang_supported(lang_type)) {
        h->status = APKC_BRANCHLESS_UNSUPPORTED_LANG;
        return 1;
    }

    h->lang_profile_id = lang_type;
    h->frontend_kind = APKC_FRONTEND_LANGUAGE_SPECIFIC;
    h->semantic_proof = APKC_SEMANTIC_UNPROVEN;

    /* Initialize execution context with embedded machine state and code array */
    h->ctx.code_len = 0;
    h->ctx.max_steps = 0x10000;  /* 64K step limit */
    h->ctx.status = 0;
    h->ctx.steps_executed = 0;
    h->ctx.result = 0;

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

    /* G-S2 routing fix: the active branchless path no longer calls the legacy
     * pattern-only compile_universal() dispatcher.  It enters the exact
     * language-specific scanner/statement frontend selected by LP_*.
     */
    if (apkc_compile_language_direct(&uc, src, src_len, lang_type)) {
        h->status = APKC_BRANCHLESS_COMPILE_ERROR;
        return 1;
    }

    /* Snapshot instruction count */
    h->code_len = uc.cg.pos;
    if (h->code_len == 0 || h->code_len > 0x10000) {
        h->status = APKC_BRANCHLESS_COMPILE_ERROR;
        return 1;
    }

    /* Copy generated code into the execution context now, so the future
     * semantic/action oracle consumes the exact instruction stream that is
     * subsequently encoded.  Execution itself remains gated below.
     */
    h->ctx.code_len = h->code_len;
    for (i = 0; i < h->code_len; i++) {
        h->ctx.code[i] = h->code[i];
    }

    /* === PHASE 3: SEMANTIC/ACTION VALIDATION GATE === */

    /* Deliberately not promoted yet.
     *
     * Current lang_stmt/lang_expr paths still contain structural-only and
     * placeholder semantics.  Therefore semantic_proof stays UNPROVEN and
     * steps_executed stays zero.  G-S3/G-S4/G-S5 must close before this phase
     * may call execute() as an equivalence oracle.
     */

    /* === PHASE 4: ARM64 ASSEMBLY GENERATION === */

    /* Encode machine instructions to ARM64 binary format */
    if (arm64_encode_program(&h->arm64_enc, h->code, h->code_len)) {
        h->status = APKC_BRANCHLESS_ENCODE_ERROR;
        return 1;
    }

    /* Snapshot ARM64 assembly size */
    h->arm64_len = h->arm64_enc.pos;

    /* === PHASE 5: STRUCTURAL SUCCESS === */

    h->status = APKC_BRANCHLESS_OK;
    h->steps_executed = h->ctx.steps_executed;

    return 0;
}

/* === OPTIONAL: VALIDATION WITH HARDENING GATES === */

/* apkc_branchless_validate_with_gates remains a future integration point for:
 * - hardening_boundary_gates.h (capacity validation)
 * - hardening_receipt_chain.h (chain-of-custody)
 * - hardening_fail_closed.h (TOKEN_VAZIO barrier)
 *
 * Semantic execution must not be enabled here until G-S3/G-S4/G-S5 have an
 * executable oracle and falsifiers.
 */

/* === INTROSPECTION === */

/* Get status string for debugging (freestanding-safe) */
static inline const char* apkc_branchless_status_str(u8 status) {
    switch (status) {
    case APKC_BRANCHLESS_OK:               return "success";
    case APKC_BRANCHLESS_COMPILE_ERROR:    return "compile_error";
    case APKC_BRANCHLESS_ENCODE_ERROR:     return "encode_error";
    case APKC_BRANCHLESS_UNSUPPORTED_LANG: return "unsupported_language";
    default:                               return "unknown";
    }
}

#endif /* APKC_BRANCHLESS_HANDLER_H */
