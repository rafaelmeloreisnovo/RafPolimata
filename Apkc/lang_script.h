/* lang_script.h — ARM64 execve bootstrap for sh/py script packaging.
 * Generates ANativeActivity_onCreate body that exec's a script from the
 * app's own data directory. No heap. No libc. Pure machine words.
 *
 * Memory layout of emitted code:
 *   [0  .. 4*N-1]  N ARM64 instructions (variable, ends with ret)
 *   [4*N .. end ]  string pool: interp\0 arg1\0 script\0
 *
 * The pool offset is known at generation time so adr is encoded directly.
 */
#pragma once
#include "mem.h"
#include "arch_arm64.h"

/* scratch registers used by the bootstrap */
#define SCRIP_INTERP  19u  /* x19 = &interp_path */
#define SCRIP_ARG1    20u  /* x20 = &arg1 ("-c")  */
#define SCRIP_SCRIPT  21u  /* x21 = &script_body  */
#define SCRIP_NULL    22u  /* x22 = 0 (NULL)       */

/* slen: branchless strlen for compile-time strings */
static inline sz scrip_slen(const char *s) {
    sz n=0; while(s[n]) n++; return n;
}

/*
 * gen_script_code64 — emit bootstrap + pool into out[0..out_cap).
 * interp : absolute path to interpreter  (e.g. "/system/bin/sh")
 * arg1   : first arg after interpreter   (e.g. "-c"), or NULL
 * script : script source text to exec    (may contain newlines)
 * Returns bytes written, or 0 on overflow.
 */
static sz gen_script_code64(
    const char *interp, const char *arg1, const char *script,
    u8 *out, sz out_cap)
{
    if (!interp || !script || !out || out_cap < 128u) return 0;

    sz interp_len = scrip_slen(interp);
    sz arg1_len   = arg1 ? scrip_slen(arg1) : 0;
    sz script_len = scrip_slen(script);

    /* pool offsets relative to start of code */
    /* instructions: sub,adr,add,add,str×4,movz,str,str,orr,add,add,movz,svc,movz,ret = 17 insns */
#define SCRIP_INSN_COUNT 17u
    sz pool_start = (sz)(SCRIP_INSN_COUNT * 4u);
    sz off_interp = pool_start;
    sz off_arg1   = off_interp + interp_len + 1u;
    sz off_script = off_arg1 + (arg1 ? (arg1_len + 1u) : 0u);
    sz total      = off_script + script_len + 1u;

    if (total > out_cap) return 0;

    /* ── emit instructions ── */
    sz pos = 0;
#define WI(w) do{ w32(out+pos,(w)); pos+=4; }while(0)

    /* 0: sub sp, sp, #48  — 4 argv ptrs (32B) + envp ptr (8B) + pad (8B) */
    WI(a64_sub_imm(RSP, RSP, 48, 0, 1));

    /* 4: adr x19, #(pool_start - 4) — PC=4, so adr from PC=4 to pool */
    WI(a64_adr(SCRIP_INTERP, (i32)(pool_start - 4u)));

    /* 8: add x20, x19, #(interp_len+1) — x20 = &arg1 or &script */
    if (arg1) {
        WI(a64_add_imm(SCRIP_ARG1, SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
        /* 12: add x21, x20, #(arg1_len+1) — x21 = &script */
        WI(a64_add_imm(SCRIP_SCRIPT, SCRIP_ARG1, (u16)(arg1_len+1u), 0, 1));
    } else {
        /* no arg1: x20 = &script directly */
        WI(a64_add_imm(SCRIP_ARG1, SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
        WI(a64_add_imm(SCRIP_SCRIPT, SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
    }

    /* 16: str x19, [sp, #0]  — argv[0] = interp */
    WI(a64_str(SCRIP_INTERP, RSP, 0, 1));
    /* 20: str x20, [sp, #8]  — argv[1] = arg1 or script */
    WI(a64_str(SCRIP_ARG1, RSP, 8, 1));
    /* 24: str x21, [sp, #16] — argv[2] = script (when arg1 present) */
    WI(a64_str(SCRIP_SCRIPT, RSP, 16, 1));

    /* 28: movz x22, #0 — NULL sentinel */
    WI(a64_movz(SCRIP_NULL, 0, 0, 1));
    /* 32: str x22, [sp, #24] — argv[3 or 2] = NULL */
    WI(a64_str(SCRIP_NULL, RSP, 24, 1));
    /* 36: str x22, [sp, #32] — envp[0] = NULL */
    WI(a64_str(SCRIP_NULL, RSP, 32, 1));

    /* 40: mov x0, x19 — execve path */
    WI(a64_orr_reg(0, RZR, SCRIP_INTERP, 1));
    /* 44: add x1, sp, #0 — argv array */
    WI(a64_add_imm(1, RSP, 0, 0, 1));
    /* 48: add x2, sp, #32 — envp */
    WI(a64_add_imm(2, RSP, 32, 0, 1));
    /* 52: movz x8, #221 — __NR_execve */
    WI(a64_movz(8, 221, 0, 1));
    /* 56: svc #0 */
    WI(a64_svc(0));
    /* 60: movz x0, #0 — return 0 if execve somehow returns */
    WI(a64_movz(0, 0, 0, 1));
    /* 64: ret */
    WI(A64_RET);

#undef WI
#undef SCRIP_INSN_COUNT

    /* ── emit string pool ── */
    sz pi = pos; /* should equal pool_start */
    /* interp */
    m_cpy(out+pi, (const u8*)interp, interp_len); pi += interp_len; out[pi++] = 0;
    /* arg1 */
    if (arg1) {
        m_cpy(out+pi, (const u8*)arg1, arg1_len); pi += arg1_len; out[pi++] = 0;
    }
    /* script */
    m_cpy(out+pi, (const u8*)script, script_len); pi += script_len; out[pi++] = 0;

    return pi;
}
