#include "Apkc/lang_freestanding_policy.h"
#include "rafaelia/raf_lane16_patch.h"

/*
 * M063 — Language completion and library assimilation contract.
 *
 * This method does not execute foreign runtimes. It validates the canonical
 * language policy, selective masked replacement and a static 16-lane
 * dependency plan. Language-specific frontends must lower into RAF IR/C/ASM
 * before they can claim the strict final-binary profile.
 */

int rafaelia_m063_selftest(void) {
    unsigned long long words[2] = {
        0xAAAAAAAAAAAAAAAAULL,
        0x1122334455667788ULL
    };
    rafaelia_overlay64_t overlay;
    rafaelia_lane16_plan_t plan;
    unsigned short ready;

    if (!rafaelia_m063_policy_validate()) return -1;
    if (!rafaelia_m063_policy_is_standalone(RAF_M063_LP_ASM)) return -1;
    if (!rafaelia_m063_policy_is_standalone(RAF_M063_LP_C)) return -1;
    if (!rafaelia_m063_policy_requires_lowering(RAF_M063_LP_PY)) return -1;
    if (!rafaelia_m063_policy_requires_lowering(RAF_M063_LP_JAVA)) return -1;
    if (rafaelia_m063_policy_is_standalone(RAF_M063_LP_GO)) return -1;
    if (rafaelia_m063_policy_get(RAF_M063_LP_GLSL)->final_class
        != RAF_M063_FINAL_DEVICE_ONLY) return -1;
    if (rafaelia_m063_policy_get(RAF_M063_LP_TFLITE)->final_class
        != RAF_M063_FINAL_DATA_ONLY) return -1;

    overlay.word_index = 1u;
    overlay.patch.mask = 0x000000000000FF00ULL;
    overlay.patch.value = 0x000000000000AA00ULL;
    if (!rafaelia_overlay64_apply(words, 2u, overlay)) return -1;
    if (words[0] != 0xAAAAAAAAAAAAAAAAULL) return -1;
    if (words[1] != 0x112233445566AA88ULL) return -1;
    if ((rafaelia_patch64_changed_bits(0x1122334455667788ULL, words[1])
         & ~overlay.patch.mask) != 0ULL) return -1;

    rafaelia_lane16_init(&plan);
    if (!rafaelia_lane16_configure(
            &plan, RAF_LANE_CPU0, 0u,
            (unsigned short)(1u << RAF_LANE_CPU0))) return -1;
    if (!rafaelia_lane16_configure(
            &plan, RAF_LANE_NEON, 0u,
            (unsigned short)(1u << RAF_LANE_NEON))) return -1;
    if (!rafaelia_lane16_configure(
            &plan, RAF_LANE_CRC32_SW,
            (unsigned short)((1u << RAF_LANE_CPU0) |
                             (1u << RAF_LANE_NEON)),
            (unsigned short)(1u << RAF_LANE_CRC32_SW))) return -1;

    ready = rafaelia_lane16_ready_mask(&plan);
    if ((ready & (unsigned short)(1u << RAF_LANE_CPU0)) == 0u) return -1;
    if ((ready & (unsigned short)(1u << RAF_LANE_NEON)) == 0u) return -1;
    if ((ready & (unsigned short)(1u << RAF_LANE_CRC32_SW)) != 0u) return -1;

    if (!rafaelia_lane16_complete(&plan, RAF_LANE_CPU0)) return -1;
    if (!rafaelia_lane16_complete(&plan, RAF_LANE_NEON)) return -1;
    ready = rafaelia_lane16_ready_mask(&plan);
    if ((ready & (unsigned short)(1u << RAF_LANE_CRC32_SW)) == 0u) return -1;

    return 0;
}

#ifdef RAFAELIA_M063_SELFTEST_MAIN
int main(void) {
    return rafaelia_m063_selftest();
}
#endif
