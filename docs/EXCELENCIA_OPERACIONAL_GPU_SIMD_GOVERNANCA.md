# Excelência operacional GPU/SIMD, governança e execução sem gargalos

> **Resumo executivo em `docs/AGENTES.md` §5** — o pipeline de promoção
> (VOID → BASELINE → CANDIDATE → VALIDATED → ROLLBACK) e a matriz de decisão
> por arquitetura aparecem condensados lá. Este documento contém a metodologia
> completa com todas as tabelas, regras de governança e critério enterprise.

Este documento transforma as sementes conceituais do RafPolimata em uma metodologia operacional auditável para execução em CPU, GPU, NEON/SIMD, cache L1/L2, buffer/RAM/storage e pipelines paralelos. A regra principal é separar metáfora, hipótese e prova: quando não houver medição, o estado correto é `VOID`/`SKIPPED`, não uma promessa de desempenho.

## 1. Objetivo

Criar uma rota técnica para escolher o melhor caminho de execução por capacidade efetiva, com:

- governança explícita de decisão;
- documentação verificável por artefatos;
- código com hot paths pequenos, previsíveis e mensuráveis;
- planos de fail-safe, failover, rollback e mitigação;
- uso de branchless/SIMD/GPU apenas quando a medição demonstrar benefício real.

## 2. Mapa de cinco níveis de profundidade

A leitura operacional do repositório deve cobrir até cinco níveis, sem presumir que todos existam:

| Nível | Escopo | Evidência esperada |
|---|---|---|
| L0 | raiz do repositório | `README.md`, índice, scripts, fontes C principais |
| L1 | domínios diretos | `docs/`, `configs/`, `scripts/`, `Benchmark/`, `Apkc/`, `results/` |
| L2 | protocolos e módulos | YAML canônico, validadores, headers, benchmarks |
| L3 | artefatos gerados | relatórios JSON, objetos de build, logs temporários |
| L4 | integrações futuras | Android/Termux, NDK, QEMU, hardware real |
| L5 | evidência externa controlada | captura de hardware, perf counters, revisão jurídica/técnica |

## 3. Matriz de decisão por arquitetura

| Caminho | Quando usar | Quando rejeitar | Métrica mínima |
|---|---|---|---|
| Genérico C | baseline, portabilidade, auditoria | quando virar gargalo comprovado | correção + latência média |
| Branchless inteiro | decisões pequenas e previsíveis | se aumentar instruções/cache miss | p95/p99 e tamanho |
| ARM32 NEON | vetores curtos no Android legado | se custo de empacotar/desempacotar for maior | ciclos por elemento |
| ARM64 NEON | hot path vetorial estável | se dados não forem contíguos/alinháveis | throughput e energia |
| GPU | lote grande e paralelismo massivo | se transferência RAM/GPU dominar | tempo total fim-a-fim |
| Syscall direta | caminho mínimo e controlado | se ABI/segurança/manutenção piorar | latência + fallback libc |
| Storage/buffer | logs binários e batching | se comprometer integridade | perda zero + checksum |

## 4. Pipeline de execução sem fricção

1. **Classificar estado**: `VOID`, `BASELINE`, `CANDIDATE`, `VALIDATED` ou `ROLLBACK`.
2. **Medir baseline** em C portável antes de otimizar.
3. **Isolar hot path** com entradas determinísticas e sem alocação dinâmica no trecho crítico.
4. **Aplicar especialização**: branchless, NEON, GPU ou batching.
5. **Comparar contra baseline** com hash, p95, p99, tamanho e erro absoluto/relativo.
6. **Ativar morph-on-runtime** apenas se o seletor tiver fallback seguro e registro de decisão.
7. **Promover** somente quando os gates bloqueantes passarem.

## 5. Regras de governança técnica

- `TOKEN_VAZIO` é um estado válido: melhor registrar ausência de evidência do que fabricar conclusão.
- Toda otimização precisa declarar hipótese, métrica, baseline e rollback.
- SIMD/GPU não são objetivos; são rotas condicionais por evidência.
- Nenhum caminho criptográfico conceitual deve ser anunciado como seguro sem revisão específica.
- A documentação deve distinguir analogia física/linguística de afirmação empírica.

## 6. Fail-safe, failover, rollback e mitigação

| Mecanismo | Ação operacional | Artefato |
|---|---|---|
| Fail-safe | retornar ao baseline correto e auditado | gate bloqueante + log |
| Failover | trocar backend em runtime quando ABI/recurso falhar | seletor de arquitetura |
| Rollback | desativar módulo candidato e restaurar versão validada | changelog + tag |
| Mitigação | limitar lote, reduzir paralelismo, preservar integridade | relatório de risco |

## 7. Critério de pronto enterprise

Um módulo só deve ser tratado como enterprise-ready quando tiver:

- contrato de entrada/saída;
- baseline reprodutível;
- teste de equivalência;
- teste de degradação/falha;
- matriz de arquitetura preenchida;
- decisão de governança documentada;
- plano de rollback acionável.


## 8. Seletor runtime e morph-on-runtime

O seletor runtime implementado em `Benchmark/raf_runtime_router.h` é deliberadamente pequeno: recebe capacidades detectadas, recursos degradados, tamanho de lote e estado de governança; devolve backend, fallback e flags de fail-safe/failover/rollback/mitigação. O fallback obrigatório é `generic_c`, para que GPU/NEON/syscall/storage sejam acelerações condicionais e nunca dependência única.

A ordem operacional é: `generic_c` como baseline, `arm32_neon`/`arm64_neon` quando SIMD estiver disponível e validado, `gpu_batch` somente quando o lote superar o limiar de transferência, e retorno automático ao baseline quando o estado for `VOID` ou `ROLLBACK`.

## 9. Testes automáticos de rota

O teste `Benchmark/raf_runtime_router_test.c` compila um programa C mínimo, sem heap e sem dependências externas, cobrindo baseline, seleção ARM64 NEON, promoção GPU por lote, failover de GPU degradada para ARM64 e rollback para `generic_c`.


## 10. Selo local dos gates

O arquivo `assets/raf_operational_seal.svg` é um selo visual do estado local dos gates (`PASS`, `FAILSAFE`, `FAILOVER`, `ROLLBACK`). Ele não declara certificação externa; apenas resume evidência local produzida pelos testes versionados.
