/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: bounded, fixed-prefix matrix. SCOPE: OS-neutral PURE_CORE.
 * PRECONDITIONS: raf_fs_probe.h; caller supplies at least eight output cells.
 * REGISTER_OWNERSHIP: compiler; CLOBBERS: NONE; MEMORY_ORDER: NONE.
 * TAIL_SHADOW: no residual loop, no variable-length byte access.
 * EVIDENCE: candidate detection; never binary authenticity/execution.
 */
#include "raf_fs_probe.h"
void raf_fs_probe(const raf_u8 *input, raf_u32 length, raf_u32 matrix[RAF_CELL_COUNT]) {
    if (!matrix) return;  /* Without output no safe reporting path. */
    matrix[RAF_CELL_STATE] = RAF_ST_INVALID_POINTER;
    matrix[RAF_CELL_FORMAT] = RAF_FMT_NONE;
    matrix[RAF_CELL_INPUT_LENGTH] = length;
    matrix[RAF_CELL_MIN_BYTES] = 4u;
    matrix[RAF_CELL_AMBIGUOUS] = 1u;
    matrix[RAF_CELL_VALIDATED_FORMAT] = 0u;
    matrix[RAF_CELL_OWNERSHIP] = 0u;
    matrix[RAF_CELL_VERSION] = 1u;
    if (!input) return; /* Necessary bounds/ownership guard, not tail. */
    if (length < 4u) {
        matrix[RAF_CELL_STATE] = RAF_ST_NEED_BYTES;
        return;
    }
    /* Prefix comparisons use byte values. No endian-dependent aliasing. */
    const raf_u32 p = (raf_u32)input[0] | ((raf_u32)input[1] << 8u)
                    | ((raf_u32)input[2] << 16u) | ((raf_u32)input[3] << 24u);
    const raf_u32 elf   = (p == 0x464c457fu);
    const raf_u32 dex   = (p == 0x0a786564u);
    const raf_u32 cdex  = (p == 0x78656463u);
    const raf_u32 oat   = (p == 0x0a74616fu);
    const raf_u32 vdex  = (p == 0x78656476u);
    const raf_u32 art   = (p == 0x0a747261u);
    const raf_u32 zip   = (p == 0x04034b50u || p == 0x06054b50u || p == 0x08074b50u);
    const raf_u32 ar    = (p == 0x72613c21u);
    const raf_u32 wasm  = (p == 0x6d736100u);
    const raf_u32 png   = (p == 0x474e5089u);
    const raf_u32 gzip  = ((p & 0x00ffffffu) == 0x00088b1fu);
    const raf_u32 mz    = ((p & 0x0000ffffu) == 0x00005a4du);
    const raf_u32 macho = (p == 0xfeedfaceu || p == 0xcefaedfeu ||
                           p == 0xfeedfacfu || p == 0xcffaedfeu);
    const raf_u32 f = elf*RAF_FMT_ELF + dex*RAF_FMT_DEX + cdex*RAF_FMT_CDEX
                    + oat*RAF_FMT_OAT + vdex*RAF_FMT_VDEX + art*RAF_FMT_ART
                    + zip*RAF_FMT_ZIP + ar*RAF_FMT_AR + wasm*RAF_FMT_WASM
                    + png*RAF_FMT_PNG + gzip*RAF_FMT_GZIP + mz*RAF_FMT_MZ
                    + macho*RAF_FMT_MACHO;
    /* A prefix is insufficient to validate every format. In particular ART,
     * OAT, VDEX and MZ are ambiguous and version/target dependent.
     */
    matrix[RAF_CELL_STATE] = f ? RAF_ST_HEADER_CANDIDATE : RAF_ST_UNRECOGNIZED;
    matrix[RAF_CELL_FORMAT] = f;
    matrix[RAF_CELL_MIN_BYTES] = (dex | cdex | ar | wasm | png) ? 8u : 4u;
    matrix[RAF_CELL_AMBIGUOUS] = 1u;
    /* Candidate labels with required 8-byte prefix need more bytes to
       avoid 'recognized' reports from severely truncated headers. */
    if (f && (length < matrix[RAF_CELL_MIN_BYTES])) {
        matrix[RAF_CELL_STATE] = RAF_ST_NEED_BYTES;
        matrix[RAF_CELL_FORMAT] = RAF_FMT_NONE;
    }
}
