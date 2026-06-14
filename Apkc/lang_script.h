/* lang_script.h — ARM64 execve bootstrap for interpreter-based languages.
 * Generates a minimal .so .text section that calls execve(interp, argv, NULL).
 * No heap. No libc. Pure machine words.
 *
 * Memory layout of emitted code:
 *   [0  .. 4*N-1]  N ARM64 instructions (ends with ret)
 *   [4*N .. end ]  string pool: interp\0 arg1\0 script\0
 *
 * Used for: sh, py, perl, js, php — any language whose "compile" step is
 * just embedding the source text and calling execve with the right interpreter.
 */
#pragma once
#include "mem.h"
#include "arch_arm64.h"

/* scratch registers used by the bootstrap */
#define SCRIP_INTERP  19u  /* x19 = &interp_path */
#define SCRIP_ARG1    20u  /* x20 = &arg1 ("-c"/"-e"/"-r") */
#define SCRIP_SCRIPT  21u  /* x21 = &script_body  */
#define SCRIP_NULL    22u  /* x22 = 0 (NULL)       */

static inline sz scrip_slen(const char *s) {
    sz n=0; while(s[n]) n++; return n;
}

/*
 * gen_script_code64 — emit bootstrap + string pool into out[0..out_cap).
 *   interp  : absolute path to interpreter (e.g. "/usr/bin/python3")
 *   arg1    : first argument after interpreter (e.g. "-c"), or NULL
 *   script  : script source text
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

    /* instructions: sub,adr,add,add,str×4,movz,str,str,orr,add,add,movz,svc,movz,ret = 17 insns */
#define SCRIP_INSN_COUNT 17u
    sz pool_start = (sz)(SCRIP_INSN_COUNT * 4u);
    sz off_interp = pool_start;
    sz off_arg1   = off_interp + interp_len + 1u;
    sz off_script = off_arg1 + (arg1 ? (arg1_len + 1u) : 0u);
    sz total      = off_script + script_len + 1u;

    if (total > out_cap) return 0;

    sz pos = 0;
#define WI(w) do{ w32(out+pos,(w)); pos+=4; }while(0)

    /* sub sp, sp, #48 */
    WI(a64_sub_imm(RSP, RSP, 48, 0, 1));
    /* adr x19, #(pool_start - 4) — PC at byte 4, pool starts at pool_start */
    WI(a64_adr(SCRIP_INTERP, (i32)(pool_start - 4u)));
    /* x20 = &arg1 or &script */
    if (arg1) {
        WI(a64_add_imm(SCRIP_ARG1,   SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
        WI(a64_add_imm(SCRIP_SCRIPT, SCRIP_ARG1,   (u16)(arg1_len+1u),   0, 1));
    } else {
        WI(a64_add_imm(SCRIP_ARG1,   SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
        WI(a64_add_imm(SCRIP_SCRIPT, SCRIP_INTERP, (u16)(interp_len+1u), 0, 1));
    }
    /* argv[0]=interp, argv[1]=arg1_or_script, argv[2]=script_when_arg1, argv[3]=NULL */
    WI(a64_str(SCRIP_INTERP, RSP,  0, 1));
    WI(a64_str(SCRIP_ARG1,   RSP,  8, 1));
    WI(a64_str(SCRIP_SCRIPT, RSP, 16, 1));
    /* NULL sentinel */
    WI(a64_movz(SCRIP_NULL, 0, 0, 1));
    WI(a64_str(SCRIP_NULL,  RSP, 24, 1));
    WI(a64_str(SCRIP_NULL,  RSP, 32, 1));
    /* execve(interp, argv, NULL) */
    WI(a64_orr_reg(0, RZR, SCRIP_INTERP, 1));  /* x0 = interp */
    WI(a64_add_imm(1, RSP, 0,  0, 1));          /* x1 = argv */
    WI(a64_add_imm(2, RSP, 32, 0, 1));          /* x2 = envp (NULL) */
    WI(a64_movz(8, 221, 0, 1));                  /* x8 = __NR_execve */
    WI(a64_svc(0));
    /* fallback return 0 if execve fails */
    WI(a64_movz(0, 0, 0, 1));
    WI(A64_RET);

#undef WI
#undef SCRIP_INSN_COUNT

    /* string pool */
    sz pi = pos;
    m_cpy(out+pi, (const u8*)interp, interp_len); pi += interp_len; out[pi++] = 0;
    if (arg1) {
        m_cpy(out+pi, (const u8*)arg1, arg1_len); pi += arg1_len; out[pi++] = 0;
    }
    m_cpy(out+pi, (const u8*)script, script_len); pi += script_len; out[pi++] = 0;

    return pi;
}
