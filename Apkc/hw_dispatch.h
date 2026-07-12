/* hw_dispatch.h — freestanding hardware capability detection + backend dispatch.
 * Probes CPU ISA extensions via /proc/cpuinfo, GPU via /dev nodes, DSP/NPU
 * via /dev/fastrpc-* and /dev/npu*. No malloc. No libc. sys.h only.
 *
 * Usage:
 *   HWProfile hw; hw_probe(&hw);
 *   pr("hw: "); hw_caps_pr(&hw);
 *   int backend = hw_select_backend(&hw, HW_WORK_COMPUTE);
 *
 * Backend selection is deterministic: same caps → same phi_attractor → same
 * result every build (not randomized). Scalar CPU is always the final fallback.
 */
#pragma once
#include "sys.h"
#include "mem.h"
#include "coherence.h"

/* ── Hardware capability bits ────────────────────────────────────────────── */
/* CPU ISA extensions — ARMv8-A family */
#define HW_CAP_NEON     (1u<<0)    /* ASIMD/NEON — mandatory on arm64 */
#define HW_CAP_SVE      (1u<<1)    /* ARM Scalable Vector Extension */
#define HW_CAP_SVE2     (1u<<2)    /* ARM SVE2 */
#define HW_CAP_CRC32    (1u<<3)    /* CRC32 hardware instruction */
#define HW_CAP_AES      (1u<<4)    /* AES hardware acceleration */
#define HW_CAP_SHA2     (1u<<5)    /* SHA-256 / SHA-512 hardware */
#define HW_CAP_ATOMICS  (1u<<6)    /* LSE large-system extension atomics */
#define HW_CAP_BF16     (1u<<7)    /* BFloat16 (Cortex-X series) */
#define HW_CAP_I8MM     (1u<<8)    /* Int8 matrix multiply SMMLA/UMMLA */
#define HW_CAP_DOTPROD  (1u<<9)    /* SDOT/UDOT dot-product */
#define HW_CAP_FP16     (1u<<10)   /* Half-precision float compute */
#define HW_CAP_PMULL    (1u<<11)   /* Polynomial multiply (crypto) */
#define HW_CAP_SHA3     (1u<<12)   /* SHA-3 hardware */
#define HW_CAP_SM4      (1u<<13)   /* SM4 cipher hardware */
#define HW_CAP_JSCVT    (1u<<14)   /* JS int↔float conversion */
#define HW_CAP_FCMA     (1u<<15)   /* Complex number multiply-accumulate */
/* GPU / accelerator presence — bit 16+ */
#define HW_CAP_GPU_VK   (1u<<16)   /* Vulkan compute GPU detected */
#define HW_CAP_GPU_CL   (1u<<17)   /* OpenCL GPU detected */
#define HW_CAP_GPU_GL   (1u<<18)   /* OpenGL ES 3.2+ detected */
#define HW_CAP_GPU_WGSL (1u<<19)   /* WebGPU / WGSL capable runtime */
#define HW_CAP_DSP_HXN  (1u<<24)   /* Qualcomm Hexagon DSP */
#define HW_CAP_NPU      (1u<<25)   /* Neural Processing Unit */
/* GPU vendor tags */
#define HW_CAP_ADRENO   (1u<<26)   /* Qualcomm Adreno (kgsl) */
#define HW_CAP_MALI     (1u<<27)   /* ARM Mali */
#define HW_CAP_POWERVR  (1u<<28)   /* Imagination PowerVR */
#define HW_CAP_DRM_GEN  (1u<<29)   /* Generic DRM GPU (Mesa/etc.) */

/* ── Backend IDs (returned by hw_select_backend) ─────────────────────────── */
#define HW_BACKEND_CPU_SCALAR   0  /* generic C scalar — always available */
#define HW_BACKEND_CPU_NEON     1  /* ARM64 NEON/ASIMD */
#define HW_BACKEND_CPU_SVE      2  /* ARM SVE / SVE2 */
#define HW_BACKEND_CPU_I8MM     3  /* Int8 matrix multiply (SMMLA) */
#define HW_BACKEND_GPU_CL       4  /* OpenCL compute kernel */
#define HW_BACKEND_GPU_VULKAN   5  /* Vulkan compute (SPIR-V) */
#define HW_BACKEND_GPU_WGSL     6  /* WebGPU (WGSL) */
#define HW_BACKEND_DSP_HXN      7  /* Qualcomm Hexagon DSP (FastRPC) */
#define HW_BACKEND_NPU          8  /* Neural accelerator offload */
#define HW_BACKEND_COUNT        9

/* ── Workload type for backend selection ─────────────────────────────────── */
#define HW_WORK_COMPUTE  0   /* general GPGPU / parallel batch compute */
#define HW_WORK_TENSOR   1   /* matrix / neural ops (prefer NPU > I8MM > NEON) */
#define HW_WORK_SIGNAL   2   /* DSP / audio / fixed-point (prefer HXN > NEON) */
#define HW_WORK_CRYPTO   3   /* cipher / hash (prefer HW AES/SHA > NEON) */

/* ── Hardware profile struct ─────────────────────────────────────────────── */
typedef struct {
    u32 caps;           /* HW_CAP_* bitmask */
    u8  sve_vlen_128;   /* SVE vector length / 128 (0 = SVE absent) */
    u8  gpu_vendor;     /* 0=unknown 1=adreno 2=mali 3=pvr 4=drm-generic */
    u8  dsp_present;    /* 1 = Hexagon DSP node accessible */
    u8  npu_present;    /* 1 = NPU node accessible */
} HWProfile;

/* ── Internal: CPU feature → cap map ────────────────────────────────────── */
typedef struct { const char *feat; u32 cap; } _HWFeatMap;
static const _HWFeatMap _hw_feat_map[] = {
    { "asimd",   HW_CAP_NEON    },
    { "neon",    HW_CAP_NEON    },  /* some kernels report "neon" */
    { "sve",     HW_CAP_SVE     },
    { "sve2",    HW_CAP_SVE2    },
    { "crc32",   HW_CAP_CRC32   },
    { "aes",     HW_CAP_AES     },
    { "sha2",    HW_CAP_SHA2    },
    { "sha512",  HW_CAP_SHA2    },  /* sha512 implies sha2 */
    { "lse",     HW_CAP_ATOMICS },
    { "atomics", HW_CAP_ATOMICS },
    { "bf16",    HW_CAP_BF16    },
    { "svebf16", HW_CAP_BF16    },
    { "i8mm",    HW_CAP_I8MM    },
    { "svei8mm", HW_CAP_I8MM    },
    { "asimdhp", HW_CAP_FP16    },
    { "fphp",    HW_CAP_FP16    },
    { "asimddp", HW_CAP_DOTPROD },
    { "pmull",   HW_CAP_PMULL   },
    { "sha3",    HW_CAP_SHA3    },
    { "sm4",     HW_CAP_SM4     },
    { "jscvt",   HW_CAP_JSCVT  },
    { "fcma",    HW_CAP_FCMA    },
    { NULL, 0 }
};

/* ── Freestanding string helpers ─────────────────────────────────────────── */
static inline int _hw_str_eq(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (*a == 0) & (*b == 0);
}
static inline int _hw_str_n_eq(const char *a, const char *b, sz n) {
    for (sz i = 0; i < n; i++) if (a[i] != b[i]) return 0;
    return 1;
}

/* Parse one space-/tab-separated token from src[0..slen), starting at *off.
 * Skips whitespace (space, tab). Returns token length; 0 = nothing left. */
static inline sz _hw_tok(const char *src, sz slen, sz *off,
                          char *out, sz outcap) {
    /* skip spaces and tabs (not newline — callers bound by line_end) */
    while (*off < slen && (src[*off] == ' ' || src[*off] == '\t')) (*off)++;
    if (*off >= slen) return 0;
    sz start = *off;
    while (*off < slen && src[*off] != ' ' && src[*off] != '\t') (*off)++;
    sz tlen = *off - start;
    if (tlen >= outcap) tlen = outcap - 1;
    for (sz i = 0; i < tlen; i++) out[i] = src[start + i];
    out[tlen] = 0;
    return tlen;
}

/* Check if a /dev node is accessible (O_RDONLY probe) */
static inline int _hw_dev_ok(const char *path) {
    i32 fd = os_open(path, O_RDONLY, 0);
    if (fd >= 0) { os_close(fd); return 1; }
    return 0;
}

/* ── hw_probe_cpu: parse /proc/cpuinfo Features: line ───────────────────── */
static void hw_probe_cpu(HWProfile *hw) {
    static u8 _cpubuf[8192];
    i32 fd = os_open("/proc/cpuinfo", O_RDONLY, 0);
    if (fd < 0) {
        hw->caps |= HW_CAP_NEON; /* arm64 baseline */
        return;
    }
    sz total = 0;
    while (total < sizeof(_cpubuf) - 1u) {
        i64 n = os_read(fd, _cpubuf + total, sizeof(_cpubuf) - total - 1u);
        if (n <= 0) break;
        total += (sz)n;
    }
    os_close(fd);
    _cpubuf[total] = 0;

    hw->caps |= HW_CAP_NEON; /* mandatory on ARMv8-A */

    const char feat_hdr[] = "Features";
    sz hdr_len = 8; /* strlen("Features") */
    sz i = 0;
    while (i + hdr_len <= total) {
        if (_hw_str_n_eq((const char*)_cpubuf + i, feat_hdr, hdr_len)) {
            /* advance past "Features" to ':' */
            while (i < total && _cpubuf[i] != ':') i++;
            if (i < total) i++; /* skip ':' */
            /* bound to end of this line */
            sz line_end = i;
            while (line_end < total && _cpubuf[line_end] != '\n') line_end++;
            /* parse feature tokens within [i, line_end) */
            while (i < line_end) {
                char tok[32];
                sz tl = _hw_tok((const char*)_cpubuf, line_end, &i, tok, sizeof(tok));
                if (!tl) break;
                for (int k = 0; _hw_feat_map[k].feat; k++) {
                    if (_hw_str_eq(tok, _hw_feat_map[k].feat))
                        hw->caps |= _hw_feat_map[k].cap;
                }
            }
            break; /* first Features: line is authoritative */
        }
        while (i < total && _cpubuf[i] != '\n') i++;
        if (i < total) i++;
    }
}

/* ── hw_probe_gpu: probe /dev GPU nodes ─────────────────────────────────── */
static void hw_probe_gpu(HWProfile *hw) {
    /* Qualcomm Adreno — kgsl = Kernel Graphics Support Layer */
    if (_hw_dev_ok("/dev/kgsl-3d0") || _hw_dev_ok("/dev/kgsl3d")) {
        hw->caps |= HW_CAP_ADRENO | HW_CAP_GPU_VK | HW_CAP_GPU_CL | HW_CAP_GPU_GL;
        if (!hw->gpu_vendor) hw->gpu_vendor = 1;
    }
    /* ARM Mali */
    if (_hw_dev_ok("/dev/mali0") || _hw_dev_ok("/dev/mali")) {
        hw->caps |= HW_CAP_MALI | HW_CAP_GPU_VK | HW_CAP_GPU_CL | HW_CAP_GPU_GL;
        if (!hw->gpu_vendor) hw->gpu_vendor = 2;
    }
    /* Imagination PowerVR */
    if (_hw_dev_ok("/dev/pvrsrvkm") || _hw_dev_ok("/dev/rogue")) {
        hw->caps |= HW_CAP_POWERVR | HW_CAP_GPU_CL | HW_CAP_GPU_GL;
        if (!hw->gpu_vendor) hw->gpu_vendor = 3;
    }
    /* Generic DRM (Mesa / PC / Chromebook) */
    if (_hw_dev_ok("/dev/dri/card0") || _hw_dev_ok("/dev/dri/renderD128")) {
        hw->caps |= HW_CAP_DRM_GEN | HW_CAP_GPU_VK | HW_CAP_GPU_GL;
        if (!hw->gpu_vendor) hw->gpu_vendor = 4;
    }
    /* WebGPU / WGSL: infer from Vulkan presence */
    if (hw->caps & HW_CAP_GPU_VK)
        hw->caps |= HW_CAP_GPU_WGSL;
}

/* ── hw_probe_accel: probe DSP and NPU device nodes ─────────────────────── */
static void hw_probe_accel(HWProfile *hw) {
    /* Qualcomm Hexagon DSP (FastRPC paths: sDSP, cDSP, aDSP) */
    if (_hw_dev_ok("/dev/fastrpc-sdsp") || _hw_dev_ok("/dev/fastrpc-cdsp") ||
        _hw_dev_ok("/dev/fastrpc-adsp") || _hw_dev_ok("/dev/cdsp0")         ||
        _hw_dev_ok("/dev/mdsp0")) {
        hw->caps |= HW_CAP_DSP_HXN;
        hw->dsp_present = 1;
    }
    /* Neural accelerators (vendor-specific) */
    if (_hw_dev_ok("/dev/npu0")       || _hw_dev_ok("/dev/hisi_hiai") ||
        _hw_dev_ok("/dev/mtk_mdla0")  || _hw_dev_ok("/dev/myriad_ion") ||
        _hw_dev_ok("/dev/qrtr")  /* QNN/Qualcomm AI stack */) {
        hw->caps |= HW_CAP_NPU;
        hw->npu_present = 1;
    }
}

/* ── hw_probe: full hardware capability scan ─────────────────────────────── */
static void hw_probe(HWProfile *hw) {
    hw->caps        = 0;
    hw->sve_vlen_128 = 0;
    hw->gpu_vendor  = 0;
    hw->dsp_present = 0;
    hw->npu_present = 0;
    hw_probe_cpu(hw);
    hw_probe_gpu(hw);
    hw_probe_accel(hw);
}

/* ── hw_select_backend: deterministic backend selection ──────────────────── */
static int hw_select_backend(const HWProfile *hw, int workload) {
    int cands[HW_BACKEND_COUNT];
    int nc = 0;

    if (workload == HW_WORK_TENSOR) {
        /* NPU > I8MM/SVE2 > Vulkan compute > OpenCL > SVE > NEON */
        if (hw->caps & HW_CAP_NPU)    cands[nc++] = HW_BACKEND_NPU;
        if (hw->caps & HW_CAP_I8MM)   cands[nc++] = HW_BACKEND_CPU_I8MM;
        if (hw->caps & HW_CAP_GPU_VK) cands[nc++] = HW_BACKEND_GPU_VULKAN;
        if (hw->caps & HW_CAP_GPU_CL) cands[nc++] = HW_BACKEND_GPU_CL;
        if (hw->caps & HW_CAP_SVE2)   cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_SVE)    cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_NEON)   cands[nc++] = HW_BACKEND_CPU_NEON;
    } else if (workload == HW_WORK_SIGNAL) {
        /* HXN DSP > SVE2 > NEON */
        if (hw->caps & HW_CAP_DSP_HXN) cands[nc++] = HW_BACKEND_DSP_HXN;
        if (hw->caps & HW_CAP_SVE2)    cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_SVE)     cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_NEON)    cands[nc++] = HW_BACKEND_CPU_NEON;
    } else if (workload == HW_WORK_CRYPTO) {
        /* HW AES/SHA via NEON crypto ext > SVE > generic NEON */
        if ((hw->caps & HW_CAP_AES) || (hw->caps & HW_CAP_SHA2))
            cands[nc++] = HW_BACKEND_CPU_NEON;
        if (hw->caps & HW_CAP_SVE)  cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_NEON) cands[nc++] = HW_BACKEND_CPU_NEON;
    } else { /* HW_WORK_COMPUTE — GPU-first, CPU fallback */
        if (hw->caps & HW_CAP_GPU_VK)  cands[nc++] = HW_BACKEND_GPU_VULKAN;
        if (hw->caps & HW_CAP_GPU_CL)  cands[nc++] = HW_BACKEND_GPU_CL;
        if (hw->caps & HW_CAP_GPU_WGSL)cands[nc++] = HW_BACKEND_GPU_WGSL;
        if (hw->caps & HW_CAP_DSP_HXN) cands[nc++] = HW_BACKEND_DSP_HXN;
        if (hw->caps & HW_CAP_SVE2)    cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_SVE)     cands[nc++] = HW_BACKEND_CPU_SVE;
        if (hw->caps & HW_CAP_NEON)    cands[nc++] = HW_BACKEND_CPU_NEON;
    }
    cands[nc++] = HW_BACKEND_CPU_SCALAR; /* scalar fallback — always present */

    if (nc == 1) return cands[0];

    /* Deterministic selection: phi_fst over caps → attractor → candidate index.
     * Same caps + workload → same phi_seed → same attractor → same backend. */
    u32 hw_seed[2] = { hw->caps, (u32)workload };
    u32 phi_seed = phi_fst((const u8*)hw_seed, 8u);
    u32 attr     = phi_attractor(phi_seed);
    return cands[attr % (u32)nc];
}

/* ── hw_caps_pr: print capability summary to fd (freestanding, no printf) ── */
static void hw_caps_pr(const HWProfile *hw) {
    /* print hex caps via pr() from sys.h (already included in apkc.c context) */
    u32 c = hw->caps;
    char hbuf[9]; int hi = 8;
    hbuf[hi] = 0;
    while (hi-- > 0) {
        int d = (int)(c & 0xFu);
        hbuf[hi] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        c >>= 4;
    }
    pr("caps=0x"); pr(hbuf);
    if (hw->caps & HW_CAP_NEON)    pr(" neon");
    if (hw->caps & HW_CAP_SVE)     pr(" sve");
    if (hw->caps & HW_CAP_SVE2)    pr(" sve2");
    if (hw->caps & HW_CAP_I8MM)    pr(" i8mm");
    if (hw->caps & HW_CAP_DOTPROD) pr(" dotprod");
    if (hw->caps & HW_CAP_BF16)    pr(" bf16");
    if (hw->caps & HW_CAP_AES)     pr(" aes");
    if (hw->caps & HW_CAP_SHA2)    pr(" sha2");
    if (hw->caps & HW_CAP_CRC32)   pr(" crc32");
    if (hw->caps & HW_CAP_ATOMICS) pr(" lse");
    if (hw->caps & HW_CAP_PMULL)   pr(" pmull");
    if (hw->caps & HW_CAP_FP16)    pr(" fp16");
    if (hw->caps & HW_CAP_GPU_VK)  pr(" vulkan");
    if (hw->caps & HW_CAP_GPU_CL)  pr(" opencl");
    if (hw->caps & HW_CAP_GPU_WGSL)pr(" wgsl");
    if (hw->caps & HW_CAP_ADRENO)  pr(" adreno");
    if (hw->caps & HW_CAP_MALI)    pr(" mali");
    if (hw->caps & HW_CAP_POWERVR) pr(" pvr");
    if (hw->caps & HW_CAP_DRM_GEN) pr(" drm");
    if (hw->caps & HW_CAP_DSP_HXN) pr(" hxn-dsp");
    if (hw->caps & HW_CAP_NPU)     pr(" npu");
    pr("\n");
}
