#ifndef RAF_CSV_OUT_H
#define RAF_CSV_OUT_H

/*
 * S28 — Gerar CSV/JSON em todo benchmark
 *
 * Utilitário freestanding de geração de CSV sem malloc, sem sprintf,
 * sem dependência de libc além de stdint/stddef.
 * Complementa RAF_050 (JSON) com saída CSV para planilhas/gnuplot.
 *
 * Uso:
 *   raf_csv_ctx_t ctx;
 *   char buf[256];
 *   raf_csv_init(&ctx, buf, sizeof(buf));
 *   raf_csv_header(&ctx, "metric,value_ns,iterations,throughput_ops_s");
 *   raf_csv_row(&ctx, "bus_throughput", 4250, 1000000, 235294117);
 *   raf_csv_row(&ctx, "cache_hit",       120,  100000, 833333333);
 *   // buf now contains the CSV lines
 */

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char    *buf;
    size_t   cap;
    size_t   pos;
} raf_csv_ctx_t;

/* Inicializa o contexto com o buffer fornecido. */
static inline void raf_csv_init(raf_csv_ctx_t *c, char *buf, size_t cap) {
    c->buf = buf;
    c->cap = cap;
    c->pos = 0;
    if (cap > 0) buf[0] = '\0';
}

/* Escreve um caractere no buffer; retorna 0 se OK, -1 se overflow. */
static inline int raf_csv_putc(raf_csv_ctx_t *c, char ch) {
    if (c->pos + 1u >= c->cap) return -1;
    c->buf[c->pos++] = ch;
    c->buf[c->pos]   = '\0';
    return 0;
}

/* Escreve uma string literal no buffer. */
static inline int raf_csv_puts(raf_csv_ctx_t *c, const char *s) {
    while (*s) { if (raf_csv_putc(c, *s++) < 0) return -1; }
    return 0;
}

/* Converte uint64_t para decimal sem sprintf. */
static inline int raf_csv_putu64(raf_csv_ctx_t *c, uint64_t v) {
    char tmp[21];
    int i = 20;
    tmp[i] = '\0';
    if (v == 0u) { tmp[--i] = '0'; }
    else { while (v) { tmp[--i] = (char)('0' + (int)(v % 10u)); v /= 10u; } }
    return raf_csv_puts(c, &tmp[i]);
}

/* Escreve a linha de cabeçalho (string literal com vírgulas). */
static inline int raf_csv_header(raf_csv_ctx_t *c, const char *hdr) {
    int r = raf_csv_puts(c, hdr);
    r |= raf_csv_putc(c, '\n');
    return r;
}

/* Escreve uma linha de dados: metric_name,v0,v1,v2 */
static inline int raf_csv_row(raf_csv_ctx_t *c,
                               const char    *metric,
                               uint64_t       elapsed_ns,
                               uint64_t       iterations,
                               uint64_t       throughput_ops_s) {
    int r = 0;
    r |= raf_csv_puts(c, metric);
    r |= raf_csv_putc(c, ',');
    r |= raf_csv_putu64(c, elapsed_ns);
    r |= raf_csv_putc(c, ',');
    r |= raf_csv_putu64(c, iterations);
    r |= raf_csv_putc(c, ',');
    r |= raf_csv_putu64(c, throughput_ops_s);
    r |= raf_csv_putc(c, '\n');
    return r;
}

/* Retorna a string CSV produzida (sempre terminada em '\0'). */
static inline const char *raf_csv_str(const raf_csv_ctx_t *c) {
    return c->buf;
}

/* Retorna 1 se o buffer não teve overflow, 0 se transbordou. */
static inline int raf_csv_ok(const raf_csv_ctx_t *c) {
    return (c->pos < c->cap) ? 1 : 0;
}

#endif /* RAF_CSV_OUT_H */
