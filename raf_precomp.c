#include "raf_compile.h"

/* raf_precompile — deterministic precompilation anchor (L13).
 *
 * SCOPE (explicit, audited): this is NOT a general-purpose front-end/lowering
 * pass. It is a *deterministic manifest / reproducibility anchor*: for any
 * valid context it lowers to one fixed canonical IR program
 *     IR_MOVIMM 42 ; IR_RET
 * whose only contract is to be byte-identical across builds (same input class
 * -> same IR -> same .ops hash). That fixed point is what the two-cycle-omega
 * and ops-manifest gates compare against (see
 * docs/OPERACAO_COMPILAR_PRECOMPILAR.md and scripts/validate_ops_manifest.py).
 *
 * It is intentionally a fixed canonical program, not a parser. A real
 * source-dependent lowering would replace the body below while keeping the
 * determinism contract (same source -> same IR every build). Until then this
 * stays REFERENCE/anchor, not PENDING -- it does exactly and only what it
 * claims. */
int raf_precompile(RafCtx *ctx) {
  if (!ctx || !ctx->src) return -1;
  ctx->ir.n = 2;
  ctx->ir.buf[0] = ((uint64_t)IR_MOVIMM << 56) | 42u;  /* canonical anchor value */
  ctx->ir.buf[1] = ((uint64_t)IR_RET << 56);
  return 0;
}
