#ifndef RAF_COMPILE_H
#define RAF_COMPILE_H

#include <stddef.h>
#include <stdint.h>

#define RAF_ARCH_X86_64 0
#define RAF_ARCH_ARM64 1
#define RAF_ARCH_ARM32 2
#define RAF_ARCH_RV64 3
#define RAF_ARCH_UNKNOWN 4

#define RAF_LANG_C 0
#define RAF_LANG_CPP 1
#define RAF_LANG_S 2
#define RAF_LANG_PY 3
#define RAF_LANG_RS 4
#define RAF_LANG_KT 5
#define RAF_LANG_JAVA 6
#define RAF_LANG_SH 7
#define RAF_LANG_PL 8
#define RAF_LANG_JS 9
#define RAF_LANG_PHP 10
#define RAF_LANG_JSX 11
/* hardware-direct compute languages (mirror APKc LP_GLSL…LP_TFLITE) */
#define RAF_LANG_GLSL   12  /* Vulkan GLSL compute → SPIR-V APK asset */
#define RAF_LANG_CL     13  /* OpenCL C source → APK asset */
#define RAF_LANG_HLSL   14  /* HLSL compute → SPIR-V APK asset */
#define RAF_LANG_WGSL   15  /* WebGPU WGSL source → APK asset */
#define RAF_LANG_DSP    16  /* Hexagon DSP C → DSP .so APK asset */
#define RAF_LANG_TFLITE 17  /* TFLite model flatbuffer → NPU APK asset */
#define RAF_LANG_COUNT 18

#define RAF_OPT_0 0
#define RAF_OPT_1 1
#define RAF_OPT_2 2
#define RAF_OPT_3 3
#define RAF_OPT_S 4

#define RAF_FEAT_SSE4    0x001u  /* x86 SSE4.2 */
#define RAF_FEAT_AVX2    0x002u  /* x86 AVX2 */
#define RAF_FEAT_AVX512  0x004u  /* x86 AVX-512 */
#define RAF_FEAT_NEON    0x008u  /* ARM64 ASIMD/NEON (mandatory on arm64) */
#define RAF_FEAT_SVE     0x010u  /* ARM64 Scalable Vector Extension */
#define RAF_FEAT_I8MM    0x020u  /* ARM64 Int8 matrix multiply (SMMLA) */
#define RAF_FEAT_SME     0x040u  /* ARM64 Scalable Matrix Extension */
#define RAF_FEAT_GPU_VK  0x080u  /* Vulkan-capable GPU detected at runtime */
#define RAF_FEAT_GPU_CL  0x100u  /* OpenCL-capable GPU detected at runtime */
#define RAF_FEAT_DSP_HXN 0x200u  /* Qualcomm Hexagon DSP detected at runtime */
#define RAF_FEAT_NPU     0x400u  /* Neural accelerator detected at runtime */

#define RAF_IR_CAP (1u << 16)
#define RAF_ASM_CAP (1u << 15)
#define RAF_ASM_LINE 128
#define RAF_HEX_CAP (1u << 20)

typedef enum { IR_NOP = 0, IR_MOVIMM, IR_RET } RafIROp;
typedef uint64_t RafIR;

typedef struct {
  uint8_t arch;
  uint32_t feat;
  char brand[64];
  uint32_t cores;
} RafCPU;

typedef struct {
  RafIR buf[RAF_IR_CAP];
  uint32_t n;
} RafIRBuf;

typedef struct {
  char lines[RAF_ASM_CAP][RAF_ASM_LINE];
  uint32_t n;
} RafAsmBuf;

typedef struct {
  uint8_t bytes[RAF_HEX_CAP];
  uint32_t n;
} RafBin;

typedef struct {
  RafCPU cpu;
  uint8_t lang;
  uint8_t opt;
  uint32_t feat;
  const char *src;
  size_t src_len;
  RafIRBuf ir;
  RafAsmBuf asm_out;
  RafBin bin;
  char out_asm[256];
  char out_hex[256];
  char out_bin[256];
  char out_ops[256];
  char flags[128];
  uint64_t src_hash;

  /* Deterministic RMR-CTI/Omega source profile. These fields classify the
   * source byte stream for routing and audit; they do not claim semantic truth. */
  uint32_t omega_entropy_milli;
  uint32_t omega_phi_q16;
  uint32_t omega_attractor;
  uint32_t omega_flags;
  uint8_t omega_path;

  uint64_t ops_signature;
  int rollback_code;
  uint64_t elapsed_ns;
} RafCtx;

void raf_cpu_detect(RafCPU *cpu);
uint8_t raf_lang_from_ext(const char *path);
void raf_flag_matrix_get(uint8_t arch, uint8_t lang, uint8_t opt, uint32_t feat,
                         char *out_flags, int cap);
void raf_ctx_init(RafCtx *ctx);
int raf_precompile(RafCtx *ctx);
int raf_asm_emit(RafCtx *ctx);
int raf_hex_encode(RafCtx *ctx);
int raf_ir_lower(RafCtx *ctx);
int raf_compile_file(RafCtx *ctx, const char *src_path, const char *out_base,
                     int do_native);
void raf_ctx_report(const RafCtx *ctx);

/* ── Language × Architecture capability matrix ───────────────────────────
 * RAF_CAP_MATRIX[lang][arch]: 1 = can produce valid APK on this host arch.
 * use_script langs (py/sh/pl/js/php): any host with apkc binary → 1 on x86+arm
 * use_asm (asm/S): internal 2-pass assembler, no external toolchain needed
 * use_fork langs (c/cpp/rs/kt/java/jsx): fork_exec_wait is arm64-only → 0 on x86
 *
 * Columns: [0]=x86_64 [1]=arm64 [2]=arm32 [3]=rv64 [4]=unknown
 * Rows:    C CPP ASM PY RS KT JAVA SH PL JS PHP JSX
 *          GLSL CL HLSL WGSL DSP TFLITE  (RAF_LANG_* order) */
static const uint8_t RAF_CAP_MATRIX[RAF_LANG_COUNT][5] = {
 /* lang          x86_64 arm64 arm32 rv64  unk  */
 /* C      fork */  { 0,   1,   0,   0,   0 },
 /* CPP    fork */  { 0,   1,   0,   0,   0 },
 /* ASM    asm  */  { 1,   1,   1,   0,   0 },
 /* PY     scr  */  { 1,   1,   1,   0,   1 },
 /* RS     fork */  { 0,   1,   0,   0,   0 },
 /* KT     fork */  { 0,   1,   0,   0,   0 },
 /* JAVA   fork */  { 0,   1,   0,   0,   0 },
 /* SH     scr  */  { 1,   1,   1,   0,   1 },
 /* PL     scr  */  { 1,   1,   1,   0,   1 },
 /* JS     scr  */  { 1,   1,   1,   0,   1 },
 /* PHP    scr  */  { 1,   1,   1,   0,   1 },
 /* JSX    fork */  { 0,   1,   0,   0,   0 },
 /* GLSL   gpu  */  { 0,   1,   0,   0,   0 },
 /* CL     gpu  */  { 0,   1,   0,   0,   0 },
 /* HLSL   gpu  */  { 0,   1,   0,   0,   0 },
 /* WGSL   gpu  */  { 0,   1,   0,   0,   0 },
 /* DSP    dsp  */  { 0,   1,   0,   0,   0 },
 /* TFLITE npu  */  { 0,   1,   0,   0,   0 },
};

/* Query helper: returns 1 if host_arch can compile lang to valid APK, 0 otherwise. */
static inline int raf_cap_query(uint8_t lang, uint8_t arch) {
    if (lang >= RAF_LANG_COUNT || arch > RAF_ARCH_UNKNOWN) return 0;
    return (int)RAF_CAP_MATRIX[lang][arch];
}

/* ── APKc bridge ──────────────────────────────────────────────────────────
 * These two inline helpers let any caller bridge RafCtx → APKc dispatch
 * without including any Apkc/ headers directly.
 * Usage:
 *   #include "raf_compile.h"
 *   #include "Apkc/lang_profile.h"
 *   const LangProfile *prof =
 *       lang_profile_find(raf_lang_to_apkc_name(ctx.lang));
 *   int do64, do32;
 *   raf_cpu_to_apkc_modes(&ctx.cpu, &do64, &do32);
 * ─────────────────────────────────────────────────────────────────────── */

/* Maps RAF_LANG_* to the APKc lang_profile name string.
 * Returns NULL for values without an APKc mapping. */
static inline const char *raf_lang_to_apkc_name(uint8_t lang) {
    switch (lang) {
    case RAF_LANG_C:    return "c";
    case RAF_LANG_CPP:  return "cpp";
    case RAF_LANG_S:    return "asm";
    case RAF_LANG_PY:   return "py";
    case RAF_LANG_RS:   return "rs";
    case RAF_LANG_KT:   return "kt";
    case RAF_LANG_JAVA: return "java";
    case RAF_LANG_SH:   return "sh";
    case RAF_LANG_PL:   return "pl";
    case RAF_LANG_JS:   return "js";
    case RAF_LANG_PHP:  return "php";
    case RAF_LANG_JSX:    return "jsx";
    case RAF_LANG_GLSL:   return "glsl";
    case RAF_LANG_CL:     return "cl";
    case RAF_LANG_HLSL:   return "hlsl";
    case RAF_LANG_WGSL:   return "wgsl";
    case RAF_LANG_DSP:    return "dsp";
    case RAF_LANG_TFLITE: return "tflite";
    default:              return (const char *)0;
    }
}

/* Select best APKc compute language for detected hardware features.
 * Priority: Vulkan GPU > OpenCL GPU > Hexagon DSP > NPU/TFLite > C scalar.
 * Pass RafCPU.feat populated from raf_cpu_detect() + runtime hw probing. */
static inline const char *raf_hw_select_compute_lang(uint32_t feat) {
    if (feat & RAF_FEAT_GPU_VK)   return "glsl";
    if (feat & RAF_FEAT_GPU_CL)   return "cl";
    if (feat & RAF_FEAT_DSP_HXN)  return "dsp";
    if (feat & RAF_FEAT_NPU)      return "tflite";
    return "c";  /* scalar CPU fallback */
}

/* Maps RafCPU arch + feat flags to APKc do64/do32 output mode. */
static inline void raf_cpu_to_apkc_modes(const RafCPU *cpu,
                                          int *do64, int *do32) {
    *do64 = (cpu->arch == RAF_ARCH_ARM64)
         || (cpu->arch == RAF_ARCH_UNKNOWN && (cpu->feat & RAF_FEAT_NEON));
    *do32 = (cpu->arch == RAF_ARCH_ARM32);
}

#endif
