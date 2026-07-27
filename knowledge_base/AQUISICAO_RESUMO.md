# AQUISICAO_RESUMO — Science Learning Engine

Gerado: 2026-07-27T18:27:00.303059+00:00

## Totais por domínio e estágio

| Domínio | Estágio 1 | Estágio 2 | Estágio 3 | Estágio 4 | Estado |
|---------|----------:|----------:|----------:|----------:|--------|
| physics | 80 | 41 | 11 | 0 | TOKEN_VAZIO |
| chemistry | 80 | 38 | 11 | 0 | TOKEN_VAZIO |
| biology | 80 | 45 | 18 | 0 | TOKEN_VAZIO |
| mathematics | 80 | 45 | 15 | 0 | TOKEN_VAZIO |

## DOIs canônicos (estágio 4)

_Nenhum registro atingiu o estágio 4 nesta execução._

## Instrução de uso — verbovivo/vv_scan_buf()

Os textos em `stage_4_canonical/synthesis.txt` são compatíveis com `vv_scan_buf()`:

```bash
# Compilar verbovivo:
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \
    rafaelia/verbovivo.c rafaelia/fiber_relmat.c -lm -o verbovivo

# Alimentar com sínteses canônicas:
for domain in physics chemistry biology mathematics; do
    f="knowledge_base/${domain}/stage_4_canonical/synthesis.txt"
    [ -f "$f" ] && cat "$f" | ./verbovivo -s > "engram_${domain}.svg" || true
done
```

## Referência estrutural

> **Entrada canônica:** `docs/AGENTES.md §7` (governança documental) e `§6` (excelência operacional).  
> Script: `scripts/science_learning_engine.py`  
> Documentação: `docs/SCIENCE_LEARNING_ENGINE.md`
