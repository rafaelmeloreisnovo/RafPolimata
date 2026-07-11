# Protocolo canônico de coerência, integridade e falsificabilidade

> **Entrada canônica:** `docs/AGENTES.md` §5 descreve o pipeline VOID → VALIDATED e §7 lista
> os gates de CI (G1–G7). Este documento expande os invariantes de coerência (phi, T^7, 42
> atratores), integridade (FNV/CRC32C/Merkle) e falsificabilidade (P(k)).

Este documento traduz as 50 sementes matemático-semânticas do projeto para um contrato operacional auditável. A função dele não é declarar verdade metafísica: é separar **metáfora útil**, **invariante implementável**, **medida observável** e **critério de queda**.

## Núcleo canônico

- **Estado toroidal**: `s = (u, v, psi, chi, rho, delta, sigma)` em `[0,1)^7`.
- **Entrada operacional**: `x = (dados, entropia, hash, estado)`.
- **Memória coerente**: `C` e `H` evoluem por filtro IIR com `alpha = 0.25` (`16384` em Q16.16).
- **Potencial de liberação**: `phi = (1 - H) * C`; quanto maior a coerência e menor a entropia, maior a prioridade de estabilização.
- **Atratores**: o ciclo canônico é limitado a 42 atratores documentais/operacionais.
- **Integridade**: FNV1a/PHI64, CRC32C e cadeia tipo Merkle são trilhas de detecção de alteração; não são, por si só, uma promessa criptográfica completa.
- **Falsificabilidade**: dados observados podem retornar `FAIL`. Esse resultado é conhecimento útil, pois impede que a teoria seja preservada por ajuste posterior não congelado.

## Mapa das 50 sementes para responsabilidade técnica

| Faixa | Responsabilidade | Arquivo canônico |
|---|---|---|
| F01–F10 | Toro `T^7`, vetor de estado, IIR, `phi`, 42 atratores | `Benchmark/raf_toroid.h`, `configs/semantic_coherence.yml` |
| F11–F14 | Observável espectral, ressonância, camadas linguísticas, entropia aproximada | `scripts/first_test_pk.py`, `docs/DEZ_DIMENSOES_SEMANTICAS.md` |
| F15–F17, F30–F33 | Integridade por hash, FNV1a, PHI64, CRC32C, Merkle | `Benchmark/raf_hash.h`, `Benchmark/raf_crc32c.h` |
| F18–F23, F37–F39 | Geometria de espiral, energia angular, recorrência e ciclos | `Benchmark/raf_toroid.h`, este documento |
| F24–F29, F46, F49 | Capacidade de grade, bits geométricos, caminhadas coprimas | `configs/semantic_coherence.yml` |
| F34–F36, F40–F48, F50 | Analogias físicas/linguísticas e integração multidimensional | este documento |

## Regras de interpretação para física, língua e semântica

1. **Física quântica, Hamiltonianos e campos** são tratados como linguagem formal de acoplamento, distância, projeção e energia de ligação. Sem adaptador empírico, permanecem analogias matemáticas, não alegações experimentais.
2. **Linguagens naturais** (português, inglês, chinês, japonês, hebraico, aramaico, grego etc.) entram como camadas `L` com distâncias próprias: fonética, grafia, direção de leitura, cadência, acento, polissemia e tradição interpretativa.
3. **Distâncias podem divergir**: `d_theta(u,v) != d_gamma(u,v)` significa que proximidade sonora, proximidade semântica e proximidade gráfica não precisam concordar.
4. **Criptografia conceitual não é segurança aprovada**: expressões como `k(t)=Q(VFC(t))` e `c_i=p_i xor k(t_i)` só podem virar mecanismo de segurança depois de revisão criptográfica, ameaça formal e testes adversariais.
5. **Sintropia operacional** é o ganho de coerência medido por invariantes que continuam válidos depois de compilação, execução, benchmark e auditoria.

## Gates de prova e queda

A orquestração viva está em `configs/semantic_coherence.yml` e expõe sete gates:

1. `G1`: valida o próprio protocolo canônico e suas 50 fórmulas.
2. `G2`: executa checagem sintática de C no host.
3. `G3`: compila o núcleo do compilador com `-Wall -Wextra -Werror`.
4. `G4`: gera o relatório de falsificabilidade `P(k)` e bloqueia release quando o modelo calibrado/congelado reprova.
5. `G5`: compila e executa validador C nativo do manifesto de excelência operacional.
6. `G6`: compila e executa teste C nativo do seletor runtime com fail-safe, failover e rollback.
7. `G7`: compila e executa benchmark C nativo do roteador com prewarm, warmup, mediana e p95.

## Benchmark mutável com invariantes fixos

Os parâmetros de benchmark podem mudar por arquitetura, mas os invariantes não devem mudar sem revisão:

- zero heap no hot path;
- mediana e percentis reportados;
- flags e alvo registrados;
- hash/CRC preservados como trilha de integridade;
- qualquer mudança de critério deve ser feita antes de observar o resultado.

## Próximo trabalho recomendado

1. Gerar relatórios JSON por gate (`coherence`, `host_syntax`, `compiler_build`, `pk_falsifiability`).
2. Transformar cada fórmula do YAML em teste unitário ou property test quando houver implementação executável.
3. Separar explicitamente modelos empíricos, analogias heurísticas e garantias formais.
4. Adicionar datasets reais versionados por hash para que `P(k)` deixe de depender de amostra sintética.
