/* fmt_elf.h — Minimal ELF shared library generator.
 * Exports caller-specified symbols at caller-specified code offsets.
 * ARM64: 9 sections — .text, .rodata (optional), .hash, .dynsym, .dynstr,
 *        .dynamic, .shstrtab, .ARM.attributes (ARMv8-A + VFPv4 + NEONv2).
 * ARM32: 7 sections (no .rodata, no .ARM.attributes).
 * Symbols and dynstr are built dynamically on the stack — no heap. */
#pragma once
#include "arch_arm64.h"
#include "arch_arm32.h"

/* ── ELF constants ────────────────────────────────────────────────────────── */
#define ET_DYN              3u
#define EM_AARCH64          0xB7u
#define EM_ARM              0x28u
#define PT_LOAD             1u
#define PT_DYNAMIC          2u
#define PF_R                4u
#define PF_W                2u
#define PF_X                1u
#define SHT_NULL            0u
#define SHT_PROGBITS        1u
#define SHT_STRTAB          3u
#define SHT_HASH            5u
#define SHT_DYNAMIC         6u
#define SHT_DYNSYM          11u
#define SHF_ALLOC           0x2u
#define SHF_EXECINSTR       0x4u
#define SHF_WRITE           0x1u
#define STB_GLOBAL          1u
#define STT_FUNC            2u
#define STV_DEFAULT         0u
#define DT_NULL             0u
#define DT_HASH             4u
#define DT_STRTAB           5u
#define DT_SYMTAB           6u
#define DT_STRSZ            10u
#define DT_SYMENT           11u
#define SHT_ARM_ATTRIBUTES  0x70000003u

/* ── ELF buffer writer ───────────────────────────────────────────────────── */
typedef struct { u8 *p; sz pos; } ELFBuf;
static inline void _eb8 (ELFBuf *b, u8  v){ b->p[b->pos++]=v; }
static inline void _eb16(ELFBuf *b, u16 v){ w16(b->p+b->pos,v); b->pos+=2; }
static inline void _eb32(ELFBuf *b, u32 v){ w32(b->p+b->pos,v); b->pos+=4; }
static inline void _eb64(ELFBuf *b, u64 v){ w64(b->p+b->pos,v); b->pos+=8; }
static inline void _epad(ELFBuf *b, sz n) { for(sz i=0;i<n;i++) b->p[b->pos++]=0; }

/* ── Symbol descriptor ───────────────────────────────────────────────────── */
/* va = byte offset from the start of the .text code buffer */
typedef struct { const char *name; u32 va; } ElfSym;

/* ── Stack-built .dynstr ─────────────────────────────────────────────────── */
/* sym_name_off[i] = byte offset of syms[i].name within the dynstr buffer. */
static inline u32 _build_dynstr(const ElfSym *syms, int nsyms,
                                  u8 *buf, u32 *sym_name_off) {
    u32 pos = 0;
    buf[pos++] = 0; /* leading null */
    for (int i = 0; i < nsyms; i++) {
        sym_name_off[i] = pos;
        const char *n = syms[i].name ? syms[i].name : "";
        while (*n) buf[pos++] = (u8)*n++;
        buf[pos++] = 0;
    }
    buf[pos++] = 0; /* trailing null */
    while (pos & 3u) buf[pos++] = 0; /* align to 4 */
    return pos;
}

/* ── Stack-built ELF hash table (nbucket=1) ─────────────────────────────── */
/* nbucket=1 chains all symbols sequentially — always correct, never wrong. */
static inline u32 _build_elfhash(int nsyms, u8 *buf) {
    u32 nchain = (u32)(nsyms + 1);
    w32(buf+0,  1u);                      /* nbucket */
    w32(buf+4,  nchain);                  /* nchain  */
    w32(buf+8,  nsyms>0 ? 1u : 0u);      /* bucket[0] → first non-null sym */
    w32(buf+12, 0u);                      /* chain[0] = 0 (null sym) */
    for (int i = 0; i < nsyms-1; i++)
        w32(buf+16+i*4, (u32)(i+2));      /* chain[i+1] → next sym */
    if (nsyms > 0)
        w32(buf+16+(nsyms-1)*4, 0u);      /* chain[nsyms] = 0 (end) */
    return (2u + 1u + nchain) * 4u;
}

/* ── .shstrtab for ARM64 (9 sections) ───────────────────────────────────── */
static const u8 _shstrtab64[] =
    "\x00"                   /* 0  */
    ".text\x00"              /* 1  */
    ".rodata\x00"            /* 7  */
    ".hash\x00"              /* 15 */
    ".dynsym\x00"            /* 21 */
    ".dynstr\x00"            /* 29 */
    ".dynamic\x00"           /* 37 */
    ".shstrtab\x00"          /* 46 */
    ".ARM.attributes\x00";   /* 56 */
#define SHSTRTAB64_SZ  72u
#define SHN64_TEXT      1u
#define SHN64_RODATA    7u
#define SHN64_HASH     15u
#define SHN64_DYNSYM   21u
#define SHN64_DYNSTR   29u
#define SHN64_DYN      37u
#define SHN64_SHSTR    46u
#define SHN64_ARMATTR  56u

/* ── .ARM.attributes content — ARMv8-A + VFPv4 + NEONv2 ────────────────── */
static const u8 _arm_attrs[] = {
    0x41,                           /* 'A' format version */
    0x15,0x00,0x00,0x00,            /* vendor section length = 21 */
    0x61,0x65,0x61,0x62,0x69,0x00, /* "aeabi\0" */
    0x01,                           /* Tag_File = 1 */
    0x0B,0x00,0x00,0x00,            /* sub-section size = 11 (1+4+6) */
    0x06,0x0E,                      /* Tag_CPU_arch = 14 (ARMv8-A) */
    0x0A,0x04,                      /* Tag_FP_arch  = 4  (VFPv4)   */
    0x0C,0x02,                      /* Tag_Advanced_SIMD_arch = 2 (NEONv2) */
};
#define ARM_ATTRS_SZ 22u

/* ── .shstrtab for ARM32 (7 sections) ───────────────────────────────────── */
static const u8 _shstrtab32[] =
    "\x00"           /* 0  */
    ".text\x00"      /* 1  */
    ".hash\x00"      /* 7  */
    ".dynsym\x00"    /* 13 */
    ".dynstr\x00"    /* 21 */
    ".dynamic\x00"   /* 29 */
    ".shstrtab\x00"; /* 38 */
#define SHSTRTAB32_SZ 48u
#define SHN32_TEXT    1u
#define SHN32_HASH    7u
#define SHN32_DYNSYM 13u
#define SHN32_DYNSTR 21u
#define SHN32_DYN    29u
#define SHN32_SHSTR  38u

/* ═══════════════════════════ ARM64 .so generator ═══════════════════════════
 * Layout: ELF header (64B) + 2×PHT (56B each) = 0x0B0 for .text start.
 * Sections: 0=null 1=text 2=rodata 3=hash 4=dynsym 5=dynstr 6=dynamic
 *           7=shstrtab 8=ARM.attributes
 * sym.va values are code-buffer byte offsets (translated to file offsets here).
 * rodata/rodata_sz may be NULL/0; section header is still emitted (size=0).
 */
#define A64SO_TEXT_BASE 0x0B0u
#define A64SO_NDYN      6u       /* DT_* entries in .dynamic */
#define A64SO_NSECT     9u       /* total section headers */

static sz elf64_build_so(u8 *out, const u8 *text, u32 text_sz,
                          const ElfSym *syms, int nsyms,
                          const u8 *rodata, u32 rodata_sz)
{
    if (nsyms > 8) nsyms = 8;

    /* Build dynstr + elfhash on stack */
    u8  _ds[512]; u32 _sym_off[8];
    u32 dynstr_sz  = _build_dynstr(syms, nsyms, _ds, _sym_off);
    u8  _eh[64];
    u32 elfhash_sz = _build_elfhash(nsyms, _eh);

    /* Compute section layout */
    u32 TEXT_OFF    = A64SO_TEXT_BASE;
    u32 actual_text = text_sz   ? u32_aln(text_sz,   4u) : 16u;
    u32 actual_rdat = rodata_sz ? u32_aln(rodata_sz, 4u) : 0u;
    u32 RODATA_OFF  = u32_aln(TEXT_OFF    + actual_text,           8u);
    u32 HASH_OFF    = u32_aln(RODATA_OFF  + actual_rdat,           8u);
    u32 DSYM_OFF    = u32_aln(HASH_OFF    + elfhash_sz,            8u);
    u32 DSTR_OFF    = u32_aln(DSYM_OFF    + (u32)(nsyms+1)*24u,   8u);
    u32 DYN_OFF     = u32_aln(DSTR_OFF    + dynstr_sz,             8u);
    u32 SHSTR_OFF   = u32_aln(DYN_OFF     + A64SO_NDYN*16u,       8u);
    u32 ARMATTR_OFF = u32_aln(SHSTR_OFF   + SHSTRTAB64_SZ,        8u);
    u32 SHT_OFF     = u32_aln(ARMATTR_OFF + ARM_ATTRS_SZ,         8u);
    u32 TOTAL       = SHT_OFF + A64SO_NSECT*64u;

    m_set(out, 0, (sz)TOTAL);
    ELFBuf B; B.p = out; B.pos = 0;

    /* ELF header (64 bytes) */
    _eb8(&B,0x7F);_eb8(&B,'E');_eb8(&B,'L');_eb8(&B,'F');
    _eb8(&B,2); _eb8(&B,1); _eb8(&B,1); _eb8(&B,0);
    _epad(&B,8);
    _eb16(&B,(u16)ET_DYN);
    _eb16(&B,(u16)EM_AARCH64);
    _eb32(&B,1u);
    _eb64(&B,0u);              /* e_entry */
    _eb64(&B,0x40u);           /* e_phoff */
    _eb64(&B,(u64)SHT_OFF);    /* e_shoff */
    _eb32(&B,0u);              /* e_flags */
    _eb16(&B,64u);             /* e_ehsize */
    _eb16(&B,56u);             /* e_phentsize */
    _eb16(&B,2u);              /* e_phnum */
    _eb16(&B,64u);             /* e_shentsize */
    _eb16(&B,(u16)A64SO_NSECT);
    _eb16(&B,7u);              /* e_shstrndx = 7 (.shstrtab) */

    /* Program header table (2 × 56 bytes) */
    /* PT_LOAD: covers entire file */
    _eb32(&B,(u32)PT_LOAD);  _eb32(&B,(u32)(PF_R|PF_X));
    _eb64(&B,0u); _eb64(&B,0u); _eb64(&B,0u);
    _eb64(&B,(u64)TOTAL); _eb64(&B,(u64)TOTAL); _eb64(&B,0x1000u);
    /* PT_DYNAMIC */
    _eb32(&B,(u32)PT_DYNAMIC); _eb32(&B,(u32)(PF_R|PF_W));
    _eb64(&B,(u64)DYN_OFF); _eb64(&B,(u64)DYN_OFF); _eb64(&B,(u64)DYN_OFF);
    _eb64(&B,(u64)(A64SO_NDYN*16u)); _eb64(&B,(u64)(A64SO_NDYN*16u));
    _eb64(&B,8u);

    /* .text */
    if (text && text_sz) {
        m_cpy(out+TEXT_OFF, text, (sz)text_sz);
    } else {
        w32(out+TEXT_OFF+0, A64_RET); w32(out+TEXT_OFF+4, A64_RET);
        w32(out+TEXT_OFF+8, A64_NOP); w32(out+TEXT_OFF+12, A64_NOP);
    }

    /* .rodata (may be empty) */
    if (rodata && rodata_sz) m_cpy(out+RODATA_OFF, rodata, (sz)rodata_sz);

    /* .hash */
    m_cpy(out+HASH_OFF, _eh, elfhash_sz);

    /* .dynsym — Elf64_Sym: name(4) info(1) other(1) shndx(2) value(8) size(8) */
    u8 *ds = out + DSYM_OFF;
    m_set(ds, 0, 24u); /* sym[0] = null symbol */
    for (int i = 0; i < nsyms; i++) {
        u8 *se = ds + (i+1)*24;
        u32 fva = TEXT_OFF + syms[i].va;
        w32(se+0,  _sym_off[i]);
        se[4] = (u8)((STB_GLOBAL<<4)|STT_FUNC);
        se[5] = STV_DEFAULT;
        w16(se+6,  1u);         /* st_shndx = 1 (.text) */
        w64(se+8,  (u64)fva);
        w64(se+16, 4u);
    }

    /* .dynstr */
    m_cpy(out+DSTR_OFF, _ds, dynstr_sz);

    /* .dynamic (6 × 16 bytes) */
    u8 *dy = out + DYN_OFF;
    w64(dy,    (u64)DT_HASH);   w64(dy+8,  (u64)HASH_OFF);  dy+=16;
    w64(dy,    (u64)DT_STRTAB); w64(dy+8,  (u64)DSTR_OFF);  dy+=16;
    w64(dy,    (u64)DT_SYMTAB); w64(dy+8,  (u64)DSYM_OFF);  dy+=16;
    w64(dy,    (u64)DT_STRSZ);  w64(dy+8,  (u64)dynstr_sz); dy+=16;
    w64(dy,    (u64)DT_SYMENT); w64(dy+8,  24ULL);           dy+=16;
    w64(dy,    0ULL);           w64(dy+8,  0ULL);

    /* .shstrtab */
    m_cpy(out+SHSTR_OFF, _shstrtab64, SHSTRTAB64_SZ);

    /* .ARM.attributes */
    m_cpy(out+ARMATTR_OFF, _arm_attrs, ARM_ATTRS_SZ);

    /* Section header table (9 × 64 bytes)
     * Indices: 0=null 1=text 2=rodata 3=hash 4=dynsym 5=dynstr 6=dyn 7=shstr 8=armattr */
#define SH64(nm,ty,fl,ad,of,sz,lk,in,al,es) do { \
    _eb32(&B,(nm));_eb32(&B,(ty)); \
    _eb64(&B,(fl));_eb64(&B,(ad)); \
    _eb64(&B,(of));_eb64(&B,(sz)); \
    _eb32(&B,(lk));_eb32(&B,(in)); \
    _eb64(&B,(al));_eb64(&B,(es)); \
} while(0)
    B.pos = SHT_OFF;
    SH64(0, SHT_NULL, 0,0,0,0, 0,0,0,0);
    SH64(SHN64_TEXT,    SHT_PROGBITS, SHF_ALLOC|SHF_EXECINSTR,
         TEXT_OFF,   TEXT_OFF,   actual_text,           0,0,4,0);
    SH64(SHN64_RODATA,  SHT_PROGBITS, SHF_ALLOC,
         RODATA_OFF, RODATA_OFF, actual_rdat,            0,0,4,0);
    SH64(SHN64_HASH,    SHT_HASH,     SHF_ALLOC,
         HASH_OFF,   HASH_OFF,   elfhash_sz,             4,0,4,4); /* link→dynsym[4] */
    SH64(SHN64_DYNSYM,  SHT_DYNSYM,   SHF_ALLOC,
         DSYM_OFF,   DSYM_OFF,   (u32)(nsyms+1)*24u,    5,1,8,24);/* link→dynstr[5] */
    SH64(SHN64_DYNSTR,  SHT_STRTAB,   SHF_ALLOC,
         DSTR_OFF,   DSTR_OFF,   dynstr_sz,              0,0,1,0);
    SH64(SHN64_DYN,     SHT_DYNAMIC,  SHF_ALLOC|SHF_WRITE,
         DYN_OFF,    DYN_OFF,    A64SO_NDYN*16u,         5,0,8,16);/* link→dynstr[5] */
    SH64(SHN64_SHSTR,   SHT_STRTAB,   0,
         0,           SHSTR_OFF,  SHSTRTAB64_SZ,         0,0,1,0);
    SH64(SHN64_ARMATTR, SHT_ARM_ATTRIBUTES, 0,
         0,           ARMATTR_OFF, ARM_ATTRS_SZ,         0,0,1,0);
#undef SH64
    return (sz)TOTAL;
}

/* ═══════════════════════════ ARM32 .so generator ═══════════════════════════
 * Layout: ELF header (52B) + 2×PHT (32B each) = 0x074 for .text start.
 * Sections: 0=null 1=text 2=hash 3=dynsym 4=dynstr 5=dynamic 6=shstrtab
 */
#define A32SO_TEXT_BASE 0x074u
#define A32SO_NDYN      6u
#define A32SO_NSECT     7u

static sz elf32_build_so(u8 *out, const u8 *text, u32 text_sz,
                          const ElfSym *syms, int nsyms)
{
    if (nsyms > 8) nsyms = 8;

    u8  _ds[512]; u32 _sym_off[8];
    u32 dynstr_sz  = _build_dynstr(syms, nsyms, _ds, _sym_off);
    u8  _eh[64];
    u32 elfhash_sz = _build_elfhash(nsyms, _eh);

    u32 TEXT_OFF    = A32SO_TEXT_BASE;
    u32 actual_text = text_sz ? u32_aln(text_sz, 4u) : 16u;
    u32 HASH_OFF    = u32_aln(TEXT_OFF   + actual_text,          4u);
    u32 DSYM_OFF    = u32_aln(HASH_OFF   + elfhash_sz,           4u);
    u32 DSTR_OFF    = u32_aln(DSYM_OFF   + (u32)(nsyms+1)*16u,  4u);
    u32 DYN_OFF     = u32_aln(DSTR_OFF   + dynstr_sz,            4u);
    u32 SHSTR_OFF   = u32_aln(DYN_OFF    + A32SO_NDYN*8u,        4u);
    u32 SHT_OFF     = u32_aln(SHSTR_OFF  + SHSTRTAB32_SZ,        4u);
    u32 TOTAL       = SHT_OFF + A32SO_NSECT*40u;

    m_set(out, 0, (sz)TOTAL);
    ELFBuf B; B.p = out; B.pos = 0;

    /* ELF header (52 bytes) */
    _eb8(&B,0x7F);_eb8(&B,'E');_eb8(&B,'L');_eb8(&B,'F');
    _eb8(&B,1); _eb8(&B,1); _eb8(&B,1); _eb8(&B,0);
    _epad(&B,8);
    _eb16(&B,(u16)ET_DYN);
    _eb16(&B,(u16)EM_ARM);
    _eb32(&B,1u);
    _eb32(&B,0u);              /* e_entry */
    _eb32(&B,0x34u);           /* e_phoff */
    _eb32(&B,SHT_OFF);
    _eb32(&B,0x05000200u);     /* e_flags: EABI v5 */
    _eb16(&B,52u);
    _eb16(&B,32u);             /* e_phentsize */
    _eb16(&B,2u);
    _eb16(&B,40u);             /* e_shentsize */
    _eb16(&B,(u16)A32SO_NSECT);
    _eb16(&B,6u);              /* e_shstrndx = 6 */

    /* PHT (2 × 32 bytes) */
    B.pos = 0x34u;
    _eb32(&B,(u32)PT_LOAD);  _eb32(&B,0u); _eb32(&B,0u); _eb32(&B,0u);
    _eb32(&B,TOTAL); _eb32(&B,TOTAL); _eb32(&B,(u32)(PF_R|PF_X)); _eb32(&B,0x1000u);
    _eb32(&B,(u32)PT_DYNAMIC); _eb32(&B,DYN_OFF); _eb32(&B,DYN_OFF); _eb32(&B,DYN_OFF);
    _eb32(&B,A32SO_NDYN*8u); _eb32(&B,A32SO_NDYN*8u); _eb32(&B,(u32)(PF_R|PF_W)); _eb32(&B,4u);

    /* .text */
    if (text && text_sz) {
        m_cpy(out+TEXT_OFF, text, (sz)text_sz);
    } else {
        w32(out+TEXT_OFF+0, A32_BXLR); w32(out+TEXT_OFF+4, A32_BXLR);
        w32(out+TEXT_OFF+8, A32_NOP);  w32(out+TEXT_OFF+12, A32_NOP);
    }

    /* .hash */
    m_cpy(out+HASH_OFF, _eh, elfhash_sz);

    /* .dynsym — Elf32_Sym: name(4) value(4) size(4) info(1) other(1) shndx(2) */
    u8 *ds = out + DSYM_OFF;
    m_set(ds, 0, 16u);
    for (int i = 0; i < nsyms; i++) {
        u8 *se = ds + (i+1)*16;
        u32 fva = TEXT_OFF + syms[i].va;
        w32(se+0,  _sym_off[i]);
        w32(se+4,  fva);
        w32(se+8,  4u);
        se[12] = (u8)((STB_GLOBAL<<4)|STT_FUNC);
        se[13] = STV_DEFAULT;
        w16(se+14, 1u); /* st_shndx = 1 (.text) */
    }

    /* .dynstr */
    m_cpy(out+DSTR_OFF, _ds, dynstr_sz);

    /* .dynamic (6 × 8 bytes) */
    u8 *dy = out + DYN_OFF;
    w32(dy,DT_HASH);   w32(dy+4,HASH_OFF);  dy+=8;
    w32(dy,DT_STRTAB); w32(dy+4,DSTR_OFF);  dy+=8;
    w32(dy,DT_SYMTAB); w32(dy+4,DSYM_OFF);  dy+=8;
    w32(dy,DT_STRSZ);  w32(dy+4,dynstr_sz); dy+=8;
    w32(dy,DT_SYMENT); w32(dy+4,16u);        dy+=8;
    w32(dy,DT_NULL);   w32(dy+4,0u);

    /* .shstrtab */
    m_cpy(out+SHSTR_OFF, _shstrtab32, SHSTRTAB32_SZ);

    /* Section header table (7 × 40 bytes)
     * Indices: 0=null 1=text 2=hash 3=dynsym 4=dynstr 5=dynamic 6=shstrtab */
#define SH32(nm,ty,fl,ad,of,sz,lk,in,al,es) do { \
    _eb32(&B,(nm));_eb32(&B,(ty));_eb32(&B,(fl));_eb32(&B,(ad)); \
    _eb32(&B,(of));_eb32(&B,(sz));_eb32(&B,(lk));_eb32(&B,(in)); \
    _eb32(&B,(al));_eb32(&B,(es)); \
} while(0)
    B.pos = SHT_OFF;
    SH32(0, SHT_NULL, 0,0,0,0, 0,0,0,0);
    SH32(SHN32_TEXT,   SHT_PROGBITS, SHF_ALLOC|SHF_EXECINSTR,
         TEXT_OFF,  TEXT_OFF,  actual_text,        0,0,4,0);
    SH32(SHN32_HASH,   SHT_HASH,     SHF_ALLOC,
         HASH_OFF,  HASH_OFF,  elfhash_sz,         3,0,4,4); /* link→dynsym[3] */
    SH32(SHN32_DYNSYM, SHT_DYNSYM,   SHF_ALLOC,
         DSYM_OFF,  DSYM_OFF,  (u32)(nsyms+1)*16u, 4,1,4,16);/* link→dynstr[4] */
    SH32(SHN32_DYNSTR, SHT_STRTAB,   SHF_ALLOC,
         DSTR_OFF,  DSTR_OFF,  dynstr_sz,          0,0,1,0);
    SH32(SHN32_DYN,    SHT_DYNAMIC,  SHF_ALLOC|SHF_WRITE,
         DYN_OFF,   DYN_OFF,   A32SO_NDYN*8u,      4,0,4,8); /* link→dynstr[4] */
    SH32(SHN32_SHSTR,  SHT_STRTAB,   0,
         0,          SHSTR_OFF, SHSTRTAB32_SZ,      0,0,1,0);
#undef SH32
    return (sz)TOTAL;
}
