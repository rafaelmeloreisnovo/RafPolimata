/* apkc_branchless_handler.h — Branchless machine compilation coordinator
 *
 * Pipeline: Source -> canonical LangProfile -> scoped semantic subset or
 *          language-specific frontend -> linear machine -> scoped VM oracle
 *          (when eligible) -> ARM64
 *
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
#define APKC_BRANCHLESS_VM_ACTION_ERROR     4u

/* VM action observation is not, by itself, semantic equivalence proof. */
#define APKC_VM_ACTION_NOT_RUN 0u
#define APKC_VM_ACTION_OBSERVED 1u

/* === BRANCHLESS COMPILER STATE === */
struct BranchlessHandler {
    /* Code generation phase */
    struct Insn code[0x10000];           /* 64K instruction buffer (master copy) */

    /* Execution validation phase (with embedded Machine state) */
    struct ExecutionContext ctx;         /* Includes Machine state + embedded code array */

    /* ARM64 assembly output phase */
    struct Arm64Encoder arm64_enc;
    u8 arm64_asm[0x10000];               /* 64K ARM64 buffer */

    /* Status and evidence metrics */
    u8 status;                           /* APKC_BRANCHLESS_* */
    u8 lang_profile_id;                  /* canonical LP_* value from lang_profile.h */
    u8 frontend_kind;                    /* APKC_FRONTEND_* */
    u8 semantic_scope;                   /* APKC_SEMANTIC_SCOPE_* */
    u8 semantic_proof;                   /* APKC_SEMANTIC_*; external expected-value gate required */
    u8 vm_action_observed;               /* APKC_VM_ACTION_* */
    u8 vm_status;                        /* ExecutionContext status */
    u8 _reserved0;
    u64 vm_result;                       /* observed r0 after scoped VM action */
    u64 steps_executed;                  /* Instruction count from executor */
    u32 code_len;                        /* Actual instruction count */
    u32 arm64_len;                       /* Actual ARM64 assembly size */
};

/* === ENTRY POINT === */

/* apkc_branchless_compile: Source -> canonical route -> Machine -> ARM64
 *
 * A bounded semantic subset may additionally execute the exact VM instruction
 * stream.  The handler records that observation but cannot know the caller's
 * expected value, so semantic_proof remains UNPROVEN here.  CI/test code must
 * compare vm_result against an independently declared expected result.
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
    h->frontend_kind = APKC_FRONTEND_NONE;
    h->semantic_scope = APKC_SEMANTIC_SCOPE_NONE;
    h->semantic_proof = APKC_SEMANTIC_UNPROVEN;
    h->vm_action_observed = APKC_VM_ACTION_NOT_RUN;

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

    if (apkc_compile_language_direct_scoped(
            &uc, src, src_len, lang_type,
            &h->frontend_kind, &h->semantic_scope)) {
        h->status = APKC_BRANCHLESS_COMPILE_ERROR;
        return 1;
    }

    /* Snapshot instruction count */
    h->code_len = uc.cg.pos;
    if (h->code_len == 0 || h->code_len > 0x10000) {
        h->status = APKC_BRANCHLESS_COMPILE_ERROR;
        return 1;
    }

    /* Copy generated code into the execution context.  The future or current
     * oracle must consume exactly the same VM instruction stream that proceeds
     * to ARM64 encoding.
     */
    h->ctx.code_len = h->code_len;
    for (i = 0; i < h->code_len; i++) {
        h->ctx.code[i] = h->code[i];
    }

    /* === PHASE 3: SCOPED VM ACTION ORACLE === */

    if (h->semantic_scope == APKC_SEMANTIC_SCOPE_RETURN_ARITHMETIC_FRAGMENT) {
        /* The bounded subset ends in VM RET.  For this synthetic caller-only
         * oracle, r14 points one instruction past the fragment so RET exits the
         * VM cleanly.  This convention does not claim general CALL/RET closure.
         */
        h->ctx.m.r[14] = (u64)h->code_len;
        h->vm_status = execute(&h->ctx);
        h->vm_result = h->ctx.result;
        h->steps_executed = h->ctx.steps_executed;

        if (h->vm_status != 1u) {
            h->status = APKC_BRANCHLESS_VM_ACTION_ERROR;
            return 1;
        }
        h->vm_action_observed = APKC_VM_ACTION_OBSERVED;
    }

    /* `semantic_proof` deliberately remains UNPROVEN: the handler can observe
     * r0 but cannot decide whether r0 equals the independently expected value.
     */

    /* === PHASE 4: ARM64 ASSEMBLY GENERATION === */

    if (arm64_encode_program(&h->arm64_enc, h->code, h->code_len)) {
        h->status = APKC_BRANCHLESS_ENCODE_ERROR;
        return 1;
    }

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
 * Only bounded scopes with explicit expected-result falsifiers may promote
 * semantic evidence.  Control flow, calls, variables and comparison semantics
 * remain independent open gates.
 */

/* === INTROSPECTION === */

static inline const char* apkc_branchless_status_str(u8 status) {
    switch (status) {
    case APKC_BRANCHLESS_OK:               return "success";
    case APKC_BRANCHLESS_COMPILE_ERROR:    return "compile_error";
    case APKC_BRANCHLESS_ENCODE_ERROR:     return "encode_error";
    case APKC_BRANCHLESS_UNSUPPORTED_LANG: return "unsupported_language";
    case APKC_BRANCHLESS_VM_ACTION_ERROR:  return "vm_action_error";
    default:                               return "unknown";
    }
}

#endif /* APKC_BRANCHLESS_HANDLER_H */
