#ifndef ZIPRAF_HW_H
#define ZIPRAF_HW_H

/*
 * ZIPRAF Hardware Capability Core V1
 * Copyright (c) 2024-2026 Rafael Melo Reis.
 * Provenance notice: Berne protection is automatic where applicable; this
 * header does not create, expand, or adjudicate rights. Upstream licenses and
 * per-file provenance control. See docs/RIGHTS_PROVENANCE_BERNE_BR.md.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  zh_u8;
typedef unsigned short zh_u16;
typedef unsigned int   zh_u32;
typedef unsigned long long zh_u64;
typedef signed int     zh_i32;

enum zh_arch {
    ZH_ARCH_UNKNOWN = 0,
    ZH_ARCH_PORTABLE,
    ZH_ARCH_X86_32,
    ZH_ARCH_X86_64,
    ZH_ARCH_ARMV7,
    ZH_ARCH_ARMV8_32,
    ZH_ARCH_AARCH64,
    ZH_ARCH_WASM32,
    ZH_ARCH_RISCV_PORTABLE,
    ZH_ARCH_PPC_PORTABLE
};

enum zh_state {
    ZH_TOKEN_VAZIO = 0,
    ZH_IMPLEMENTED_UNTESTED = 1,
    ZH_PASS = 2,
    ZH_FAIL = 3
};

enum zh_feature {
    ZH_FEAT_SCALAR = 1u << 0,
    ZH_FEAT_SSE2   = 1u << 1,
    ZH_FEAT_SSE41  = 1u << 2,
    ZH_FEAT_AVX2   = 1u << 3,
    ZH_FEAT_AVX512 = 1u << 4,
    ZH_FEAT_NEON   = 1u << 5,
    ZH_FEAT_WASM_SIMD = 1u << 6
};

struct zh_capability {
    zh_u32 arch;
    zh_u32 features;
    zh_u32 word_bits;
    zh_u32 cache_line_bytes;
    zh_u32 write_granule_bytes;
    zh_u32 max_vector_lanes;
};

struct zh_measurement {
    zh_u32 backend_id;
    zh_u32 required_features;
    zh_u64 p50_cycles;
    zh_u64 p95_cycles;
    zh_u64 useful_bytes;
    zh_u32 state;
};

zh_u64 zh_patch_u64(zh_u64 current, zh_u64 value, zh_u64 mask);
zh_u32 zh_vector_lanes(zh_u32 register_bits, zh_u32 element_bits);
zh_u32 zh_block_fit(zh_u32 container_bytes, zh_u32 block_bytes);
zh_u32 zh_write_amplification_q16(zh_u32 semantic_bytes, zh_u32 physical_granule_bytes);
zh_u32 zh_cycles_per_byte_q16(zh_u64 cycles, zh_u64 useful_bytes);
zh_i32 zh_measurement_compatible(const struct zh_capability *cap, const struct zh_measurement *m);
zh_i32 zh_select_measured_backend(const struct zh_capability *cap,
                                  const struct zh_measurement *m,
                                  zh_u32 count,
                                  zh_u32 *backend_id_out);

#ifdef __cplusplus
}
#endif

#endif
