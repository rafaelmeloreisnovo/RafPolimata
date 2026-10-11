#ifndef RAF_FS_JMV_H
#define RAF_FS_JMV_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Deterministic, bounded polyphonic event-tape kernel (JMV).
 * SCOPE: Pure scalar L0 event arbitration; NOT a Java VM/JVM.
 * PRECONDITIONS: Caller owns nonoverlapping buffers and eight state slots.
 * REGISTER_OWNERSHIP: Compiler scalar registers; no OS/device/clock owner.
 * CLOBBERS: STEP changes chosen voice, consumed mask and explicit outputs; invalid input changes nothing.
 * MEMORY_ORDER: Single-threaded caller-defined order; no atomic publication guarantee.
 * TAIL_SHADOW: Explicit count <= 32 and consumed bitmask; no hidden tail, cache, retry or shadow.
 * EVIDENCE: New authorial source; host/cross-target probes required; no device proof.
 * RIGHTS: The repository-wide redistribution license remains separately unresolved.
 */
#include "raf_fs_types.h"

#define RAF_FS_JMV_MAX_EVENTS 32u
#define RAF_FS_JMV_VOICES 8u
#define RAF_FS_JMV_NOTE 1u
#define RAF_FS_JMV_DELTA 2u
#define RAF_FS_JMV_GAIN 3u
#define RAF_FS_JMV_REST 4u
#define RAF_FS_JMV_STEP 0x61u
#define RAF_FS_JMV_DONE 0x62u
#define RAF_FS_JMV_BAD_INPUT 0x63u
#define RAF_FS_JMV_BAD_TAPE 0x64u
#define RAF_FS_JMV_TOKEN_VAZIO 0x65u

/* Exact saturation with no evaluated signed overflow. */
#define RAF_FS_JMV_CLAMP32(x) ( \
    ((x) > 2147483647LL) ? (raf_i32)2147483647 : \
    (((x) < (-2147483647LL - 1LL)) ? (raf_i32)(-2147483647 - 1) : (raf_i32)(x)))

/* One transaction: validate -> pick minimum (tick,voice,index) -> commit.
 * Source tape and its order are immutable. Caller invokes once per event.
 * No external helper, runtime, Java class, JNI, malloc, I/O or syscall.
 */
static inline __attribute__((always_inline)) raf_u32
raf_fs_jmv_step(const raf_u32 *ticks, const raf_u8 *voices,
                const raf_u8 *opcodes, const raf_i32 *values,
                raf_u32 count, raf_u32 *consumed, raf_i32 *state,
                raf_u32 *out_tick, raf_u32 *out_index,
                raf_u32 *out_voice, raf_i32 *out_voice_value,
                raf_i32 *out_mix)
{
    raf_u32 i;
    raf_u32 chosen = RAF_FS_JMV_MAX_EVENTS;
    raf_u32 used;
    raf_i64 next;
    raf_i64 mix;
    raf_u32 voice;
    if (!consumed || !state || !out_tick || !out_index || !out_voice ||
        !out_voice_value || !out_mix || count > RAF_FS_JMV_MAX_EVENTS ||
        (count && (!ticks || !voices || !opcodes || !values))) {
        return RAF_FS_JMV_BAD_INPUT;
    }
    used = *consumed;
    if (count < 32u && (used >> count) != 0u) return RAF_FS_JMV_BAD_INPUT;
    /* Whole-tape validation precedes all mutation: corrupted tape fails closed. */
    for (i = 0u; i < count; ++i) {
        if (voices[i] >= RAF_FS_JMV_VOICES ||
            opcodes[i] < RAF_FS_JMV_NOTE || opcodes[i] > RAF_FS_JMV_REST ||
            (opcodes[i] == RAF_FS_JMV_NOTE &&
             (values[i] < 0 || values[i] > 127)) ||
            (opcodes[i] == RAF_FS_JMV_GAIN &&
             (values[i] < -16 || values[i] > 16)) ||
            (opcodes[i] == RAF_FS_JMV_REST && values[i] != 0)) {
            return RAF_FS_JMV_BAD_TAPE;
        }
    }
    for (i = 0u; i < count; ++i) {
        if ((used & ((raf_u32)1u << i)) != 0u) continue;
        if (chosen == RAF_FS_JMV_MAX_EVENTS ||
            ticks[i] < ticks[chosen] ||
            (ticks[i] == ticks[chosen] && voices[i] < voices[chosen])) {
            chosen = i;
        }
    }
    if (chosen == RAF_FS_JMV_MAX_EVENTS) return RAF_FS_JMV_DONE;
    voice = voices[chosen];
    next = (raf_i64)state[voice];
    if (opcodes[chosen] == RAF_FS_JMV_NOTE) next = (raf_i64)values[chosen];
    if (opcodes[chosen] == RAF_FS_JMV_DELTA) next += (raf_i64)values[chosen];
    if (opcodes[chosen] == RAF_FS_JMV_GAIN) next *= (raf_i64)values[chosen];
    if (opcodes[chosen] == RAF_FS_JMV_REST) next = 0;
    next = (raf_i64)RAF_FS_JMV_CLAMP32(next);
    mix = next;
    for (i = 0u; i < RAF_FS_JMV_VOICES; ++i) {
        if (i != voice) mix += (raf_i64)state[i];
    }
    state[voice] = (raf_i32)next;
    *consumed = used | ((raf_u32)1u << chosen);
    *out_tick = ticks[chosen];
    *out_index = chosen;
    *out_voice = voice;
    *out_voice_value = (raf_i32)next;
    *out_mix = RAF_FS_JMV_CLAMP32(mix);
    return RAF_FS_JMV_STEP;
}
#endif
