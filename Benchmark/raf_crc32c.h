/* raf_crc32c.h — CRC32C hardware profile on ARM64/x86-64, software on ARM32.
 * Performance is current-receipt scoped; source comments carry no throughput claim.
 * Poly CRC32C: 0x1EDC6F41 normal / 0x82F63B78 reversed.                    */
#pragma once
#include "raf_types.h"

/* ── ARM64: crc32cx — requires the ARMv8-A CRC extension profile ───────── */
#ifdef RAF_ARCH_A64
static __attribute__((always_inline)) inline
u32 crc32c_u64(u32 crc, u64 w) {
    __asm__("crc32cx %w0, %w0, %x1" : "+r"(crc) : "r"(w));
    return crc;
}
static __attribute__((always_inline)) inline
u32 crc32c_u8(u32 crc, u8 b) {
    __asm__("crc32cb %w0, %w0, %w1" : "+r"(crc) : "r"((u32)b));
    return crc;
}
#endif /* RAF_ARCH_A64 */

/* ── x86-64: crc32q — SSE4.2, 3 ciclos latência, 1/ciclo throughput ─────── */
#ifdef RAF_ARCH_X64
static __attribute__((always_inline)) inline
u32 crc32c_u64(u32 crc, u64 w) {
    __asm__("crc32q %1, %q0" : "+r"(crc) : "rm"(w));
    return crc;
}
static __attribute__((always_inline)) inline
u32 crc32c_u8(u32 crc, u8 b) {
    __asm__("crc32b %1, %0" : "+r"(crc) : "rm"(b));
    return crc;
}
#endif /* RAF_ARCH_X64 */

/* ── ARM32: software — poly reversed 0x82F63B78 ─────────────────────────── */
#ifdef RAF_ARCH_A32
static __attribute__((always_inline)) inline
u32 crc32c_u8(u32 crc, u8 b) {
    crc ^= b;
    /* 8 iterações unrolled — branch-free via conditional XOR                */
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    crc = (crc >> 1) ^ (0x82F63B78U & -(crc & 1));
    return crc;
}
static __attribute__((always_inline)) inline
u32 crc32c_u64(u32 crc, u64 w) {
    crc = crc32c_u8(crc,(u8)(w));        crc = crc32c_u8(crc,(u8)(w>>8));
    crc = crc32c_u8(crc,(u8)(w>>16));   crc = crc32c_u8(crc,(u8)(w>>24));
    crc = crc32c_u8(crc,(u8)(w>>32));   crc = crc32c_u8(crc,(u8)(w>>40));
    crc = crc32c_u8(crc,(u8)(w>>48));   crc = crc32c_u8(crc,(u8)(w>>56));
    return crc;
}
#endif /* RAF_ARCH_A32 */

/* Constant-size copy must inline; ARM32 stays byte-wise to avoid alignment-sensitive u64 loads. */
static __attribute__((always_inline)) inline u64 crc_load_u64(const u8 *p) {
    u64 w; __builtin_memcpy(&w, p, sizeof(w)); return w;
}
static u32 crc32c_buf(const u8 *buf, usize len, u32 seed) {
    u32 crc = ~seed;
#ifdef RAF_ARCH_A32
    while (len--) crc = crc32c_u8(crc, *buf++);
#else
    usize n64 = len >> 3;
    while (n64 >= 8) {
        crc = crc32c_u64(crc, crc_load_u64(buf+0));  crc = crc32c_u64(crc, crc_load_u64(buf+8));
        crc = crc32c_u64(crc, crc_load_u64(buf+16)); crc = crc32c_u64(crc, crc_load_u64(buf+24));
        crc = crc32c_u64(crc, crc_load_u64(buf+32)); crc = crc32c_u64(crc, crc_load_u64(buf+40));
        crc = crc32c_u64(crc, crc_load_u64(buf+48)); crc = crc32c_u64(crc, crc_load_u64(buf+56));
        buf += 64; n64 -= 8;
    }
    while (n64--) { crc = crc32c_u64(crc, crc_load_u64(buf)); buf += 8; }
    usize rem = len & 7;
    while (rem--) crc = crc32c_u8(crc, *buf++);
#endif
    return ~crc;
}
/* XOR AETHER: dual CRC32C com seed complementar — GAIA-BBS dual hash        */
static void crc32c_aether(const u8 *buf, usize len,
                           u32 *out_a, u32 *out_b) {
    *out_a = crc32c_buf(buf, len, 0x00000000U);
    *out_b = crc32c_buf(buf, len, 0xFFFFFFFFU);
}
