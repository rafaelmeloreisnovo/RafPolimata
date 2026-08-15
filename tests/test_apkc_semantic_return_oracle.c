#include <stdio.h>
#include "../Apkc/apkc_branchless_handler.h"

static int pass_count, fail_count;
static struct BranchlessHandler h;

#define CHECK(name, cond) do { \
    if (cond) { pass_count++; printf("PASS: %s\n", name); } \
    else { fail_count++; printf("FAIL: %s\n", name); } \
} while (0)

static u32 cstrlen(const char *s) {
    u32 n = 0;
    while (s[n]) n++;
    return n;
}

static void expect_result(const char *src, u64 expected) {
    u8 rc = apkc_branchless_compile(&h, (const u8*)src, cstrlen(src), LP_RS);
    CHECK("compile", rc == 0);
    CHECK("bounded scope", h.frontend_kind == APKC_FRONTEND_BOUNDED_SEMANTIC_SUBSET &&
                           h.semantic_scope == APKC_SEMANTIC_SCOPE_RETURN_ARITHMETIC_FRAGMENT);
    CHECK("vm observed", h.vm_action_observed == APKC_VM_ACTION_OBSERVED);
    CHECK("expected result", h.vm_result == expected);
    CHECK("steps", h.steps_executed > 0);
    CHECK("ret terminator", h.code_len > 0 && h.code[h.code_len - 1].op == OP_RET);
    CHECK("arm64 encoded", h.arm64_len > 0);
    CHECK("no self promotion", h.semantic_proof == APKC_SEMANTIC_UNPROVEN);
}

int main(void) {
    const u8 normal[] = "x = 1 + 2";
    const u8 bad[] = "return 2 +";

    expect_result("return 42", 42u);
    expect_result("return 2 + 3 * 4", 14u);
    expect_result("return (2 + 3) * 4;", 20u);

    CHECK("invalid return fails closed",
          apkc_branchless_compile(&h, bad, (u32)(sizeof(bad) - 1), LP_RS) != 0 &&
          h.status == APKC_BRANCHLESS_COMPILE_ERROR &&
          h.vm_action_observed == APKC_VM_ACTION_NOT_RUN);

    CHECK("normal structural path stays unproved",
          apkc_branchless_compile(&h, normal, (u32)(sizeof(normal) - 1), LP_PY) == 0 &&
          h.frontend_kind == APKC_FRONTEND_LANGUAGE_SPECIFIC &&
          h.semantic_scope == APKC_SEMANTIC_SCOPE_NONE &&
          h.vm_action_observed == APKC_VM_ACTION_NOT_RUN &&
          h.semantic_proof == APKC_SEMANTIC_UNPROVEN);

    printf("SUMMARY pass=%d fail=%d\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
