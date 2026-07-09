# Rotina operacional de auditoria e benchmark estrutural

> **Entrada canônica:** `docs/AGENTES.md` §5 resume o pipeline de excelência
> operacional (VOID → BASELINE → CANDIDATE → VALIDATED → ROLLBACK) e o critério
> de parada da rotina de auditoria. Este documento detalha as condições e sequências
> de benchmark estrutural.

Este guia descreve como o agente deve trabalhar: conferir arquivos, validar dados, medir caminhos de execução e só promover uma otimização quando a evidência for melhor que a alternativa simples. Ele não declara certificação, conformidade formal ou selo normativo do software.

## Rotina de auditoria obrigatória do agente

1. **Dados**: entrada versionada, formato declarado, hash/reprodutibilidade e ausência de preenchimento falso.
2. **Qualidade**: requisito, execução, evidência, não conformidade e ação corretiva documentada.
3. **Segurança operacional**: menor privilégio, integridade, rollback, trilha de comando e não exposição de segredo.
4. **Variação de benchmark**: prewarm, warmup, mediana, p95, p99 e análise de regressão antes de promover.
5. **TOKEN_VAZIO**: quando falta evidência, registrar `VOID`/`SKIPPED`; nunca inventar PASS.

## Condições estruturais de benchmark

| Etapa | Regra | Motivo |
|---|---|---|
| prewarm | executar rota sem medir | estabilizar cache/instruções |
| warmup | repetir antes das amostras úteis | reduzir primeira execução fria |
| amostragem | usar lote fixo e estático | evitar malloc/GC/ruído |
| estatística | mediana + p95/p99 | reduzir outliers e enxergar cauda |
| promoção | comparar com baseline | impedir otimização que piora |
| rollback | retornar para `generic_c` | preservar operação segura |

## Top 20 modelos/aplicações de execução

1. Baseline `generic_c` portável.
2. Branchless inteiro.
3. ARM32 NEON.
4. ARM64 NEON.
5. GPU batch.
6. Syscall direta controlada.
7. Storage/buffer binário.
8. Cache L1/L2 locality.
9. Ring buffer sem heap.
10. Batching de operações repetidas.
11. Mediana com prewarm/warmup.
12. p95/p99 de latência.
13. Fail-safe para baseline.
14. Failover por backend degradado.
15. Rollback por estado `VOID`/`ROLLBACK`.
16. Mitigação por redução de lote.
17. Hash/integridade do relatório.
18. Falsificabilidade P(k).
19. Manifesto operacional auditável.
20. Revisão de risco operacional antes de release.

## Critério de parada

A rotina para somente quando os gates bloqueantes passam, o relatório P(k) está em PASS, o roteador runtime tem fail-safe/failover/rollback testado e não há mudança não auditada no índice de arquivos críticos.
