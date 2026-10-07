#ifndef RAFBBS_MANIFEST_BIN_CORE_H
#define RAFBBS_MANIFEST_BIN_CORE_H

/*
 * RafBBS binary manifest codec V1.
 *
 * Pure authorial wire codec:
 * - caller-owned buffers only;
 * - zero hosted headers, libc, heap, syscall or provider API;
 * - fixed 96-byte little-endian wire image;
 * - raw SHA-256 bytes are copied verbatim;
 * - filesystem persistence remains in rafbbs_manifest_bin.h.
 *
 * The little-endian field encoding preserves the byte layout historically
 * emitted on the project's ARM/x86 little-endian hosts while removing native
 * struct-layout/endianness dependence from the wire contract.
 */

#include "rafbbs_baremetal.h"

#define RAFBBS_BIN_MANIFEST_V1_SIZE 96u

static inline void raf_bin_put_u32_le(RafU8 *out, RafU32 value)
{
    out[0] = (RafU8)(value);
    out[1] = (RafU8)(value >> 8u);
    out[2] = (RafU8)(value >> 16u);
    out[3] = (RafU8)(value >> 24u);
}

static inline RafU32 raf_bin_get_u32_le(const RafU8 *in)
{
    return ((RafU32)in[0]) |
           ((RafU32)in[1] << 8u) |
           ((RafU32)in[2] << 16u) |
           ((RafU32)in[3] << 24u);
}

static inline int
raf_bin_manifest_encode_v1(RafU8 *out, RafU32 cap, const RafBinManifest *m)
{
    RafU32 i;

    if (out == (RafU8 *)0 || m == (const RafBinManifest *)0)
        return -1;
    if (cap < RAFBBS_BIN_MANIFEST_V1_SIZE)
        return -1;

    raf_bin_put_u32_le(out + 0u, m->magic);
    raf_bin_put_u32_le(out + 4u, m->version);
    raf_bin_put_u32_le(out + 8u, m->status);
    raf_bin_put_u32_le(out + 12u, m->arch);
    raf_bin_put_u32_le(out + 16u, m->input_crc32);
    raf_bin_put_u32_le(out + 20u, m->output_crc32);

    for (i = 0u; i < 32u; i++)
        out[24u + i] = m->input_sha256[i];
    for (i = 0u; i < 32u; i++)
        out[56u + i] = m->output_sha256[i];

    raf_bin_put_u32_le(out + 88u, m->hash_state);
    raf_bin_put_u32_le(out + 92u, m->gaps);
    return 0;
}

static inline int
raf_bin_manifest_decode_v1(
    RafBinManifest *m,
    const RafU8 *in,
    RafU32 len
)
{
    RafU32 i;

    if (m == (RafBinManifest *)0 || in == (const RafU8 *)0)
        return -1;
    if (len != RAFBBS_BIN_MANIFEST_V1_SIZE)
        return -1;

    m->magic = raf_bin_get_u32_le(in + 0u);
    m->version = raf_bin_get_u32_le(in + 4u);
    m->status = raf_bin_get_u32_le(in + 8u);
    m->arch = raf_bin_get_u32_le(in + 12u);
    m->input_crc32 = raf_bin_get_u32_le(in + 16u);
    m->output_crc32 = raf_bin_get_u32_le(in + 20u);

    for (i = 0u; i < 32u; i++)
        m->input_sha256[i] = in[24u + i];
    for (i = 0u; i < 32u; i++)
        m->output_sha256[i] = in[56u + i];

    m->hash_state = raf_bin_get_u32_le(in + 88u);
    m->gaps = raf_bin_get_u32_le(in + 92u);

    if (m->magic != RAFBBS_BIN_MANIFEST_MAGIC || m->version != 1u)
        return -1;
    return 0;
}

#endif
