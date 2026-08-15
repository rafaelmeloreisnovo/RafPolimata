/* Canonical language identity + branchless dispatcher gate.
 *
 * This test intentionally proves only G-S1 plus active dispatcher selection.
 * It does NOT claim semantic equivalence of the current statement/expression
 * frontends; semantic_proof must remain APKC_SEMANTIC_UNPROVEN.
 */
#include <stdio.h>

#include "../Apkc/apkc_branchless_handler.h"

static int pass_count = 0;
static int fail_count = 0;

#define CHECK(name, cond) do { \
    if (cond) { pass_count++; printf("PASS: %s\n", name); } \
    else { fail_count++; printf("FAIL: %s\n", name); } \
} while (0)

static void test_canonical_identity(void) {
    static struct BranchlessHandler h;
    const u8 src[] = "x = 5 + 3";

    CHECK("LP_PY is not legacy private id zero", LP_PY != 0);
    CHECK("python profile is branchless-enabled", apkc_branchless_lang_supported(LP_PY) == 1);
    CHECK("C profile is branchless-enabled", apkc_branchless_lang_supported(LP_C) == 1);
    CHECK("C++ profile is not branchless-enabled", apkc_branchless_lang_supported(LP_CPP) == 0);

    CHECK("canonical LP_PY compile accepted",
          apkc_branchless_compile(&h, src, (u32)(sizeof(src) - 1), LP_PY) == 0);
    CHECK("handler preserved exact LP_PY id", h.lang_profile_id == LP_PY);
    CHECK("language-specific frontend selected",
          h.frontend_kind == APKC_FRONTEND_LANGUAGE_SPECIFIC);
    CHECK("semantic proof remains unpromoted",
          h.semantic_proof == APKC_SEMANTIC_UNPROVEN);
    CHECK("machine instructions emitted", h.code_len > 0);
    CHECK("exact code stream copied into execution oracle",
          h.ctx.code_len == h.code_len && h.ctx.code[0].op == h.code[0].op);
}

static void test_fail_closed_language_ids(void) {
    static struct BranchlessHandler h;
    const u8 src[] = "x = 1";

    CHECK("legacy id zero rejected instead of reinterpreted as Python",
          apkc_branchless_compile(&h, src, (u32)(sizeof(src) - 1), 0) != 0 &&
          h.status == APKC_BRANCHLESS_UNSUPPORTED_LANG);

    CHECK("LP_CPP rejected because profile does not enable branchless",
          apkc_branchless_compile(&h, src, (u32)(sizeof(src) - 1), LP_CPP) != 0 &&
          h.status == APKC_BRANCHLESS_UNSUPPORTED_LANG);

    CHECK("out-of-range language id rejected",
          apkc_branchless_compile(&h, src, (u32)(sizeof(src) - 1), 255) != 0 &&
          h.status == APKC_BRANCHLESS_UNSUPPORTED_LANG);

    CHECK("empty source rejected fail-closed",
          apkc_branchless_compile(&h, src, 0, LP_PY) != 0 &&
          h.status == APKC_BRANCHLESS_COMPILE_ERROR);
}

static void test_all_enabled_profiles_route(void) {
    static struct BranchlessHandler h;
    const u8 src[] = "x = 1 + 2";
    const u8 langs[] = {LP_C, LP_RS, LP_JAVA, LP_PY, LP_JS, LP_GO, LP_SWIFT};
    u32 i;

    for (i = 0; i < (u32)(sizeof(langs) / sizeof(langs[0])); i++) {
        u8 lang = langs[i];
        u8 rc = apkc_branchless_compile(&h, src, (u32)(sizeof(src) - 1), lang);
        CHECK("enabled profile routes through canonical dispatcher",
              rc == 0 && h.lang_profile_id == lang &&
              h.frontend_kind == APKC_FRONTEND_LANGUAGE_SPECIFIC);
        CHECK("enabled profile does not self-promote semantic proof",
              h.semantic_proof == APKC_SEMANTIC_UNPROVEN && h.steps_executed == 0);
    }
}

int main(void) {
    printf("=== APKc Canonical Language Dispatch Gate ===\n");
    test_canonical_identity();
    test_fail_closed_language_ids();
    test_all_enabled_profiles_route();
    printf("SUMMARY pass=%d fail=%d\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
