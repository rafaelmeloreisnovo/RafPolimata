#define _POSIX_C_SOURCE 200809L

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

static uint64_t raf_now_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
  return ((uint64_t)ts.tv_sec * UINT64_C(1000000000)) + (uint64_t)ts.tv_nsec;
}

static uint64_t raf_hash_update(uint64_t h, const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    h ^= (uint64_t)buf[i];
    h *= UINT64_C(1099511628211);
  }
  return h;
}

static uint64_t raf_fnv1a64(const uint8_t *buf, size_t len) {
  return raf_hash_update(UINT64_C(1469598103934665603), buf, len);
}

static uint64_t raf_hash_u64(uint64_t h, uint64_t v) {
  uint8_t b[8];
  for (uint32_t i = 0; i < 8u; ++i) b[i] = (uint8_t)(v >> (i * 8u));
  return raf_hash_update(h, b, sizeof(b));
}

static uint64_t raf_ops_signature(const RafCtx *ctx) {
  uint64_t h = UINT64_C(1469598103934665603);
  h = raf_hash_u64(h, ctx->cpu.arch);
  h = raf_hash_update(h, (const uint8_t *)ctx->cpu.brand, strlen(ctx->cpu.brand));
  h = raf_hash_u64(h, ctx->lang);
  h = raf_hash_u64(h, ctx->opt);
  h = raf_hash_u64(h, ctx->feat);
  h = raf_hash_update(h, (const uint8_t *)ctx->flags, strlen(ctx->flags));
  h = raf_hash_u64(h, (uint64_t)ctx->src_len);
  h = raf_hash_u64(h, ctx->src_hash);
  h = raf_hash_u64(h, ctx->ir.n);
  h = raf_hash_u64(h, ctx->asm_out.n);
  h = raf_hash_u64(h, ctx->bin.n);
  h = raf_hash_u64(h, (uint64_t)(int64_t)ctx->rollback_code);
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

static void prepare_manifest(RafCtx *ctx, const char *out_base) {
  snprintf(ctx->out_asm, sizeof(ctx->out_asm), "%s.s", out_base);
  snprintf(ctx->out_hex, sizeof(ctx->out_hex), "%s.hex", out_base);
  snprintf(ctx->out_ops, sizeof(ctx->out_ops), "%s.ops", out_base);
}

static int write_ops_manifest(RafCtx *ctx) {
  ctx->ops_signature = raf_ops_signature(ctx);
  FILE *fo = fopen(ctx->out_ops, "w");
  if (!fo) return -12;
  fprintf(fo, "ops_schema=1\narch=%u\nbrand=%s\nlang=%u\nopt=%u\nfeat=0x%08x\nflags=%s\nsrc_len=%zu\nsrc_hash=%016llx\nir=%u\nasm=%u\nbin=%u\nelapsed_ns=%llu\nrollback_code=%d\nops_signature=%016llx\n",
          ctx->cpu.arch, ctx->cpu.brand, ctx->lang, ctx->opt, ctx->feat,
          ctx->flags, ctx->src_len, (unsigned long long)ctx->src_hash,
          ctx->ir.n, ctx->asm_out.n, ctx->bin.n,
          (unsigned long long)ctx->elapsed_ns, ctx->rollback_code,
          (unsigned long long)ctx->ops_signature);
  fclose(fo);
  return 0;
}

static int write_artifacts(RafCtx *ctx) {
  FILE *fa = fopen(ctx->out_asm, "w");
  if (!fa) { ctx->rollback_code = -10; (void)write_ops_manifest(ctx); return -10; }
  for (uint32_t i = 0; i < ctx->asm_out.n; ++i) {
    fprintf(fa, "%s\n", ctx->asm_out.lines[i]);
  }
  fclose(fa);

  FILE *fh = fopen(ctx->out_hex, "w");
  if (!fh) { ctx->rollback_code = -11; (void)write_ops_manifest(ctx); return -11; }
  for (uint32_t i = 0; i < ctx->bin.n; ++i) {
    fprintf(fh, "%02X%s", ctx->bin.bytes[i], ((i + 1u) % 16u) ? " " : "\n");
  }
  fclose(fh);
  return write_ops_manifest(ctx);
}

static int fail_with_manifest(RafCtx *ctx, int code, uint64_t t0) {
  uint64_t t1 = raf_now_ns();
  ctx->rollback_code = code;
  ctx->elapsed_ns = t1 >= t0 ? t1 - t0 : 0;
  (void)write_ops_manifest(ctx);
  return code;
}

int raf_compile_file(RafCtx *ctx, const char *src_path, const char *out_base,
                     int do_native) {
  (void)do_native;
  prepare_manifest(ctx, out_base);
  uint64_t t0 = raf_now_ns();
  ctx->lang = raf_lang_from_ext(src_path);
  ctx->feat = ctx->cpu.feat;
  raf_flag_matrix_get(ctx->cpu.arch, ctx->lang, ctx->opt, ctx->feat, ctx->flags,
                      (int)sizeof(ctx->flags));
  if (read_src(ctx, src_path) != 0) return fail_with_manifest(ctx, -1, t0);
  ctx->src_hash = raf_fnv1a64((const uint8_t *)ctx->src, ctx->src_len);
  if (raf_ir_lower(ctx) != 0) return fail_with_manifest(ctx, -2, t0);
  if (raf_asm_emit(ctx) != 0) return fail_with_manifest(ctx, -3, t0);
  if (raf_hex_encode(ctx) != 0) return fail_with_manifest(ctx, -4, t0);

  uint64_t t1 = raf_now_ns();
  ctx->elapsed_ns = t1 >= t0 ? t1 - t0 : 0;
  ctx->rollback_code = 0;
  return write_artifacts(ctx);
}
