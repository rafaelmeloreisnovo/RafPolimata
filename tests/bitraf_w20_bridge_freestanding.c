/* ARM32 freestanding compile-and-undefined-symbol probe.
 * Kept separate from the host test driver. No libc, heap, syscalls or JNI. */
#include "../rafaelia/bitraf_w20_bridge_v1.h"

bitraf_u32 raf_w20_arm32_probe(const bitraf_state *state,
                             bitraf_u8 subvalue,
                             bitraf_u32 *word,
                             raf_w20_decoded *decoded) {
    bitraf_u32 error;
    if (!state) return RAF_W20_ERR_INPUT;
    error = raf_w20_pack(*state, subvalue, word);
    if (error) return error;
    return raf_w20_unpack(*word, decoded);
}
