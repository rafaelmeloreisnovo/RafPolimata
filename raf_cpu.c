#include "raf_compile.h"

#include <stdio.h>
#include <string.h>

void raf_cpu_detect(RafCPU *cpu) {
  memset(cpu, 0, sizeof(*cpu));
#if defined(__x86_64__)
  cpu->arch = RAF_ARCH_X86_64;
  cpu->feat = RAF_FEAT_SSE4;
#if defined(__AVX2__)
  cpu->feat |= RAF_FEAT_AVX2;
#endif
#if defined(__AVX512F__)
  cpu->feat |= RAF_FEAT_AVX512;
#endif
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-x86_64");
#elif defined(__aarch64__)
  cpu->arch = RAF_ARCH_ARM64;
  cpu->feat = RAF_FEAT_NEON;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm64");
#elif defined(__arm__)
  cpu->arch = RAF_ARCH_ARM32;
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  cpu->feat = RAF_FEAT_NEON;
#endif
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm32");
#elif defined(__riscv) && (__riscv_xlen == 64)
  cpu->arch = RAF_ARCH_RV64;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-rv64");
#else
  cpu->arch = RAF_ARCH_UNKNOWN;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic");
#endif
  cpu->cores = 1;
}

uint8_t raf_lang_from_ext(const char *path) {
  const char *dot = strrchr(path, '.');
  if (!dot) return RAF_LANG_C;
  if (!strcmp(dot, ".c")) return RAF_LANG_C;
  if (!strcmp(dot, ".cpp") || !strcmp(dot, ".cc")) return RAF_LANG_CPP;
  if (!strcmp(dot, ".s")) return RAF_LANG_S;
  if (!strcmp(dot, ".py")) return RAF_LANG_PY;
  if (!strcmp(dot, ".rs")) return RAF_LANG_RS;
  if (!strcmp(dot, ".kt")) return RAF_LANG_KT;
  if (!strcmp(dot, ".java")) return RAF_LANG_JAVA;
  return RAF_LANG_C;
}

void raf_flag_matrix_get(uint8_t arch, uint8_t lang, uint8_t opt, uint32_t feat,
                         char *out_flags, int cap) {
  const char *base = "-O2";
  const char *isa = "";
  const char *lang_flags = " -ffreestanding -fno-builtin -fno-exceptions";
  if (opt == RAF_OPT_0) base = "-O0 -g";
  else if (opt == RAF_OPT_1) base = "-O1";
  else if (opt == RAF_OPT_3) base = "-O3";
  else if (opt == RAF_OPT_S) base = "-Os";

  if (arch == RAF_ARCH_X86_64) {
    if ((feat & RAF_FEAT_AVX512) != 0u) isa = " -mavx512f";
    else if ((feat & RAF_FEAT_AVX2) != 0u) isa = " -mavx2";
    else if ((feat & RAF_FEAT_SSE4) != 0u) isa = " -msse4.2";
  } else if (arch == RAF_ARCH_ARM64) {
    isa = " -march=armv8-a+simd";
  } else if (arch == RAF_ARCH_ARM32) {
    isa = ((feat & RAF_FEAT_NEON) != 0u) ? " -mfpu=neon" : " -march=armv7-a";
  } else if (arch == RAF_ARCH_RV64) {
    isa = " -march=rv64gc";
  }

  if (lang == RAF_LANG_PY || lang == RAF_LANG_JAVA || lang == RAF_LANG_KT) {
    lang_flags = "";
  }
  snprintf(out_flags, (size_t)cap, "%s%s%s", base, isa, lang_flags);
}
