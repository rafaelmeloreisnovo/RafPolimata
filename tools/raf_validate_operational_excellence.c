#include <stdio.h>
#include <string.h>

#define BUF_CAP 262144u
static char buf[BUF_CAP];

static int read_all(const char *path) {
    FILE *f = fopen(path, "rb");
    size_t n;
    if (!f) return 1;
    n = fread(buf, 1u, BUF_CAP - 1u, f);
    fclose(f);
    buf[n] = 0;
    return 0;
}

static int has(const char *s) {
    return strstr(buf, s) != 0;
}

static int require_file_has(const char *path, const char *needle) {
    if (read_all(path)) return 1;
    return has(needle) ? 0 : 2;
}

static int validate_logo_60(void) {
    int in_block = 0;
    int checked = 0;
    unsigned col = 0;
    char prev = 0;
    char *p;
    if (read_all("docs/LOGOTIPO_RAFAELIA_60COL.md")) return 1;
    for (p = buf; *p; p++) {
        if (!in_block && strncmp(p, "```text", 7u) == 0) in_block = 1;
        else if (in_block && strncmp(p, "```", 3u) == 0 && prev == '\n') break;
        if (in_block) {
            if (*p == '\n') {
                if ((prev == '+' || prev == '|') && col != 60u) return 2;
                if (prev == '+' || prev == '|') checked++;
                col = 0u;
            } else if (*p == '+' || *p == '|' || col != 0u) {
                col++;
            }
        }
        prev = *p;
    }
    return checked >= 10 ? 0 : 3;
}

int main(void) {
    if (require_file_has("configs/operational_excellence.yml", "traversal_depth: 5")) return 1;
    if (!has("prewarm_iterations: 16")) return 2;
    if (!has("warmup_iterations: 64")) return 3;
    if (!has("samples: 31")) return 4;
    if (!has("Benchmark/raf_runtime_router_test.c")) return 5;
    if (!has("Benchmark/raf_runtime_benchmark.c")) return 6;
    if (require_file_has("docs/ROTINA_OPERACIONAL_BENCHMARKS.md", "Dados")) return 7;
    if (!has("Qualidade")) return 8;
    if (!has("Segurança operacional")) return 9;
    if (!has("Variação de benchmark")) return 10;
    if (!has("20. Revis")) return 11;
    if (require_file_has("docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md", "morph-on-runtime")) return 12;
    if (!has("TOKEN_VAZIO")) return 13;
    if (validate_logo_60()) return 14;
    if (require_file_has("assets/raf_operational_seal.svg", "RAFAELIA Operational Gate Seal")) return 15;
    if (!has("no certification claim")) return 16;
    if (require_file_has("tools/raf_emit_iconic_seal.c", "FAILOVER | ROLLBACK")) return 17;
    puts("operational_excellence=PASS");
    return 0;
}
