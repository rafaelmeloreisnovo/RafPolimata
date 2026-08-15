/* apkc_language_dispatch.h — canonical LangProfile -> branchless frontend routing
 *
 * Invariant: branchless language identity is the LP_* index from lang_profile.h.
 * No second/private numbering is permitted at this boundary.
 *
 * This dispatcher wires each currently-enabled branchless LangProfile to its
 * language-specific scanner/statement frontend.  Those frontends are still
 * structurally incomplete for several semantic constructs; this file does not
 * promote structural parsing to semantic proof.
 *
 * FREESTANDING: no malloc, no libc, stack-only temporary compiler state.
 */
#ifndef APKC_LANGUAGE_DISPATCH_H
#define APKC_LANGUAGE_DISPATCH_H 1

#include "compiler_language_direct.h"
#include "lang_profile.h"

#define APKC_FRONTEND_NONE              0u
#define APKC_FRONTEND_LANGUAGE_SPECIFIC 1u

/* Semantic proof is deliberately separate from frontend selection.
 * Current statement/expression frontends still contain structural-only paths.
 */
#define APKC_SEMANTIC_UNPROVEN 0u
#define APKC_SEMANTIC_PROVEN   1u

static inline u8 apkc_branchless_lang_supported(u8 lang_type) {
    if (lang_type >= LP_COUNT) return 0;
    if (!lang_profile_table_validate()) return 0;
    return _lang_table[lang_type].use_branchless ? 1u : 0u;
}

/* Route one canonical LP_* language id into its specific frontend.
 *
 * Return 0 only when the selected frontend accepted the source and emitted at
 * least one machine instruction.  This is a FRONTEND/STRUCTURAL acceptance
 * result, not semantic equivalence proof.
 */
static inline u8 apkc_compile_language_direct(
    struct UniversalCompiler *uc,
    const u8 *src, u32 src_len,
    u8 lang_type)
{
    if (!uc || !src || src_len == 0) return 1;
    if (!apkc_branchless_lang_supported(lang_type)) return 1;

    u32 start_pos = uc->cg.pos;
    uc->lang = lang_type;

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
    return 0;
}

#endif /* APKC_LANGUAGE_DISPATCH_H */
