#ifndef RAFBBS_BAREMETAL_H
#define RAFBBS_BAREMETAL_H

#include "rafbbs_freestanding.h"

#define RAFBBS_BAREMETAL_OUT_CAP 1024u
#define RAFBBS_BIN_MANIFEST_MAGIC 0x5242464du /* RBFM */
#define RAFBBS_HASH_SHA256_OK 0x01u
#define RAFBBS_HASH_CRC32_OK 0x02u
#define RAFBBS_HASH_TOKEN_VAZIO 0x80u

typedef enum {
    RAF_ARCH_GENERIC = 0,
    RAF_ARCH_ARM32 = 1,
    RAF_ARCH_ARM32_NEON = 2,
    RAF_ARCH_ARM64 = 3,
    RAF_ARCH_X86_64 = 4
} RafArchProfile;

typedef struct {
    RafU8 buf[RAFBBS_BAREMETAL_OUT_CAP];
    RafU32 pos;
    RafU32 dropped;
} RafBaremetalOut;

typedef struct {
    RafU32 magic;
    RafU32 version;
    RafU32 status;
    RafU32 arch;
    RafU32 input_crc32;
    RafU32 output_crc32;
    RafU8 input_sha256[32];
    RafU8 output_sha256[32];
    RafU32 hash_state;
    RafU32 gaps;
} RafBinManifest;

typedef struct {
    RafU32 cflags;
    RafU32 no_heap;
    RafU32 no_syscall;
    RafU32 simd;
    RafU32 cache_line;
    RafU32 watchdog_budget;
} RafArchFlags;

typedef void (*RafBaremetalSink)(RafU8 byte, void *user);

typedef struct {
    RafBaremetalSink sink;
    void *user;
} RafBaremetalPort;

static const RafArchFlags raf_arch_flag_table[] = {
    {0x00000001u, 1u, 1u, 0u, 64u, RAFBBS_WATCHDOG_DEFAULT_TICKS},
    {0x00000032u, 1u, 1u, 0u, 32u, RAFBBS_WATCHDOG_DEFAULT_TICKS},
    {0x0000AAE0u, 1u, 1u, 1u, 32u, RAFBBS_WATCHDOG_DEFAULT_TICKS},
    {0x00000064u, 1u, 1u, 1u, 64u, RAFBBS_WATCHDOG_DEFAULT_TICKS},
    {0x00008664u, 1u, 1u, 0u, 64u, RAFBBS_WATCHDOG_DEFAULT_TICKS}
};

static inline void raf_baremetal_out_init(RafBaremetalOut *o) {
    o->pos = 0u;
    o->dropped = 0u;
}

static inline void raf_baremetal_putc(RafBaremetalOut *o, RafU8 c) {
    RafU32 room = (RafU32)(o->pos < RAFBBS_BAREMETAL_OUT_CAP);
    if (room) o->buf[o->pos++] = c;
    o->dropped += (RafU32)(room ^ 1u);
}

static inline void raf_baremetal_write(RafBaremetalOut *o, const char *s) {
    while (*s) {
        raf_baremetal_putc(o, (RafU8)*s);
        s++;
    }
}

static inline void raf_baremetal_flush(RafBaremetalOut *o, RafBaremetalPort port) {
    RafU32 i;
    if (!port.sink) return;
    for (i = 0u; i < o->pos; i++) port.sink(o->buf[i], port.user);
}

static inline RafBinManifest raf_bin_manifest_make(
    RafU32 status, RafU32 arch, RafU32 in_crc, RafU32 out_crc,
    RafU32 hash_state, RafU32 gaps
) {
    RafBinManifest m;
    RafU32 i;
    m.magic = RAFBBS_BIN_MANIFEST_MAGIC;
    m.version = 1u;
    m.status = status;
    m.arch = arch;
    m.input_crc32 = in_crc;
    m.output_crc32 = out_crc;
    for (i = 0u; i < 32u; i++) {
        m.input_sha256[i] = 0u;
        m.output_sha256[i] = 0u;
    }
    m.hash_state = hash_state;
    m.gaps = gaps;
    return m;
}

static inline RafU32 raf_hash_failover_state(RafU32 sha_ok, RafU32 crc_ok) {
    RafU32 state = 0u;
    state |= (sha_ok ? RAFBBS_HASH_SHA256_OK : 0u);
    state |= (crc_ok ? RAFBBS_HASH_CRC32_OK : 0u);
    state |= ((sha_ok | crc_ok) ? 0u : RAFBBS_HASH_TOKEN_VAZIO);
    return state;
}

static inline RafArchFlags raf_arch_flags(RafArchProfile arch) {
    RafU32 idx = (RafU32)arch;
    RafU32 max = (RafU32)(sizeof(raf_arch_flag_table) / sizeof(raf_arch_flag_table[0]));
    idx = (idx < max) ? idx : 0u;
    return raf_arch_flag_table[idx];
}
#endif
