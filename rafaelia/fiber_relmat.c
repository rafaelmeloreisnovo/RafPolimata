/* rafaelia/fiber_relmat.c — see fiber_relmat.h for design notes. */

#include <string.h>
#include <stdio.h>

#include "fiber_relmat.h"

void relmat_init(FiberRelMat *m) {
    if (!m) return;
    memset(m->counts, 0, sizeof(m->counts));
}

void relmat_update(FiberRelMat *m, const FiberHash *h) {
    if (!m || !h) return;

    int bits[RELMAT_BITS];
    int n = 0;
    vv_u64 words[4]; words[0] = h->a; words[1] = h->b; words[2] = h->c; words[3] = h->d;
    for (int w = 0; w < 4; w++) {
        vv_u64 v = words[w];
        for (int b = 0; b < 64 && v; b++) {
            if (v & 1ull) bits[n++] = w * 64 + b;
            v >>= 1;
        }
    }

    for (int x = 0; x < n; x++) {
        for (int y = x + 1; y < n; y++) {
            size_t idx = relmat_idx(bits[x], bits[y]);
            if (m->counts[idx] < 0xFFu) m->counts[idx]++;
        }
    }
}

vv_u8 relmat_query(const FiberRelMat *m, int i, int j) {
    if (!m || i < 0 || j < 0 || i >= RELMAT_BITS || j >= RELMAT_BITS || i == j) return 0;
    return m->counts[relmat_idx(i, j)];
}

void relmat_audit(const FiberRelMat *m, int top_n) {
    if (!m || top_n <= 0) return;

    /* Stack-only working copy so found maxima can be zeroed without malloc
     * or mutating the caller's matrix. */
    static vv_u8 work[RELMAT_PAIRS];
    memcpy(work, m->counts, sizeof(work));

    fprintf(stderr, "[FIBER RELMAT AUDIT] top=%d most co-occurring bit pairs\n", top_n);
    for (int k = 0; k < top_n; k++) {
        size_t best_idx = 0;
        vv_u8  best_val = 0;
        for (size_t idx = 0; idx < RELMAT_PAIRS; idx++) {
            if (work[idx] > best_val) { best_val = work[idx]; best_idx = idx; }
        }
        if (best_val == 0) {
            fprintf(stderr, "  (no further non-zero pairs)\n");
            break;
        }
        /* Recover (i,j) from packed index by linear scan over rows. */
        int i = 0;
        size_t row_start = 0, row_len = (size_t)(RELMAT_BITS - 1);
        while (row_start + row_len <= best_idx) {
            row_start += row_len;
            i++;
            row_len--;
        }
        int j = i + 1 + (int)(best_idx - row_start);
        fprintf(stderr, "  [%d] bit(%d,%d) = %u\n", k, i, j, (unsigned)best_val);
        work[best_idx] = 0;
    }
}
