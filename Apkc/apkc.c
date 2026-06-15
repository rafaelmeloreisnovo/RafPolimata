/* apkc.c — freestanding APK compiler, no libc, no heap, no abstractions.
 * Unified pipeline: 12 languages via declarative LangProfile table.
 * Technological determinism — source extension drives the whole pipeline. */
#include "sys.h"
#include "mem.h"
#include "arch_arm64.h"
#include "arch_arm32.h"
#include "lang_script.h"
#include "lang_profile.h"
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
            /* mov rd, rn — ORR rd, xzr, rn */
            i32 rn=reg64(l->cur); lex_next(l);
            /* ORR (shifted reg): sf|01010|shift2|0|rm5|imm6|rn5|rd5 */
            u32 sf2=(u32)sf;
            u32 w=((sf2)<<31)|(0x2Au<<24)|((u32)rn<<16)|(31u<<5)|(u32)rd;
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
    /* ldr/str rd, [rn, #off] */
    if (tok_eqi(mn,"ldr")||tok_eqi(mn,"str")) {
        int sf=reg64_sf(l->cur);
        i32 rd=reg64(l->cur); lex_next(l);
        if (l->cur.kind==TK_COMMA) lex_next(l);
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
        /* placeholder */
        emit32(em,a64_adr((u32)rd,0)); return;
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
    /* unknown mnemonic — emit NOP placeholder so label offsets stay correct */
    pr_err("apkc: unknown ARM64 mnemonic\n");
    emit32(em,A64_NOP);
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
    if (tok_eq(mn,".word")||tok_eqi(mn,".word")) {
        u32 v=(u32)lex_imm(l);
        emit32(em,v); return;
    }
    /* unknown mnemonic — emit NOP placeholder so label offsets stay correct */
    pr_err("apkc: unknown ARM32 mnemonic\n");
    emit32(em,A32_NOP);
}

/* ── two-pass assembler ───────────────────────────────────────────── */
typedef struct { sz size; u32 sym1_va; u32 sym2_va; int has_sym1; int has_sym2; } AsmResult;

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
            } else if ((insn&0x7E000000u)==0x36000000u) { /* TBZ/TBNZ imm14 */
                insn=(insn&0xFFF8001Fu)|(((u32)(delta/4)&0x3FFFu)<<5);
            } else { /* ADR fallback */
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
    const u8 *dex_buf_ptr = _dex_buf; /* points to whichever buffer holds the DEX */

    if (prof->use_asm) {
        /* internal ARM assembler */
        if (do64) r64  = assemble(src, src_len, 64, _code64);
        if (do32) r32_ = assemble(src, src_len, 32, _code32);

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
    }

    /* build AndroidManifest.xml */
    sz axsz = axml_build(pkg, label, libname, min_sdk, tgt_sdk,
                         NULL, 0, NULL, 0, _axml_buf, sizeof(_axml_buf));
    if (!axsz) { pr_err("axml_build failed\n"); return -1; }

    /* build classes.dex if not already produced by fork+exec */
    if (!dexsz) dexsz = dex_build(_dex_buf);

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
    if (zip_add(&zw, "AndroidManifest.xml", _axml_buf, (u32)axsz)<0) { pr_err("zip_add manifest failed\n"); return -1; }

    return _write_apk(&zw, outpath);
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
        pr_err("  -lang <name>   force language (asm/c/cpp/rs/kt/java/py/sh/pl/js/php/jsx)\n");
        pr_err("  (auto-detected from file extension otherwise)\n");
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
