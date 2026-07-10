/*
 * RAFAELIA Q16 portable reference implementation
 * Author: Rafael Melo Reis — RAFCODE-Φ / ∆RafaelVerboΩ
 *
 * This unit contains no platform syscall and no dynamic allocation.
 * Compile as a normal hosted test or include the core functions in an
 * Android/native runtime.
 *
 * Hosted example:
 *   cc -O2 -std=c11 -Wall -Wextra -DRAFAELIA_Q16_HOSTED_TEST \
 *      rafaelia_q16_reference.c -o rafaelia_q16_reference
 *   ./rafaelia_q16_reference
 */

#include <stdint.h>

#define RAFAELIA_Q16_SHIFT       16
#define RAFAELIA_Q16_GEOM        INT32_C(56756)
#define RAFAELIA_Q16_FORCE       INT32_C(203333)
#define RAFAELIA_Q16_TARGET      INT32_C(1517675)
#define RAFAELIA_Q16_TOLERANCE   INT32_C(64)

typedef struct RafaeliaQ16Vector {
    int32_t result_48;
    int32_t result_96;
    int32_t error;
    int32_t fixed_min;
    int32_t fixed_max;
    int32_t reached_fixed;
    uint32_t fixed_count;
    uint32_t first_fixed_iteration;
} RafaeliaQ16Vector;

int32_t rafaelia_q16_step(int32_t current)
{
    int64_t product = (int64_t)current * (int64_t)RAFAELIA_Q16_GEOM;
    return (int32_t)(product >> RAFAELIA_Q16_SHIFT)
         + RAFAELIA_Q16_FORCE;
}

int32_t rafaelia_q16_iterate(int32_t seed, uint32_t iterations)
{
    uint32_t i;
    int32_t current = seed;
    for (i = 0u; i < iterations; ++i) {
        current = rafaelia_q16_step(current);
    }
    return current;
}

static int32_t q16_abs_difference(int32_t a, int32_t b)
{
    int64_t difference = (int64_t)a - (int64_t)b;
    return (int32_t)(difference < 0 ? -difference : difference);
}

static uint32_t q16_find_fixed_band(int32_t center,
                                    int32_t radius,
                                    int32_t *minimum,
                                    int32_t *maximum)
{
    uint32_t count = 0u;
    int32_t value;

    if (minimum == 0 || maximum == 0 || radius < 0) {
        return 0u;
    }

    for (value = center - radius; value <= center + radius; ++value) {
        if (rafaelia_q16_step(value) == value) {
            if (count == 0u) {
                *minimum = value;
            }
            *maximum = value;
            ++count;
        }
    }
    return count;
}

static uint32_t q16_first_fixed(int32_t seed,
                                uint32_t limit,
                                int32_t *fixed_value)
{
    uint32_t iteration;
    int32_t current = seed;

    if (fixed_value == 0) {
        return 0u;
    }

    for (iteration = 1u; iteration <= limit; ++iteration) {
        current = rafaelia_q16_step(current);
        if (rafaelia_q16_step(current) == current) {
            *fixed_value = current;
            return iteration;
        }
    }

    *fixed_value = current;
    return 0u;
}

RafaeliaQ16Vector rafaelia_q16_measure(void)
{
    RafaeliaQ16Vector vector;

    vector.result_48 = rafaelia_q16_iterate(0, 48u);
    vector.result_96 = rafaelia_q16_iterate(0, 96u);
    vector.error = q16_abs_difference(vector.result_96,
                                      RAFAELIA_Q16_TARGET);
    vector.fixed_min = 0;
    vector.fixed_max = 0;
    vector.reached_fixed = 0;
    vector.fixed_count = q16_find_fixed_band(RAFAELIA_Q16_TARGET,
                                              256,
                                              &vector.fixed_min,
                                              &vector.fixed_max);
    vector.first_fixed_iteration = q16_first_fixed(0, 256u,
                                                    &vector.reached_fixed);
    return vector;
}

int rafaelia_q16_selftest(void)
{
    RafaeliaQ16Vector v = rafaelia_q16_measure();

    return v.result_48 == 1516200
        && v.result_96 == 1517719
        && v.error == 44
        && v.error <= RAFAELIA_Q16_TOLERANCE
        && v.fixed_count == 7u
        && v.fixed_min == 1517719
        && v.fixed_max == 1517725
        && v.first_fixed_iteration == 90u
        && v.reached_fixed == 1517719;
}

#ifdef RAFAELIA_Q16_HOSTED_TEST
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    RafaeliaQ16Vector v = rafaelia_q16_measure();
    int ok = rafaelia_q16_selftest();

    printf("result_48=%" PRId32 "\n", v.result_48);
    printf("result_96=%" PRId32 "\n", v.result_96);
    printf("error=%" PRId32 "\n", v.error);
    printf("fixed_count=%" PRIu32 "\n", v.fixed_count);
    printf("fixed_min=%" PRId32 "\n", v.fixed_min);
    printf("fixed_max=%" PRId32 "\n", v.fixed_max);
    printf("first_fixed_iteration=%" PRIu32 "\n",
           v.first_fixed_iteration);
    printf("reached_fixed=%" PRId32 "\n", v.reached_fixed);
    printf("contract=%s\n", ok ? "PASS" : "FAIL");

    return ok ? 0 : 1;
}
#endif
