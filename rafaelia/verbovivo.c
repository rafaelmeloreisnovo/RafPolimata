/* verbovivo.c — cognitive convergence engine.
 *
 * Pipeline:
 *   APK/ELF binary → T^7 toroid (7-dim, 42 attractors, phi_ethica)
 *                  → 1024-dim HDC hypervector (XOR-mixing expansion)
 *                  → SVG engram (2D trajectory + phi bar)
 *
 * Entry: verbovivo_main(apk_path, svg_out)
 *        Returns 0 on success, -1 on error.
 *
 * T^7 API:
 *   t7_init(t)              — seed at KAM-stable position (40503)
 *   t7_map_input(t, &inp)   — map data_hash/entropy/hw_state → coordinates
 *   t7_step(t, H, C)        — IIR update + phi_ethica + attractor
 *   t7_coherence(t)         — normalized dot with KAM seed
 *
 * phi_ethica = (1-H)*C: 0 = pure noise, Q16_ONE = perfect coherence. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Benchmark/raf_toroid.h"

/* ── HDC expansion: T^7 7-dim → 1024-dim via XOR-cyclic mixing ───────── */
#define HDC_DIM 1024

static void hdc_expand(const T7State *t, uint32_t hdc[HDC_DIM]) {
    for (int d = 0; d < T7_DIM; d++) hdc[d] = t->s[d];
    for (int d = T7_DIM; d < HDC_DIM; d++) {
        uint32_t x = hdc[d - T7_DIM] ^ hdc[d - 1];
        int sh = (d % 31) + 1;  /* cyclic left-rotate by 1..31 bits */
        hdc[d] = (x << sh) | (x >> (32 - sh));
    }
}

/* Hamming diversity: fraction of bit-flips between adjacent hdc words */
static double hamming_diversity(const uint32_t hdc[HDC_DIM]) {
    uint64_t diff = 0;
    for (int d = 0; d < HDC_DIM - 1; d++) {
        uint32_t x = hdc[d] ^ hdc[d + 1];
        while (x) { x &= x - 1u; diff++; }
    }
    return (double)diff / (double)((uint64_t)(HDC_DIM - 1) * 32u);
}

/* ── SVG writer ──────────────────────────────────────────────────────── */
#define SVG_W    512
#define SVG_H    480
#define SVG_PHI_H 32
#define MAX_TRAJ 4096

typedef struct { int x, y; } Pt;

static void svg_write(FILE *f,
                      const Pt *traj, int ntraj,
                      double phi, int attractor, double hamming)
{
    int total_h = SVG_H + SVG_PHI_H + 8;
    fprintf(f,
        "<?xml version='1.0' encoding='UTF-8'?>\n"
        "<svg xmlns='http://www.w3.org/2000/svg' "
             "width='%d' height='%d' "
             "style='background:#0a0a0f'>\n",
        SVG_W, total_h);

    fprintf(f,
        "  <text x='8' y='18' font-family='monospace' font-size='11' "
               "fill='#7aecb4'>"
               "verbovivo T^7 — attractor=%d phi=%.4f hamming=%.4f"
               "</text>\n",
        attractor, phi, hamming);

    if (ntraj > 1) {
        fprintf(f,
            "  <polyline fill='none' stroke='#4e9eff' "
                       "stroke-width='1' opacity='0.75' points='");
        for (int i = 0; i < ntraj; i++)
            fprintf(f, "%d,%d ", traj[i].x, traj[i].y);
        fprintf(f, "'/>\n");

        fprintf(f,
            "  <circle cx='%d' cy='%d' r='4' "
                     "fill='#ff6e6e' stroke='#fff' stroke-width='1'/>\n",
            traj[ntraj-1].x, traj[ntraj-1].y);
    }

    /* phi bar */
    int bar_w = (int)(phi * SVG_W);
    if (bar_w < 0) bar_w = 0;
    if (bar_w > SVG_W) bar_w = SVG_W;
    const char *bar_col = (phi > 0.6) ? "#4e9eff"
                        : (phi > 0.3) ? "#f0c040"
                                      : "#ff6e6e";
    fprintf(f,
        "  <rect x='0' y='%d' width='%d' height='%d' fill='#1a1a1f'/>\n"
        "  <rect x='0' y='%d' width='%d' height='%d' fill='%s'/>\n"
        "  <text x='4' y='%d' font-family='monospace' font-size='10' "
               "fill='#ccc'>phi_ethica=%.4f</text>\n",
        SVG_H + 4, SVG_W, SVG_PHI_H,
        SVG_H + 4, bar_w, SVG_PHI_H, bar_col,
        SVG_H + SVG_PHI_H, phi);

    fprintf(f, "</svg>\n");
}

/* ── Main entry ──────────────────────────────────────────────────────── */
int verbovivo_main(const char *apk_path, const char *svg_out) {
    FILE *fin = fopen(apk_path, "rb");
    if (!fin) {
        fprintf(stderr, "verbovivo: cannot open %s\n", apk_path);
        return -1;
    }

    T7State t7;
    t7_init(&t7);

    Pt    *traj  = (Pt *)malloc(MAX_TRAJ * sizeof(Pt));
    int    ntraj = 0;
    size_t nbytes = 0;

    uint8_t blk[256];
    size_t  n;
    while ((n = fread(blk, 1, sizeof(blk), fin)) > 0) {
        /* compute per-block entropy proxy and coherence from byte stats */
        uint32_t unique = 0, transitions = 0;
        uint8_t  seen[256];
        memset(seen, 0, 256);
        for (size_t i = 0; i < n; i++) {
            if (!seen[blk[i]]) { seen[blk[i]] = 1; unique++; }
            if (i > 0 && blk[i] != blk[i-1]) transitions++;
        }

        /* H_in: entropy proxy in Q16 (higher unique → higher entropy) */
        q16_t H_in = (q16_t)((unique  * Q16_ONE) / 256);
        /* C_in: coherence proxy (lower transitions → higher coherence) */
        q16_t C_in = (n > 1)
            ? (q16_t)(Q16_ONE - (transitions * Q16_ONE) / (uint32_t)(n - 1))
            : (q16_t)Q16_ONE;

        /* map input to toroid coordinates, then step */
        T7Input inp;
        uint32_t h = 2166136261u;
        for (size_t i = 0; i < n; i++) h = (h ^ blk[i]) * 16777619u;
        inp.data_hash = h;
        inp.entropy   = H_in;
        inp.hw_state  = (uint32_t)nbytes;
        t7_map_input(&t7, &inp);
        t7_step(&t7, H_in, C_in);

        nbytes += n;

        /* record trajectory (one point per block, subsampled to MAX_TRAJ) */
        if (ntraj < MAX_TRAJ) {
            traj[ntraj].x = (int)((t7.s[0] * (uint32_t)(SVG_W - 16)) >> 16) + 8;
            traj[ntraj].y = (int)((t7.s[1] * (uint32_t)(SVG_H - 32)) >> 16) + 24;
            ntraj++;
        }
    }
    fclose(fin);

    if (!nbytes) {
        fprintf(stderr, "verbovivo: empty file %s\n", apk_path);
        free(traj);
        return -1;
    }

    /* HDC expansion + diversity */
    uint32_t hdc[HDC_DIM];
    hdc_expand(&t7, hdc);
    double hamming = hamming_diversity(hdc);
    double phi     = (double)(uint32_t)t7.phi / 65536.0;
    if (phi < 0.0) phi = 0.0;

    /* emit SVG */
    FILE *fout = svg_out ? fopen(svg_out, "w") : stdout;
    if (!fout) {
        fprintf(stderr, "verbovivo: cannot write %s\n", svg_out ? svg_out : "(stdout)");
        free(traj);
        return -1;
    }
    svg_write(fout, traj, ntraj, phi, (int)t7.attractor, hamming);
    if (svg_out) fclose(fout);

    fprintf(stderr,
        "verbovivo: %zu bytes  phi=%.4f  attractor=%u  "
        "hamming=%.4f  hdc[0]=%08x\n",
        nbytes, phi, t7.attractor, hamming, hdc[0]);

    free(traj);
    return 0;
}

/* ── CLI wrapper (build with -DVERBOVIVO_MAIN) ──────────────────────── */
#ifdef VERBOVIVO_MAIN
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: verbovivo <apk_or_elf> [out.svg]\n");
        return 1;
    }
    return verbovivo_main(argv[1], argc >= 3 ? argv[2] : NULL) == 0 ? 0 : 1;
}
#endif
