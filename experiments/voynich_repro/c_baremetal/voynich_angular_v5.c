/*
 * VOYNICH ANGULAR EXTRACTOR v5-audit
 * Freestanding C, no libc/heap. Syscalls are supplied by architecture assembly.
 * Epistemic gate: directionality != hyperlink proof.
 */
typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long      u64;
typedef signed int         i32;
typedef signed long        i64;

extern i64 sys_read(long fd, void *buf, u64 n);
extern i64 sys_write(long fd, const void *buf, u64 n);
extern i64 sys_openat(long dirfd, const char *path, long flags, long mode);
extern i64 sys_close(long fd);

#define AT_FDCWD (-100)
#define O_RDONLY 0
#define IW 256u
#define IH 256u
#define INP (IW * IH)
#define MGH 128u
#define GW 24u
#define GH 28u
#define CHI2_005_DF7_X100 1407u
#define MIN_CHI2_N 40u /* expected count >=5 in each of 8 bins */

typedef struct { u32 x, y; i32 deg; u32 conf; } Hit;

static volatile u8 PIX[INP];
static u8 FBUF[INP + 512u];
static u32 OHIST[256];
static u32 AHIST[8];
static Hit HITS[MGH];
static u8 NB[16];
static u32 PW, PH;

static u64 slen(const char *s) { u64 n = 0; while (s && s[n]) n++; return n; }
static void out(const char *s) { (void)sys_write(1, s, slen(s)); }
static void pn(u32 v) {
    u8 *p = NB + sizeof(NB) - 1u;
    *p = 0;
    if (!v) *--p = '0';
    while (v) { *--p = (u8)('0' + (v % 10u)); v /= 10u; }
    out((const char*)p);
}
static void pi32(i32 v) { if (v < 0) { out("-"); pn((u32)(-v)); } else pn((u32)v); }
static void phex(u32 v) {
    static const char h[] = "0123456789ABCDEF";
    u8 b[8];
    for (i32 i = 7; i >= 0; i--) { b[i] = (u8)h[v & 15u]; v >>= 4; }
    (void)sys_write(1, b, 8);
}
static void pdir(u32 s) {
    /* image coordinates: +x right, +y down, angle grows clockwise */
    static const char d[] = "E  SE S  SW W  NW N  NE ";
    (void)sys_write(1, d + (s & 7u) * 3u, 3);
}
static void fill(volatile u8 *p, u8 value, u32 n) {
    while (n--) *p++ = value;
    __asm__ volatile("" ::: "memory");
}

static u8 otsu_thr(void) {
    u64 total = 0, sum = 0;
    for (u32 i = 0; i < 256u; i++) { total += OHIST[i]; sum += (u64)i * OHIST[i]; }
    if (!total) return 128;
    u64 sum_b = 0, weight_b = 0, best = 0;
    u8 threshold = 0;
    for (u32 i = 0; i < 256u; i++) {
        weight_b += OHIST[i];
        if (!weight_b) continue;
        u64 weight_f = total - weight_b;
        if (!weight_f) break;
        sum_b += (u64)i * OHIST[i];
        u64 mean_b_q16 = (sum_b << 16) / weight_b;
        u64 mean_f_q16 = ((sum - sum_b) << 16) / weight_f;
        u64 delta = mean_b_q16 > mean_f_q16 ? mean_b_q16 - mean_f_q16 : mean_f_q16 - mean_b_q16;
        u64 scaled = (delta >> 8) * (delta >> 8);
        scaled = (scaled >> 8) * (weight_b >> 4) * (weight_f >> 4);
        if (scaled > best) { best = scaled; threshold = (u8)i; }
    }
    return threshold;
}
static u8 compute_otsu(const volatile u8 *pixels, u32 n) {
    for (u32 i = 0; i < 256u; i++) OHIST[i] = 0;
    for (u32 i = 0; i < n; i++) OHIST[pixels[i]]++;
    return otsu_thr();
}

static u32 is_q_candidate(const volatile u8 *im, u32 iw, u32 ih, u32 x, u32 y, u8 th) {
    if (x + GW > iw || y + GH > ih) return 0;
    u32 q00 = 0, q01 = 0, q10 = 0, q11 = 0;
    const u32 half_w = GW / 2u, half_h = GH / 2u;
    for (u32 dy = 0; dy < GH; dy++) {
        for (u32 dx = 0; dx < GW; dx++) {
            u32 dark = im[(y + dy) * iw + x + dx] < th;
            u32 right = dx >= half_w, bottom = dy >= half_h;
            q00 += dark & (1u - right) & (1u - bottom);
            q01 += dark & right & (1u - bottom);
            q10 += dark & (1u - right) & bottom;
            q11 += dark & right & bottom;
        }
    }
    u32 total = q00 + q01 + q10 + q11;
    if (total < 15u) return 0;
    u32 r00 = q00 * 100u / total, r11 = q11 * 100u / total;
    return (r00 > 28u) & (r00 < 66u) & (r11 > 3u) & (r11 < 33u) & (q01 < q00);
}

/* CORDIC vectoring, degrees in image coordinates, integer output [0,359]. */
static i32 atan2_deg_image(i32 y, i32 x) {
    static const i32 atan_q16[16] = {
        2949120, 1740967, 919879, 467587, 234379, 117304, 58666, 29335,
        14668, 7334, 3667, 1833, 917, 458, 229, 115
    };
    if (x == 0 && y == 0) return 0;
    i64 xx = (i64)x << 16, yy = (i64)y << 16;
    i64 angle = 0;
    if (xx < 0) { xx = -xx; yy = -yy; angle = 180LL << 16; }
    for (u32 i = 0; i < 16u; i++) {
        i64 old_x = xx;
        if (yy > 0) {
            xx += yy >> i;
            yy -= old_x >> i;
            angle += atan_q16[i];
        } else if (yy < 0) {
            xx -= yy >> i;
            yy += old_x >> i;
            angle -= atan_q16[i];
        }
    }
    i32 deg = (i32)((angle + (1LL << 15)) >> 16);
    deg %= 360;
    if (deg < 0) deg += 360;
    return deg;
}

static void get_angle(Hit *hit, const volatile u8 *im, u32 iw, u8 th) {
    u32 stem_x = 0, stem_best = 0;
    for (u32 dx = 0; dx < GW; dx++) {
        u32 count = 0;
        for (u32 dy = GH / 4u; dy < (GH * 3u) / 4u; dy++)
            count += im[(hit->y + dy) * iw + hit->x + dx] < th;
        if (count > stem_best) { stem_best = count; stem_x = dx; }
    }

    i64 first_x = 0, last_x = 0;
    i32 first_y = -1, last_y = -1;
    u32 active_rows = 0;
    for (u32 dy = GH / 2u; dy < GH; dy++) {
        i64 sx = 0;
        u32 n = 0;
        for (u32 dx = 0; dx < GW; dx++) {
            i32 delta = (i32)dx - (i32)stem_x;
            if (delta < 0) delta = -delta;
            if (delta <= 1) continue;
            if (im[(hit->y + dy) * iw + hit->x + dx] < th) { sx += (i64)dx << 8; n++; }
        }
        if (!n) continue;
        i64 mean_x = sx / (i64)n;
        if (first_y < 0) { first_y = (i32)dy; first_x = mean_x; }
        last_y = (i32)dy; last_x = mean_x;
        active_rows++;
    }

    if (first_y < 0 || last_y <= first_y) {
        hit->deg = 90;
        hit->conf = stem_best ? 50u : 0u;
        return;
    }
    i32 dx = (i32)(last_x - first_x);
    i32 dy = (last_y - first_y) << 8;
    hit->deg = atan2_deg_image(dy, dx);
    hit->conf = active_rows * 100u / (GH / 2u);
    if (hit->conf > 100u) hit->conf = 100u;
}

static u32 detect(const volatile u8 *im, u32 iw, u32 ih, u8 th) {
    u32 count = 0;
    for (u32 y = 0; y + GH <= ih && count < MGH; y += 4u) {
        for (u32 x = 0; x + GW <= iw && count < MGH; x += 4u) {
            if (!is_q_candidate(im, iw, ih, x, y, th)) continue;
            u32 close = 0;
            for (u32 k = 0; k < count; k++) {
                i32 dx = (i32)HITS[k].x - (i32)x, dy = (i32)HITS[k].y - (i32)y;
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;
                close |= ((u32)dx < GW) & ((u32)dy < GH);
            }
            if (close) continue;
            HITS[count].x = x; HITS[count].y = y;
            get_angle(&HITS[count], im, iw, th);
            count++;
        }
    }
    return count;
}

static u32 crc32c_step(u32 c, u8 b) {
    c ^= b;
    for (u32 i = 0; i < 8u; i++) c = (c >> 1) ^ (0x82F63B78u & (0u - (c & 1u)));
    return c;
}

static u32 chi2x100(u32 n) {
    if (!n) return 0;
    u64 numerator = 0;
    for (u32 i = 0; i < 8u; i++) {
        i64 d = (i64)(8u * AHIST[i]) - (i64)n;
        numerator += (u64)(d * d) * 100u;
    }
    return (u32)(numerator / (8u * (u64)n));
}

static i32 pgm_load(const u8 *raw, u64 raw_n) {
    if (raw_n < 8u || raw[0] != 'P' || raw[1] != '5') return 0;
    u64 p = 2;
#define SKIP_SPACE_AND_COMMENTS() do { \
    for (;;) { \
        while (p < raw_n && raw[p] <= ' ') p++; \
        if (p < raw_n && raw[p] == '#') { while (p < raw_n && raw[p] != '\n') p++; continue; } \
        break; \
    } \
} while (0)
#define READ_U32(dst) do { \
    SKIP_SPACE_AND_COMMENTS(); \
    u32 value = 0, digits = 0; \
    while (p < raw_n && raw[p] >= '0' && raw[p] <= '9') { value = value * 10u + (u32)(raw[p++] - '0'); digits++; } \
    if (!digits) return 0; \
    (dst) = value; \
} while (0)
    u32 w, h, maxv;
    READ_U32(w); READ_U32(h); READ_U32(maxv);
    if (p >= raw_n || raw[p] > ' ') return 0;
    if (raw[p] == '\r' && p + 1u < raw_n && raw[p + 1u] == '\n') p += 2u; else p++;
    if (!w || !h || w > IW || h > IH || maxv == 0u || maxv > 255u) return 0;
    u64 pixels = (u64)w * h;
    if (p + pixels > raw_n) return 0;
    for (u64 i = 0; i < pixels; i++) PIX[i] = raw[p + i];
    PW = w; PH = h;
    return 1;
#undef SKIP_SPACE_AND_COMMENTS
#undef READ_U32
}

static void synth_q(u32 x, u32 y, i32 vx, i32 vy) {
    for (u32 dy = 0; dy < GH / 2u; dy++) {
        for (u32 dx = 0; dx < GW / 2u; dx++) {
            i32 cx = (i32)dx - (i32)GW / 4, cy = (i32)dy - (i32)GH / 4;
            if ((u32)(cx * cx + cy * cy) < (GW / 4u) * (GW / 4u)) PIX[(y + dy) * IW + x + dx] = 0;
        }
    }
    const i32 stem_x = (i32)x + (i32)GW - 9;
    for (u32 dy = 0; dy < (GH * 3u) / 4u; dy++) {
        PIX[(y + dy) * IW + (u32)stem_x] = 0;
        PIX[(y + dy) * IW + (u32)(stem_x + 1)] = 0;
    }
    const i32 start_y = (i32)y + (i32)(GH * 3u) / 4;
    const u32 steps = 7u;
    for (u32 s = 0; s < steps; s++) {
        i32 px = stem_x + (vx * (i32)s) / (i32)(steps - 1u);
        i32 py = start_y + (vy * (i32)s) / (i32)(steps - 1u);
        if (px >= 0 && py >= 0 && px < (i32)IW && py < (i32)IH) {
            PIX[(u32)py * IW + (u32)px] = 0;
            if (px + 1 < (i32)IW) PIX[(u32)py * IW + (u32)(px + 1)] = 0;
        }
    }
}

static void report(u32 n) {
    out("  k   x   y  graus conf\n");
    for (u32 k = 0; k < n; k++) {
        out("  "); pn(k); out("  "); pn(HITS[k].x); out("  "); pn(HITS[k].y); out("  ");
        pi32(HITS[k].deg); out("deg  "); pn(HITS[k].conf); out("%\n");
    }
    for (u32 i = 0; i < 8u; i++) AHIST[i] = 0;
    for (u32 k = 0; k < n; k++) AHIST[((u32)HITS[k].deg % 360u) / 45u & 7u]++;
    out("\nHistograma angular:\n");
    for (u32 i = 0; i < 8u; i++) {
        out("  ["); pdir(i); out("] "); pn(AHIST[i]); out(" ");
        for (u32 b = 0; b < AHIST[i] && b < 30u; b++) out("=");
        out("\n");
    }
    u32 c2 = chi2x100(n);
    out("\nChi2*100="); pn(c2); out("  (critico df=7,p=0.05: 1407)\n");
    if (n < MIN_CHI2_N) out("GATE=TOKEN_VAZIO_SAMPLE_TOO_SMALL\n");
    else if (c2 > CHI2_005_DF7_X100) out("GATE=DIRECTIONALITY_DETECTED_NOT_HYPERLINK_PROOF\n");
    else out("GATE=UNIFORMITY_NOT_REJECTED\n");
    u32 crc = 0xFFFFFFFFu;
    for (u32 k = 0; k < n; k++) {
        crc = crc32c_step(crc, (u8)HITS[k].x);
        crc = crc32c_step(crc, (u8)HITS[k].y);
        crc = crc32c_step(crc, (u8)HITS[k].deg);
    }
    crc ^= 0xFFFFFFFFu;
    out("CRC32c=0x"); phex(crc); out("\nclaim_allowed=false\n");
}

i64 voy_angular_main(i32 argc, char **argv) {
    i32 is_test = argc > 1 && argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] == 't';
    const char *path = argc > 1 && !is_test ? argv[1] : 0;
    if (is_test || !path) {
        out("=== VOYNICH ANGULAR v5 AUDIT ===\n");
        out("Angle-kernel vectors:\n");
        const i32 vx[4] = { 8, 5, 0, -5 };
        const i32 vy[4] = { 3, 7, 8, 6 };
        for (u32 i = 0; i < 4u; i++) { out("  vec="); pi32(vx[i]); out(","); pi32(vy[i]); out(" angle="); pi32(atan2_deg_image(vy[i], vx[i])); out("deg\n"); }
        fill(PIX, 210, INP);
        const u32 xs[4] = { 8, 72, 136, 200 };
        for (u32 i = 0; i < 4u; i++) synth_q(xs[i], 16, vx[i], vy[i]);
        u8 th = compute_otsu(PIX, INP); if (th == 0u) th = 1u;
        out("Otsu="); pn(th); out("\n");
        u32 n = detect(PIX, IW, IH, th);
        out("Detectados="); pn(n); out("\n\n");
        report(n);
        return 0;
    }
    i64 fd = sys_openat(AT_FDCWD, path, O_RDONLY, 0);
    if (fd < 0) { out("ERRO: nao abre arquivo\n"); return 1; }
    i64 n = sys_read(fd, FBUF, sizeof(FBUF));
    (void)sys_close(fd);
    if (n <= 0) { out("ERRO: vazio\n"); return 1; }
    if (!pgm_load(FBUF, (u64)n)) { out("ERRO: requer PGM P5 <=256x256\n"); return 1; }
    out("=== VOYNICH ANGULAR v5 AUDIT ===\nImagem: "); pn(PW); out("x"); pn(PH); out("px\n");
    u8 th = compute_otsu(PIX, PW * PH); if (th == 0u) th = 1u;
    out("Otsu="); pn(th); out("\n");
    u32 found = detect(PIX, PW, PH, th);
    out("Detectados="); pn(found); out("\n\n");
    report(found);
    return 0;
}
