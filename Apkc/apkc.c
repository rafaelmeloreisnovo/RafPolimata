/* apkc.c — freestanding APK compiler, no libc, no heap, no abstractions.
 * Unified pipeline: 12 languages via declarative LangProfile table.
 * Technological determinism — source extension drives the whole pipeline. */
#include "sys.h"
#include "mem.h"
#include "coherence.h"
#include "codegen_select.h"
#include "arch_arm64.h"
#include "arch_arm32.h"
#include "lang_script.h"
#include "lang_profile.h"
#include "hw_dispatch.h"
#include "fmt_zip.h"
#include "fmt_dex.h"
#include "fmt_axml.h"
#include "fmt_elf.h"

/* language modes */
typedef enum { LANG_ASM=0, LANG_SH=1, LANG_PY=2, LANG_RS=3 } LangMode;

/* ── token kinds ─────────────────────────────────────────────────── */
typedef enum {
    TK_EOF=0, TK_NL, TK_IDENT, TK_INT, TK_STR, TK_COMMA,
    TK_LBRK, TK_RBRK, TK_BANG, TK_HASH, TK_COLON, TK_PLUS, TK_MINUS
} TKind;

typedef struct { const char *p; sz len; u64 ival; TKind kind; } Tok;
typedef struct { const char *s; const char *e; Tok cur; } Lex;

static int _is_sp(char c)  { return c==' '||c=='\t'||c=='\r'; }
static int _is_al(char c)  { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||c=='.'||c=='@'; }
static int _is_dg(char c)  { return c>='0'&&c<='9'; }
static int _is_hex(char c) {
    return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');
}
static u64 _hv(char c) {
    u64 d=(u64)(c-'0'); u64 la=(u64)(c-'a'+10); u64 ua=(u64)(c-'A'+10);
    u64 sd=(u64)((i64)(-(c>='0'&&c<='9'))&d);
    u64 sl=(u64)((i64)(-(c>='a'&&c<='f'))&la);
    u64 su=(u64)((i64)(-(c>='A'&&c<='F'))&ua);
    return sd|sl|su;
}

static void lex_skip(Lex *l) {
    while (l->s < l->e && _is_sp(*l->s)) l->s++;
    if (l->s < l->e && *l->s == ';') { while (l->s<l->e && *l->s!='\n') l->s++; }
    if (l->s < l->e && *l->s == '/') {
        if (l->s+1<l->e && l->s[1]=='/') { while (l->s<l->e && *l->s!='\n') l->s++; }
    }
}

static void lex_next(Lex *l) {
    lex_skip(l);
    Tok t; t.p=l->s; t.len=0; t.ival=0;
    if (l->s>=l->e) { t.kind=TK_EOF; l->cur=t; return; }
    char c=*l->s;
    if (c=='\n') { t.kind=TK_NL; t.len=1; l->s++; l->cur=t; return; }
    if (c==',') { t.kind=TK_COMMA; t.len=1; l->s++; l->cur=t; return; }
    if (c=='['||c=='{') { t.kind=TK_LBRK; t.len=1; l->s++; l->cur=t; return; }
    if (c==']'||c=='}') { t.kind=TK_RBRK; t.len=1; l->s++; l->cur=t; return; }
    if (c=='!') { t.kind=TK_BANG; t.len=1; l->s++; l->cur=t; return; }
    if (c=='#') { t.kind=TK_HASH; t.len=1; l->s++; l->cur=t; return; }
    if (c==':') { t.kind=TK_COLON; t.len=1; l->s++; l->cur=t; return; }
    if (c=='+') { t.kind=TK_PLUS; t.len=1; l->s++; l->cur=t; return; }
    if (c=='-') {
        if (l->s+1<l->e && _is_dg(l->s[1])) {
            l->s++; u64 v=0;
            while (l->s<l->e && _is_dg(*l->s)) { v=v*10+(u64)(*l->s-'0'); l->s++; }
            t.ival=(u64)(-(i64)v); t.kind=TK_INT; t.len=(sz)(l->s-t.p); l->cur=t; return;
        }
        t.kind=TK_MINUS; t.len=1; l->s++; l->cur=t; return;
    }
    if (c=='0' && l->s+1<l->e && (l->s[1]=='x'||l->s[1]=='X')) {
        l->s+=2; u64 v=0;
        while (l->s<l->e && _is_hex(*l->s)) { v=(v<<4)|_hv(*l->s); l->s++; }
        t.ival=v; t.kind=TK_INT; t.len=(sz)(l->s-t.p); l->cur=t; return;
    }
    if (_is_dg(c)) {
        u64 v=0; while (l->s<l->e && _is_dg(*l->s)) { v=v*10+(u64)(*l->s-'0'); l->s++; }
        t.ival=v; t.kind=TK_INT; t.len=(sz)(l->s-t.p); l->cur=t; return;
    }
    if (_is_al(c)) {
        while (l->s<l->e && (_is_al(*l->s)||_is_dg(*l->s))) l->s++;
        t.kind=TK_IDENT; t.len=(sz)(l->s-t.p); l->cur=t; return;
    }
    if (c=='"') {
        l->s++; t.p=l->s;
        while (l->s<l->e && *l->s!='"') l->s++;
        t.kind=TK_STR; t.len=(sz)(l->s-t.p);
        if (l->s<l->e) l->s++;
        l->cur=t; return;
    }
    /* skip unknown */ t.kind=TK_EOF; l->s++; l->cur=t;
}

static int tok_eq(Tok t, const char *s) {
    sz n=0; while(s[n]) n++;
    if (t.len!=n) return 0;
    for (sz i=0;i<n;i++) if (t.p[i]!=s[i]) return 0;
    return 1;
}

/* case-insensitive ident compare */
static int tok_eqi(Tok t, const char *s) {
    sz n=0; while(s[n]) n++;
    if (t.len!=n) return 0;
    for (sz i=0;i<n;i++) {
        char a=t.p[i]; char b=s[i];
        char al=(a>='A'&&a<='Z')?(char)(a+32):a;
        char bl=(b>='A'&&b<='Z')?(char)(b+32):b;
        if (al!=bl) return 0;
    }
    return 1;
}

/* ── label / backpatch tables ─────────────────────────────────────── */
#define MAX_LBL 256
#define MAX_PAT 256

typedef struct { char name[80]; u32 off; } Lbl;
typedef struct { u32 insn_off; char tgt[80]; int arch; } Pat; /* arch: 64 or 32 */

static Lbl _lbls[MAX_LBL];
static u32 _nlbl;
static Pat _pats[MAX_PAT];
static u32 _npat;

static void lbl_reset(void) { _nlbl=0; _npat=0; }

static void tok_copy(char *dst, Tok t) {
    sz n=t.len<79?t.len:79;
    m_cpy(dst,(const u8*)t.p,n); dst[n]=0;
}

static i32 lbl_find(const char *nm) {
    for (u32 i=0;i<_nlbl;i++) {
        sz j=0; while (_lbls[i].name[j]&&nm[j]&&_lbls[i].name[j]==nm[j]) j++;
        if (!_lbls[i].name[j]&&!nm[j]) return (i32)i;
    }
    return -1;
}

static void lbl_def(const char *nm, u32 off) {
    i32 idx=lbl_find(nm);
    if (idx>=0) { _lbls[idx].off=off; return; }
    if (_nlbl>=MAX_LBL) return;
    sz j=0; while (nm[j]&&j<79) { _lbls[_nlbl].name[j]=nm[j]; j++; }
    _lbls[_nlbl].name[j]=0;
    _lbls[_nlbl].off=off;
    _nlbl++;
}

static void pat_add(u32 ioff, Tok tgt, int arch) {
    if (_npat>=MAX_PAT) return;
    _pats[_npat].insn_off=ioff;
    tok_copy(_pats[_npat].tgt, tgt);
    _pats[_npat].arch=arch;
    _npat++;
}

/* ── register parsers ─────────────────────────────────────────────── */
static i32 reg64(Tok t) {
    if (t.kind!=TK_IDENT) return -1;
    /* xN or wN */
    char lo=(t.p[0]>='A'&&t.p[0]<='Z')?(char)(t.p[0]+32):t.p[0];
    if ((lo=='x'||lo=='w') && t.len>=2) {
        u32 n=0; for(sz i=1;i<t.len;i++) n=n*10+(u32)(t.p[i]-'0');
        return (i32)n;
    }
    if (tok_eqi(t,"sp")) return 31;
    if (tok_eqi(t,"xzr")||tok_eqi(t,"wzr")) return 31;
    if (tok_eqi(t,"lr")) return 30;
    if (tok_eqi(t,"fp")) return 29;
    return -1;
}

static int reg64_sf(Tok t) { /* 0=w, 1=x */
    if (t.kind!=TK_IDENT||t.len<1) return 1;
    char lo=(t.p[0]>='A'&&t.p[0]<='Z')?(char)(t.p[0]+32):t.p[0];
    return lo=='w'?0:1;
}

/* reg_neon: parse vN or qN or vN.size — returns 0..31, -1 on fail */
static i32 reg_neon(Tok t) {
    if (t.kind!=TK_IDENT||t.len<2) return -1;
    char lo=(t.p[0]>='A'&&t.p[0]<='Z')?(char)(t.p[0]+32):t.p[0];
    if (lo!='v'&&lo!='q') return -1;
    u32 n=0;
    sz i=1;
    while (i<t.len && t.p[i]>='0' && t.p[i]<='9') { n=n*10+(u32)(t.p[i]-'0'); i++; }
    return (i32)n;
}

static i32 reg32a(Tok t) {
    if (t.kind!=TK_IDENT) return -1;
    char lo=(t.p[0]>='A'&&t.p[0]<='Z')?(char)(t.p[0]+32):t.p[0];
    if (lo=='r' && t.len>=2) {
        u32 n=0; for(sz i=1;i<t.len;i++) n=n*10+(u32)(t.p[i]-'0');
        return (i32)n;
    }
    if (tok_eqi(t,"sp")) return A32_SP;
    if (tok_eqi(t,"lr")) return A32_LR;
    if (tok_eqi(t,"pc")) return A32_PC;
    return -1;
}

/* ── condition code parser ────────────────────────────────────────── */
static u32 parse_cc64(Tok t) {
    if (tok_eqi(t,"eq")) return CC_EQ;
    if (tok_eqi(t,"ne")) return CC_NE;
    if (tok_eqi(t,"cs")||tok_eqi(t,"hs")) return CC_CS;
    if (tok_eqi(t,"cc")||tok_eqi(t,"lo")) return CC_CC;
    if (tok_eqi(t,"mi")) return CC_MI;
    if (tok_eqi(t,"pl")) return CC_PL;
    if (tok_eqi(t,"vs")) return CC_VS;
    if (tok_eqi(t,"vc")) return CC_VC;
    if (tok_eqi(t,"hi")) return CC_HI;
    if (tok_eqi(t,"ls")) return CC_LS;
    if (tok_eqi(t,"ge")) return CC_GE;
    if (tok_eqi(t,"lt")) return CC_LT;
    if (tok_eqi(t,"gt")) return CC_GT;
    if (tok_eqi(t,"le")) return CC_LE;
    return CC_AL;
}

static u32 parse_cc32(Tok t) {
    if (tok_eqi(t,"eq")) return A32_EQ;
    if (tok_eqi(t,"ne")) return A32_NE;
    if (tok_eqi(t,"cs")||tok_eqi(t,"hs")) return A32_CS;
    if (tok_eqi(t,"cc")||tok_eqi(t,"lo")) return A32_CC;
    if (tok_eqi(t,"mi")) return A32_MI;
    if (tok_eqi(t,"pl")) return A32_PL;
    if (tok_eqi(t,"vs")) return A32_VS;
    if (tok_eqi(t,"vc")) return A32_VC;
    if (tok_eqi(t,"hi")) return A32_HI;
    if (tok_eqi(t,"ls")) return A32_LS;
    if (tok_eqi(t,"ge")) return A32_GE;
    if (tok_eqi(t,"lt")) return A32_LT;
    if (tok_eqi(t,"gt")) return A32_GT;
    if (tok_eqi(t,"le")) return A32_LE;
    return A32_AL;
}

/* skip optional # prefix and get integer */
static u64 lex_imm(Lex *l) {
    if (l->cur.kind==TK_HASH) lex_next(l);
    u64 v=l->cur.ival; lex_next(l); return v;
}

/* ── Emit context ─────────────────────────────────────────────────── */
typedef struct {
    u8 *buf; sz cap; sz pos;
    u32 sym1_va; u32 sym2_va;
    int has_sym1; int has_sym2;
    int err; /* count of unknown/invalid mnemonics encountered */
} Emit;

static void emit32(Emit *e, u32 w) {
    if (e->pos+4>e->cap) return;
    w32(e->buf+e->pos,w); e->pos+=4;
}
static void emit8(Emit *e, u8 b) {
    if (e->pos<e->cap) e->buf[e->pos++]=b;
}
static void emit_bytes(Emit *e, const u8 *src, sz n) {
    for (sz i=0;i<n;i++) emit8(e,(u8)src[i]);
}
static void emit_align(Emit *e, u32 pow2) {
    sz mask=(sz)((1u<<pow2)-1u);
    while (e->pos & mask) emit8(e,0);
}

/* ── codegen equivalence-family audit trail ──────────────────────────
 * Records which encoder variant codegen_select() picked for each
 * instance of the MOV equivalence family, but only during the final
 * real-emission pass (pass==1 in assemble()) — the sizing/label passes
 * call asm_insn64() speculatively and would otherwise double-count. */
static u8  _codegen_variant_log[256];
static u32 _codegen_variant_log_n = 0;
static u32 _codegen_variant_count[3] = {0,0,0};
static int _codegen_log_on = 0;

/* Policy gate (L16): by default an assembly error (unknown mnemonic → UNDEF
 * placeholder) makes build_apk() refuse to write the APK. --allow-undef flips
 * this to experimental "degradation permitted" mode. Default = strict. */
static int _apkc_allow_undef = 0;

static void codegen_log_variant(u32 variant) {
    if (!_codegen_log_on) return;
    if (variant < 3u) _codegen_variant_count[variant]++;
    if (_codegen_variant_log_n < sizeof(_codegen_variant_log))
        _codegen_variant_log[_codegen_variant_log_n++] = (u8)variant;
}

/* ── ARM64 assembler ──────────────────────────────────────────────── */
static void asm_insn64(Emit *em, Tok mn, Lex *l) {
    lex_next(l); /* advance past mnemonic */
    u32 pos=(u32)em->pos;

    /* detect .sym1 / .sym2 markers */
    if (tok_eq(mn,".sym1")) { em->sym1_va=pos; em->has_sym1=1; return; }
    if (tok_eq(mn,".sym2")) { em->sym2_va=pos; em->has_sym2=1; return; }

    if (tok_eqi(mn,"nop"))  { emit32(em,A64_NOP); return; }
    if (tok_eqi(mn,"ret"))  {
        if (l->cur.kind==TK_IDENT && reg64(l->cur)>=0) {
            i32 rn=reg64(l->cur); lex_next(l);
            emit32(em,a64_ret((u32)rn));
        } else {
            emit32(em,A64_RET);
        }
        return;
    }
    if (tok_eqi(mn,"brk"))  {
        u32 imm=(u32)lex_imm(l);
        emit32(em,0xD4200000u|((imm&0xFFFFu)<<5)); return;
    }
    if (tok_eqi(mn,"svc"))  {
        u32 imm=(u32)lex_imm(l);
        emit32(em,a64_svc(imm)); return;
    }
    if (tok_eqi(mn,"blr"))  {
        i32 rn=reg64(l->cur); lex_next(l);
        emit32(em,a64_blr((u32)rn)); return;
    }
    if (tok_eqi(mn,"br"))   {
        i32 rn=reg64(l->cur); lex_next(l);
        emit32(em,a64_br((u32)rn)); return;
    }
    /* movz/movk/movn rd, #imm [, lsl #hw*16] */
    if (tok_eqi(mn,"movz")||tok_eqi(mn,"movk")||tok_eqi(mn,"movn")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u64 imm=lex_imm(l);
        u32 hw=0;
        if (l->cur.kind==TK_COMMA) {
            lex_next(l); /* skip comma */
            /* expect lsl */
            lex_next(l); /* skip 'lsl' ident */
            u64 sh=lex_imm(l);
            hw=(u32)(sh/16);
        }
        u32 w;
        if (tok_eqi(mn,"movz")) w=a64_movz((u32)rd,(u32)imm,hw,(u32)sf);
        else if (tok_eqi(mn,"movk")) w=a64_movk((u32)rd,(u32)imm,hw,(u32)sf);
        else w=a64_movn((u32)rd,(u32)imm,hw,(u32)sf);
        emit32(em,w); return;
    }
    /* mov rd, #imm64 — expands to movz + movk chain */
    if (tok_eqi(mn,"mov")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u64 imm=lex_imm(l);
            /* emit movz+movk chain manually */
            u32 parts[4]; int np=0;
            for (int hw=0;hw<4;hw++) {
                u16 chunk=(u16)((imm>>(hw*16))&0xFFFFu);
                if (hw==0||chunk) {
                    if (np==0) parts[np++]=a64_movz((u32)rd,chunk,(u32)hw,(u32)sf);
                    else        parts[np++]=a64_movk((u32)rd,chunk,(u32)hw,(u32)sf);
                }
            }
            if (np==0) parts[np++]=a64_movz((u32)rd,0,0,(u32)sf);
            for(int i=0;i<np;i++) emit32(em,parts[i]);
        } else {
            /* mov rd, rn — equivalence family: ORR rd,xzr,rn / ADD rd,rn,#0 /
             * SUB rd,rn,#0 all leave rd==rn with no flag side effects.
             * codegen_select() deterministically picks among them from the
             * bytes already emitted, so the same source always picks the
             * same variant (reproducible, auditable — see
             * _codegen_variant_log). */
            i32 rn=reg64(l->cur); lex_next(l);
            u32 sf2=(u32)sf;
            u32 variant = codegen_select(em->buf, (u32)em->pos, 3u);
            u32 w;
            switch (variant) {
            case 0:
                /* ORR (shifted reg): sf|01010|shift2|0|rm5|imm6|rn5|rd5 */
                w=((sf2)<<31)|(0x2Au<<24)|((u32)rn<<16)|(31u<<5)|(u32)rd;
                break;
            case 1: w=a64_add_imm((u32)rd,(u32)rn,0u,0,sf2); break;
            default: w=a64_sub_imm((u32)rd,(u32)rn,0u,0,sf2); break;
            }
            codegen_log_variant(variant);
            emit32(em,w);
        }
        return;
    }
    /* add/sub/and/orr/eor rd, rn, #imm or rm */
    if (tok_eqi(mn,"add")||tok_eqi(mn,"sub")||tok_eqi(mn,"and")||
        tok_eqi(mn,"orr")||tok_eqi(mn,"eor")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            u32 w;
            if (tok_eqi(mn,"add")) w=a64_add_imm((u32)rd,(u32)rn,imm,0,(u32)sf);
            else                   w=a64_sub_imm((u32)rd,(u32)rn,imm,0,(u32)sf);
            emit32(em,w);
        } else {
            i32 rm=reg64(l->cur); lex_next(l);
            u32 w;
            if      (tok_eqi(mn,"add")) w=a64_add_reg((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else if (tok_eqi(mn,"sub")) w=a64_sub_reg((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else if (tok_eqi(mn,"and")) w=a64_and_reg((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else if (tok_eqi(mn,"orr")) w=a64_orr_reg((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else                        w=a64_eor_reg((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            emit32(em,w);
        }
        return;
    }
    /* cmp rn, #imm */
    if (tok_eqi(mn,"cmp")) {
        int sf=reg64_sf(l->cur);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            emit32(em,a64_cmp_imm((u32)rn,imm,(u32)sf));
        } else {
            i32 rm=reg64(l->cur); lex_next(l);
            emit32(em,a64_cmp_reg((u32)rn,(u32)rm,(u32)sf));
        }
        return;
    }
    /* csel rd, rn, rm, cond */
    if (tok_eqi(mn,"csel")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 cc=parse_cc64(l->cur); lex_next(l);
        emit32(em,a64_csel((u32)rd,(u32)rn,(u32)rm,cc,(u32)sf));
        return;
    }
    /* ldr/str rd, [rn, #off]  or  ldr rd, label (PC-relative literal) */
    if (tok_eqi(mn,"ldr")||tok_eqi(mn,"str")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        /* ldr x0, label — PC-relative literal load (no bracket) */
        if (tok_eqi(mn,"ldr") && l->cur.kind==TK_IDENT && reg64(l->cur)<0) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em, a64_ldr_lit((u8)rd, 0)); return;
        }
        lex_next(l); /* skip [ */
        i32 rn=reg64(l->cur); lex_next(l);
        i32 off=0;
        if (l->cur.kind==TK_COMMA) {
            lex_next(l);
            off=(i32)lex_imm(l);
        }
        lex_next(l); /* skip ] */
        u32 w;
        if (tok_eqi(mn,"ldr")) w=a64_ldr((u32)rd,(u32)rn,(u32)off,(u32)sf);
        else                   w=a64_str((u32)rd,(u32)rn,(u32)off,(u32)sf);
        emit32(em,w); return;
    }
    /* adr/adrp rd, label */
    if (tok_eqi(mn,"adr")||tok_eqi(mn,"adrp")) {
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
        } else {
            i32 off=(i32)lex_imm(l);
            u32 w=tok_eqi(mn,"adr")?a64_adr((u32)rd,off):a64_adrp((u32)rd,off);
            emit32(em,w); return;
        }
        emit32(em,tok_eqi(mn,"adrp")?a64_adrp((u32)rd,0):a64_adr((u32)rd,0)); return;
    }
    /* b / bl / b.cond */
    if (tok_eqi(mn,"b")||tok_eqi(mn,"bl")) {
        int is_bl=tok_eqi(mn,"bl");
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em,is_bl?a64_bl(0):a64_b(0)); return;
        }
        i32 off=(i32)lex_imm(l);
        emit32(em,is_bl?a64_bl(off/4):a64_b(off/4)); return;
    }
    /* b.cond label */
    if (tok_eqi(mn,"beq")||tok_eqi(mn,"bne")||tok_eqi(mn,"blt")||
        tok_eqi(mn,"bgt")||tok_eqi(mn,"ble")||tok_eqi(mn,"bge")) {
        /* strip leading 'b', rest is cc */
        char ccbuf[4]; ccbuf[0]=mn.p[1]; ccbuf[1]=mn.p[2]; ccbuf[2]=0;
        Tok cct; cct.p=ccbuf; cct.len=2; cct.kind=TK_IDENT;
        u32 cc=parse_cc64(cct);
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em,a64_bcond(cc,0)); return;
        }
        i32 off=(i32)lex_imm(l);
        emit32(em,a64_bcond(cc,off/4)); return;
    }
    /* cbz/cbnz */
    if (tok_eqi(mn,"cbz")||tok_eqi(mn,"cbnz")) {
        int sf=reg64_sf(l->cur);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em,tok_eqi(mn,"cbz")?a64_cbz((u32)rn,0,(u32)sf):a64_cbnz((u32)rn,0,(u32)sf));
            return;
        }
        i32 off=(i32)lex_imm(l);
        u32 w=tok_eqi(mn,"cbz")?a64_cbz((u32)rn,off/4,(u32)sf):a64_cbnz((u32)rn,off/4,(u32)sf);
        emit32(em,w); return;
    }
    /* stp/ldp (simplified: pre-index) */
    if (tok_eqi(mn,"stp")) {
        int sf=reg64_sf(l->cur);
        i32 r1=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 r2=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        i32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(i32)lex_imm(l); }
        int pre=0;
        lex_next(l); /* ] */
        if (l->cur.kind==TK_BANG) { pre=1; lex_next(l); }
        if (pre) emit32(em,a64_stp_pre((u32)r1,(u32)r2,(u32)rn,off,(u32)sf));
        else     emit32(em,a64_stp((u32)r1,(u32)r2,(u32)rn,off,(u32)sf));
        return;
    }
    if (tok_eqi(mn,"ldp")) {
        int sf=reg64_sf(l->cur);
        i32 r1=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 r2=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        i32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(i32)lex_imm(l); }
        lex_next(l); /* ] */
        int post=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(i32)lex_imm(l); post=1; }
        if (post) emit32(em,a64_ldp_post((u32)r1,(u32)r2,(u32)rn,off,(u32)sf));
        else      emit32(em,a64_ldp((u32)r1,(u32)r2,(u32)rn,off,(u32)sf));
        return;
    }
    /* mul/sdiv/udiv rd, rn, rm */
    if (tok_eqi(mn,"mul")||tok_eqi(mn,"sdiv")||tok_eqi(mn,"udiv")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg64(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"mul"))  w=a64_mul((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        else if (tok_eqi(mn,"sdiv")) w=a64_sdiv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        else                         w=a64_udiv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        emit32(em,w); return;
    }
    /* lsl/lsr/asr — immediate or register form */
    if (tok_eqi(mn,"lsl")||tok_eqi(mn,"lsr")||tok_eqi(mn,"asr")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 w;
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 sh=(u32)lex_imm(l);
            if      (tok_eqi(mn,"lsl")) w=a64_lsl_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
            else if (tok_eqi(mn,"lsr")) w=a64_lsr_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
            else                        w=a64_asr_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
        } else {
            i32 rm=reg64(l->cur); lex_next(l);
            if      (tok_eqi(mn,"lsl")) w=a64_lslv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else if (tok_eqi(mn,"lsr")) w=a64_lsrv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else                        w=a64_asrv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        }
        emit32(em,w); return;
    }
    /* ldrb/strb rt, [rn, #off] */
    if (tok_eqi(mn,"ldrb")||tok_eqi(mn,"strb")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(u32)lex_imm(l); }
        lex_next(l); /* ] */
        u32 w=tok_eqi(mn,"ldrb")?a64_ldrb((u32)rt,(u32)rn,(u16)off)
                                 :a64_strb((u32)rt,(u32)rn,(u16)off);
        emit32(em,w); return;
    }
    /* ldrh/strh rt, [rn, #off] */
    if (tok_eqi(mn,"ldrh")||tok_eqi(mn,"strh")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(u32)lex_imm(l); }
        lex_next(l); /* ] */
        u32 w=tok_eqi(mn,"ldrh")?a64_ldrh((u32)rt,(u32)rn,(u16)off)
                                 :a64_strh((u32)rt,(u32)rn,(u16)off);
        emit32(em,w); return;
    }
    /* tbz/tbnz rt, #bit, label */
    if (tok_eqi(mn,"tbz")||tok_eqi(mn,"tbnz")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 bit=(u32)lex_imm(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em,tok_eqi(mn,"tbz")?a64_tbz((u32)rt,(u8)bit,0)
                                       :a64_tbnz((u32)rt,(u8)bit,0));
        } else {
            i32 off=(i32)lex_imm(l);
            emit32(em,tok_eqi(mn,"tbz")?a64_tbz((u32)rt,(u8)bit,(i16)(off/4))
                                       :a64_tbnz((u32)rt,(u8)bit,(i16)(off/4)));
        }
        return;
    }
    /* .word — raw 32-bit literal */
    if (tok_eq(mn,".word")||tok_eqi(mn,".word")) {
        u32 v=(u32)lex_imm(l);
        emit32(em,v); return;
    }
    /* ── tbz / tbnz ── */
    if (tok_eqi(mn,"tbz")||tok_eqi(mn,"tbnz")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 bit=(u32)lex_imm(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,64); lex_next(l);
            emit32(em,tok_eqi(mn,"tbz")?a64_tbz((u32)rt,(u8)bit,0)
                                       :a64_tbnz((u32)rt,(u8)bit,0));
        } else {
            i32 off=(i32)lex_imm(l);
            emit32(em,tok_eqi(mn,"tbz")?a64_tbz((u32)rt,(u8)bit,(i16)(off/4))
                                       :a64_tbnz((u32)rt,(u8)bit,(i16)(off/4)));
        }
        return;
    }
    /* ── mul / sdiv / udiv ── */
    if (tok_eqi(mn,"mul")||tok_eqi(mn,"sdiv")||tok_eqi(mn,"udiv")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg64(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"mul"))  w=a64_mul((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        else if (tok_eqi(mn,"sdiv")) w=a64_sdiv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        else                         w=a64_udiv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        emit32(em,w); return;
    }
    /* ── lsl / lsr / asr (imm or register) ── */
    if (tok_eqi(mn,"lsl")||tok_eqi(mn,"lsr")||tok_eqi(mn,"asr")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 w;
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 sh=(u32)lex_imm(l);
            if      (tok_eqi(mn,"lsl")) w=a64_lsl_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
            else if (tok_eqi(mn,"lsr")) w=a64_lsr_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
            else                        w=a64_asr_imm((u32)rd,(u32)rn,(u8)sh,(u32)sf);
        } else {
            i32 rm=reg64(l->cur); lex_next(l);
            if      (tok_eqi(mn,"lsl")) w=a64_lslv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else if (tok_eqi(mn,"lsr")) w=a64_lsrv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
            else                        w=a64_asrv((u32)rd,(u32)rn,(u32)rm,(u32)sf);
        }
        emit32(em,w); return;
    }
    /* ── ldrb / strb / ldrh / strh ── */
    if (tok_eqi(mn,"ldrb")||tok_eqi(mn,"strb")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(u32)lex_imm(l); }
        lex_next(l); /* ] */
        emit32(em,tok_eqi(mn,"ldrb")?a64_ldrb((u32)rt,(u32)rn,(u16)off)
                                     :a64_strb((u32)rt,(u32)rn,(u16)off));
        return;
    }
    if (tok_eqi(mn,"ldrh")||tok_eqi(mn,"strh")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(u32)lex_imm(l); }
        lex_next(l); /* ] */
        emit32(em,tok_eqi(mn,"ldrh")?a64_ldrh((u32)rt,(u32)rn,(u16)off)
                                     :a64_strh((u32)rt,(u32)rn,(u16)off));
        return;
    }
    /* ── NEON: ld1 {vt.size}, [xn] / st1 ── */
    if (tok_eqi(mn,"ld1")||tok_eqi(mn,"st1")) {
        lex_next(l); /* skip { */
        i32 vt=reg_neon(l->cur); lex_next(l);
        lex_next(l); /* skip } */
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* skip [ */
        i32 rn=reg64(l->cur); lex_next(l);
        lex_next(l); /* skip ] */
        emit32(em,tok_eqi(mn,"ld1")?a64_ld1_16b((u32)vt,(u32)rn)
                                   :a64_st1_16b((u32)vt,(u32)rn));
        return;
    }
    /* ── NEON vector arithmetic: add/sub/mul/fmul/fadd/eor/and/orr (v/q registers) ── */
    if ((tok_eqi(mn,"add")||tok_eqi(mn,"sub")||tok_eqi(mn,"mul")||
         tok_eqi(mn,"fmul")||tok_eqi(mn,"fadd")||
         tok_eqi(mn,"eor")||tok_eqi(mn,"and")||tok_eqi(mn,"orr")) &&
        reg_neon(l->cur)>=0) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"add"))  w=a64_add_4s((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"sub"))  w=a64_sub_4s((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"mul"))  w=a64_mul_4s((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"fmul")) w=a64_fmul_4s((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"fadd")) w=a64_fadd_4s((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"eor"))  w=a64_eor_16b((u32)vd,(u32)vn,(u32)vm);
        else if (tok_eqi(mn,"and"))  w=a64_and_16b((u32)vd,(u32)vn,(u32)vm);
        else                         w=a64_orr_16b((u32)vd,(u32)vn,(u32)vm);
        emit32(em,w); return;
    }
    /* ── NEON: cnt, addv (2-operand) ── */
    if (tok_eqi(mn,"cnt")||tok_eqi(mn,"addv")||tok_eqi(mn,"rev64")) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"cnt"))   w=a64_cnt_16b((u32)vd,(u32)vn);
        else if (tok_eqi(mn,"addv"))  w=a64_addv_4s((u32)vd,(u32)vn);
        else                          w=a64_rev64_16b((u32)vd,(u32)vn);
        emit32(em,w); return;
    }
    /* ── CRC32 ── */
    if (tok_eqi(mn,"crc32b")||tok_eqi(mn,"crc32h")||
        tok_eqi(mn,"crc32w")||tok_eqi(mn,"crc32x")||
        tok_eqi(mn,"crc32cb")||tok_eqi(mn,"crc32ch")||
        tok_eqi(mn,"crc32cw")||tok_eqi(mn,"crc32cx")) {
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg64(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"crc32b"))  w=a64_crc32b((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32h"))  w=a64_crc32h((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32w"))  w=a64_crc32w((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32x"))  w=a64_crc32x((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32cb")) w=a64_crc32cb((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32ch")) w=a64_crc32ch((u32)rd,(u32)rn,(u32)rm);
        else if (tok_eqi(mn,"crc32cw")) w=a64_crc32cw((u32)rd,(u32)rn,(u32)rm);
        else                            w=a64_crc32cx((u32)rd,(u32)rn,(u32)rm);
        emit32(em,w); return;
    }
    /* ── SHA256 ── */
    if (tok_eqi(mn,"sha256h")||tok_eqi(mn,"sha256h2")||
        tok_eqi(mn,"sha256su0")||tok_eqi(mn,"sha256su1")) {
        i32 qd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 qn=reg_neon(l->cur); lex_next(l);
        if (tok_eqi(mn,"sha256su0")) { emit32(em,a64_sha256su0((u32)qd,(u32)qn)); return; }
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"sha256h"))   w=a64_sha256h((u32)qd,(u32)qn,(u32)vm);
        else if (tok_eqi(mn,"sha256h2"))  w=a64_sha256h2((u32)qd,(u32)qn,(u32)vm);
        else                              w=a64_sha256su1((u32)qd,(u32)qn,(u32)vm);
        emit32(em,w); return;
    }
    /* ── AES ── */
    if (tok_eqi(mn,"aese")||tok_eqi(mn,"aesd")||
        tok_eqi(mn,"aesmc")||tok_eqi(mn,"aesimc")) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"aese"))   w=a64_aese((u32)vd,(u32)vn);
        else if (tok_eqi(mn,"aesd"))   w=a64_aesd((u32)vd,(u32)vn);
        else if (tok_eqi(mn,"aesmc"))  w=a64_aesmc((u32)vd,(u32)vn);
        else                           w=a64_aesimc((u32)vd,(u32)vn);
        emit32(em,w); return;
    }
    /* ── Acquire/release atomics ── */
    if (tok_eqi(mn,"ldar")||tok_eqi(mn,"stlr")||
        tok_eqi(mn,"ldaxr")||tok_eqi(mn,"ldxr")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        lex_next(l); /* ] */
        u32 w;
        if      (tok_eqi(mn,"ldar"))  w=a64_ldar((u32)rt,(u32)rn);
        else if (tok_eqi(mn,"stlr"))  w=a64_stlr((u32)rt,(u32)rn);
        else if (tok_eqi(mn,"ldaxr")) w=a64_ldaxr((u32)rt,(u32)rn);
        else                          w=a64_ldxr((u32)rt,(u32)rn);
        emit32(em,w); return;
    }
    if (tok_eqi(mn,"stlxr")||tok_eqi(mn,"stxr")) {
        i32 rs=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        lex_next(l); /* ] */
        emit32(em,tok_eqi(mn,"stlxr")?a64_stlxr((u32)rs,(u32)rt,(u32)rn)
                                      :a64_stxr((u32)rs,(u32)rt,(u32)rn));
        return;
    }
    /* ── prfm pldl1keep|pldl2keep, [xn, #off] ── */
    if (tok_eqi(mn,"prfm")) {
        int l2 = tok_eqi(l->cur,"pldl2keep")||tok_eqi(l->cur,"pldl2strm");
        lex_next(l); /* skip prefetch type */
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(u32)lex_imm(l); }
        lex_next(l); /* ] */
        emit32(em,l2?a64_prfm_l2((u32)rn,(u16)off):a64_prfm_l1((u32)rn,(u16)off));
        return;
    }
    /* ── dmb / dsb / isb ── */
    if (tok_eqi(mn,"dmb")) { lex_next(l); emit32(em,A64_DMB_ISH); return; }
    if (tok_eqi(mn,"dsb")) { lex_next(l); emit32(em,A64_DSB_ISH); return; }
    if (tok_eqi(mn,"isb")) {              emit32(em,A64_ISB);      return; }

    /* ── FMA: fmla / fmls ── */
    if (tok_eqi(mn,"fmla")||tok_eqi(mn,"fmls")) {
        int sub=tok_eqi(mn,"fmls");
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        emit32(em, sub ? a64_fmls_4s((u8)vd,(u8)vn,(u8)vm)
                       : a64_fmla_4s((u8)vd,(u8)vn,(u8)vm));
        return;
    }
    /* ── Widening multiply: umull / smull / umlal / smlal ── */
    if (tok_eqi(mn,"umull")||tok_eqi(mn,"smull")||
        tok_eqi(mn,"umlal")||tok_eqi(mn,"smlal")) {
        int sign=tok_eqi(mn,"smull")||tok_eqi(mn,"smlal");
        int acc =tok_eqi(mn,"umlal")||tok_eqi(mn,"smlal");
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        u32 w;
        if (acc)  w = sign ? a64_smlal_2d((u8)vd,(u8)vn,(u8)vm)
                           : a64_umlal_2d((u8)vd,(u8)vn,(u8)vm);
        else      w = sign ? a64_smull_2d((u8)vd,(u8)vn,(u8)vm)
                           : a64_umull_2d((u8)vd,(u8)vn,(u8)vm);
        emit32(em,w); return;
    }
    /* ── dup ── */
    if (tok_eqi(mn,"dup")) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        /* heuristic: if reg64 parses OK it's a GPR, else treat as NEON */
        i32 gpr=reg64(l->cur);
        if (gpr>=0) { lex_next(l); emit32(em, a64_dup_4s_gpr((u8)vd,(u8)gpr)); }
        else {
            i32 vn=reg_neon(l->cur); lex_next(l);
            emit32(em, a64_dup_4s_lane0((u8)vd,(u8)vn));
        }
        return;
    }
    /* ── Bit manipulation: clz / cls / rbit / rev / rev32 ── */
    if (tok_eqi(mn,"clz")||tok_eqi(mn,"cls")||tok_eqi(mn,"rbit")||
        tok_eqi(mn,"rev")||tok_eqi(mn,"rev32")) {
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"clz"))   w=a64_clz((u8)rd,(u8)rn);
        else if (tok_eqi(mn,"cls"))   w=a64_cls((u8)rd,(u8)rn);
        else if (tok_eqi(mn,"rbit"))  w=a64_rbit((u8)rd,(u8)rn);
        else if (tok_eqi(mn,"rev32")) w=a64_rev32((u8)rd,(u8)rn);
        else                          w=a64_rev((u8)rd,(u8)rn);
        emit32(em,w); return;
    }
    /* ── extr xd, xn, xm, #lsb ── */
    if (tok_eqi(mn,"extr")) {
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 lsb=(u32)lex_imm(l);
        emit32(em, a64_extr((u8)rd,(u8)rn,(u8)rm,(u8)lsb)); return;
    }
    /* ── Polynomial multiply: pmull / pmull2 ── */
    if (tok_eqi(mn,"pmull")||tok_eqi(mn,"pmull2")) {
        int hi=tok_eqi(mn,"pmull2");
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        emit32(em, hi ? a64_pmull2((u8)vd,(u8)vn,(u8)vm)
                      : a64_pmull ((u8)vd,(u8)vn,(u8)vm));
        return;
    }
    /* ── Dot product: sdot / udot ── */
    if (tok_eqi(mn,"sdot")||tok_eqi(mn,"udot")) {
        int u=tok_eqi(mn,"udot");
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        emit32(em, u ? a64_udot_4s((u8)vd,(u8)vn,(u8)vm)
                     : a64_sdot_4s((u8)vd,(u8)vn,(u8)vm));
        return;
    }
    /* ── Interleave: zip2 / uzp1 / uzp2 / trn1 / trn2 ── */
    if (tok_eqi(mn,"zip2")||tok_eqi(mn,"uzp1")||tok_eqi(mn,"uzp2")||
        tok_eqi(mn,"trn1")||tok_eqi(mn,"trn2")) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"zip2")) w=a64_zip2_4s((u8)vd,(u8)vn,(u8)vm);
        else if (tok_eqi(mn,"uzp1")) w=a64_uzp1_4s((u8)vd,(u8)vn,(u8)vm);
        else if (tok_eqi(mn,"uzp2")) w=a64_uzp2_4s((u8)vd,(u8)vn,(u8)vm);
        else if (tok_eqi(mn,"trn1")) w=a64_trn1_4s((u8)vd,(u8)vn,(u8)vm);
        else                         w=a64_trn2_4s((u8)vd,(u8)vn,(u8)vm);
        emit32(em,w); return;
    }
    /* ── Table lookup: tbl / tbx ── */
    if (tok_eqi(mn,"tbl")||tok_eqi(mn,"tbx")) {
        int is_tbx=tok_eqi(mn,"tbx");
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_LBRK) lex_next(l); /* optional { */
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_RBRK) lex_next(l); /* optional } */
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        emit32(em, is_tbx ? a64_tbx_16b((u8)vd,(u8)vn,(u8)vm)
                           : a64_tbl_16b((u8)vd,(u8)vn,(u8)vm));
        return;
    }
    /* ── Vector extract: ext vd.16b, vn.16b, vm.16b, #imm ── */
    if (tok_eqi(mn,"ext")) {
        i32 vd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 vm=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 imm=(u32)lex_imm(l);
        emit32(em, a64_ext_16b((u8)vd,(u8)vn,(u8)vm,(u8)imm)); return;
    }
    /* ── Scalar FP: fadd/fsub/fmul/fdiv (sd,sn,sm) ── */
    if (tok_eqi(mn,"fadd")||tok_eqi(mn,"fsub")||
        tok_eqi(mn,"fmul")||tok_eqi(mn,"fdiv")) {
        i32 rd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"fadd")) w=a64_fadd_s((u8)rd,(u8)rn,(u8)rm);
        else if (tok_eqi(mn,"fsub")) w=a64_fsub_s((u8)rd,(u8)rn,(u8)rm);
        else if (tok_eqi(mn,"fmul")) w=a64_fmul_s((u8)rd,(u8)rn,(u8)rm);
        else                         w=a64_fdiv_s((u8)rd,(u8)rn,(u8)rm);
        emit32(em,w); return;
    }
    /* ── fsqrt / fabs / fneg (sd, sn) ── */
    if (tok_eqi(mn,"fsqrt")||tok_eqi(mn,"fabs")||tok_eqi(mn,"fneg")) {
        i32 rd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg_neon(l->cur); lex_next(l);
        u32 w;
        if      (tok_eqi(mn,"fsqrt")) w=a64_fsqrt_s((u8)rd,(u8)rn);
        else if (tok_eqi(mn,"fabs"))  w=a64_fabs_s((u8)rd,(u8)rn);
        else                          w=a64_fneg_s((u8)rd,(u8)rn);
        emit32(em,w); return;
    }
    /* ── fcvtzs xd, sn / scvtf sd, xn ── */
    if (tok_eqi(mn,"fcvtzs")) {
        i32 rd=reg64(l->cur);    lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg_neon(l->cur); lex_next(l);
        emit32(em, a64_fcvtzs((u8)rd,(u8)rn)); return;
    }
    if (tok_eqi(mn,"scvtf")) {
        i32 rd=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg64(l->cur);    lex_next(l);
        emit32(em, a64_scvtf((u8)rd,(u8)rn)); return;
    }
    /* ── fcmp sn, sm ── */
    if (tok_eqi(mn,"fcmp")) {
        i32 rn=reg_neon(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg_neon(l->cur); lex_next(l);
        emit32(em, a64_fcmp_s((u8)rn,(u8)rm)); return;
    }
    /* ── fmov: fmov vd, xn (gpr→fp)  or  fmov xd, vn (fp→gpr) ── */
    if (tok_eqi(mn,"fmov")) {
        /* if reg64 succeeds on first tok → fp→gpr direction; else gpr→fp */
        i32 r0=reg64(l->cur);
        if (r0>=0) {
            lex_next(l);
            if (l->cur.kind==TK_COMMA) lex_next(l);
            i32 vn=reg_neon(l->cur); lex_next(l);
            emit32(em, a64_fmov_s_to_gpr((u8)r0,(u8)vn));
        } else {
            i32 vd=reg_neon(l->cur); lex_next(l);
            if (l->cur.kind==TK_COMMA) lex_next(l);
            i32 xn=reg64(l->cur); lex_next(l);
            emit32(em, a64_fmov_gpr_to_s((u8)vd,(u8)xn));
        }
        return;
    }
    /* ── ldrsw xt, [xn, #imm*4] ── */
    if (tok_eqi(mn,"ldrsw")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        u32 imm=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); imm=(u32)lex_imm(l)/4u; }
        lex_next(l); /* ] */
        emit32(em, a64_ldrsw_imm((u8)rt,(u8)rn,imm)); return;
    }
    /* ── ld2/ld3/ld4/st2/st3/st4 {vt.4s,...}, [xn] ── */
    if (tok_eqi(mn,"ld2")||tok_eqi(mn,"ld3")||tok_eqi(mn,"ld4")||
        tok_eqi(mn,"st2")||tok_eqi(mn,"st3")||tok_eqi(mn,"st4")) {
        int nreg=(tok_eqi(mn,"ld2")||tok_eqi(mn,"st2"))?2
                :(tok_eqi(mn,"ld3")||tok_eqi(mn,"st3"))?3:4;
        int sto=tok_eqi(mn,"st2")||tok_eqi(mn,"st3")||tok_eqi(mn,"st4");
        if (l->cur.kind==TK_LBRK) lex_next(l); /* { */
        i32 vt=reg_neon(l->cur); if (vt<0) vt=0;
        while (l->cur.kind!=TK_RBRK&&l->cur.kind!=TK_NL&&l->cur.kind!=TK_EOF) lex_next(l);
        if (l->cur.kind==TK_RBRK) lex_next(l); /* } */
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg64(l->cur); lex_next(l);
        lex_next(l); /* ] */
        u32 w;
        if (sto) w=nreg==2?a64_st2_4s((u8)vt,(u8)rn):nreg==3?a64_st3_4s((u8)vt,(u8)rn):a64_st4_4s((u8)vt,(u8)rn);
        else     w=nreg==2?a64_ld2_4s((u8)vt,(u8)rn):nreg==3?a64_ld3_4s((u8)vt,(u8)rn):a64_ld4_4s((u8)vt,(u8)rn);
        emit32(em, w); return;
    }
    /* ── mrs xt, sysreg  /  msr sysreg, xt ── */
    if (tok_eqi(mn,"mrs")) {
        i32 rt=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 sr=A64_SR_NZCV;
        if      (tok_eqi(l->cur,"nzcv"))     sr=A64_SR_NZCV;
        else if (tok_eqi(l->cur,"daif"))     sr=A64_SR_DAIF;
        else if (tok_eqi(l->cur,"fpcr"))     sr=A64_SR_FPCR;
        else if (tok_eqi(l->cur,"fpsr"))     sr=A64_SR_FPSR;
        else if (tok_eqi(l->cur,"tpidr_el0")) sr=A64_SR_TPIDR;
        lex_next(l);
        emit32(em, a64_mrs((u8)rt,sr)); return;
    }
    if (tok_eqi(mn,"msr")) {
        u32 sr=A64_SR_NZCV;
        if      (tok_eqi(l->cur,"nzcv"))     sr=A64_SR_NZCV;
        else if (tok_eqi(l->cur,"daif"))     sr=A64_SR_DAIF;
        else if (tok_eqi(l->cur,"fpcr"))     sr=A64_SR_FPCR;
        else if (tok_eqi(l->cur,"fpsr"))     sr=A64_SR_FPSR;
        else if (tok_eqi(l->cur,"tpidr_el0")) sr=A64_SR_TPIDR;
        lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rt=reg64(l->cur); lex_next(l);
        emit32(em, a64_msr(sr,(u8)rt)); return;
    }
    /* unknown mnemonic — emit BRK so execution traps; mark assembly error */
    pr_err("apkc: unknown ARM64 mnemonic\n");
    emit32(em, 0xD4200020u); /* BRK #1 — traps on execution, not silent */
    em->err++;
}

/* ── ARM32 assembler ──────────────────────────────────────────────── */
static void asm_insn32(Emit *em, Tok mn, Lex *l) {
    lex_next(l);
    u32 pos=(u32)em->pos;

    if (tok_eq(mn,".sym1")) { em->sym1_va=pos; em->has_sym1=1; return; }
    if (tok_eq(mn,".sym2")) { em->sym2_va=pos; em->has_sym2=1; return; }

    if (tok_eqi(mn,"nop"))  { emit32(em,A32_NOP); return; }
    if (tok_eqi(mn,"bx"))   {
        /* bx lr or bx rN [,cond] */
        u32 cc=A32_AL;
        i32 rm=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) { lex_next(l); cc=parse_cc32(l->cur); lex_next(l); }
        emit32(em,a32_bx((u32)rm,cc)); return;
    }
    if (tok_eqi(mn,"swi")||tok_eqi(mn,"svc")) {
        u32 imm=(u32)lex_imm(l);
        emit32(em,a32_swi(imm,A32_AL)); return;
    }
    if (tok_eqi(mn,"push")) {
        /* push {r0,r1,...} */
        u32 regs=0;
        /* skip { if present */
        if (l->cur.kind==TK_LBRK||l->cur.kind==TK_IDENT) {
            /* parse register list */
            while (l->cur.kind!=TK_NL&&l->cur.kind!=TK_EOF&&l->cur.kind!=TK_RBRK) {
                if (l->cur.kind==TK_IDENT) {
                    i32 r=reg32a(l->cur);
                    if (r>=0) regs|=(1u<<(u32)r);
                    lex_next(l);
                } else lex_next(l);
            }
            if (l->cur.kind==TK_RBRK) lex_next(l);
        }
        emit32(em,a32_push(regs,A32_AL)); return;
    }
    if (tok_eqi(mn,"pop")) {
        u32 regs=0;
        while (l->cur.kind!=TK_NL&&l->cur.kind!=TK_EOF&&l->cur.kind!=TK_RBRK) {
            if (l->cur.kind==TK_IDENT) {
                i32 r=reg32a(l->cur);
                if (r>=0) regs|=(1u<<(u32)r);
                lex_next(l);
            } else lex_next(l);
        }
        if (l->cur.kind==TK_RBRK) lex_next(l);
        emit32(em,a32_pop(regs,A32_AL)); return;
    }
    if (tok_eqi(mn,"movw")||tok_eqi(mn,"movt")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 imm=(u32)lex_imm(l);
        u32 w=tok_eqi(mn,"movw")?a32_movw((u32)rd,imm,A32_AL):a32_movt((u32)rd,imm,A32_AL);
        emit32(em,w); return;
    }
    if (tok_eqi(mn,"mov")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            if (imm<=0xFFFFu) {
                emit32(em,a32_movw((u32)rd,imm,A32_AL));
                if (imm>0xFFu) emit32(em,a32_movt((u32)rd,imm>>16,A32_AL));
            } else {
                emit32(em,a32_movw((u32)rd,imm&0xFFFFu,A32_AL));
                emit32(em,a32_movt((u32)rd,imm>>16,A32_AL));
            }
        } else {
            i32 rm=reg32a(l->cur); lex_next(l);
            emit32(em,a32_mov_reg((u32)rd,(u32)rm,0u,A32_AL));
        }
        return;
    }
    if (tok_eqi(mn,"add")||tok_eqi(mn,"sub")||
        tok_eqi(mn,"and")||tok_eqi(mn,"orr")||tok_eqi(mn,"eor")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            u32 w;
            if      (tok_eqi(mn,"add")) w=a32_add_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL);
            else if (tok_eqi(mn,"sub")) w=a32_sub_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL);
            else if (tok_eqi(mn,"and")) w=a32_and_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL);
            else if (tok_eqi(mn,"orr")) w=a32_orr_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL);
            else                        w=a32_eor_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL);
            emit32(em,w);
        } else {
            i32 rm=reg32a(l->cur); lex_next(l);
            u32 w;
            if      (tok_eqi(mn,"add")) w=a32_add_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL);
            else if (tok_eqi(mn,"sub")) w=a32_sub_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL);
            else if (tok_eqi(mn,"and")) w=a32_and_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL);
            else if (tok_eqi(mn,"orr")) w=a32_orr_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL);
            else                        w=a32_eor_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL);
            emit32(em,w);
        }
        return;
    }
    if (tok_eqi(mn,"cmp")) {
        i32 rn=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            emit32(em,a32_cmp_imm((u32)rn,imm,0,A32_AL));
        } else {
            i32 rm=reg32a(l->cur); lex_next(l);
            emit32(em,a32_cmp_reg((u32)rn,(u32)rm,A32_AL));
        }
        return;
    }
    if (tok_eqi(mn,"b")||tok_eqi(mn,"bl")) {
        int is_bl=tok_eqi(mn,"bl");
        u32 cc=A32_AL;
        /* check for b.cond or bcond syntax in next token */
        if (l->cur.kind==TK_IDENT) {
            pat_add(pos,l->cur,32); lex_next(l);
            emit32(em,is_bl?a32_bl(0,cc):a32_b(0,cc)); return;
        }
        i32 off=(i32)lex_imm(l);
        emit32(em,is_bl?a32_bl(off/4,cc):a32_b(off/4,cc)); return;
    }
    if (tok_eqi(mn,"ldr")||tok_eqi(mn,"str")||
        tok_eqi(mn,"ldrh")||tok_eqi(mn,"strh")||
        tok_eqi(mn,"ldrb")||tok_eqi(mn,"strb")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        lex_next(l); /* [ */
        i32 rn=reg32a(l->cur); lex_next(l);
        i32 off=0;
        if (l->cur.kind==TK_COMMA) { lex_next(l); off=(i32)lex_imm(l); }
        lex_next(l); /* ] */
        u32 w;
        if (tok_eqi(mn,"ldr"))
            w=a32_ldr_imm((u32)rd,(u32)rn,(u16)(off>=0?off:-off),(u32)(off>=0),A32_AL);
        else if (tok_eqi(mn,"str"))
            w=a32_str_imm((u32)rd,(u32)rn,(u16)(off>=0?off:-off),(u32)(off>=0),A32_AL);
        else if (tok_eqi(mn,"ldrh"))
            w=a32_ldrh_imm((u32)rd,(u32)rn,(u8)(off>=0?off:-off),A32_AL);
        else if (tok_eqi(mn,"strh"))
            w=a32_strh_imm((u32)rd,(u32)rn,(u8)(off>=0?off:-off),A32_AL);
        else if (tok_eqi(mn,"ldrb"))
            w=a32_ldrb_imm((u32)rd,(u32)rn,(u16)(off>=0?off:-off),(u32)(off>=0),A32_AL);
        else
            w=a32_strb_imm((u32)rd,(u32)rn,(u16)(off>=0?off:-off),(u32)(off>=0),A32_AL);
        emit32(em,w); return;
    }
    if (tok_eqi(mn,"mul")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg32a(l->cur); lex_next(l);
        emit32(em,a32_mul((u32)rd,(u32)rn,(u32)rm,0,A32_AL)); return;
    }
    if (tok_eqi(mn,"mvn")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            emit32(em,a32_mvn_imm((u32)rd,(u8)imm,0,A32_AL));
        } else {
            i32 rm=reg32a(l->cur); lex_next(l);
            emit32(em,a32_mvn_reg((u32)rd,(u32)rm,0u,A32_AL));
        }
        return;
    }
    if (tok_eqi(mn,"neg")) {
        /* NEG Rd, Rm  ==  RSB Rd, Rm, #0 */
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg32a(l->cur); lex_next(l);
        emit32(em,a32_rsb_imm((u32)rd,(u32)rm,0,0,0,A32_AL)); return;
    }
    if (tok_eqi(mn,"rsb")||tok_eqi(mn,"bic")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rn=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        int is_rsb=tok_eqi(mn,"rsb");
        if (l->cur.kind==TK_HASH||l->cur.kind==TK_INT) {
            u32 imm=(u32)lex_imm(l);
            emit32(em, is_rsb ? a32_rsb_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL)
                              : a32_bic_imm((u32)rd,(u32)rn,(u8)imm,0,0,A32_AL));
        } else {
            i32 rm=reg32a(l->cur); lex_next(l);
            emit32(em, is_rsb ? a32_rsb_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL)
                              : a32_bic_reg((u32)rd,(u32)rn,(u32)rm,0u,A32_AL));
        }
        return;
    }
    if (tok_eqi(mn,"tst")||tok_eqi(mn,"teq")||tok_eqi(mn,"cmn")) {
        i32 rn=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg32a(l->cur); lex_next(l);
        u32 w = tok_eqi(mn,"tst") ? a32_tst_reg((u32)rn,(u32)rm,A32_AL)
              : tok_eqi(mn,"teq") ? a32_teq_reg((u32)rn,(u32)rm,A32_AL)
              :                     a32_cmn_reg((u32)rn,(u32)rm,A32_AL);
        emit32(em,w); return;
    }
    if (tok_eqi(mn,"lsl")||tok_eqi(mn,"lsr")||tok_eqi(mn,"asr")) {
        i32 rd=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        i32 rm=reg32a(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
        u32 sh=(u32)lex_imm(l);
        u32 w = tok_eqi(mn,"lsl") ? a32_lsl_imm((u32)rd,(u32)rm,(u8)sh,0,A32_AL)
              : tok_eqi(mn,"lsr") ? a32_lsr_imm((u32)rd,(u32)rm,(u8)sh,0,A32_AL)
              :                     a32_asr_imm((u32)rd,(u32)rm,(u8)sh,0,A32_AL);
        emit32(em,w); return;
    }
    if (tok_eqi(mn,"blx")) {
        i32 rm=reg32a(l->cur); lex_next(l);
        emit32(em,a32_blx((u32)rm)); return;
    }
    if (tok_eq(mn,".word")||tok_eqi(mn,".word")) {
        u32 v=(u32)lex_imm(l);
        emit32(em,v); return;
    }
    /* unknown mnemonic — emit UNDEF so execution faults; mark assembly error.
     * The UNDEF (not a silent NOP) guarantees a hard fault if it ever executes;
     * build_apk() additionally refuses to emit the APK unless --allow-undef is
     * set (see _apkc_allow_undef), so unknown mnemonics fail the build by default. */
    pr_err("apkc: unknown ARM32 mnemonic\n");
    emit32(em, 0xE7F000F0u); /* UNDEF instruction — architecturally undefined, not silent NOP */
    em->err++;
}

/* ── two-pass assembler ───────────────────────────────────────────── */
typedef struct { sz size; u32 sym1_va; u32 sym2_va; int has_sym1; int has_sym2; int err; } AsmResult;

static u8 _code64[0x10000];
static u8 _code32[0x10000];

static AsmResult assemble(const u8 *src, sz src_len, int arch, u8 *out_code) {
    AsmResult res; res.size=0; res.sym1_va=0; res.sym2_va=0;
    res.has_sym1=0; res.has_sym2=0;

    lbl_reset();
    Emit em; em.buf=out_code; em.cap=0x10000; em.pos=0;
    em.sym1_va=0; em.sym2_va=0; em.has_sym1=0; em.has_sym2=0;

    /* two passes: 1=labels, 2=code */
    for (int pass=0; pass<2; pass++) {
        em.pos=0;
        /* only the final real-emission pass (pass==1) is logged — the
         * sizing/label passes call asm_insn64() speculatively. */
        if (pass==1) {
            _codegen_variant_log_n=0;
            _codegen_variant_count[0]=_codegen_variant_count[1]=_codegen_variant_count[2]=0;
        }
        _codegen_log_on = (pass==1);
        Lex l; l.s=(const char*)src; l.e=(const char*)(src+src_len);
        lex_next(&l);
        while (l.cur.kind!=TK_EOF) {
            if (l.cur.kind==TK_NL) { lex_next(&l); continue; }
            if (l.cur.kind==TK_IDENT) {
                Tok mn=l.cur; lex_next(&l);
                /* check label */
                if (l.cur.kind==TK_COLON) {
                    lex_next(&l);
                    if (pass==0) lbl_def(mn.p[0]=='.'?(mn.p+1):(mn.p), (u32)em.pos); /* strip . prefix */
                    /* actually store full name */
                    if (pass==0) { char nb[80]; tok_copy(nb,mn); lbl_def(nb,(u32)em.pos); }
                    continue;
                }
                /* directive .section .text .globl .type etc — skip line */
                if (mn.p[0]=='.') {
                    if (tok_eq(mn,".sym1")||tok_eq(mn,".sym2")) {
                        if (pass==1) {
                            if (tok_eq(mn,".sym1")) { em.sym1_va=(u32)em.pos; em.has_sym1=1; }
                            else                    { em.sym2_va=(u32)em.pos; em.has_sym2=1; }
                        }
                    } else if (tok_eq(mn,".word")) {
                        if (pass==1) { u32 v=(u32)lex_imm(&l); emit32(&em,v); }
                        else         { lex_imm(&l); em.pos+=4; }
                        continue;
                    } else if (tok_eq(mn,".byte")) {
                        if (pass==1) { emit8(&em,(u8)lex_imm(&l)); }
                        else         { lex_imm(&l); em.pos+=1; }
                        continue;
                    } else if (tok_eq(mn,".ascii")||tok_eq(mn,".asciz")) {
                        int zterm=tok_eq(mn,".asciz");
                        if (l.cur.kind==TK_STR) {
                            sz slen=l.cur.len;
                            if (pass==1) { emit_bytes(&em,(const u8*)l.cur.p,slen); if(zterm) emit8(&em,0); }
                            else         { em.pos+=slen+(sz)zterm; }
                            lex_next(&l);
                        }
                        continue;
                    } else if (tok_eq(mn,".align")) {
                        u32 pow2=(u32)lex_imm(&l);
                        if (pass==1) emit_align(&em,pow2);
                        else {
                            sz mask=(sz)((1u<<pow2)-1u);
                            while (em.pos & mask) em.pos++;
                        }
                        continue;
                    }
                    /* skip rest of line for unknown directives */
                    while (l.cur.kind!=TK_NL&&l.cur.kind!=TK_EOF) lex_next(&l);
                    continue;
                }
                if (pass==1) {
                    if (arch==64) asm_insn64(&em,mn,&l);
                    else          asm_insn32(&em,mn,&l);
                } else {
                    /* pass 0: count bytes without emitting (use dummy buf) */
                    u8 *real=em.buf; em.buf=_code32; /* reuse as dummy */
                    sz rpos=em.pos;
                    if (arch==64) asm_insn64(&em,mn,&l);
                    else          asm_insn32(&em,mn,&l);
                    em.buf=real; em.pos=rpos; /* restore */
                }
            } else lex_next(&l);
        }
        if (pass==0) {
            /* also do a real first pass for label offsets — use actual buf */
            em.pos=0;
            Lex l2; l2.s=(const char*)src; l2.e=(const char*)(src+src_len);
            lex_next(&l2);
            while (l2.cur.kind!=TK_EOF) {
                if (l2.cur.kind==TK_NL) { lex_next(&l2); continue; }
                if (l2.cur.kind==TK_IDENT) {
                    Tok mn2=l2.cur; lex_next(&l2);
                    if (l2.cur.kind==TK_COLON) {
                        char nb[80]; tok_copy(nb,mn2);
                        lbl_def(nb,(u32)em.pos);
                        lex_next(&l2); continue;
                    }
                    if (mn2.p[0]=='.') {
                        if (tok_eq(mn2,".word"))  { lex_imm(&l2); em.pos+=4; }
                        else if (tok_eq(mn2,".byte")) { lex_imm(&l2); em.pos+=1; }
                        else if (tok_eq(mn2,".ascii")||tok_eq(mn2,".asciz")) {
                            int z=tok_eq(mn2,".asciz");
                            if (l2.cur.kind==TK_STR) { em.pos+=l2.cur.len+(sz)z; lex_next(&l2); }
                        }
                        else if (tok_eq(mn2,".align")) {
                            u32 p2=(u32)lex_imm(&l2);
                            sz mask=(sz)((1u<<p2)-1u);
                            while (em.pos & mask) em.pos++;
                        }
                        else while (l2.cur.kind!=TK_NL&&l2.cur.kind!=TK_EOF) lex_next(&l2);
                        continue;
                    }
                    /* simulate emit to count bytes */
                    u32 before=(u32)em.pos;
                    if (arch==64) asm_insn64(&em,mn2,&l2);
                    else          asm_insn32(&em,mn2,&l2);
                    (void)before;
                } else lex_next(&l2);
            }
        }
    }

    /* backpatch branches */
    for (u32 i=0;i<_npat;i++) {
        i32 li=lbl_find(_pats[i].tgt);
        if (li<0) continue;
        u32 loff=_lbls[li].off;
        u32 ioff=_pats[i].insn_off;
        u32 insn=r32(em.buf+ioff);
        i32 delta=(i32)((i64)loff-(i64)ioff);
        /* detect instruction type by bits */
        if (_pats[i].arch==64) {
            u32 op=insn>>26;
            if (op==0x05u) { /* B */
                insn=(insn&0xFC000000u)|((u32)(delta/4)&0x03FFFFFFu);
            } else if (op==0x25u) { /* BL */
                insn=(insn&0xFC000000u)|((u32)(delta/4)&0x03FFFFFFu);
            } else if ((insn&0xFF000010u)==0x54000000u) { /* B.cond */
                insn=(insn&0xFF00001Fu)|(((u32)(delta/4)&0x7FFFFu)<<5);
            } else if ((insn&0x7E000000u)==0x34000000u) { /* CBZ/CBNZ */
                insn=(insn&0xFF00001Fu)|(((u32)(delta/4)&0x7FFFFu)<<5);
            } else if ((insn&0xFF000000u)==0x58000000u) { /* LDR Xt, literal */
                insn=(insn&0xFF00001Fu)|(((u32)(delta/4)&0x7FFFFu)<<5);
            } else if (insn&0x80000000u) { /* ADRP: page-granular (delta in 4KiB pages) */
                i32 pdelta=delta>>12;
                u32 immlo=(u32)(pdelta)&3u;
                u32 immhi=(u32)(pdelta>>2)&0x7FFFFu;
                insn=(insn&0x9F00001Fu)|(immlo<<29)|(immhi<<5);
            } else { /* ADR: byte-granular */
                u32 immlo=(u32)(delta)&3u;
                u32 immhi=(u32)(delta>>2)&0x7FFFFu;
                insn=(insn&0x9F00001Fu)|(immlo<<29)|(immhi<<5);
            }
        } else {
            /* ARM32 B/BL: bits[23:0] = signed offset/4 - 2 */
            i32 enc=(delta/4)-2; /* PC is 8 ahead in A32 */
            insn=(insn&0xFF000000u)|((u32)enc&0x00FFFFFFu);
        }
        w32(em.buf+ioff,insn);
    }

    res.size=em.pos;
    res.sym1_va=em.sym1_va; res.sym2_va=em.sym2_va;
    res.has_sym1=em.has_sym1; res.has_sym2=em.has_sym2;
    res.err=em.err;
    return res;
}

/* ── APK builder ──────────────────────────────────────────────────── */
static u8 _axml_buf[0x4000];
static u8 _dex_buf[200];
static u8 _so64_buf[0x8000];
static u8 _so32_buf[0x8000];
static u8 _fork_out[0x100000]; /* fork+exec output buffer */

/* Build a ZIP path like "lib/arm64-v8a/lib<name>.so" */
static sz _make_so_path(u8 *out, const char *abi, const char *name) {
    sz i=0;
    const char *p="lib/"; while(*p) out[i++]=(u8)*p++;
    while(*abi) out[i++]=(u8)*abi++;
    const char *s="/lib"; while(*s) out[i++]=(u8)*s++;
    while(*name) out[i++]=(u8)*name++;
    const char *e=".so"; while(*e) out[i++]=(u8)*e++;
    out[i]=0; return i;
}

/* Write APK to outpath from ZIP writer */
static i32 _write_apk(ZipWr *zw, const char *outpath) {
    sz total=zip_finish(zw);
    if (!total) { pr_err("zip_finish failed\n"); return -1; }
    i32 fd=os_open(outpath,0x241,0x1A4);
    if (fd<0) { pr_err("open output failed\n"); return -1; }
    sz written=0;
    while (written<total) {
        sz chunk=total-written; if(chunk>0x8000) chunk=0x8000;
        i32 n=os_write(fd,_apk_buf+written,chunk);
        if (n<=0) break;
        written+=(sz)n;
    }
    if (os_close(fd)<0) { pr_err("close output failed\n"); return -1; }
    if (written!=total) { pr_err("partial APK write\n"); return -1; }
    pr("wrote "); pr_dec((u64)total); pr(" bytes to "); pr(outpath); pr_nl();
    return 0;
}

/*
 * fork_exec_wait: fork, exec compiler with args, read output file.
 * compiler: path or name of compiler to exec
 * args:     NULL-terminated argv (args[0] should be compiler name)
 * outfile:  path where compiler writes its output
 * outbuf/outsz: buffer to receive output bytes
 * Returns bytes read into outbuf, or 0 on error.
 *
 * NOTE: This is ARM64-only (uses os_fork/os_execve/os_waitpid from sys.h).
 */
#ifdef __aarch64__
static sz fork_exec_wait(const char *compiler, char *const args[],
                         const char *outfile, u8 *outbuf, sz outbuf_cap)
{
    i32 pid = os_fork();
    if (pid < 0) { pr_err("fork failed\n"); return 0; }
    if (pid == 0) {
        /* child: exec compiler */
        char *const envp[] = { NULL };
        os_execve(compiler, args, envp);
        os_exit(127); /* exec failed */
    }
    /* parent: wait for child */
    i32 status = 0;
    os_waitpid(pid, &status, 0);
    if (((status >> 8) & 0xFF) != 0) {
        pr_err("compiler exited with error\n"); return 0;
    }
    /* read output file */
    i32 fd = os_open(outfile, 0, 0);
    if (fd < 0) { pr_err("cannot open compiler output\n"); return 0; }
    sz total = 0;
    while (total < outbuf_cap) {
        i32 n = os_read(fd, outbuf + total, outbuf_cap - total);
        if (n <= 0) break;
        total += (sz)n;
    }
    os_close(fd);
    return total;
}
#else
static sz fork_exec_wait(const char *compiler, char *const args[],
                         const char *outfile, u8 *outbuf, sz outbuf_cap)
{
    (void)compiler; (void)args; (void)outfile; (void)outbuf; (void)outbuf_cap;
    pr_err("fork_exec_wait: not supported on ARM32\n");
    return 0;
}
#endif

/* Table-driven build_apk: LangProfile drives the entire pipeline */
static i32 build_apk(
    const u8 *src, sz src_len,
    const char *pkg, const char *label, const char *libname,
    u32 min_sdk, u32 tgt_sdk,
    int do64, int do32,
    const char *outpath,
    const LangProfile *prof,
    const char *inpath)  /* for fork+exec: write src to /tmp */
{
    AsmResult r64; r64.size=0; r64.sym1_va=0; r64.sym2_va=0; r64.has_sym1=0; r64.has_sym2=0;
    AsmResult r32_; r32_.size=0; r32_.sym1_va=0; r32_.sym2_va=0; r32_.has_sym1=0; r32_.has_sym2=0;
    sz so64sz=0, so32sz=0;
    sz dexsz=0;
    const u8 *dex_buf_ptr = _dex_buf;

    /* GPU / hardware-direct compute state */
    const u8   *gpu_asset_buf  = (const u8*)0;
    sz          gpu_asset_sz   = 0;
    const char *gpu_asset_name = (const char*)0;
    int         gpu_stub_so    = 0; /* set → build NOP stub .so for GPU APK */

    if (prof->use_asm) {
        /* internal ARM assembler */
        if (do64) r64  = assemble(src, src_len, 64, _code64);
        if (do32) r32_ = assemble(src, src_len, 32, _code32);

        /* L16 policy gate: refuse to ship UNDEF placeholders unless explicitly
         * allowed. Default behaviour is a hard FAIL (exit 1), so an unknown
         * mnemonic can never be silently packaged into a distributable APK. */
        if (r64.err || r32_.err) {
            pr_err("apkc: assembly produced "); pr_dec((u64)(r64.err + r32_.err));
            pr_err(" UNDEF placeholder(s) for unsupported instruction(s)\n");
            if (!_apkc_allow_undef) {
                pr_err("apkc: refusing to emit APK (use --allow-undef for experimental mode)\n");
                return 1;
            }
            pr_err("apkc: --allow-undef set; emitting UNDEF placeholders (EXPERIMENTAL)\n");
        }

        if (do64) {
            u8 *txt = r64.size ? _code64 : (u8*)0;
            ElfSym _es64[2] = {
                {"ANativeActivity_onCreate", r64.sym1_va},
                {"android_main", r64.has_sym2 ? r64.sym2_va
                                              : (r64.sym1_va ? r64.sym1_va+4u : 4u)}
            };
            so64sz = elf64_build_so(_so64_buf, txt, (u32)r64.size, _es64, 2, NULL, 0u);
        }
        if (do32) {
            u8 *txt = r32_.size ? _code32 : (u8*)0;
            ElfSym _es32[2] = {
                {"ANativeActivity_onCreate", r32_.sym1_va},
                {"android_main", r32_.has_sym2 ? r32_.sym2_va
                                               : (r32_.sym1_va ? r32_.sym1_va+4u : 4u)}
            };
            so32sz = elf32_build_so(_so32_buf, txt, (u32)r32_.size, _es32, 2);
        }

    } else if (prof->use_script) {
        /* inline execve bootstrap: interpreter runs source at app start */
        const char *ipath  = prof->compiler;
        const char *arg1   = prof->arg1;
        sz scsz = gen_script_code64(ipath, arg1, (const char*)src, _code64, sizeof(_code64));
        if (!scsz) { pr_err("script codegen failed\n"); return -1; }
        r64.size = scsz;
        ElfSym _scsyms[2] = {{"ANativeActivity_onCreate",0u},{"android_main",4u}};
        so64sz = elf64_build_so(_so64_buf, _code64, (u32)scsz, _scsyms, 2, NULL, 0u);
        do32 = 0; /* script bootstrap is arm64-only */

    } else if (prof->use_fork) {
        /* fork+exec external compiler */
        /* JSX babel writes to /tmp/jsx_out.js; all others write to /tmp/apkc_out.so */
        static const char _tmpout_so[]  = "/tmp/apkc_out.so";
        static const char _tmpout_jsx[] = "/tmp/jsx_out.js";
        const char *_tmpout = prof->jsx_node ? _tmpout_jsx : _tmpout_so;

        char *args[32];
        int na = 0;
        args[na++] = (char*)prof->compiler;
        for (int j = 0; j < 10 && prof->cc_args[j]; j++)
            args[na++] = (char*)prof->cc_args[j];
        args[na++] = (char*)_tmpout;
        args[na++] = (char*)inpath;
        args[na] = NULL;

        sz outsz = fork_exec_wait(prof->compiler, args, _tmpout,
                                  _fork_out, sizeof(_fork_out));
        if (!outsz) { pr_err("fork+exec produced no output\n"); return -1; }

        if (prof->jsx_node) {
            /* JSX two-stage: babel JS output → node execve bootstrap */
            sz scsz = gen_script_code64("/usr/bin/node", "-e",
                                        (const char*)_fork_out, _code64, sizeof(_code64));
            if (!scsz) { pr_err("jsx node codegen failed\n"); return -1; }
            ElfSym _jssyms[2] = {{"ANativeActivity_onCreate",0u},{"android_main",4u}};
            so64sz = elf64_build_so(_so64_buf, _code64, (u32)scsz, _jssyms, 2, NULL, 0u);

        } else if (prof->use_d8 && prof->dex_output) {
            /* Kotlin/Java: step 1 produced a JAR; step 2 runs d8 to get real DEX */
            static const char _d8_out[] = "/tmp/classes.dex";
            char *d8args[8];
            int nd = 0;
            d8args[nd++] = "d8";
            d8args[nd++] = "--output";
            d8args[nd++] = "/tmp/";
            d8args[nd++] = (char*)_tmpout_so;   /* JAR produced by kotlinc/javac */
            d8args[nd]   = NULL;
            sz dexout = fork_exec_wait("d8", d8args, _d8_out,
                                       _fork_out, sizeof(_fork_out));
            if (dexout) {
                /* d8 succeeded: use _fork_out directly (avoids 200B _dex_buf limit) */
                dex_buf_ptr = _fork_out;
                dexsz = dexout;
            } else {
                pr_err("d8 failed; refusing to package JAR bytes as classes.dex\n");
                return -1;
            }

        } else if (prof->dex_output) {
            /* dex_output but no d8 step: store JAR/DEX directly */
            m_cpy(_dex_buf, _fork_out, outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf));
            dexsz = outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf);

        } else {
            /* C/C++/Rust: output is a native .so */
            m_cpy(_so64_buf, _fork_out, outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf));
            so64sz = outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf);
        }
        do32 = 0;

    } else if (prof->use_gpu_spv) {
        /* Vulkan GLSL / HLSL compute: fork+exec glslc → SPIR-V APK asset.
         * APK layout: lib/arm64-v8a/libmain.so (NOP) + assets/compute.spv */
        static const char _tmpout_spv[] = "/tmp/apkc_compute.spv";
        char *args[32]; int na = 0;
        args[na++] = (char*)prof->compiler;
        for (int j = 0; j < 10 && prof->cc_args[j]; j++)
            args[na++] = (char*)prof->cc_args[j];
        args[na++] = (char*)_tmpout_spv;
        args[na++] = (char*)inpath;
        args[na]   = (char*)0;
        sz spvsz = fork_exec_wait(prof->compiler, args, _tmpout_spv,
                                  _fork_out, sizeof(_fork_out));
        if (!spvsz) { pr_err("shader compiler produced no SPIR-V output\n"); return -1; }
        gpu_asset_buf  = _fork_out;
        gpu_asset_sz   = spvsz;
        gpu_asset_name = "assets/compute.spv";
        gpu_stub_so    = 1;
        do32 = 0;

    } else if (prof->use_gpu_cl) {
        /* OpenCL C: embed source text as APK asset; no compilation needed.
         * Runtime loads via clCreateProgramWithSource from assets/compute.cl */
        gpu_asset_buf  = src;
        gpu_asset_sz   = src_len;
        gpu_asset_name = "assets/compute.cl";
        gpu_stub_so    = 1;
        do32 = 0;

    } else if (prof->use_gpu_wgsl) {
        /* WebGPU WGSL: embed source text as APK asset.
         * Runtime compiles WGSL via WebGPU / Dawn at load time. */
        gpu_asset_buf  = src;
        gpu_asset_sz   = src_len;
        gpu_asset_name = "assets/compute.wgsl";
        gpu_stub_so    = 1;
        do32 = 0;

    } else if (prof->use_dsp) {
        /* Hexagon DSP: fork+exec hexagon-clang → DSP .so for FastRPC offload.
         * APK layout: lib/arm64-v8a/libmain.so (NOP) +
         *             lib/hexagon-v65/libcompute.so (Hexagon DSP payload) */
        static const char _tmpout_dsp[] = "/tmp/apkc_dsp.so";
        char *args[32]; int na = 0;
        args[na++] = (char*)prof->compiler;
        for (int j = 0; j < 10 && prof->cc_args[j]; j++)
            args[na++] = (char*)prof->cc_args[j];
        args[na++] = (char*)_tmpout_dsp;
        args[na++] = (char*)inpath;
        args[na]   = (char*)0;
        sz dspsz = fork_exec_wait(prof->compiler, args, _tmpout_dsp,
                                  _fork_out, sizeof(_fork_out));
        if (!dspsz) { pr_err("hexagon-clang produced no DSP output\n"); return -1; }
        gpu_asset_buf  = _fork_out;
        gpu_asset_sz   = dspsz;
        gpu_asset_name = "lib/hexagon-v65/libcompute.so";
        gpu_stub_so    = 1;
        do32 = 0;
    }

    /* GPU bootstrap: build NOP stub .so for GPU/DSP compute APKs.
     * The real compute payload is packaged as an APK asset (see gpu_asset_name).
     * ANativeActivity_onCreate is a NOP; GPU dispatch happens via Java + NDK. */
    if (gpu_stub_so) {
        static const u8 _gpu_stub[8] = {
            0xC0u,0x03u,0x5Fu,0xD6u,  /* RET (x30) — ANativeActivity_onCreate NOP */
            0x1Fu,0x20u,0x03u,0xD5u,  /* NOP — android_main placeholder */
        };
        ElfSym _gs[2] = {
            {"ANativeActivity_onCreate", 0u},
            {"android_main",             0u},
        };
        so64sz = elf64_build_so(_so64_buf, _gpu_stub, 8u, _gs, 2, NULL, 0u);
        do64 = 1;
    }

    /* build AndroidManifest.xml */
    sz axsz = axml_build(pkg, label, libname, min_sdk, tgt_sdk,
                         NULL, 0, NULL, 0, _axml_buf, sizeof(_axml_buf));
    if (!axsz) { pr_err("axml_build failed\n"); return -1; }

    /* build classes.dex if not already produced by fork+exec */
    if (!dexsz) dexsz = dex_build(_dex_buf, sizeof(_dex_buf));

    /* assemble ZIP */
    ZipWr zw;
    zip_init(&zw, _apk_buf, sizeof(_apk_buf));

    if (prof->dex_output && dexsz > 200) {
        /* Kotlin/Java: real DEX from compiler or d8 */
        if (zip_add(&zw, "classes.dex", dex_buf_ptr, (u32)dexsz)<0) { pr_err("zip_add classes.dex failed\n"); return -1; }
    } else {
        /* native .so output */
        if (do64 && so64sz) {
            u8 p64[80]; _make_so_path(p64, "arm64-v8a", libname);
            if (zip_add(&zw, (const char*)p64, _so64_buf, (u32)so64sz)<0) { pr_err("zip_add arm64 lib failed\n"); return -1; }
        }
        if (do32 && so32sz) {
            u8 p32[80]; _make_so_path(p32, "armeabi-v7a", libname);
            if (zip_add(&zw, (const char*)p32, _so32_buf, (u32)so32sz)<0) { pr_err("zip_add arm32 lib failed\n"); return -1; }
        }
        if (zip_add(&zw, "classes.dex", dex_buf_ptr, (u32)dexsz)<0) { pr_err("zip_add classes.dex failed\n"); return -1; }
    }
    /* GPU / DSP compute asset (SPIR-V blob, OpenCL source, WGSL, or DSP .so) */
    if (gpu_asset_buf && gpu_asset_sz && gpu_asset_name) {
        if (zip_add(&zw, gpu_asset_name, gpu_asset_buf, (u32)gpu_asset_sz)<0)
            { pr_err("zip_add gpu asset failed\n"); return -1; }
        pr("apkc: gpu asset="); pr(gpu_asset_name);
        pr(" sz="); pr_dec((u64)gpu_asset_sz); pr_nl();
    }
    if (zip_add(&zw, "AndroidManifest.xml", _axml_buf, (u32)axsz)<0) { pr_err("zip_add manifest failed\n"); return -1; }

    i32 rc = _write_apk(&zw, outpath);
    if (rc == 0) {
        /* Compute and print geometric coherence (T^7 phi_fst invariant) */
        sz apksz = zip_finish(&zw); /* re-query size after write */
        u32 phi = phi_fst(_apk_buf, (u32)(apksz < 0x100000u ? apksz : 0x100000u));
        u32 phi_int  = phi >> 16;
        u32 phi_frac = (u32)(((u64)(phi & 0xFFFFu) * 10000u) >> 16);
        u32 attr     = phi_attractor(phi);
        pr("[phi="); pr_dec((u64)phi_int); pr(".");
        /* 4-digit zero-padded fraction */
        if (phi_frac < 1000u) pr("0");
        if (phi_frac <  100u) pr("0");
        if (phi_frac <   10u) pr("0");
        pr_dec((u64)phi_frac);
        pr(" attractor="); pr_dec((u64)attr); pr("]\n");
        if (_codegen_variant_log_n) {
            pr("[codegen mov_family: total="); pr_dec((u64)_codegen_variant_log_n);
            pr(" orr="); pr_dec((u64)_codegen_variant_count[0]);
            pr(" add0="); pr_dec((u64)_codegen_variant_count[1]);
            pr(" sub0="); pr_dec((u64)_codegen_variant_count[2]);
            pr("]\n");
        }
    }
    return rc;
}

/* ── CLI ──────────────────────────────────────────────────────────── */
static u8 _src_local[0x100000];

static int _str_eq(const char *a, const char *b) {
    while (*a && *b && *a==*b) { a++; b++; }
    return *a==*b;
}

static i32 apkc_main(i32 argc, char **argv) {
    const char *inpath  = 0;
    const char *outpath = "out.apk";
    const char *pkg     = "com.example.app";
    const char *label   = "App";
    const char *libname = "main";
    const char *lang_override = 0;
    u32 min_sdk = 21;
    u32 tgt_sdk = 33;
    int do64 = 1, do32 = 1;

    for (i32 i=1; i<argc; i++) {
        char *a = argv[i];
        if (a[0]=='-'&&a[1]=='o'&&a[2]==0 && i+1<argc) { outpath=argv[++i]; continue; }
        if (a[0]=='-'&&a[1]=='p'&&a[2]==0 && i+1<argc) { pkg=argv[++i]; continue; }
        if (a[0]=='-'&&a[1]=='l'&&a[2]==0 && i+1<argc) { label=argv[++i]; continue; }
        if (a[0]=='-'&&a[1]=='n'&&a[2]==0 && i+1<argc) { libname=argv[++i]; continue; }
        if (a[0]=='-'&&a[1]=='m'&&a[2]==0 && i+1<argc) {
            min_sdk=0; char*s=argv[++i]; while(*s) min_sdk=min_sdk*10+(u32)(*s++-'0'); continue;
        }
        if (a[0]=='-'&&a[1]=='t'&&a[2]==0 && i+1<argc) {
            tgt_sdk=0; char*s=argv[++i]; while(*s) tgt_sdk=tgt_sdk*10+(u32)(*s++-'0'); continue;
        }
        if (a[0]=='-'&&a[1]=='6'&&a[2]=='4'&&a[3]==0) { do64=1; do32=0; continue; }
        if (a[0]=='-'&&a[1]=='3'&&a[2]=='2'&&a[3]==0) { do64=0; do32=1; continue; }
        if (a[0]=='-'&&a[1]=='b'&&a[2]=='o') { do64=1; do32=1; continue; }
        if (_str_eq(a,"-lang") && i+1<argc) { lang_override=argv[++i]; continue; }
        if (_str_eq(a,"--allow-undef") || _str_eq(a,"--allow-nop-placeholder")) { _apkc_allow_undef=1; continue; }
        if (_str_eq(a,"--strict")) { _apkc_allow_undef=0; continue; }
        if (_str_eq(a,"--hw-probe")) {
            HWProfile hw; hw_probe(&hw);
            pr("apkc: hw-probe: "); hw_caps_pr(&hw);
            return 0;
        }
        if (a[0]!='-') { inpath=a; continue; }
        pr_err("unknown flag: "); pr_err(a); pr_err("\n");
    }

    if (!inpath) {
        pr_err("usage: apkc [options] source\n");
        pr_err("  -o <file>   output APK (default: out.apk)\n");
        pr_err("  -p <pkg>    package name\n");
        pr_err("  -l <label>  app label\n");
        pr_err("  -n <name>   native lib name\n");
        pr_err("  -m <sdk>    minSdkVersion\n");
        pr_err("  -t <sdk>    targetSdkVersion\n");
        pr_err("  -64/-32/-both  architecture filter\n");
        pr_err("  -lang <name>   force language\n");
        pr_err("    CPU: asm c cpp rs kt java py sh pl js php jsx go rb swift groovy clj\n");
        pr_err("    GPU: glsl cl hlsl wgsl dsp\n");
        pr_err("  --strict       fail build on unknown mnemonic (default)\n");
        pr_err("  --allow-undef  emit UNDEF placeholder for unknown mnemonic (experimental)\n");
        pr_err("  --hw-probe     detect and print hardware capabilities (CPU/GPU/DSP/NPU)\n");
        pr_err("  (language auto-detected from file extension otherwise)\n");
        return 1;
    }

    /* detect language from extension or override */
    const LangProfile *prof = lang_override
        ? lang_profile_find(lang_override)
        : lang_profile_from_path(inpath);
    if (!prof) { pr_err("unknown -lang value\n"); return 1; }

    /* arm64-only languages can't build arm32 */
    if (prof->arm64_only) do32 = 0;

    /* read source */
    sz src_len = 0;
    i32 fd = os_open(inpath, 0, 0);
    if (fd<0) { pr_err("cannot open: "); pr_err(inpath); pr_err("\n"); return 1; }
    while (src_len < sizeof(_src_local)-1) {
        i32 n = os_read(fd, _src_local+src_len, sizeof(_src_local)-src_len-1);
        if (n<=0) break;
        src_len += (sz)n;
    }
    os_close(fd);
    _src_local[src_len] = 0;

    pr("apkc: lang="); pr(prof->name); pr(" src="); pr(inpath); pr_nl();

    return build_apk(_src_local, src_len, pkg, label, libname,
                     min_sdk, tgt_sdk, do64, do32, outpath, prof, inpath);
}

/* ── freestanding entry ───────────────────────────────────────────── */
__attribute__((used))
static i32 apkc_entry(uptr *sp) {
    i32 argc=(i32)sp[0];
    char **argv=(char**)(sp+1);
    return apkc_main(argc,argv);
}

#if defined(__aarch64__)
__attribute__((naked,noreturn,section(".text.start")))
void _start(void) {
    __asm__ __volatile__(
        "mov x0, sp\n"
        "and sp, x0, #~15\n"
        "bl apkc_entry\n"
        "mov x8, #93\n"
        "svc #0\n"
        ::: "x0","x8","memory"
    );
}
#elif defined(__arm__)
__attribute__((naked,noreturn,section(".text.start")))
void _start(void) {
    __asm__ __volatile__(
        "mov r0, sp\n"
        "bic sp, r0, #7\n"
        "bl apkc_entry\n"
        "mov r7, #1\n"
        "swi #0\n"
        ::: "r0","r7","memory"
    );
}
#endif
