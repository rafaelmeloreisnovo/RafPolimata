/* lang_profile.h — declarative language dispatch table for APKc.
 * Technological determinism: the source file extension or -lang flag
 * determines the ENTIRE pipeline through a single table lookup.
 * No per-language if/else chains. Zero programming per language added.
 *
 * use_asm    : internal ARM assembler (lang=asm/.s)
 * use_script : inline execve bootstrap via gen_script_code64()
 * use_fork   : fork+exec external compiler, read output artefact
 * dex_output : output goes into classes.dex (Kotlin/Java)
 * use_d8     : after fork+exec step 1, run d8 on output to produce DEX
 * jsx_node   : after fork+exec babel, embed JS via node gen_script_code64
 *
 * RAFCODE-Φ-∆RafaelVerboΩ */
#pragma once
#include "sys.h"

typedef struct {
    const char *name;        /* CLI name: "asm", "c", "py", ... */
    const char *ext;         /* file extension: ".s", ".c", ".py", ... */
    int         use_asm;     /* 1 = internal ARM assembler */
    int         use_script;  /* 1 = gen_script_code64 (execve bootstrap) */
    int         use_fork;    /* 1 = fork+exec external compiler */
    const char *compiler;    /* interpreter/compiler path or name */
    const char *arg1;        /* first arg to interpreter (e.g. "-c") */
    const char *cc_args[10]; /* extra fixed args for fork+exec compilers */
    int         dex_output;  /* 1 = output is .dex (Kotlin/Java) */
    int         arm64_only;  /* 1 = ARM32 not supported */
    int         use_d8;      /* 1 = run d8 after step-1 to convert to DEX */
    int         jsx_node;    /* 1 = run Babel output through node bootstrap */
    /* ── hardware-direct dispatch flags (added in hw-abstraction pass) ── */
    int         use_gpu_spv; /* 1 = fork shader compiler → SPIR-V APK asset */
    int         use_gpu_cl;  /* 1 = embed OpenCL C source as APK asset */
    int         use_gpu_wgsl;/* 1 = embed WebGPU/WGSL source as APK asset */
    int         use_dsp;     /* 1 = fork Hexagon DSP compiler → DSP .so asset */
} LangProfile;

/* ── Language ID constants ────────────────────────────────────────────── */
#define LP_ASM  0
#define LP_C    1
#define LP_CPP  2
#define LP_RS   3
#define LP_KT   4
#define LP_JAVA 5
#define LP_PY   6
#define LP_SH   7
#define LP_PL   8
#define LP_JS   9
#define LP_PHP  10
#define LP_JSX  11
#define LP_GO   12
#define LP_RB   13
#define LP_SWIFT 14
#define LP_GROOVY 15
#define LP_CLJ    16
/* hardware-direct targets */
#define LP_GLSL   17   /* Vulkan GLSL compute → glslc → SPIR-V APK asset */
#define LP_CL     18   /* OpenCL C source → embedded as APK asset */
#define LP_HLSL   19   /* HLSL compute → glslc HLSL frontend → SPIR-V asset */
#define LP_WGSL   20   /* WebGPU WGSL source → embedded as APK asset */
#define LP_DSP    21   /* Hexagon DSP C → hexagon-clang → DSP .so asset */
#define LP_COUNT  22

static const LangProfile _lang_table[LP_COUNT] = {
    /* ASM: internal 2-pass assembler, both arm64+arm32 */
    [LP_ASM]  = { "asm",  ".s",    1, 0, 0, NULL,
                  NULL, {NULL}, 0, 0, 0, 0 },

    /* C: clang → .so  (-o <outfile> is appended by build_apk before src) */
    [LP_C]    = { "c",    ".c",    0, 0, 1, "clang",
                  NULL,
                  {"--target","aarch64-linux-android","-shared","-fPIC","-Os","-o",NULL},
                  0, 1, 0, 0 },

    /* C++: clang++ → .so */
    [LP_CPP]  = { "cpp",  ".cpp",  0, 0, 1, "clang++",
                  NULL,
                  {"--target","aarch64-linux-android","-shared","-fPIC","-Os","-o",NULL},
                  0, 1, 0, 0 },

    /* Rust: rustc → cdylib .so */
    [LP_RS]   = { "rs",   ".rs",   0, 0, 1, "rustc",
                  NULL,
                  {"--target","aarch64-linux-android","--crate-type","cdylib","-o",NULL},
                  0, 1, 0, 0 },

    /* Kotlin: kotlinc → .jar → d8 → classes.dex */
    [LP_KT]   = { "kt",   ".kt",   0, 0, 1, "kotlinc",
                  NULL,
                  {"-include-runtime","-d",NULL},
                  1, 0, 1, 0 },

    /* Java: javac -d /tmp/apkc_cls/ → d8 → classes.dex */
    [LP_JAVA] = { "java", ".java", 0, 0, 1, "javac",
                  NULL,
                  {"-source","8","-target","8","-d",NULL},
                  1, 0, 1, 0 },

    /* Python: gen_script_code64 execve bootstrap, arm64 only */
    [LP_PY]   = { "py",   ".py",   0, 1, 0, "/usr/bin/python3",
                  "-c", {NULL}, 0, 1, 0, 0 },

    /* Shell: gen_script_code64, /bin/sh -c */
    [LP_SH]   = { "sh",   ".sh",   0, 1, 0, "/bin/sh",
                  "-c", {NULL}, 0, 1, 0, 0 },

    /* Perl: gen_script_code64, perl -e */
    [LP_PL]   = { "pl",   ".pl",   0, 1, 0, "/usr/bin/perl",
                  "-e", {NULL}, 0, 1, 0, 0 },

    /* JavaScript (Node.js): gen_script_code64, node -e */
    [LP_JS]   = { "js",   ".js",   0, 1, 0, "/usr/bin/node",
                  "-e", {NULL}, 0, 1, 0, 0 },

    /* PHP: gen_script_code64, php -r */
    [LP_PHP]  = { "php",  ".php",  0, 1, 0, "/usr/bin/php",
                  "-r", {NULL}, 0, 1, 0, 0 },

    /* JSX: npx babel → /tmp/jsx_out.js → gen_script_code64 node bootstrap */
    [LP_JSX]  = { "jsx",  ".jsx",  0, 0, 1, "npx",
                  NULL,
                  {"babel","--presets","@babel/preset-react","--out-file",NULL},
                  0, 1, 0, 1 },

    /* Go: go build -buildmode=c-shared → .so, arm64 only */
    [LP_GO]   = { "go",   ".go",   0, 0, 1, "go",
                  NULL,
                  {"build","-buildmode=c-shared","-o",NULL},
                  0, 1, 0, 0 },

    /* Ruby: gen_script_code64 execve bootstrap, ruby -e */
    [LP_RB]   = { "rb",   ".rb",   0, 1, 0, "/usr/bin/ruby",
                  "-e", {NULL}, 0, 1, 0, 0 },

    /* Swift: swiftc -emit-library → .so, arm64 only */
    [LP_SWIFT] = { "swift",".swift",0, 0, 1, "swiftc",
                  NULL,
                  {"-emit-library","-o",NULL},
                  0, 1, 0, 0 },

    /* Groovy: groovyc → .jar → d8 → classes.dex */
    [LP_GROOVY]= { "groovy",".groovy",0,0, 1, "groovyc",
                  NULL,
                  {"-d",NULL},
                  1, 0, 1, 0 },

    /* Clojure: gen_script_code64 execve bootstrap, clojure -e */
    [LP_CLJ]  = { "clj",  ".clj",  0, 1, 0, "/usr/bin/clojure",
                  "-e", {NULL}, 0, 1, 0, 0, 0, 0, 0, 0 },

    /* ── Hardware-direct compute targets ──────────────────────────────── */

    /* GLSL compute: glslc -fshader-stage=compute → SPIR-V APK asset.
     * APK contains: lib/arm64-v8a/libmain.so (NOP stub) +
     *               assets/compute.spv (SPIR-V blob).
     * Command: glslc -fshader-stage=compute -o /tmp/compute.spv source.comp */
    [LP_GLSL] = { "glsl", ".comp", 0, 0, 0, "glslc",
                  NULL, {"-fshader-stage=compute", "-o", NULL},
                  0, 1, 0, 0,
                  1, 0, 0, 0 },

    /* OpenCL C: source embedded verbatim as assets/compute.cl (no compilation).
     * The Android runtime loads the source via clCreateProgramWithSource. */
    [LP_CL]   = { "cl",   ".cl",   0, 0, 0, NULL,
                  NULL, {NULL},
                  0, 1, 0, 0,
                  0, 1, 0, 0 },

    /* HLSL compute: glslc HLSL frontend → SPIR-V APK asset.
     * Command: glslc -fshader-stage=compute -x hlsl -o /tmp/compute.spv source.hlsl */
    [LP_HLSL] = { "hlsl", ".hlsl", 0, 0, 0, "glslc",
                  NULL, {"-fshader-stage=compute", "-x", "hlsl", "-o", NULL},
                  0, 1, 0, 0,
                  1, 0, 0, 0 },

    /* WebGPU WGSL: source embedded as assets/compute.wgsl (no compilation).
     * The WebGPU / Dawn runtime compiles WGSL at load time. */
    [LP_WGSL] = { "wgsl", ".wgsl", 0, 0, 0, NULL,
                  NULL, {NULL},
                  0, 1, 0, 0,
                  0, 0, 1, 0 },

    /* Hexagon DSP: hexagon-clang → DSP shared library for FastRPC offload.
     * APK contains: lib/arm64-v8a/libmain.so (NOP stub) +
     *               lib/hexagon-v65/libcompute.so (DSP compute .so).
     * Command: hexagon-clang -mv65 -shared -o /tmp/dsp.so source.dsp */
    [LP_DSP]  = { "dsp",  ".dsp",  0, 0, 0, "hexagon-clang",
                  NULL, {"-mv65", "-shared", "-o", NULL},
                  0, 1, 0, 0,
                  0, 0, 0, 1 },
};

/* Find profile by CLI name — returns NULL for unknown names */
static inline const LangProfile *lang_profile_find(const char *name) {
    for (int i = 0; i < LP_COUNT; i++) {
        const char *n = _lang_table[i].name;
        sz j = 0;
        while (n[j] && name[j] && n[j]==name[j]) j++;
        if (!n[j] && !name[j]) return &_lang_table[i];
    }
    return (const LangProfile *)0; /* unknown: caller must handle */
}

/* Detect profile from file path extension */
static inline const LangProfile *lang_profile_from_path(const char *path) {
    if (!path) return &_lang_table[LP_ASM];
    /* find last dot */
    sz last_dot = (sz)-1;
    for (sz i = 0; path[i]; i++) if (path[i]=='.') last_dot = i;
    if (last_dot == (sz)-1) return &_lang_table[LP_ASM];
    const char *ext = path + last_dot;
    for (int i = 0; i < LP_COUNT; i++) {
        const char *e = _lang_table[i].ext;
        sz j = 0;
        while (e[j] && ext[j] && e[j]==ext[j]) j++;
        if (!e[j] && !ext[j]) return &_lang_table[i];
    }
    return &_lang_table[LP_ASM];
}
