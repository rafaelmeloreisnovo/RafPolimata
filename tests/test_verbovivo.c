/* tests/test_verbovivo.c — golden unit tests for the verbovivo/HDC engine.
 *
 * Tests the freestanding-safe API: vv_init, vv_scan_buf, vv_hamming_256,
 * vv_monobit. Compiled as hosted (gcc -I. -IBenchmark -lm). */
#include "rafaelia/verbovivo.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    VerbVivoState s;
    vv_init(&s);

    /* V1: after init, chunk_count = 0 */
    if (s.chunk_count != 0) {
        printf("FAIL V1: chunk_count=%u (want 0)\n", s.chunk_count); return 1;
    }

    /* V2: Hamming distance of identical 256-bit vectors = 0 */
    unsigned char a[32]; memset(a, 0xAB, 32);
    if (vv_hamming_256(a, a) != 0) {
        printf("FAIL V2: hamming(a,a)=%d (want 0)\n", vv_hamming_256(a, a)); return 2;
    }

    /* V3: Hamming distance of all-zero vs all-one 256-bit vectors = 256 */
    unsigned char z[32]; memset(z, 0x00, 32);
    unsigned char o[32]; memset(o, 0xFF, 32);
    if (vv_hamming_256(z, o) != 256) {
        printf("FAIL V3: hamming(z,o)=%d (want 256)\n", vv_hamming_256(z, o)); return 3;
    }

    /* V4: monobit(zeros) = 0 set bits */
    if (vv_monobit(z) != 0) {
        printf("FAIL V4: monobit(z)=%d (want 0)\n", vv_monobit(z)); return 4;
    }

    /* V5: monobit(ones) = 256 set bits */
    if (vv_monobit(o) != 256) {
        printf("FAIL V5: monobit(o)=%d (want 256)\n", vv_monobit(o)); return 5;
    }

    /* V6: after scanning a buffer, chunk_count > 0 */
    const unsigned char inp[] = "hello world verbovivo golden input";
    vv_scan_buf(&s, inp, sizeof(inp) - 1, NULL);
    if (s.chunk_count == 0) { puts("FAIL V6: chunk_count still 0 after scan"); return 6; }

    /* V7: scanning a second distinct buffer increases chunk_count */
    unsigned int c1 = s.chunk_count;
    const unsigned char inp2[] = "second distinct buffer for engram ring";
    vv_scan_buf(&s, inp2, sizeof(inp2) - 1, NULL);
    if (s.chunk_count <= c1) {
        printf("FAIL V7: chunk_count=%u not > c1=%u\n", s.chunk_count, c1); return 7;
    }

    puts("VERBOVIVO_TEST PASS");
    return 0;
}
