/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: candidate binary prefix recognition with caller-owned matrix.
 * SCOPE: PURE_CORE only; does NOT parse or authenticate files.
 * PRECONDITIONS: unsigned char=8 bits, unsigned int=32 bits, caller owns 8 cells.
 * REGISTER_OWNERSHIP: compiler-selected; matrix and input borrowed, no allocation.
 * CLOBBERS: NONE declared at C level. MEMORY_ORDER: NONE.
 * TAIL_SHADOW: no dynamic iteration, hidden tail, or shadow copy.
 * EVIDENCE: header candidate only. CI object check != physical execution.
 */
#ifndef RAF_FS_PROBE_H
#define RAF_FS_PROBE_H
typedef unsigned char raf_u8;
typedef unsigned int raf_u32;
_Static_assert(sizeof(raf_u8) == 1, "8-bit byte expected");
_Static_assert(sizeof(raf_u32) == 4, "32-bit unsigned expected");
enum {
 RAF_ST_INVALID_POINTER=1, RAF_ST_NEED_BYTES=2,
 RAF_ST_UNRECOGNIZED=3, RAF_ST_HEADER_CANDIDATE=4
};
enum {
 RAF_FMT_NONE=0, RAF_FMT_ELF=1, RAF_FMT_DEX=2, RAF_FMT_CDEX=3,
 RAF_FMT_OAT=4, RAF_FMT_VDEX=5, RAF_FMT_ART=6, RAF_FMT_ZIP=7,
 RAF_FMT_AR=8, RAF_FMT_WASM=9, RAF_FMT_PNG=10, RAF_FMT_GZIP=11,
 RAF_FMT_MZ=12, RAF_FMT_MACHO=13
};
enum {
 RAF_CELL_STATE=0, RAF_CELL_FORMAT=1, RAF_CELL_INPUT_LENGTH=2,
 RAF_CELL_MIN_BYTES=3, RAF_CELL_AMBIGUOUS=4,
 RAF_CELL_VALIDATED_FORMAT=5, RAF_CELL_OWNERSHIP=6,
 RAF_CELL_VERSION=7, RAF_CELL_COUNT=8
};
/* C ABI: input is caller-owned byte frame. State matrix is caller-owned.
 * No JNI, OS ABI, callback, memory allocation or platform function.
 */
void raf_fs_probe(const raf_u8 *input, raf_u32 length, raf_u32 matrix[RAF_CELL_COUNT]);
#endif
