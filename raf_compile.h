#ifndef RAF_COMPILE_H
#define RAF_COMPILE_H

#include <stddef.h>
#include <stdint.h>

#define RAF_ARCH_X86_64 0
#define RAF_ARCH_ARM64 1
#define RAF_ARCH_ARM32 2
#define RAF_ARCH_RV64 3
#define RAF_ARCH_UNKNOWN 4

/* Stable compiler IDs. Existing IDs are preserved; profiles added later are
 * appended. ApkC LP_* IDs have their own order and are mapped by name. */
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
#define RAF_LANG_GLSL 12
#define RAF_LANG_CL 13
#define RAF_LANG_HLSL 14
#define RAF_LANG_WGSL 15
#define RAF_LANG_DSP 16
#define RAF_LANG_TFLITE 17
/* additional fork/script languages mirroring LP_GO…LP_CLJ */
#define RAF_LANG_GO      18
#define RAF_LANG_RB      19
#define RAF_LANG_SWIFT   20
#define RAF_LANG_GROOVY  21
#define RAF_LANG_CLJ     22
#define RAF_LANG_UNKNOWN 23
#define RAF_RECOGNIZED_LANG_COUNT 23
#define RAF_LANG_COUNT   24

#define RAF_OPT_0 0
#define RAF_OPT_1 1
#define RAF_OPT_2 2
#define RAF_OPT_3 3
#define RAF_OPT_S 4

#define RAF_FEAT_SSE4    0x001u
#define RAF_FEAT_AVX2    0x002u
#define RAF_FEAT_AVX512  0x004u
#define RAF_FEAT_NEON    0x008u
#define RAF_FEAT_SVE     0x010u
#define RAF_FEAT_I8MM    0x020u
#define RAF_FEAT_SME     0x040u
#define RAF_FEAT_GPU_VK  0x080u
#define RAF_FEAT_GPU_CL  0x100u
#define RAF_FEAT_DSP_HXN 0x200u
#define RAF_FEAT_NPU     0x400u
/* Presence-only probes. These bits do not authorize compute dispatch. */
#define RAF_FEAT_GPU_NODE 0x0800u
#define RAF_FEAT_DSP_NODE 0x1000u
#define RAF_FEAT_NPU_NODE 0x2000u

#define RAF_IR_CAP (1u << 16)
#define RAF_ASM_CAP (1u << 15)
#define RAF_ASM_LINE 128
#define RAF_HEX_CAP (1u << 20)
/* RAF_SOURCE_MAX is the exact accepted payload. CAP includes one NUL byte. */
#define RAF_SOURCE_MAX (1u << 20)
#define RAF_SOURCE_CAP (RAF_SOURCE_MAX + 1u)

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
  char src_storage[RAF_SOURCE_CAP];
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
  uint8_t native_requested;
  uint8_t native_written;
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

/* Language × architecture routing matrix.
 * A value of 1 means that a hosted or direct route exists; it does not prove
 * that the strict final binary is freestanding. M063 applies the final-runtime
 * policy separately in ApkC/lang_freestanding_policy.h. */
static const uint8_t RAF_CAP_MATRIX[RAF_LANG_COUNT][5] = {
 /* lang             x86_64 arm64 arm32 rv64 unk */
 /* C       fork */   { 0, 1, 0, 0, 0 },
 /* CPP     fork */   { 0, 1, 0, 0, 0 },
 /* ASM     asm  */   { 1, 1, 1, 0, 0 },
 /* PY      scr  */   { 1, 1, 1, 0, 1 },
 /* RS      fork */   { 0, 1, 0, 0, 0 },
 /* KT      fork */   { 0, 1, 0, 0, 0 },
 /* JAVA    fork */   { 0, 1, 0, 0, 0 },
 /* SH      scr  */   { 1, 1, 1, 0, 1 },
 /* PL      scr  */   { 1, 1, 1, 0, 1 },
 /* JS      scr  */   { 1, 1, 1, 0, 1 },
 /* PHP     scr  */   { 1, 1, 1, 0, 1 },
 /* JSX     fork */   { 0, 1, 0, 0, 0 },
 /* GLSL    fork */   { 0, 1, 0, 0, 0 },
 /* CL      fork */   { 0, 1, 0, 0, 0 },
 /* HLSL    fork */   { 0, 1, 0, 0, 0 },
 /* WGSL    fork */   { 0, 1, 0, 0, 0 },
 /* DSP     fork */   { 0, 1, 0, 0, 0 },
 /* TFLITE  fork */   { 0, 1, 0, 0, 0 },
 /* GO      fork */   { 0, 1, 0, 0, 0 },
 /* RB      scr  */   { 1, 1, 1, 0, 1 },
 /* SWIFT   fork */   { 0, 1, 0, 0, 0 },
 /* GROOVY  fork */   { 0, 1, 0, 0, 0 },
 /* CLJ     fork */   { 0, 1, 0, 0, 0 },
 /* UNKNOWN      */   { 0, 0, 0, 0, 0 }
};

static inline const char *raf_lang_to_apkc_name(uint8_t lang) {
  switch (lang) {
    case RAF_LANG_C: return "c";
    case RAF_LANG_CPP: return "cpp";
    case RAF_LANG_S: return "asm";
    case RAF_LANG_PY: return "py";
    case RAF_LANG_RS: return "rs";
    case RAF_LANG_KT: return "kt";
    case RAF_LANG_JAVA: return "java";
    case RAF_LANG_SH: return "sh";
    case RAF_LANG_PL: return "pl";
    case RAF_LANG_JS: return "js";
    case RAF_LANG_PHP: return "php";
    case RAF_LANG_JSX: return "jsx";
    case RAF_LANG_GLSL: return "glsl";
    case RAF_LANG_CL: return "cl";
    case RAF_LANG_HLSL: return "hlsl";
    case RAF_LANG_WGSL: return "wgsl";
    case RAF_LANG_DSP: return "dsp";
    case RAF_LANG_TFLITE: return "tflite";
    case RAF_LANG_GO: return "go";
    case RAF_LANG_RB: return "rb";
    case RAF_LANG_SWIFT: return "swift";
    case RAF_LANG_GROOVY: return "groovy";
    case RAF_LANG_CLJ: return "clj";
    default: return "unknown";
  }
}

#endif
