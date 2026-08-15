/* apkc_language_dispatch.h — canonical LangProfile -> branchless frontend routing
 *
 * Invariant: branchless language identity is the LP_* index from lang_profile.h.
 * No second/private numbering is permitted at this boundary.
 *
 * Cycle 2 adds a strictly bounded common RETURN-arithmetic statement fragment
 * before the wider language-specific structural frontends.  Its scope is
 * recorded explicitly and must not be generalized to full language semantics.
 *
 * FREESTANDING: no malloc, no libc, stack-only temporary compiler state.
 */
#ifndef APKC_LANGUAGE_DISPATCH_H
#define APKC_LANGUAGE_DISPATCH_H 1

#include "compiler_language_direct.h"
#include "lang_profile.h"
#include "apkc_semantic_return_subset.h"

#define APKC_FRONTEND_NONE                     0u
#define APKC_FRONTEND_LANGUAGE_SPECIFIC        1u
#define APKC_FRONTEND_BOUNDED_SEMANTIC_SUBSET  2u

/* Semantic proof is deliberately separate from frontend selection.
 * VM execution and an external expected-result assertion are independent.
 */
#define APKC_SEMANTIC_UNPROVEN 0u
#define APKC_SEMANTIC_PROVEN   1u

static inline u8 apkc_branchless_lang_supported(u8 lang_type) {
    if (lang_type >= LP_COUNT) return 0;
    if (!lang_profile_table_validate()) return 0;
    return _lang_table[lang_type].use_branchless ? 1u : 0u;
}

/* Scoped route. semantic_scope_out is always initialized when provided.
 *
 * Return 0 only when one bounded semantic subset or the selected structural
 * frontend accepted the source and emitted machine instructions.
 */
static inline u8 apkc_compile_language_direct_scoped(
    struct UniversalCompiler *uc,
    const u8 *src, u32 src_len,
    u8 lang_type,
    u8 *frontend_kind_out,
    u8 *semantic_scope_out)
{
    u8 subset;

    if (frontend_kind_out) *frontend_kind_out = APKC_FRONTEND_NONE;
    if (semantic_scope_out) *semantic_scope_out = APKC_SEMANTIC_SCOPE_NONE;

    if (!uc || !src || src_len == 0) return 1;
    if (!apkc_branchless_lang_supported(lang_type)) return 1;

    uc->lang = lang_type;

    /* The common subset is entered only after canonical LP_* validation.
     * If the input begins with `return` but violates the subset grammar, fail
     * closed instead of falling through to a permissive structural parser.
     */
    subset = apkc_compile_return_arithmetic_subset(&uc->cg, src, src_len);
    if (subset == APKC_RETURN_SUBSET_OK) {
        if (frontend_kind_out) *frontend_kind_out = APKC_FRONTEND_BOUNDED_SEMANTIC_SUBSET;
        if (semantic_scope_out)
            *semantic_scope_out = APKC_SEMANTIC_SCOPE_RETURN_ARITHMETIC_FRAGMENT;
        return 0;
    }
    if (subset == APKC_RETURN_SUBSET_INVALID) return 1;

    {
        u32 start_pos = uc->cg.pos;

        switch (lang_type) {
        case LP_PY: {
            struct PythonCompiler py;
            py.cg = uc->cg;
            scanner_init(&py.sc, src, src_len, LP_PY);
            if (compile_python_assign(&py)) return 1;
            uc->cg = py.cg;
            break;
        }
        case LP_GO: {
            struct GoCompiler go;
            go.cg = uc->cg;
            scanner_init(&go.sc, src, src_len, LP_GO);
            if (compile_go_func(&go)) return 1;
            uc->cg = go.cg;
            break;
        }
        case LP_RS: {
            struct RustCompiler rs;
            rs.cg = uc->cg;
            scanner_init(&rs.sc, src, src_len, LP_RS);
            if (compile_rust_fn(&rs)) return 1;
            uc->cg = rs.cg;
            break;
        }
        case LP_C: {
            struct CCompiler c;
            c.cg = uc->cg;
            scanner_init(&c.sc, src, src_len, LP_C);
            if (compile_c_fn(&c)) return 1;
            uc->cg = c.cg;
            break;
        }
        case LP_JS: {
            struct JsCompiler js;
            js.cg = uc->cg;
            scanner_init(&js.sc, src, src_len, LP_JS);
            if (compile_js_arrow(&js)) return 1;
            uc->cg = js.cg;
            break;
        }
        case LP_JAVA: {
            struct JavaCompiler jc;
            jc.cg = uc->cg;
            scanner_init(&jc.sc, src, src_len, LP_JAVA);
            if (compile_java_method(&jc)) return 1;
            uc->cg = jc.cg;
            break;
        }
        case LP_SWIFT: {
            struct SwiftCompiler sw;
            sw.cg = uc->cg;
            scanner_init(&sw.sc, src, src_len, LP_SWIFT);
            if (compile_swift_fn(&sw)) return 1;
            uc->cg = sw.cg;
            break;
        }
        default:
            return 1;
        }

        if (uc->cg.pos <= start_pos) return 1;
    }

    if (frontend_kind_out) *frontend_kind_out = APKC_FRONTEND_LANGUAGE_SPECIFIC;
    return 0;
}

/* Compatibility wrapper for callers that do not consume scope metadata. */
static inline u8 apkc_compile_language_direct(
    struct UniversalCompiler *uc,
    const u8 *src, u32 src_len,
    u8 lang_type)
{
    u8 frontend_kind;
    u8 semantic_scope;
    return apkc_compile_language_direct_scoped(
        uc, src, src_len, lang_type, &frontend_kind, &semantic_scope);
}

#endif /* APKC_LANGUAGE_DISPATCH_H */
