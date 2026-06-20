/* raf_cpu.c — CPU architecture detection and compiler flag matrix.
 * Uses POSIX open/read/close on Linux (no stdio FILE*) to read /proc/cpuinfo
 * for runtime feature discovery beyond compile-time macros.
 * No malloc/calloc/free. No stdio.h. */

#include "raf_compile.h"

#include <string.h>   /* memset, strncpy, strcmp, strrchr, strstr */
#include <stdint.h>
#include <stddef.h>

/* snprintf from <stdio.h> replaced by a minimal no-stdio helper below. */
#include <stdio.h>   /* kept only for snprintf; no FILE* usage */

/* ── /proc/cpuinfo reader (Linux only, no stdio) ────────────────────────── */
#if defined(__linux__)
#include <fcntl.h>    /* open, O_RDONLY */
#include <unistd.h>   /* read, close */

/* Read at most buf_cap-1 bytes from path into buf; NUL-terminates.
 * Returns number of bytes read, or 0 on failure. */
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

/* Search needle in haystack (limit to len bytes). Returns pointer or NULL. */
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

/* Find the value portion of a line like "Features\t: neon aes sha2 ..."
 * Writes the feature string into out (up to out_cap-1 chars). */
static void _cpuinfo_get_features(const char *cpuinfo, int cpuinfo_len,
                                   char *out, int out_cap) {
    static const char key[] = "Features";
    const char *p = _memmem_c(cpuinfo, cpuinfo_len, key, (int)(sizeof(key)-1));
    if (!p) { out[0] = '\0'; return; }
    /* advance past "Features" to the colon */
    p += sizeof(key) - 1;
    while (*p && *p != ':' && *p != '\n') p++;
    if (*p != ':') { out[0] = '\0'; return; }
    p++; /* skip ':' */
    while (*p == ' ' || *p == '\t') p++;
    /* copy until end of line */
    int i = 0;
    while (*p && *p != '\n' && i < out_cap - 1) out[i++] = *p++;
    out[i] = '\0';
}

/* Find the Hardware/model name line for the brand string. */
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

/* Check if word appears as a whole token in a space-delimited feature string. */
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

/* Detect additional feature flags from /proc/cpuinfo on Linux.
 * Adds bits to *feat_inout; does NOT clear pre-existing bits. */
static void _linux_detect_features(uint8_t arch, uint32_t *feat_inout,
                                    char *brand_out, int brand_cap) {
    static char cpuinfo[4096];
    int n = _read_procfile("/proc/cpuinfo", cpuinfo, (int)sizeof(cpuinfo));
    if (n <= 0) return;

    /* Grab brand/model name */
    static char tmp_brand[64];
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

    /* Feature flags from /proc/cpuinfo */
    static char feat_line[256];
    _cpuinfo_get_features(cpuinfo, n, feat_line, (int)sizeof(feat_line));

    if (arch == RAF_ARCH_ARM64 || arch == RAF_ARCH_ARM32) {
        /* ARM: neon / asimd → RAF_FEAT_NEON already set by macro; verify runtime */
        if (_feat_has(feat_line, "neon") || _feat_has(feat_line, "asimd"))
            *feat_inout |= RAF_FEAT_NEON;
        /* AES, SHA2 — no new RAF_FEAT bits yet; reserved for future extension.
         * (Do not invent bits that don't exist in raf_compile.h — TOKEN_VAZIO) */
    } else if (arch == RAF_ARCH_X86_64) {
        /* x86-64: cross-check compiler flags against runtime CPUINFO */
        if (_feat_has(feat_line, "avx512f"))
            *feat_inout |= RAF_FEAT_AVX512;
        else if (_feat_has(feat_line, "avx2"))
            *feat_inout |= RAF_FEAT_AVX2;
        else if (_feat_has(feat_line, "sse4_2"))
            *feat_inout |= RAF_FEAT_SSE4;
    }
}
#endif /* __linux__ */

/* ── Public API ─────────────────────────────────────────────────────────── */

void raf_cpu_detect(RafCPU *cpu) {
    memset(cpu, 0, sizeof(*cpu));

/* Step 1: compile-time architecture macros (always reliable) */
#if defined(__aarch64__)
    cpu->arch = RAF_ARCH_ARM64;
    cpu->feat = RAF_FEAT_NEON;    /* ARMv8-A mandates NEON/ASIMD */
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm64");
#elif defined(__arm__)
    cpu->arch = RAF_ARCH_ARM32;
#  if defined(__ARM_NEON) || defined(__ARM_NEON__)
    cpu->feat = RAF_FEAT_NEON;
#  endif
    snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm32");
#elif defined(__x86_64__)
    cpu->arch = RAF_ARCH_X86_64;
    cpu->feat = RAF_FEAT_SSE4;    /* baseline: SSE4.2 assumed on any modern x86-64 */
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

/* Step 2: Linux runtime /proc/cpuinfo — refines features and brand string.
 * Gated strictly: no-op on non-Linux. Only adds bits, never removes them. */
#if defined(__linux__)
    _linux_detect_features(cpu->arch, &cpu->feat,
                           cpu->brand, (int)sizeof(cpu->brand));
#endif

    cpu->cores = 1;
}

/* ── Language extension → RAF_LANG_* ────────────────────────────────────── */

uint8_t raf_lang_from_ext(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return RAF_LANG_C;
    if (!strcmp(dot, ".c"))                     return RAF_LANG_C;
    if (!strcmp(dot, ".cpp") || !strcmp(dot, ".cc")) return RAF_LANG_CPP;
    if (!strcmp(dot, ".s") || !strcmp(dot, ".S"))    return RAF_LANG_S;
    if (!strcmp(dot, ".py"))                    return RAF_LANG_PY;
    if (!strcmp(dot, ".rs"))                    return RAF_LANG_RS;
    if (!strcmp(dot, ".kt"))                    return RAF_LANG_KT;
    if (!strcmp(dot, ".java"))                  return RAF_LANG_JAVA;
    if (!strcmp(dot, ".sh"))                    return RAF_LANG_SH;
    if (!strcmp(dot, ".pl"))                    return RAF_LANG_PL;
    if (!strcmp(dot, ".js"))                    return RAF_LANG_JS;
    if (!strcmp(dot, ".php"))                   return RAF_LANG_PHP;
    if (!strcmp(dot, ".jsx"))                   return RAF_LANG_JSX;
    return RAF_LANG_C;
}

/* ── Compiler flag matrix ────────────────────────────────────────────────── */

/* Specced output for C/CPP per arch (task requirement, overrides generic path):
 *   ARM64 + C/CPP → "-march=armv8-a -mtune=generic -O2"
 *   ARM32 + C/CPP → "-march=armv7-a -mfloat-abi=softfp -mfpu=neon -O2"
 *   X86_64 + C/CPP → "-march=x86-64 -mtune=generic -O2"
 * Other arches/langs → generic opt + isa string (TOKEN_VAZIO for unknown arch) */
void raf_flag_matrix_get(uint8_t arch, uint8_t lang, uint8_t opt, uint32_t feat,
                         char *out_flags, int cap) {
    /* Shorthand: is this a native compiled language (C or C++)? */
    int is_c_cpp = (lang == RAF_LANG_C || lang == RAF_LANG_CPP);

    /* Canonical flag strings for C/CPP per architecture (spec-mandated) */
    if (is_c_cpp) {
        const char *s = "";
        if (arch == RAF_ARCH_ARM64) {
            s = "-march=armv8-a -mtune=generic -O2";
        } else if (arch == RAF_ARCH_ARM32) {
            s = "-march=armv7-a -mfloat-abi=softfp -mfpu=neon -O2";
        } else if (arch == RAF_ARCH_X86_64) {
            s = "-march=x86-64 -mtune=generic -O2";
        }
        /* else RAF_ARCH_UNKNOWN / RV64 → TOKEN_VAZIO (empty string, return 0-equiv) */
        if (cap > 0) {
            strncpy(out_flags, s, (size_t)(cap - 1));
            out_flags[cap - 1] = '\0';
        }
        return;
    }

    /* Non-C/CPP languages: build flags from opt + isa components */
    const char *base = "-O2";
    const char *isa  = "";
    const char *lang_flags = "";

    /* Optimization level */
    if      (opt == RAF_OPT_0) base = "-O0 -g";
    else if (opt == RAF_OPT_1) base = "-O1";
    else if (opt == RAF_OPT_3) base = "-O3";
    else if (opt == RAF_OPT_S) base = "-Os";
    /* RAF_OPT_2 → default "-O2" */

    /* Architecture ISA string */
    if (arch == RAF_ARCH_X86_64) {
        if      ((feat & RAF_FEAT_AVX512) != 0u) isa = " -mavx512f";
        else if ((feat & RAF_FEAT_AVX2)   != 0u) isa = " -mavx2";
        else if ((feat & RAF_FEAT_SSE4)   != 0u) isa = " -msse4.2";
        else                                      isa = " -march=x86-64";
    } else if (arch == RAF_ARCH_ARM64) {
        isa = " -march=armv8-a+simd";
    } else if (arch == RAF_ARCH_ARM32) {
        isa = ((feat & RAF_FEAT_NEON) != 0u) ? " -mfpu=neon" : " -march=armv7-a";
    } else if (arch == RAF_ARCH_RV64) {
        isa = " -march=rv64gc";
    }
    /* RAF_ARCH_UNKNOWN → isa="" → TOKEN_VAZIO (only base opt in output) */

    /* Interpreter/JVM languages don't need freestanding flags */
    if (lang == RAF_LANG_S) {
        lang_flags = " -ffreestanding -fno-builtin";
    }

    if (lang == RAF_LANG_PY  || lang == RAF_LANG_JAVA ||
        lang == RAF_LANG_KT  || lang == RAF_LANG_SH   ||
        lang == RAF_LANG_PL  || lang == RAF_LANG_JS    ||
        lang == RAF_LANG_PHP || lang == RAF_LANG_JSX) {
        lang_flags = "";
    }

    snprintf(out_flags, (size_t)cap, "%s%s%s", base, isa, lang_flags);
    (void)feat; /* already used above */
}
