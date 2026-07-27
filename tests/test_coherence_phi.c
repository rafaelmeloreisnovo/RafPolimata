/* tests/test_coherence_phi.c — golden tests for phi_fst() and phi_attractor().
 *
 * Compiled as hosted (gcc -I.) — sys.h types (u8/u32) resolve normally.
 * Six cases covering: all-zero (max phi), all-high-byte (phi=0), full entropy
 * (phi=0), empty buffer (early return), and attractor mapping. */
#include "Apkc/sys.h"
#include "Apkc/coherence.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    u8 buf[256];

    /* C1: 64 zero bytes → unique=1, H=256, C clamped to 65536, phi = 65280 */
    memset(buf, 0x00, 64);
    u32 phi = phi_fst(buf, 64);
    if (phi != 65280u) { printf("FAIL C1: got %u (want 65280)\n", phi); return 1; }

    /* C2: all 256 distinct bytes → unique=256, H=65536, oneMinH=0, phi=0 */
    for (int i = 0; i < 256; i++) buf[i] = (u8)i;
    phi = phi_fst(buf, 256);
    if (phi != 0u) { printf("FAIL C2: got %u (want 0)\n", phi); return 2; }

    /* C3: 64 bytes of 0xFF → byte 255 outside KAM-7 window [0..6], ns=0, C=0, phi=0 */
    memset(buf, 0xFF, 64);
    phi = phi_fst(buf, 64);
    if (phi != 0u) { printf("FAIL C3: got %u (want 0)\n", phi); return 3; }

    /* C4: empty buffer → early return 0 */
    phi = phi_fst(buf, 0);
    if (phi != 0u) { printf("FAIL C4: got %u (want 0)\n", phi); return 4; }

    /* C5: phi_attractor(0) = (0 ^ 0) % 42 = 0 */
    if (phi_attractor(0u) != 0u) {
        printf("FAIL C5: got %u (want 0)\n", phi_attractor(0u)); return 5;
    }

    /* C6: phi_attractor(65280):
     *   65280 = 0xFF00; 65280>>7 = 510 = 0x01FE
     *   0xFF00 ^ 0x01FE = 0xFEFE = 65278
     *   65278 % 42 = 65278 - 42*1554 = 65278 - 65268 = 10 */
    u32 a6 = phi_attractor(65280u);
    if (a6 != 10u) { printf("FAIL C6: got %u (want 10)\n", a6); return 6; }

    puts("COHERENCE_PHI_TEST PASS");
    return 0;
}
