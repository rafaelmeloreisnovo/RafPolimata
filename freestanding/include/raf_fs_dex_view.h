#ifndef RAF_FS_DEX_VIEW_H
#define RAF_FS_DEX_VIEW_H
/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Bounded DEX v035/v037-v040 structural header and map inspection.
 * SCOPE: Read-only byte-layout checks, not dexopt, checksum, SHA-1 or bytecode VM.
 * PRECONDITIONS: Caller owns stable readable bytes, output does not alias input.
 * REGISTER_OWNERSHIP: Compiler scalar registers only.
 * CLOBBERS: Output changes only after successful structural checks.
 * MEMORY_ORDER: Caller owns synchronization.
 * TAIL_SHADOW: Explicit six fixed ID tables and bounded map scan.
 * EVIDENCE: Synthetic and cross-ISA object checks; no ART/Android runtime proof.
 */
#include "raf_fs_binary_view.h"
#define RAF_FS_DEX_OK 0x81u
#define RAF_FS_DEX_BAD 0x82u
#define RAF_FS_DEX_UNSUPPORTED 0x83u

typedef struct raf_fs_dex_view {
    raf_u32 file_size;
    raf_u32 map_offset;
    raf_u32 map_items;
    raf_u32 data_offset;
    raf_u16 version;
} raf_fs_dex_view;

static inline __attribute__((always_inline)) raf_u32
raf_fs_dex_inspect(const raf_u8 *bytes, raf_usize length, raf_fs_dex_view *out)
{
    raf_u32 file, map, data, data_size, map_count, i;
    raf_u32 vers;
    static const raf_u8 stride[6]={4u,4u,12u,8u,8u,32u};
    raf_fs_dex_view tmp;
    if (!bytes || !out || !raf_fs_bv_range(length,0u,112u)) return RAF_FS_DEX_BAD;
    if (bytes[0]!='d' || bytes[1]!='e' || bytes[2]!='x' || bytes[3]!='\n' ||
        bytes[7]!=0 || bytes[4]<'0' || bytes[4]>'9' ||
        bytes[5]<'0' || bytes[5]>'9' || bytes[6]<'0' || bytes[6]>'9') return RAF_FS_DEX_BAD;
    vers=(raf_u32)(bytes[4]-'0')*100u+(raf_u32)(bytes[5]-'0')*10u+(raf_u32)(bytes[6]-'0');
    if (vers!=35u && vers!=37u && vers!=38u && vers!=39u && vers!=40u)
        return RAF_FS_DEX_UNSUPPORTED;
    file=raf_fs_bv_u32(bytes+32,0);
    map=raf_fs_bv_u32(bytes+52,0);
    data_size=raf_fs_bv_u32(bytes+104,0);
    data=raf_fs_bv_u32(bytes+108,0);
    if ((raf_u64)file!=(raf_u64)length ||
        raf_fs_bv_u32(bytes+36,0)!=112u ||
        raf_fs_bv_u32(bytes+40,0)!=0x12345678u ||
        data<112u || (data&3u)!=0u || data_size<4u || (data_size&3u)!=0u ||
        !raf_fs_bv_range(length,data,data_size) || (raf_u64)data+data_size!=file ||
        map<data || (map&3u)!=0u || !raf_fs_bv_range(length,map,4u) ||
        raf_fs_bv_u32(bytes+44,0)!=0u || raf_fs_bv_u32(bytes+48,0)!=0u)
        return RAF_FS_DEX_BAD;
    /* Six fixed-length tables must reside before data, with coherent empty offsets. */
    for (i=0u;i<6u;++i) {
        raf_u32 size=raf_fs_bv_u32(bytes+56u+i*8u,0);
        raf_u32 off=raf_fs_bv_u32(bytes+60u+i*8u,0);
        if ((size==0u && off!=0u) || (size!=0u &&
            (off<112u || (off&3u)!=0u ||
             (raf_u64)size*stride[i]>(raf_u64)data-off || off>data)))
            return RAF_FS_DEX_BAD;
    }
    map_count=raf_fs_bv_u32(bytes+map,0);
    if (map_count==0u || (raf_u64)map_count*12u > (raf_u64)file-map-4u)
        return RAF_FS_DEX_BAD;
    for (i=0u;i<map_count;++i) {
        const raf_u8 *item=bytes+map+4u+(raf_usize)i*12u;
        raf_u32 count=raf_fs_bv_u32(item+4,0);
        raf_u32 off=raf_fs_bv_u32(item+8,0);
        if (!count || !raf_fs_bv_range(length,off,1u)) return RAF_FS_DEX_BAD;
    }
    tmp.file_size=file;
    tmp.map_offset=map;
    tmp.map_items=map_count;
    tmp.data_offset=data;
    tmp.version=(raf_u16)vers;
    *out=tmp;
    return RAF_FS_DEX_OK;
}
#endif
