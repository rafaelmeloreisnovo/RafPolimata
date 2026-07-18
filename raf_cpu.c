/* raf_cpu.c — CPU architecture detection and compiler flag matrix.
 * Uses POSIX open/read/close on Linux to read /proc/cpuinfo.
 * No malloc/calloc/free. */

#include "raf_compile.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>

static int _read_procfile(const char *path, char *buf, int buf_cap) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;
    int total = 0, n;
    while (total < buf_cap - 1) {
        n = (int)read(fd, buf + total, (size_t)(buf_cap - 1 - total));
        if (n <= 0) break;
        total += n;
    }
    close(fd);
    buf[total] = '\0';
    return total;
}

static const char *_memmem_c(const char *hay, int hlen,
                              const char *needle, int nlen) {
    if (nlen == 0) return hay;
    for (int i = 0; i <= hlen - nlen; i++) {
        int j = 0;
        while (j < nlen && hay[i + j] == needle[j]) j++;
        if (j == nlen) return hay + i;
    }
    return (const char *)0;
}

static void _cpuinfo_get_features(const char *cpuinfo, int cpuinfo_len,
                                   char *out, int out_cap) {
    static const char key[] = "Features";
    const char *p = _memmem_c(cpuinfo, cpuinfo_len, key, (int)(sizeof(key)-1));
    if (!p) { out[0] = '\0'; return; }
    p += sizeof(key) - 1;
    while (*p && *p != ':' && *p != '\n') p++;
    if (*p != ':') { out[0] = '\0'; return; }
    p++;
    while (*p == ' ' || *p == '\t') p++;
    int i = 0;
    while (*p && *p != '\n' && i < out_cap - 1) out[i++] = *p++;
    out[i] = '\0';
}

static void _cpuinfo_get_brand(const char *cpuinfo, int cpuinfo_len,
                                const char *field,
                                char *out, int out_cap) {
    int flen = (int)strlen(field);
    const char *p = _memmem_c(cpuinfo, cpuinfo_len, field, flen);
    if (!p) { out[0] = '\0'; return; }
    p += flen;
    while (*p && *p != ':' && *p != '\n') p++;
    if (*p != ':') { out[0] = '\0'; return; }
    p++;
    while (*p == ' ' || *p == '\t') p++;
    int i = 0;
    while (*p && *p != '\n' && i < out_cap - 1) out[i++] = *p++;
    out[i] = '\0';
}

static int _feat_has(const char *feat_str, const char *word) {
    int wlen = (int)strlen(word);
    const char *p = feat_str;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        const char *tok = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        int tlen = (int)(p - tok);
        if (tlen == wlen && memcmp(tok, word, (size_t)wlen) == 0) return 1;
    }
    return 0;
}

static int _dev_ok(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd >= 0) { close(fd); return 1; }
    return 0;
}

/* Device nodes are presence evidence only. They must not be promoted directly
 * to Vulkan/OpenCL/DSP/NPU execution capability. Dedicated API enumeration and
 * a minimal kernel/runtime test are required before setting capability bits. */
static void _linux_detect_hw_nodes(uint32_t *feat_inout) {
    if (_dev_ok("/dev/kgsl-3d0") || _dev_ok("/dev/kgsl3d") ||
        _dev_ok("/dev/mali0") || _dev_ok("/dev/mali") ||
        _dev_ok("/dev/pvrsrvkm") || _dev_ok("/dev/rogue") ||
        _dev_ok("/dev/dri/card0") || _dev_ok("/dev/dri/renderD128"))
        *feat_inout |= RAF_FEAT_GPU_NODE;

    if (_dev_ok("/dev/fastrpc-sdsp") || _dev_ok("/dev/fastrpc-cdsp") ||
        _dev_ok("/dev/fastrpc-adsp") || _dev_ok("/dev/cdsp0") ||
        _dev_ok("/dev/mdsp0"))
        *feat_inout |= RAF_FEAT_DSP_NODE;

    if (_dev_ok("/dev/npu0") || _dev_ok("/dev/hisi_hiai") ||
        _dev_ok("/dev/mtk_mdla0") || _dev_ok("/dev/myriad_ion") ||
        _dev_ok("/dev/qrtr"))
        *feat_inout |= RAF_FEAT_NPU_NODE;
}

static void _linux_detect_features(uint8_t arch, uint32_t *feat_inout,
                                    char *brand_out, int brand_cap) {
    static char cpuinfo[4096];
    int n = _read_procfile("/proc/cpuinfo", cpuinfo, (int)sizeof(cpuinfo));
    if (n <= 0) return;

    static char tmp_brand[64];
    tmp_brand[0] = '\0';
    if (arch == RAF_ARCH_ARM64 || arch == RAF_ARCH_ARM32) {
        _cpuinfo_get_brand(cpuinfo, n, "Hardware", tmp_brand, (int)sizeof(tmp_brand));
        if (tmp_brand[0] == '\0')
            _cpuinfo_get_brand(cpuinfo, n, "Model name", tmp_brand, (int)sizeof(tmp_brand));
    } else if (arch == RAF_ARCH_X86_64) {
        _cpuinfo_get_brand(cpuinfo, n, "model name", tmp_brand, (int)sizeof(tmp_brand));
    }
    if (tmp_brand[0] != '\0') {
        int blen = (int)strlen(tmp_brand);
        int bcopy = blen < (brand_cap - 1) ? blen : (brand_cap - 1);
        memcpy(brand_out, tmp_brand, (size_t)bcopy);
        brand_out[bcopy] = '\0';
    }

    static char feat_line[256];
    _cpuinfo_get_features(cpuinfo, n, feat_line, (int)sizeof(feat_line));

    if (arch == RAF_ARCH_ARM64 || arch == RAF_ARCH_ARM32) {
        if (_feat_has(feat_line, "neon") || _feat_has(feat_line, "asimd"))
            *feat_inout |= RAF_FEAT_NEON;
        if (_feat_has(feat_line, "sve") || _feat_has(feat_line, "sve2"))
            *feat_inout |= RAF_FEAT_SVE;
        if (_feat_has(feat_line, "i8mm") || _feat_has(feat_line, "svei8mm"))
            *feat_inout |= RAF_FEAT_I8MM;
        if (_feat_has(feat_line, "sme") || _feat_has(feat_line, "sme2"))
            *feat_inout |= RAF_FEAT_SME;
    } else if (arch == RAF_ARCH_X86_64) {
        if (_feat_has(feat_line, "avx512f")) *feat_inout |= RAF_FEAT_AVX512;
        if (_feat_has(feat_line, "avx2")) *feat_inout |= RAF_FEAT_AVX2;
        if (_feat_has(feat_line, "sse4_2")) *feat_inout |= RAF_FEAT_SSE4;
    }
}
#endif

void raf_cpu_detect(RafCPU *cpu) {
    memset(cpu, 0, sizeof(*cpu));

#if defined(__aarch64__)
    cpu->arch = RAF_ARCH_ARM64;
    cpu->feat = RAF_FEAT_NEON;
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm64");
#elif defined(__arm__)
    cpu->arch = RAF_ARCH_ARM32;
#  if defined(__ARM_NEON) || defined(__ARM_NEON__)
    cpu->feat = RAF_FEAT_NEON;
#  endif
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm32");
#elif defined(__x86_64__)
    cpu->arch = RAF_ARCH_X86_64;
    cpu->feat = 0u;
#  if defined(__SSE4_2__)
    cpu->feat |= RAF_FEAT_SSE4;
#  endif
#  if defined(__AVX2__)
    cpu->feat |= RAF_FEAT_AVX2;
#  endif
#  if defined(__AVX512F__)
    cpu->feat |= RAF_FEAT_AVX512;
#  endif
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-x86_64");
#elif defined(__i386__)
    cpu->arch = RAF_ARCH_UNKNOWN;
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-i386");
#elif defined(__riscv) && (__riscv_xlen == 64)
    cpu->arch = RAF_ARCH_RV64;
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-rv64");
#else
    cpu->arch = RAF_ARCH_UNKNOWN;
    snprintf(cpu->brand, sizeof(cpu->brand), "generic");
#endif

#if defined(__linux__)
    _linux_detect_features(cpu->arch, &cpu->feat,
                           cpu->brand, (int)sizeof(cpu->brand));
    _linux_detect_hw_nodes(&cpu->feat);
    long online = sysconf(_SC_NPROCESSORS_ONLN);
    cpu->cores = online > 0 && online <= UINT32_MAX ? (uint32_t)online : 1u;
#else
    cpu->cores = 1u;
#endif
}

uint8_t raf_lang_from_ext(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return RAF_LANG_UNKNOWN;
    if (!strcmp(dot, ".c"))                         return RAF_LANG_C;
    if (!strcmp(dot, ".cpp") || !strcmp(dot, ".cc")) return RAF_LANG_CPP;
    if (!strcmp(dot, ".s") || !strcmp(dot, ".S")) return RAF_LANG_S;
    if (!strcmp(dot, ".py"))                        return RAF_LANG_PY;
    if (!strcmp(dot, ".rs"))                        return RAF_LANG_RS;
    if (!strcmp(dot, ".kt"))                        return RAF_LANG_KT;
    if (!strcmp(dot, ".java"))                      return RAF_LANG_JAVA;
    if (!strcmp(dot, ".sh"))                        return RAF_LANG_SH;
    if (!strcmp(dot, ".pl"))                        return RAF_LANG_PL;
    if (!strcmp(dot, ".js"))                        return RAF_LANG_JS;
    if (!strcmp(dot, ".php"))                       return RAF_LANG_PHP;
    if (!strcmp(dot, ".jsx"))                       return RAF_LANG_JSX;
    if (!strcmp(dot, ".comp"))                      return RAF_LANG_GLSL;
    if (!strcmp(dot, ".cl"))                        return RAF_LANG_CL;
    if (!strcmp(dot, ".hlsl"))                      return RAF_LANG_HLSL;
    if (!strcmp(dot, ".wgsl"))                      return RAF_LANG_WGSL;
    if (!strcmp(dot, ".dsp"))                       return RAF_LANG_DSP;
    if (!strcmp(dot, ".tflite"))                    return RAF_LANG_TFLITE;
    return RAF_LANG_UNKNOWN;
}

void raf_flag_matrix_get(uint8_t arch, uint8_t lang, uint8_t opt, uint32_t feat,
                         char *out_flags, int cap) {
    if (cap <= 0) return;
    if (lang == RAF_LANG_UNKNOWN) {
        out_flags[0] = '\0';
        return;
    }

    if (lang == RAF_LANG_GLSL || lang == RAF_LANG_CL ||
        lang == RAF_LANG_HLSL || lang == RAF_LANG_WGSL ||
        lang == RAF_LANG_DSP || lang == RAF_LANG_TFLITE) {
        out_flags[0] = '\0';
        return;
    }

    int is_c_cpp = (lang == RAF_LANG_C || lang == RAF_LANG_CPP);
    if (is_c_cpp) {
        const char *s = "";
        if (arch == RAF_ARCH_ARM64) {
            s = "-march=armv8-a -mtune=generic -O2";
        } else if (arch == RAF_ARCH_ARM32) {
            s = "-march=armv7-a -mfloat-abi=softfp -mfpu=neon -O2";
        } else if (arch == RAF_ARCH_X86_64) {
            s = "-march=x86-64 -mtune=generic -O2";
        }
        strncpy(out_flags, s, (size_t)(cap - 1));
        out_flags[cap - 1] = '\0';
        return;
    }

    const char *base = "-O2";
    const char *isa = "";
    const char *lang_flags = "";

    if (opt == RAF_OPT_0) base = "-O0 -g";
    else if (opt == RAF_OPT_1) base = "-O1";
    else if (opt == RAF_OPT_3) base = "-O3";
    else if (opt == RAF_OPT_S) base = "-Os";

    if (arch == RAF_ARCH_X86_64) {
        if ((feat & RAF_FEAT_AVX512) != 0u) isa = " -mavx512f";
        else if ((feat & RAF_FEAT_AVX2) != 0u) isa = " -mavx2";
        else if ((feat & RAF_FEAT_SSE4) != 0u) isa = " -msse4.2";
        else isa = " -march=x86-64";
    } else if (arch == RAF_ARCH_ARM64) {
        isa = " -march=armv8-a+simd";
    } else if (arch == RAF_ARCH_ARM32) {
        isa = ((feat & RAF_FEAT_NEON) != 0u) ? " -mfpu=neon" : " -march=armv7-a";
    } else if (arch == RAF_ARCH_RV64) {
        isa = " -march=rv64gc";
    }

    if (lang == RAF_LANG_S) {
        lang_flags = " -ffreestanding -fno-builtin";
    }

    if (lang == RAF_LANG_PY || lang == RAF_LANG_JAVA ||
        lang == RAF_LANG_KT || lang == RAF_LANG_SH ||
        lang == RAF_LANG_PL || lang == RAF_LANG_JS ||
        lang == RAF_LANG_PHP || lang == RAF_LANG_JSX) {
        lang_flags = "";
    }

    snprintf(out_flags, (size_t)cap, "%s%s%s", base, isa, lang_flags);
}
