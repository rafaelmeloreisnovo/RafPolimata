#ifndef RAFAELIA_LANG_FREESTANDING_POLICY_H
#define RAFAELIA_LANG_FREESTANDING_POLICY_H

/*
 * M063 — Canonical language completion policy.
 *
 * This header has no includes, no heap and no runtime dependency. It mirrors
 * ApkC/lang_profile.h profile ids so the hosted dispatch table can coexist with
 * a stricter final-binary contract without pretending that interpreters, DEX,
 * drivers or managed runtimes are freestanding.
 */

#define RAF_M063_LANGUAGE_COUNT 23u
#define RAF_M063_MAX_LANES      16u

typedef enum {
    RAF_M063_LP_ASM = 0,
    RAF_M063_LP_C = 1,
    RAF_M063_LP_CPP = 2,
    RAF_M063_LP_RS = 3,
    RAF_M063_LP_KT = 4,
    RAF_M063_LP_JAVA = 5,
    RAF_M063_LP_PY = 6,
    RAF_M063_LP_SH = 7,
    RAF_M063_LP_PL = 8,
    RAF_M063_LP_JS = 9,
    RAF_M063_LP_PHP = 10,
    RAF_M063_LP_JSX = 11,
    RAF_M063_LP_GO = 12,
    RAF_M063_LP_RB = 13,
    RAF_M063_LP_SWIFT = 14,
    RAF_M063_LP_GROOVY = 15,
    RAF_M063_LP_CLJ = 16,
    RAF_M063_LP_GLSL = 17,
    RAF_M063_LP_CL = 18,
    RAF_M063_LP_HLSL = 19,
    RAF_M063_LP_WGSL = 20,
    RAF_M063_LP_DSP = 21,
    RAF_M063_LP_TFLITE = 22
} rafaelia_m063_language_id_t;

typedef enum {
    RAF_M063_DIRECT_ISA = 0,
    RAF_M063_NATIVE_SUBSET = 1,
    RAF_M063_LOWER_TO_RAF_IR = 2,
    RAF_M063_DEVICE_KERNEL = 3,
    RAF_M063_MODEL_TO_INTERNAL_KERNELS = 4
} rafaelia_m063_route_t;

typedef enum {
    RAF_M063_FINAL_STANDALONE = 0,
    RAF_M063_FINAL_CONDITIONAL = 1,
    RAF_M063_FINAL_REQUIRES_LOWERING = 2,
    RAF_M063_FINAL_DEVICE_ONLY = 3,
    RAF_M063_FINAL_DATA_ONLY = 4
} rafaelia_m063_final_class_t;

#define RAF_M063_REQ_NO_HEAP             (1ULL << 0)
#define RAF_M063_REQ_NO_GC               (1ULL << 1)
#define RAF_M063_REQ_NO_EXTERNAL_RUNTIME (1ULL << 2)
#define RAF_M063_REQ_NO_LIBC             (1ULL << 3)
#define RAF_M063_REQ_NO_EXCEPTIONS       (1ULL << 4)
#define RAF_M063_REQ_NO_RTTI             (1ULL << 5)
#define RAF_M063_REQ_NO_TLS              (1ULL << 6)
#define RAF_M063_REQ_NO_TAILCALL         (1ULL << 7)
#define RAF_M063_REQ_NO_SHADOW           (1ULL << 8)
#define RAF_M063_REQ_NO_UNWIND           (1ULL << 9)
#define RAF_M063_REQ_HIDDEN_SYMBOLS      (1ULL << 10)
#define RAF_M063_REQ_GC_SECTIONS         (1ULL << 11)
#define RAF_M063_REQ_NO_BUILD_ID         (1ULL << 12)
#define RAF_M063_REQ_ZERO_UNDEFINED      (1ULL << 13)
#define RAF_M063_REQ_STATIC_STORAGE      (1ULL << 14)
#define RAF_M063_REQ_CALLER_BUFFER       (1ULL << 15)
#define RAF_M063_REQ_BIT_PATCH           (1ULL << 16)
#define RAF_M063_REQ_CYCLE_DETERMINISM   (1ULL << 17)
#define RAF_M063_REQ_BRANCHLESS_HOTPATH  (1ULL << 18)
#define RAF_M063_REQ_NO_DUP_KERNEL       (1ULL << 19)

#define RAF_M063_BASE_REQUIREMENTS ( \
    RAF_M063_REQ_NO_HEAP             | \
    RAF_M063_REQ_NO_GC               | \
    RAF_M063_REQ_NO_EXTERNAL_RUNTIME | \
    RAF_M063_REQ_NO_LIBC             | \
    RAF_M063_REQ_NO_TAILCALL         | \
    RAF_M063_REQ_NO_SHADOW           | \
    RAF_M063_REQ_NO_UNWIND           | \
    RAF_M063_REQ_HIDDEN_SYMBOLS      | \
    RAF_M063_REQ_GC_SECTIONS         | \
    RAF_M063_REQ_NO_BUILD_ID         | \
    RAF_M063_REQ_ZERO_UNDEFINED      | \
    RAF_M063_REQ_STATIC_STORAGE      | \
    RAF_M063_REQ_CALLER_BUFFER       | \
    RAF_M063_REQ_BIT_PATCH           | \
    RAF_M063_REQ_CYCLE_DETERMINISM   | \
    RAF_M063_REQ_NO_DUP_KERNEL )

typedef struct {
    const char *name;
    unsigned char profile_id;
    unsigned char route;
    unsigned char final_class;
    unsigned char max_lanes;
    unsigned long long requirements;
} rafaelia_m063_language_policy_t;

#define RAF_M063_NATIVE_REQ \
    (RAF_M063_BASE_REQUIREMENTS | RAF_M063_REQ_BRANCHLESS_HOTPATH)

#define RAF_M063_CPP_REQ \
    (RAF_M063_NATIVE_REQ | RAF_M063_REQ_NO_EXCEPTIONS | RAF_M063_REQ_NO_RTTI | RAF_M063_REQ_NO_TLS)

static const rafaelia_m063_language_policy_t
rafaelia_m063_language_policy[RAF_M063_LANGUAGE_COUNT] = {
    {"asm",    RAF_M063_LP_ASM,    RAF_M063_DIRECT_ISA,                RAF_M063_FINAL_STANDALONE,        16u, RAF_M063_NATIVE_REQ},
    {"c",      RAF_M063_LP_C,      RAF_M063_NATIVE_SUBSET,             RAF_M063_FINAL_STANDALONE,        16u, RAF_M063_NATIVE_REQ},
    {"cpp",    RAF_M063_LP_CPP,    RAF_M063_NATIVE_SUBSET,             RAF_M063_FINAL_CONDITIONAL,       16u, RAF_M063_CPP_REQ},
    {"rs",     RAF_M063_LP_RS,     RAF_M063_NATIVE_SUBSET,             RAF_M063_FINAL_CONDITIONAL,       16u, RAF_M063_NATIVE_REQ},
    {"kt",     RAF_M063_LP_KT,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"java",   RAF_M063_LP_JAVA,   RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"py",     RAF_M063_LP_PY,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"sh",     RAF_M063_LP_SH,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  1u, RAF_M063_BASE_REQUIREMENTS},
    {"pl",     RAF_M063_LP_PL,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"js",     RAF_M063_LP_JS,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"php",    RAF_M063_LP_PHP,    RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"jsx",    RAF_M063_LP_JSX,    RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"go",     RAF_M063_LP_GO,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"rb",     RAF_M063_LP_RB,     RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"swift",  RAF_M063_LP_SWIFT,  RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"groovy", RAF_M063_LP_GROOVY, RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"clj",    RAF_M063_LP_CLJ,    RAF_M063_LOWER_TO_RAF_IR,           RAF_M063_FINAL_REQUIRES_LOWERING,  8u, RAF_M063_BASE_REQUIREMENTS},
    {"glsl",   RAF_M063_LP_GLSL,   RAF_M063_DEVICE_KERNEL,             RAF_M063_FINAL_DEVICE_ONLY,       16u, RAF_M063_NATIVE_REQ},
    {"cl",     RAF_M063_LP_CL,     RAF_M063_DEVICE_KERNEL,             RAF_M063_FINAL_DEVICE_ONLY,       16u, RAF_M063_NATIVE_REQ},
    {"hlsl",   RAF_M063_LP_HLSL,   RAF_M063_DEVICE_KERNEL,             RAF_M063_FINAL_DEVICE_ONLY,       16u, RAF_M063_NATIVE_REQ},
    {"wgsl",   RAF_M063_LP_WGSL,   RAF_M063_DEVICE_KERNEL,             RAF_M063_FINAL_DEVICE_ONLY,       16u, RAF_M063_NATIVE_REQ},
    {"dsp",    RAF_M063_LP_DSP,    RAF_M063_DEVICE_KERNEL,             RAF_M063_FINAL_CONDITIONAL,       16u, RAF_M063_NATIVE_REQ},
    {"tflite", RAF_M063_LP_TFLITE, RAF_M063_MODEL_TO_INTERNAL_KERNELS, RAF_M063_FINAL_DATA_ONLY,         16u, RAF_M063_BASE_REQUIREMENTS}
};

static inline const rafaelia_m063_language_policy_t *
rafaelia_m063_policy_get(unsigned int id) {
    if (id >= RAF_M063_LANGUAGE_COUNT) {
        return (const rafaelia_m063_language_policy_t *)0;
    }
    return &rafaelia_m063_language_policy[id];
}

static inline int rafaelia_m063_policy_is_standalone(unsigned int id) {
    const rafaelia_m063_language_policy_t *p = rafaelia_m063_policy_get(id);
    return p != (const rafaelia_m063_language_policy_t *)0 &&
           p->final_class == RAF_M063_FINAL_STANDALONE;
}

static inline int rafaelia_m063_policy_requires_lowering(unsigned int id) {
    const rafaelia_m063_language_policy_t *p = rafaelia_m063_policy_get(id);
    return p != (const rafaelia_m063_language_policy_t *)0 &&
           p->route == RAF_M063_LOWER_TO_RAF_IR;
}

static inline int rafaelia_m063_policy_validate(void) {
    unsigned int i;
    for (i = 0u; i < RAF_M063_LANGUAGE_COUNT; ++i) {
        const rafaelia_m063_language_policy_t *p = &rafaelia_m063_language_policy[i];
        if (p->profile_id != i || p->name == (const char *)0 || p->name[0] == '\0') {
            return 0;
        }
        if (p->max_lanes == 0u || p->max_lanes > RAF_M063_MAX_LANES) {
            return 0;
        }
        if ((p->requirements & RAF_M063_BASE_REQUIREMENTS) != RAF_M063_BASE_REQUIREMENTS) {
            return 0;
        }
        if (p->route == RAF_M063_LOWER_TO_RAF_IR &&
            p->final_class != RAF_M063_FINAL_REQUIRES_LOWERING) {
            return 0;
        }
    }
    return 1;
}

#endif
