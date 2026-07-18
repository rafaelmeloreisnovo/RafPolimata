#define _POSIX_C_SOURCE 200809L

#include "raf_compile.h"
#include "Apkc/omega_classifier.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* Returns 0 on success, -1 on I/O failure and -2 when the source exceeds the
 * bounded no-heap storage. Oversized input is rejected instead of truncated. */
static int read_src(RafCtx *ctx, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return -1;

  size_t n = fread(ctx->src_storage, 1, RAF_SOURCE_CAP - 1u, f);
  if (ferror(f)) {
    fclose(f);
    return -1;
  }

  int extra = fgetc(f);
  if (extra != EOF) {
    fclose(f);
    return -2;
  }

  fclose(f);
  ctx->src_storage[n] = '\0';
  ctx->src = ctx->src_storage;
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

/* Canonical FNV-1a 64-bit offset basis. */
static uint64_t raf_fnv1a64(const uint8_t *buf, size_t len) {
  return raf_hash_update(UINT64_C(14695981039346656037), buf, len);
}

static uint64_t raf_hash_u64(uint64_t h, uint64_t v) {
  uint8_t b[8];
  for (uint32_t i = 0; i < 8u; ++i) b[i] = (uint8_t)(v >> (i * 8u));
  return raf_hash_update(h, b, sizeof(b));
}

static uint64_t raf_ops_signature(const RafCtx *ctx) {
  uint64_t h = UINT64_C(14695981039346656037);
  h = raf_hash_u64(h, ctx->cpu.arch);
  h = raf_hash_update(h, (const uint8_t *)ctx->cpu.brand, strlen(ctx->cpu.brand));
  h = raf_hash_u64(h, ctx->cpu.cores);
  h = raf_hash_u64(h, ctx->lang);
  h = raf_hash_u64(h, ctx->opt);
  h = raf_hash_u64(h, ctx->feat);
  h = raf_hash_update(h, (const uint8_t *)ctx->flags, strlen(ctx->flags));
  h = raf_hash_u64(h, (uint64_t)ctx->src_len);
  h = raf_hash_u64(h, ctx->src_hash);
  h = raf_hash_u64(h, ctx->omega_entropy_milli);
  h = raf_hash_u64(h, ctx->omega_phi_q16);
  h = raf_hash_u64(h, ctx->omega_attractor);
  h = raf_hash_u64(h, ctx->omega_flags);
  h = raf_hash_u64(h, ctx->omega_path);
  h = raf_hash_u64(h, ctx->ir.n);
  h = raf_hash_u64(h, ctx->asm_out.n);
  h = raf_hash_u64(h, ctx->bin.n);
  h = raf_hash_u64(h, ctx->native_requested);
  h = raf_hash_u64(h, ctx->native_written);
  h = raf_hash_u64(h, (uint64_t)(int64_t)ctx->rollback_code);
  return h;
}

int raf_ir_lower(RafCtx *ctx) { return raf_precompile(ctx); }

void raf_ctx_init(RafCtx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->opt = RAF_OPT_2;
  ctx->lang = RAF_LANG_UNKNOWN;
  ctx->omega_path = (uint8_t)RAF_OMEGA_VOID;
  raf_cpu_detect(&ctx->cpu);
  ctx->feat = ctx->cpu.feat;
}

void raf_ctx_report(const RafCtx *ctx) {
  printf("[raf] arch=%u brand=%s cores=%u ir=%u asm=%u bin=%u native=%u/%u "
         "src_hash=%016llx omega=%s/%u phi=%u entropy=%u flags=%s\n",
         ctx->cpu.arch, ctx->cpu.brand, ctx->cpu.cores, ctx->ir.n,
         ctx->asm_out.n, ctx->bin.n, ctx->native_requested,
         ctx->native_written, (unsigned long long)ctx->src_hash,
         raf_omega_path_name((RafOmegaPath)ctx->omega_path),
         ctx->omega_attractor, ctx->omega_phi_q16,
         ctx->omega_entropy_milli, ctx->flags);
}

static void prepare_manifest(RafCtx *ctx, const char *out_base) {
  snprintf(ctx->out_asm, sizeof(ctx->out_asm), "%s.s", out_base);
  snprintf(ctx->out_hex, sizeof(ctx->out_hex), "%s.hex", out_base);
  snprintf(ctx->out_bin, sizeof(ctx->out_bin), "%s.bin", out_base);
  snprintf(ctx->out_ops, sizeof(ctx->out_ops), "%s.ops", out_base);
}

static int write_ops_manifest(RafCtx *ctx) {
  ctx->ops_signature = raf_ops_signature(ctx);
  FILE *fo = fopen(ctx->out_ops, "w");
  if (!fo) return -12;
  fprintf(fo,
          "ops_schema=3\n"
          "arch=%u\n"
          "brand=%s\n"
          "cores=%u\n"
          "lang=%u\n"
          "opt=%u\n"
          "feat=0x%08x\n"
          "flags=%s\n"
          "src_len=%zu\n"
          "src_hash=%016llx\n"
          "omega_entropy_milli=%u\n"
          "omega_phi_q16=%u\n"
          "omega_attractor=%u\n"
          "omega_flags=0x%08x\n"
          "omega_path=%u\n"
          "omega_path_name=%s\n"
          "ir=%u\n"
          "asm=%u\n"
          "bin=%u\n"
          "native_requested=%u\n"
          "native_written=%u\n"
          "elapsed_ns=%llu\n"
          "rollback_code=%d\n"
          "ops_signature=%016llx\n",
          ctx->cpu.arch, ctx->cpu.brand, ctx->cpu.cores, ctx->lang, ctx->opt,
          ctx->feat, ctx->flags, ctx->src_len,
          (unsigned long long)ctx->src_hash,
          ctx->omega_entropy_milli, ctx->omega_phi_q16,
          ctx->omega_attractor, ctx->omega_flags, ctx->omega_path,
          raf_omega_path_name((RafOmegaPath)ctx->omega_path),
          ctx->ir.n, ctx->asm_out.n, ctx->bin.n,
          ctx->native_requested, ctx->native_written,
          (unsigned long long)ctx->elapsed_ns, ctx->rollback_code,
          (unsigned long long)ctx->ops_signature);
  fclose(fo);
  return 0;
}

static int write_artifacts(RafCtx *ctx) {
  FILE *fa = fopen(ctx->out_asm, "w");
  if (!fa) {
    ctx->rollback_code = -10;
    (void)write_ops_manifest(ctx);
    return -10;
  }
  for (uint32_t i = 0; i < ctx->asm_out.n; ++i) {
    fprintf(fa, "%s\n", ctx->asm_out.lines[i]);
  }
  fclose(fa);

  FILE *fh = fopen(ctx->out_hex, "w");
  if (!fh) {
    ctx->rollback_code = -11;
    (void)write_ops_manifest(ctx);
    return -11;
  }
  for (uint32_t i = 0; i < ctx->bin.n; ++i) {
    fprintf(fh, "%02X%s", ctx->bin.bytes[i], ((i + 1u) % 16u) ? " " : "\n");
  }
  fclose(fh);

  if (ctx->native_requested) {
    FILE *fb = fopen(ctx->out_bin, "wb");
    if (!fb) {
      ctx->rollback_code = -13;
      (void)write_ops_manifest(ctx);
      return -13;
    }
    size_t written = fwrite(ctx->bin.bytes, 1, ctx->bin.n, fb);
    if (fclose(fb) != 0 || written != ctx->bin.n) {
      ctx->rollback_code = -13;
      (void)write_ops_manifest(ctx);
      return -13;
    }
    ctx->native_written = 1u;
  }

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
  prepare_manifest(ctx, out_base);
  uint64_t t0 = raf_now_ns();
  ctx->native_requested = do_native ? 1u : 0u;
  ctx->native_written = 0u;
  ctx->lang = raf_lang_from_ext(src_path);
  if (ctx->lang == RAF_LANG_UNKNOWN) return fail_with_manifest(ctx, -6, t0);

  ctx->feat = ctx->cpu.feat;
  raf_flag_matrix_get(ctx->cpu.arch, ctx->lang, ctx->opt, ctx->feat, ctx->flags,
                      (int)sizeof(ctx->flags));

  int read_rc = read_src(ctx, src_path);
  if (read_rc == -2) return fail_with_manifest(ctx, -5, t0);
  if (read_rc != 0) return fail_with_manifest(ctx, -1, t0);
  ctx->src_hash = raf_fnv1a64((const uint8_t *)ctx->src, ctx->src_len);

  {
    RafOmegaMetrics omega = raf_omega_classify(
        (const raf_omega_u8 *)ctx->src, (raf_omega_u32)ctx->src_len);
    ctx->omega_entropy_milli = omega.entropy_milli;
    ctx->omega_phi_q16 = omega.phi_q16;
    ctx->omega_attractor = omega.attractor;
    ctx->omega_flags = omega.flags;
    ctx->omega_path = (uint8_t)omega.path;
  }

  if (raf_ir_lower(ctx) != 0) return fail_with_manifest(ctx, -2, t0);
  if (raf_asm_emit(ctx) != 0) return fail_with_manifest(ctx, -3, t0);
  if (raf_hex_encode(ctx) != 0) return fail_with_manifest(ctx, -4, t0);

  uint64_t t1 = raf_now_ns();
  ctx->elapsed_ns = t1 >= t0 ? t1 - t0 : 0;
  ctx->rollback_code = 0;
  return write_artifacts(ctx);
}
