# Mapa estrutural do repositório RafPolimata

Este documento organiza a leitura técnica do repositório em até **5 níveis de profundidade**, seguindo o princípio de que um `token vazio` é melhor do que uma alegação falsa: quando uma pasta ou arquivo não tem função operacional comprovada, ele deve aparecer como `VOID`, `PENDING` ou `AUDIT` até receber teste, documentação e dono.

## Estados canônicos de uso

| Estado | Uso | Critério de saída |
|---|---|---|
| `VOID` | Conteúdo ausente, diretório vazio ou referência sem artefato. | Criar artefato mínimo ou remover em mudança dedicada. |
| `PENDING` | Conteúdo existe, mas ainda sem gate automatizado suficiente. | Adicionar teste, manifesto ou validação documental. |
| `AUDIT` | Conteúdo é evidência, relatório ou matriz de rastreio. | Manter reprodutibilidade e data/hash quando aplicável. |
| `RUNTIME` | Código executável, cabeçalho, script ou rota operacional. | Compilar/executar em CI ou teste local. |
| `REFERENCE` | Documentação, especificação, roteiro ou explicação conceitual. | Linkar a uma rotina, gate ou arquivo de implementação. |

## Disposição por tags/diretórios de raiz

| Tag estrutural | Caminho | Conteúdo | Uso recomendado | Estado |
|---|---|---|---|---|
| `ci` | `.github/workflows/` | Pipeline GitHub Actions. | Reproduzir gates de coerência, C host, manifesto operacional e relatório P(k). | `RUNTIME` |
| `apk-android` | `Apkc/` | Protótipos C/ASM e formatos APK/DEX/ELF/ZIP. | Base de baixo nível para Android; manter sem heap quando possível e com fallback por ABI. | `PENDING` |
| `benchmark` | `Benchmark/` | Router de runtime, benchmark, tipos, arena e primitivas. | Medir latência, fallback, failover e seleção de backend. | `RUNTIME` |
| `configs` | `configs/` | Manifestos YAML de coerência e excelência operacional. | Fonte de verdade para gates, estados, arquiteturas e limites. | `AUDIT` |
| `data` | `data/` | Dados observados para falsificabilidade. | Entrada controlada para P(k), anomalias e relatórios. | `AUDIT` |
| `docs` | `docs/` | Documentação técnica, jurídica, operacional e semântica. | Explicar conceitos e vincular cada metáfora a prova operacional. | `REFERENCE` |
| `results` | `results/` | Saídas reprodutíveis de testes. | Evidência gerada por scripts; não usar como fonte única sem comando. | `AUDIT` |
| `scripts` | `scripts/` | Validações sem dependências externas e planos de build. | Automatizar fail-safe/failover/rollback e auditoria de estrutura. | `RUNTIME` |
| `tools` | `tools/` | Utilitários C de validação. | Compilar com `-Wall -Wextra -Werror` e usar em gates. | `RUNTIME` |
| `root-methods` | `RAF_001_*.c` … `RAF_056_*.c` | 56 métodos C de baixo nível. | Catálogo operacional de técnicas bare-metal/registrador/syscall/Android. | `PENDING` |
| `root-compiler` | `raf_*.c`, `raf_*.h`, `raiz_*` | Compilador/otimizador raiz e exemplos/auditorias geradas. | Núcleo compilável do projeto; preservar smoke test antes de refatorar. | `RUNTIME` |

## Diferenças entre tags de conteúdo

- **`REFERENCE` não prova execução**: documentação deve apontar para `scripts/`, `configs/`, `Benchmark/` ou arquivos C quando declarar comportamento verificável.
- **`AUDIT` não substitui teste**: JSON/CSV em `results/` e `data/` registra evidência, mas o comando reprodutível precisa continuar documentado.
- **`RUNTIME` precisa fallback**: rotas SIMD, ARM, syscall direta, GPU ou Android devem manter caminho `generic_c`/baseline para fail-safe.
- **`PENDING` é honesto**: arquivos conceituais ou métodos ainda não conectados à CI ficam marcados sem prometer performance, segurança ou completude.
- **`VOID` deve ser rastreado**: diretório vazio, link quebrado ou arquivo solto deve ser detectado por auditoria antes de virar dívida invisível.

## Varredura de estrutura e dados

A auditoria canônica é:

```sh
python3 scripts/audit_repository_structure.py --depth 5
```

Ela verifica, de forma determinística e sem dependências externas:

1. quantidade de diretórios e arquivos visíveis até o nível configurado;
2. diretórios vazios (`VOID` estrutural);
3. arquivos de raiz fora dos padrões conhecidos;
4. diretórios esperados ausentes;
5. links Markdown quebrados em `README.md` e `docs/*.md`;
6. divergência informativa entre `RAF_INDEX.md` e a disposição atual dos 56 métodos `RAF_###_*.c` na raiz.

## Política de refatoração profissional

1. **Mapear antes de mover**: qualquer reorganização de arquivos deve atualizar README, índices, CI e referências internas no mesmo commit.
2. **Sem ocultar falhas**: testes com falha devem aparecer como `FAIL` ou `SKIPPED` com motivo explícito.
3. **Fallback primeiro**: otimizações branchless/SIMD/syscall/bare-metal só podem substituir baseline depois de comparação e rollback.
4. **Reduzir fricção sem remover semântica**: eliminar loops, símbolos ou condicionais apenas quando a equivalência estiver testada.
5. **Conteúdo multidisciplinar com prova**: metáforas, fórmulas e camadas semânticas devem ser ligadas a invariantes, dados ou limites de escopo.
