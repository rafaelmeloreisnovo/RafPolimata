#include "RAF_rafaelia_common.h"

/*
 * Método M050: Exportação de resultado em JSON
 * Alvo: Todos
 * Domínio: Benchmark
 * Ganho estimado: automação
 *
 * Saída estruturada.
 *
 * Status: skeleton C — monta um objeto JSON minimalista em buffer estático
 * (sem malloc, sem sprintf/snprintf) a partir de um valor de amostra.
 */

#define M050_BUF_CAP 64

static char _m050_buf[M050_BUF_CAP];

static inline int rafaelia_u32_to_dec_m050(uint32_t v, char *out, int cap) {
    char tmp[10];
    int n = 0;
    if (v == 0) { if (cap < 2) return 0; out[0] = '0'; out[1] = 0; return 1; }
    while (v && n < 10) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    if (n >= cap) return 0;
    for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    out[n] = 0;
    return n;
}

int rafaelia_m050_exportacao_de_resultado_em_json(void) {
    uint32_t sample_value = 42;
    int pos = 0;

    const char *prefix = "{\"metric\":\"rafaelia\",\"value\":";
    while (prefix[pos] && pos < M050_BUF_CAP - 1) { _m050_buf[pos] = prefix[pos]; pos++; }

    char numbuf[10];
    int numlen = rafaelia_u32_to_dec_m050(sample_value, numbuf, (int)sizeof(numbuf));
    for (int i = 0; i < numlen && pos < M050_BUF_CAP - 2; i++) _m050_buf[pos++] = numbuf[i];

    if (pos < M050_BUF_CAP - 1) _m050_buf[pos++] = '}';
    _m050_buf[pos] = 0;

    /* sanity: well-formed JSON object opens with '{' and closes with '}' */
    return (_m050_buf[0] == '{' && _m050_buf[pos - 1] == '}') ? 0 : -1;
}
