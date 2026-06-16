#ifndef RAFBBS_BAREMETAL_H
#define RAFBBS_BAREMETAL_H
#include <stdint.h>
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
    uint8_t buf[RAFBBS_BAREMETAL_OUT_CAP];
    uint32_t pos;
    uint32_t dropped;
} RafBaremetalOut;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t status;
    uint32_t arch;
    uint32_t input_crc32;
    uint32_t output_crc32;
    uint8_t input_sha256[32];
    uint8_t output_sha256[32];
    uint32_t hash_state;
    uint32_t gaps;
} RafBinManifest;

typedef struct {
    uint32_t cflags;
    uint32_t no_heap;
    uint32_t no_syscall;
    uint32_t simd;
    uint32_t cache_line;
    uint32_t watchdog_budget;
} RafArchFlags;

typedef void (*RafBaremetalSink)(uint8_t byte, void *user);

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

static inline void raf_baremetal_putc(RafBaremetalOut *o, uint8_t c) {
    uint32_t room = (uint32_t)(o->pos < RAFBBS_BAREMETAL_OUT_CAP);
    if (room) o->buf[o->pos++] = c;
    o->dropped += (uint32_t)(room ^ 1u);
}

static inline void raf_baremetal_write(RafBaremetalOut *o, const char *s) {
    while (*s) {
        raf_baremetal_putc(o, (uint8_t)*s);
        s++;
    }
}

static inline void raf_baremetal_flush(RafBaremetalOut *o, RafBaremetalPort port) {
    uint32_t i;
    if (!port.sink) return;
    for (i = 0u; i < o->pos; i++) port.sink(o->buf[i], port.user);
}

static inline RafBinManifest raf_bin_manifest_make(uint32_t status, uint32_t arch, uint32_t in_crc, uint32_t out_crc, uint32_t hash_state, uint32_t gaps) {
    RafBinManifest m;
    uint32_t i;
    m.magic = RAFBBS_BIN_MANIFEST_MAGIC;
    m.version = 1u;
    m.status = status;
    m.arch = arch;
    m.input_crc32 = in_crc;
    m.output_crc32 = out_crc;
    for (i = 0u; i < 32u; i++) { m.input_sha256[i] = 0u; m.output_sha256[i] = 0u; }
    m.hash_state = hash_state;
    m.gaps = gaps;
    return m;
}

static inline uint32_t raf_hash_failover_state(uint32_t sha_ok, uint32_t crc_ok) {
    uint32_t state = 0u;
    state |= (sha_ok ? RAFBBS_HASH_SHA256_OK : 0u);
    state |= (crc_ok ? RAFBBS_HASH_CRC32_OK : 0u);
    state |= ((sha_ok | crc_ok) ? 0u : RAFBBS_HASH_TOKEN_VAZIO);
    return state;
}

static inline RafArchFlags raf_arch_flags(RafArchProfile arch) {
    uint32_t idx = (uint32_t)arch;
    uint32_t max = (uint32_t)(sizeof(raf_arch_flag_table) / sizeof(raf_arch_flag_table[0]));
    idx = (idx < max) ? idx : 0u;
    return raf_arch_flag_table[idx];
}
#endif
