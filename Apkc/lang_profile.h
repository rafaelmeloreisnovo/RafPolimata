/* lang_profile.h — declarative language dispatch table for APKc.
 * Technological determinism: a recognized source extension or explicit -lang
 * selects exactly one pipeline. Unknown input is rejected; it is never silently
 * reclassified as assembly.
 *
 * Bare compiler names and conventional Unix paths are resolved by sys.h through
 * deterministic Android/Termux prefixes; execve itself does not search PATH.
 *
 * use_asm    : internal ARM assembler (lang=asm/.s)
 * use_script : inline execve bootstrap via gen_script_code64()
 * use_fork   : fork+exec external compiler, read output artefact
 * dex_output : output goes into classes.dex (Kotlin/Java/Groovy)
 * use_d8     : after fork+exec step 1, run d8 on output to produce DEX
 * jsx_node   : after fork+exec babel, embed JS via node gen_script_code64
 *
 * RAFCODE-Φ-∆RafaelVerboΩ */
#pragma once
#include "sys.h"

typedef struct {
    const char *name;
    const char *ext;
    int         use_asm;
    int         use_script;
    int         use_fork;
    const char *compiler;
    const char *arg1;
    const char *cc_args[10];
    int         dex_output;
    int         arm64_only;
    int         use_d8;
    int         jsx_node;
    int         use_gpu_spv;
    int         use_gpu_cl;
    int         use_gpu_wgsl;
    int         use_dsp;
    int         use_npu;
} LangProfile;

#define LP_ASM     0
#define LP_C       1
#define LP_CPP     2
#define LP_RS      3
#define LP_KT      4
#define LP_JAVA    5
#define LP_PY      6
#define LP_SH      7
#define LP_PL      8
#define LP_JS      9
#define LP_PHP     10
#define LP_JSX     11
#define LP_GO      12
#define LP_RB      13
#define LP_SWIFT   14
#define LP_GROOVY  15
#define LP_CLJ     16
#define LP_GLSL    17
#define LP_CL      18
#define LP_HLSL    19
#define LP_WGSL    20
#define LP_DSP     21
#define LP_TFLITE  22
#define LP_COUNT   23

static const LangProfile _lang_table[LP_COUNT] = {
    [LP_ASM] = { "asm", ".s", 1, 0, 0, NULL,
                 NULL, {NULL}, 0, 0, 0, 0 },

    [LP_C] = { "c", ".c", 0, 0, 1, "clang",
               NULL,
               {"--target","aarch64-linux-android","-shared","-fPIC","-Os","-o",NULL},
               0, 1, 0, 0 },

    [LP_CPP] = { "cpp", ".cpp", 0, 0, 1, "clang++",
                 NULL,
                 {"--target","aarch64-linux-android","-shared","-fPIC","-Os","-o",NULL},
                 0, 1, 0, 0 },

    [LP_RS] = { "rs", ".rs", 0, 0, 1, "rustc",
                NULL,
                {"--target","aarch64-linux-android","--crate-type","cdylib","-o",NULL},
                0, 1, 0, 0 },

    [LP_KT] = { "kt", ".kt", 0, 0, 1, "kotlinc",
                NULL,
                {"-include-runtime","-d",NULL},
                1, 0, 1, 0 },

    [LP_JAVA] = { "java", ".java", 0, 0, 1, "sh",
                  NULL,
                  {"scripts/apkc_java_to_jar.sh",NULL},
                  1, 0, 1, 0 },

    [LP_PY] = { "py", ".py", 0, 1, 0, "/usr/bin/python3",
                "-c", {NULL}, 0, 1, 0, 0 },

    [LP_SH] = { "sh", ".sh", 0, 1, 0, "/bin/sh",
                "-c", {NULL}, 0, 1, 0, 0 },

    [LP_PL] = { "pl", ".pl", 0, 1, 0, "/usr/bin/perl",
                "-e", {NULL}, 0, 1, 0, 0 },

    [LP_JS] = { "js", ".js", 0, 1, 0, "/usr/bin/node",
                "-e", {NULL}, 0, 1, 0, 0 },

    [LP_PHP] = { "php", ".php", 0, 1, 0, "/usr/bin/php",
                 "-r", {NULL}, 0, 1, 0, 0 },

    [LP_JSX] = { "jsx", ".jsx", 0, 0, 1, "npx",
                 NULL,
                 {"babel","--presets","@babel/preset-react","--out-file",NULL},
                 0, 1, 0, 1 },

    [LP_GO] = { "go", ".go", 0, 0, 1, "go",
                NULL,
                {"build","-buildmode=c-shared","-o",NULL},
                0, 1, 0, 0 },

    [LP_RB] = { "rb", ".rb", 0, 1, 0, "/usr/bin/ruby",
                "-e", {NULL}, 0, 1, 0, 0 },

    [LP_SWIFT] = { "swift", ".swift", 0, 0, 1, "swiftc",
                   NULL,
                   {"-emit-library","-o",NULL},
                   0, 1, 0, 0 },

    [LP_GROOVY] = { "groovy", ".groovy", 0, 0, 1, "sh",
                    NULL,
                    {"scripts/apkc_groovy_to_jar.sh",NULL},
                    1, 0, 1, 0 },

    [LP_CLJ] = { "clj", ".clj", 0, 1, 0, "/usr/bin/clojure",
                 "-e", {NULL}, 0, 1, 0, 0, 0, 0, 0, 0, 0 },

    [LP_GLSL] = { "glsl", ".comp", 0, 0, 0, "glslc",
                  NULL, {"-fshader-stage=compute", "-o", NULL},
                  0, 1, 0, 0,
                  1, 0, 0, 0, 0 },

    [LP_CL] = { "cl", ".cl", 0, 0, 0, NULL,
                NULL, {NULL},
                0, 1, 0, 0,
                0, 1, 0, 0, 0 },

    [LP_HLSL] = { "hlsl", ".hlsl", 0, 0, 0, "glslc",
                  NULL, {"-fshader-stage=compute", "-x", "hlsl", "-o", NULL},
                  0, 1, 0, 0,
                  1, 0, 0, 0, 0 },

    [LP_WGSL] = { "wgsl", ".wgsl", 0, 0, 0, NULL,
                  NULL, {NULL},
                  0, 1, 0, 0,
                  0, 0, 1, 0, 0 },

    [LP_DSP] = { "dsp", ".dsp", 0, 0, 0, "hexagon-clang",
                 NULL, {"-mv65", "-shared", "-o", NULL},
                 0, 1, 0, 0,
                 0, 0, 0, 1, 0 },

    [LP_TFLITE] = { "tflite", ".tflite", 0, 0, 0, NULL,
                    NULL, {NULL},
                    0, 1, 0, 0,
                    0, 0, 0, 0, 1 },
};

static inline char _lp_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c + ('a' - 'A')) : c;
}

static inline int _lp_eq_ci(const char *a, const char *b) {
    if (!a || !b) return 0;
    sz i = 0;
    while (a[i] && b[i]) {
        if (_lp_lower(a[i]) != _lp_lower(b[i])) return 0;
        i++;
    }
    return !a[i] && !b[i];
}

static inline int lang_profile_validate(const LangProfile *p) {
    if (!p || !p->name || !p->name[0] || !p->ext || p->ext[0] != '.') return 0;

    int families = p->use_asm + p->use_script + p->use_fork +
                   p->use_gpu_spv + p->use_gpu_cl + p->use_gpu_wgsl +
                   p->use_dsp + p->use_npu;
    if (families != 1) return 0;

    if ((p->use_script || p->use_fork || p->use_gpu_spv || p->use_dsp) &&
        (!p->compiler || !p->compiler[0])) return 0;
    if (p->use_d8 && (!p->use_fork || !p->dex_output)) return 0;
    if (p->jsx_node && !p->use_fork) return 0;
    if (p->dex_output && !(p->use_fork || p->use_d8)) return 0;
    return 1;
}

static inline int lang_profile_table_validate(void) {
    for (int i = 0; i < LP_COUNT; i++) {
        if (!lang_profile_validate(&_lang_table[i])) return 0;
        for (int j = i + 1; j < LP_COUNT; j++) {
            if (_lp_eq_ci(_lang_table[i].name, _lang_table[j].name)) return 0;
            if (_lp_eq_ci(_lang_table[i].ext, _lang_table[j].ext)) return 0;
        }
    }
    return 1;
}

static inline const LangProfile *lang_profile_find(const char *name) {
    if (!name || !name[0]) return (const LangProfile *)0;
    for (int i = 0; i < LP_COUNT; i++) {
        if (_lp_eq_ci(_lang_table[i].name, name))
            return lang_profile_validate(&_lang_table[i]) ? &_lang_table[i]
                                                          : (const LangProfile *)0;
    }
    return (const LangProfile *)0;
}

static inline const LangProfile *lang_profile_from_path(const char *path) {
    if (!path || !path[0]) return (const LangProfile *)0;

    sz last_dot = (sz)-1;
    for (sz i = 0; path[i]; i++) if (path[i] == '.') last_dot = i;
    if (last_dot == (sz)-1 || !path[last_dot + 1]) return (const LangProfile *)0;

    const char *ext = path + last_dot;
    for (int i = 0; i < LP_COUNT; i++) {
        if (_lp_eq_ci(_lang_table[i].ext, ext))
            return lang_profile_validate(&_lang_table[i]) ? &_lang_table[i]
                                                          : (const LangProfile *)0;
    }
    return (const LangProfile *)0;
}
