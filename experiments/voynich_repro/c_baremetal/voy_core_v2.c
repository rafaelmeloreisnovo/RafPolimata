/*
  VOY_BARECORE v0.2-audit
  ∆RafaelVerboΩ · RAFAELIA Seed-Forest Core

  Runtime:
  - sem libc
  - sem malloc
  - sem printf
  - sem stdio
  - sem imagemagick/opencv/png/jpg
  - somente _start + syscall + memória estática

  Flags:
    0x01 tokenização
    0x02 famílias
    0x04 transições
    0x08 PGM matriz
    0x10 CRC32
    0x20 lacunas
    default: 0x3F

  Uso:
    ./voy_barecore entrada.txt saida_prefixo 0x3F

  Saídas:
    saida_prefixo.txt
    saida_prefixo.pgm
*/

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef long               isz;
typedef unsigned long      usz;

extern isz sys_read(long fd, void *buf, usz n);
extern isz sys_write(long fd, const void *buf, usz n);
extern isz sys_openat(long dirfd, const char *path, long flags, long mode);
extern isz sys_close(long fd);
extern void sys_exit(long code);

#define AT_FDCWD   (-100)
#define O_RDONLY   0
#define O_WRONLY   1
#define O_CREAT    64
#define O_TRUNC    512
#define MODE_644   0644

#define MAX_IN      (1024u * 1024u)
#define MAX_OUT     (1024u * 1024u)
#define MAX_TOK     4096u
#define MAX_FAM     64u
#define MAX_LAC     256u
#define MAX_NAME    256u
#define INVALID_INDEX 0xFFFFFFFFu

#define F_TOKEN     0x01u
#define F_FAMILY    0x02u
#define F_TRANS     0x04u
#define F_PGM       0x08u
#define F_CRC       0x10u
#define F_LACUNA    0x20u
#define F_DEFAULT   0x3Fu

static u8  IN[MAX_IN + 1u];
static u8  OUT[MAX_OUT];
static u8  PGM[MAX_FAM * MAX_FAM + 128u];
static char NAME_TXT[MAX_NAME];
static char NAME_PGM[MAX_NAME];

typedef struct {
    u32 hash;
    u32 freq;
    u32 first_at;
    u16 len;
    u16 fam;
    u8  a;
    u8  b;
    u8  z;
    u8  flags;
} Token;

typedef struct {
    u32 key;
    u32 freq;
    u32 first_tok;
    u32 score;
    u8  a;
    u8  b;
    u8  z;
    u8  len_bucket;
} Family;

typedef struct {
    u32 code;
    u32 at;
    u32 extra;
} Lacuna;

static Token TOK[MAX_TOK];
static Family FAM[MAX_FAM];
static u32 MAT[MAX_FAM][MAX_FAM];
static Lacuna LAC[MAX_LAC];

static u32 TOK_N;
static u32 FAM_N;
static u32 LAC_N;
static u32 CRC;

static void memzero(void *p, usz n) {
    u8 *x = (u8*)p;
    while (n--) *x++ = 0;
}

static void add_lacuna(u32 code, u32 at, u32 extra) {
    if (LAC_N < MAX_LAC) {
        LAC[LAC_N].code = code;
        LAC[LAC_N].at = at;
        LAC[LAC_N].extra = extra;
        LAC_N++;
    }
}

static u32 is_sep(u8 c) {
    if (c <= 32) return 1;
    if (c == ',' || c == ';' || c == ':' || c == '|' || c == '\t') return 1;
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') return 1;
    if (c == '"' || c == '\'' || c == '<' || c == '>') return 1;
    return 0;
}

static u32 fnv1a_step(u32 h, u8 c) {
    h ^= c;
    h *= 16777619u;
    return h;
}

static u32 crc32_step(u32 crc, u8 b) {
    crc ^= b;
    for (u32 i = 0; i < 8u; i++) {
        u32 mask = 0u - (crc & 1u);
        crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
    return crc;
}

static u32 calc_crc(const u8 *p, usz n) {
    u32 c = 0xFFFFFFFFu;
    for (usz i = 0; i < n; i++) c = crc32_step(c, p[i]);
    return ~c;
}

static u32 fam_key(u8 a, u8 b, u8 z, u16 len) {
    u32 lb = (len > 15u) ? 15u : (u32)len;
    return ((u32)a << 24) ^ ((u32)b << 16) ^ ((u32)z << 8) ^ lb;
}

static u32 find_or_add_family(u8 a, u8 b, u8 z, u16 len, u32 tok_i) {
    u32 k = fam_key(a, b, z, len);
    for (u32 i = 0; i < FAM_N; i++) {
        if (FAM[i].key == k) {
            FAM[i].freq++;
            return i;
        }
    }
    if (FAM_N >= MAX_FAM) {
        add_lacuna(0xF001u, tok_i, k);
        return INVALID_INDEX;
    }
    u32 i = FAM_N++;
    FAM[i].key = k;
    FAM[i].freq = 1;
    FAM[i].first_tok = tok_i;
    FAM[i].score = 0;
    FAM[i].a = a;
    FAM[i].b = b;
    FAM[i].z = z;
    FAM[i].len_bucket = (len > 15u) ? 15u : (u8)len;
    return i;
}

static u32 token_bytes_equal(u32 at, u32 other_at, u16 len) {
    for (u32 j = 0; j < (u32)len; j++) if (IN[at + j] != IN[other_at + j]) return 0;
    return 1;
}

static u32 find_or_add_token(u32 h, u16 len, u8 a, u8 b, u8 z, u32 at) {
    for (u32 i = 0; i < TOK_N; i++) {
        if (TOK[i].hash == h && TOK[i].len == len && TOK[i].a == a && TOK[i].b == b && TOK[i].z == z &&
            token_bytes_equal(TOK[i].first_at, at, len)) {
            TOK[i].freq++;
            return i;
        }
    }
    if (TOK_N >= MAX_TOK) {
        add_lacuna(0x7001u, at, h);
        return INVALID_INDEX;
    }
    u32 i = TOK_N++;
    TOK[i].hash = h;
    TOK[i].freq = 1;
    TOK[i].first_at = at;
    TOK[i].len = len;
    TOK[i].fam = 0;
    TOK[i].a = a;
    TOK[i].b = b;
    TOK[i].z = z;
    TOK[i].flags = 0;
    return i;
}

static void tokenize_and_family(const u8 *p, usz n, u32 flags) {
    usz i = 0;
    u32 prev_fam = INVALID_INDEX;

    while (i < n) {
        while (i < n && is_sep(p[i])) i++;
        if (i >= n) break;

        u32 h = 2166136261u;
        u16 len = 0;
        u8 a = 0, b = 0, z = 0;
        usz start = i;

        while (i < n && !is_sep(p[i])) {
            u8 c = p[i];
            if (len == 0) a = c;
            if (len == 1) b = c;
            z = c;
            h = fnv1a_step(h, c);
            if (len < 65535u) len++;
            i++;
        }

        if (len == 0) continue;
        if (len == 1) b = 0;

        u32 ti = find_or_add_token(h, len, a, b, z, (u32)start);
        if (ti == INVALID_INDEX) { prev_fam = INVALID_INDEX; continue; }

        if (flags & F_FAMILY) {
            u32 fi = find_or_add_family(a, b, z, len, ti);
            if (fi == INVALID_INDEX) { prev_fam = INVALID_INDEX; continue; }
            TOK[ti].fam = (u16)fi;

            if ((flags & F_TRANS) && prev_fam != INVALID_INDEX && prev_fam < MAX_FAM && fi < MAX_FAM) {
                MAT[prev_fam][fi]++;
            }
            prev_fam = fi;
        }
    }
}

static usz out_ch(usz o, u8 c) {
    if (o < MAX_OUT) OUT[o++] = c;
    return o;
}

static usz out_str(usz o, const char *s) {
    while (*s && o < MAX_OUT) OUT[o++] = (u8)*s++;
    return o;
}

static usz out_dec(usz o, u64 v) {
    char tmp[32];
    u32 n = 0;
    if (v == 0) return out_ch(o, '0');
    while (v && n < 32) {
        tmp[n++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (n) o = out_ch(o, (u8)tmp[--n]);
    return o;
}

static usz out_hex32(usz o, u32 v) {
    static const char H[] = "0123456789ABCDEF";
    o = out_str(o, "0x");
    for (int i = 7; i >= 0; i--) o = out_ch(o, (u8)H[(v >> (i * 4)) & 15u]);
    return o;
}

static usz out_nl(usz o) { return out_ch(o, '\n'); }

static usz emit_report(usz o, usz input_n, u32 flags) {
    o = out_str(o, "VOY_BARECORE_REPORT\n");
    o = out_str(o, "raiz=entrada_crua\n");
    o = out_str(o, "politica_lacuna=preservar_sem_fake_fill\n");
    o = out_str(o, "flags="); o = out_hex32(o, flags); o = out_nl(o);
    o = out_str(o, "input_bytes="); o = out_dec(o, input_n); o = out_nl(o);
    o = out_str(o, "tokens_unicos="); o = out_dec(o, TOK_N); o = out_nl(o);
    o = out_str(o, "familias="); o = out_dec(o, FAM_N); o = out_nl(o);
    o = out_str(o, "lacunas="); o = out_dec(o, LAC_N); o = out_nl(o);
    o = out_str(o, "crc32="); o = out_hex32(o, CRC); o = out_nl(o);

    o = out_nl(o);
    o = out_str(o, "[FAMILIAS]\n");
    o = out_str(o, "idx,key,freq,a,b,z,len_bucket,first_token\n");
    for (u32 i = 0; i < FAM_N; i++) {
        o = out_dec(o, i); o = out_ch(o, ',');
        o = out_hex32(o, FAM[i].key); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].freq); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].a); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].b); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].z); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].len_bucket); o = out_ch(o, ',');
        o = out_dec(o, FAM[i].first_tok); o = out_nl(o);
    }

    o = out_nl(o);
    o = out_str(o, "[TOKENS_FIRST_256]\n");
    o = out_str(o, "idx,hash,freq,first_at,len,fam,a,b,z\n");
    for (u32 i = 0; i < TOK_N && i < 256u; i++) {
        o = out_dec(o, i); o = out_ch(o, ',');
        o = out_hex32(o, TOK[i].hash); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].freq); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].first_at); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].len); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].fam); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].a); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].b); o = out_ch(o, ',');
        o = out_dec(o, TOK[i].z); o = out_nl(o);
    }

    if (flags & F_LACUNA) {
        o = out_nl(o);
        o = out_str(o, "[LACUNAS]\n");
        o = out_str(o, "idx,code,at,extra\n");
        for (u32 i = 0; i < LAC_N; i++) {
            o = out_dec(o, i); o = out_ch(o, ',');
            o = out_hex32(o, LAC[i].code); o = out_ch(o, ',');
            o = out_dec(o, LAC[i].at); o = out_ch(o, ',');
            o = out_hex32(o, LAC[i].extra); o = out_nl(o);
        }
    }
    return o;
}

static usz pgm_str(usz o, const char *s) {
    while (*s) PGM[o++] = (u8)*s++;
    return o;
}

static usz pgm_dec(usz o, u32 v) {
    char tmp[16];
    u32 n = 0;
    if (v == 0) { PGM[o++] = '0'; return o; }
    while (v && n < 16) { tmp[n++] = (char)('0' + (v % 10u)); v /= 10u; }
    while (n) PGM[o++] = (u8)tmp[--n];
    return o;
}

static usz emit_pgm(void) {
    u32 w = FAM_N ? FAM_N : 1u;
    u32 h = w;
    u32 maxv = 0;
    usz o = 0;
    if (w > MAX_FAM) w = MAX_FAM;
    if (h > MAX_FAM) h = MAX_FAM;
    for (u32 y = 0; y < h; y++) for (u32 x = 0; x < w; x++) if (MAT[y][x] > maxv) maxv = MAT[y][x];
    o = pgm_str(o, "P5\n");
    o = pgm_dec(o, w); PGM[o++] = ' '; o = pgm_dec(o, h); PGM[o++] = '\n'; o = pgm_str(o, "255\n");
    for (u32 y = 0; y < h; y++) {
        for (u32 x = 0; x < w; x++) {
            u32 v = MAT[y][x];
            u32 pix = maxv ? 255u - ((v * 255u) / maxv) : 255u;
            PGM[o++] = (u8)pix;
        }
    }
    return o;
}

static void make_name(char *dst, const char *pre, const char *ext) {
    usz i = 0, j = 0;
    while (pre && pre[i] && i < (MAX_NAME - 8u)) { dst[i] = pre[i]; i++; }
    while (ext[j] && i < (MAX_NAME - 1u)) dst[i++] = ext[j++];
    dst[i] = 0;
}

static u32 parse_hex_or_dec(const char *s) {
    u32 v = 0, base = 10;
    usz i = 0;
    if (!s) return F_DEFAULT;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; i = 2; }
    for (; s[i]; i++) {
        u8 c = (u8)s[i];
        u32 d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = 10u + c - 'a';
        else if (c >= 'A' && c <= 'F') d = 10u + c - 'A';
        else break;
        if (d >= base) break;
        v = v * base + d;
    }
    return v;
}

static long write_file(const char *name, const void *buf, usz n) {
    long fd = sys_openat(AT_FDCWD, name, O_WRONLY | O_CREAT | O_TRUNC, MODE_644);
    if (fd < 0) return fd;
    usz off = 0;
    while (off < n) {
        isz w = sys_write(fd, (const u8*)buf + off, n - off);
        if (w <= 0) break;
        off += (usz)w;
    }
    sys_close(fd);
    return (off == n) ? 0 : -1;
}

long voy_core(long argc, char **argv) {
    usz n = 0;
    u32 flags = F_DEFAULT;
    memzero(TOK, sizeof(TOK));
    memzero(FAM, sizeof(FAM));
    memzero(MAT, sizeof(MAT));
    memzero(LAC, sizeof(LAC));

    if (argc < 3) {
        const char msg[] = "uso: ./voy_barecore entrada.txt saida_prefixo [flags]\nflags: 0x01 token 0x02 familia 0x04 trans 0x08 pgm 0x10 crc 0x20 lacuna\n";
        sys_write(2, msg, sizeof(msg) - 1);
        return 2;
    }
    if (argc >= 4) flags = parse_hex_or_dec(argv[3]);

    long fd = sys_openat(AT_FDCWD, argv[1], O_RDONLY, 0);
    if (fd < 0) {
        const char msg[] = "erro: nao abriu entrada\n";
        sys_write(2, msg, sizeof(msg) - 1);
        return 3;
    }
    while (n < MAX_IN) {
        isz r = sys_read(fd, IN + n, MAX_IN - n);
        if (r < 0) { sys_close(fd); return 4; }
        if (r == 0) break;
        n += (usz)r;
    }
    if (n == MAX_IN) {
        isz extra = sys_read(fd, IN + MAX_IN, 1u);
        if (extra < 0) { sys_close(fd); return 4; }
        if (extra > 0) add_lacuna(0x1A11u, (u32)n, MAX_IN);
    }
    sys_close(fd);

    CRC = (flags & F_CRC) ? calc_crc(IN, n) : 0;
    if (flags & F_TOKEN) tokenize_and_family(IN, n, flags);

    make_name(NAME_TXT, argv[2], ".txt");
    usz out_n = emit_report(0, n, flags);
    if (write_file(NAME_TXT, OUT, out_n) < 0) return 5;

    if (flags & F_PGM) {
        make_name(NAME_PGM, argv[2], ".pgm");
        usz pgm_n = emit_pgm();
        if (write_file(NAME_PGM, PGM, pgm_n) < 0) return 6;
    }
    return 0;
}
