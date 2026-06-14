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

#define RAF_OPT_0 0
#define RAF_OPT_1 1
#define RAF_OPT_2 2
#define RAF_OPT_3 3
#define RAF_OPT_S 4

#define RAF_FEAT_SSE4 0x01u
#define RAF_FEAT_AVX2 0x02u
#define RAF_FEAT_AVX512 0x04u
#define RAF_FEAT_NEON 0x08u

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

#endif
