#include "raf_compile.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static int read_src(RafCtx *ctx, const char *path) {
  static char buf[1024 * 1024];
  FILE *f = fopen(path, "rb");
  if (!f) return -1;
  size_t n = fread(buf, 1, sizeof(buf) - 1, f);
  fclose(f);
  buf[n] = '\0';
  ctx->src = buf;
  ctx->src_len = n;
  return 0;
}

static uint64_t raf_fnv1a64(const uint8_t *buf, size_t len) {
  uint64_t h = UINT64_C(1469598103934665603);
  for (size_t i = 0; i < len; ++i) {
    h ^= (uint64_t)buf[i];
    h *= UINT64_C(1099511628211);
  }
  return h;
}

int raf_ir_lower(RafCtx *ctx) { return raf_precompile(ctx); }

void raf_ctx_init(RafCtx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->opt = RAF_OPT_2;
  raf_cpu_detect(&ctx->cpu);
  ctx->feat = ctx->cpu.feat;
}

void raf_ctx_report(const RafCtx *ctx) {
  printf("[raf] arch=%u brand=%s ir=%u asm=%u bin=%u src_hash=%016llx flags=%s\n",
         ctx->cpu.arch, ctx->cpu.brand, ctx->ir.n, ctx->asm_out.n, ctx->bin.n,
         (unsigned long long)ctx->src_hash, ctx->flags);
}

static int write_outputs(RafCtx *ctx, const char *out_base) {
  snprintf(ctx->out_asm, sizeof(ctx->out_asm), "%s.s", out_base);
  snprintf(ctx->out_hex, sizeof(ctx->out_hex), "%s.hex", out_base);
  snprintf(ctx->out_ops, sizeof(ctx->out_ops), "%s.ops", out_base);

  FILE *fa = fopen(ctx->out_asm, "w");
  if (!fa) return -10;
  for (uint32_t i = 0; i < ctx->asm_out.n; ++i) fprintf(fa, "%s\n", ctx->asm_out.lines[i]);
  fclose(fa);

  FILE *fh = fopen(ctx->out_hex, "w");
  if (!fh) return -11;
  for (uint32_t i = 0; i < ctx->bin.n; ++i) fprintf(fh, "%02X%s", ctx->bin.bytes[i], ((i + 1u) % 16u) ? " " : "\n");
  fclose(fh);

  FILE *fo = fopen(ctx->out_ops, "w");
  if (!fo) return -12;
  fprintf(fo, "arch=%u\nbrand=%s\nlang=%u\nopt=%u\nfeat=0x%08x\nflags=%s\nsrc_len=%zu\nsrc_hash=%016llx\nir=%u\nasm=%u\nbin=%u\nelapsed_ns=%llu\nrollback_code=%d\n",
          ctx->cpu.arch, ctx->cpu.brand, ctx->lang, ctx->opt, ctx->feat,
          ctx->flags, ctx->src_len, (unsigned long long)ctx->src_hash,
          ctx->ir.n, ctx->asm_out.n, ctx->bin.n,
          (unsigned long long)ctx->elapsed_ns, ctx->rollback_code);
  fclose(fo);
  return 0;
}

int raf_compile_file(RafCtx *ctx, const char *src_path, const char *out_base,
                     int do_native) {
  (void)do_native;
  if (read_src(ctx, src_path) != 0) return -1;
  clock_t t0 = clock();
  ctx->lang = raf_lang_from_ext(src_path);
  ctx->feat = ctx->cpu.feat;
  ctx->src_hash = raf_fnv1a64((const uint8_t *)ctx->src, ctx->src_len);
  raf_flag_matrix_get(ctx->cpu.arch, ctx->lang, ctx->opt, ctx->feat, ctx->flags,
                      (int)sizeof(ctx->flags));
  if (raf_ir_lower(ctx) != 0) { ctx->rollback_code = -2; return -2; }
  if (raf_asm_emit(ctx) != 0) { ctx->rollback_code = -3; return -3; }
  if (raf_hex_encode(ctx) != 0) { ctx->rollback_code = -4; return -4; }

  ctx->elapsed_ns = (uint64_t)((clock() - t0) * (1000000000.0 / CLOCKS_PER_SEC));
  ctx->rollback_code = 0;
  return write_outputs(ctx, out_base);
}
