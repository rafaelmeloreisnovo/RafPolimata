#ifndef RAF_FS_ELF_VIEW_H
#define RAF_FS_ELF_VIEW_H
/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Strict bounded ELF32/ELF64 header, table and PT_LOAD byte inspection.
 * SCOPE: Structural view only; never load, link, relocate, execute or mmap.
 * PRECONDITIONS: Caller-owned stable bytes, valid length, distinct output.
 * REGISTER_OWNERSHIP: Compiler scalar registers only.
 * CLOBBERS: Output is modified only on success.
 * MEMORY_ORDER: Caller owns input synchronization.
 * TAIL_SHADOW: Bounded explicit program-header scan; no hidden scalar residual.
 * EVIDENCE: Test fixtures and target objects; not OS-loader/device proof.
 */
#include "raf_fs_binary_view.h"
#define RAF_FS_ELF_OK 0x71u
#define RAF_FS_ELF_BAD 0x72u
#define RAF_FS_ELF_UNSUPPORTED 0x73u

typedef struct raf_fs_elf_view {
    raf_u64 entry;
    raf_u16 machine;
    raf_u16 ph_count;
    raf_u16 load_count;
    raf_u8 word_class;
    raf_u8 big_endian;
} raf_fs_elf_view;

static inline __attribute__((always_inline)) raf_u32
raf_fs_elf_inspect(const raf_u8 *bytes, raf_usize length, raf_fs_elf_view *out)
{
    raf_u32 is64, be, hsize, phsize, shsize, phnum, shnum, shidx, i, load=0;
    raf_u64 phoff, shoff, entry;
    raf_fs_elf_view tmp;
    if (!bytes || !out || !raf_fs_bv_range(length,0u,16u)) return RAF_FS_ELF_BAD;
    if (bytes[0]!=127u || bytes[1]!='E' || bytes[2]!='L' || bytes[3]!='F') return RAF_FS_ELF_BAD;
    if ((bytes[4]!=1u && bytes[4]!=2u) || (bytes[5]!=1u && bytes[5]!=2u)) return RAF_FS_ELF_UNSUPPORTED;
    if (bytes[6]!=1u) return RAF_FS_ELF_BAD;
    is64=(raf_u32)(bytes[4]==2u); be=(raf_u32)(bytes[5]==2u);
    hsize=is64?64u:52u; phsize=is64?56u:32u; shsize=is64?64u:40u;
    if (!raf_fs_bv_range(length,0u,hsize)) return RAF_FS_ELF_BAD;
    if (raf_fs_bv_u32(bytes+20,be)!=1u ||
        raf_fs_bv_u16(bytes+(is64?52u:40u),be)!=hsize) return RAF_FS_ELF_BAD;
    phoff=is64?raf_fs_bv_u64(bytes+32,be):(raf_u64)raf_fs_bv_u32(bytes+28,be);
    shoff=is64?raf_fs_bv_u64(bytes+40,be):(raf_u64)raf_fs_bv_u32(bytes+32,be);
    entry=is64?raf_fs_bv_u64(bytes+24,be):(raf_u64)raf_fs_bv_u32(bytes+24,be);
    phnum=raf_fs_bv_u16(bytes+(is64?56u:44u),be);
    shnum=raf_fs_bv_u16(bytes+(is64?60u:48u),be);
    shidx=raf_fs_bv_u16(bytes+(is64?62u:50u),be);
    if (phnum==65535u || (phnum &&
          (phoff<hsize || raf_fs_bv_u16(bytes+(is64?54u:42u),be)!=phsize ||
          !raf_fs_bv_range(length,phoff,(raf_u64)phnum*phsize))) ||
        (!phnum && phoff!=0u)) return RAF_FS_ELF_BAD;
    /* Extended section numbering and SHN_XINDEX are deferred explicitly. */
    if ((shnum && (shoff<hsize ||
          raf_fs_bv_u16(bytes+(is64?58u:46u),be)!=shsize ||
          !raf_fs_bv_range(length,shoff,(raf_u64)shnum*shsize) ||
          (shidx!=0u && shidx>=shnum))) ||
        (!shnum && (shoff!=0u || shidx!=0u)) || shidx==65535u)
        return RAF_FS_ELF_UNSUPPORTED;
    for (i=0u;i<phnum;++i) {
        const raf_u8 *p=bytes+(raf_usize)(phoff+(raf_u64)i*phsize);
        raf_u32 type=raf_fs_bv_u32(p,be);
        if (type==1u) {
            raf_u64 off=is64?raf_fs_bv_u64(p+8,be):(raf_u64)raf_fs_bv_u32(p+4,be);
            raf_u64 addr=is64?raf_fs_bv_u64(p+16,be):(raf_u64)raf_fs_bv_u32(p+8,be);
            raf_u64 file=is64?raf_fs_bv_u64(p+32,be):(raf_u64)raf_fs_bv_u32(p+16,be);
            raf_u64 mem=is64?raf_fs_bv_u64(p+40,be):(raf_u64)raf_fs_bv_u32(p+20,be);
            raf_u64 align=is64?raf_fs_bv_u64(p+48,be):(raf_u64)raf_fs_bv_u32(p+28,be);
            if (mem<file || addr>(~(raf_u64)0)-mem ||
                !raf_fs_bv_range(length,off,file) ||
                (align>1u && (((align & (align-1u))!=0u) ||
                              ((off & (align-1u))!=(addr & (align-1u))))))
                return RAF_FS_ELF_BAD;
            ++load;
        }
    }
    tmp.entry=entry;
    tmp.machine=raf_fs_bv_u16(bytes+18,be);
    tmp.ph_count=(raf_u16)phnum;
    tmp.load_count=(raf_u16)load;
    tmp.word_class=(raf_u8)(is64?64u:32u);
    tmp.big_endian=(raf_u8)be;
    *out=tmp;
    return RAF_FS_ELF_OK;
}
#endif
