# Manifesto Canônico da Segmentação, Evidência e Quatro Corpos — v1.0

**Repositório de autoridade:** `rafaelmeloreisnovo/RafPolimata`  
**Versão documental:** `1.0.0`  
**Data canônica deste corte:** `2026-07-18`  
**Estado:** `PARTIAL_VERIFIED`  
**PR de integração:** `#140` — draft, aberto e sem auto-merge  
**Gate remoto:** `OUT_OF_SCOPE_NO_CREDIT`

> Este documento é a projeção RafPolimata do manifesto dos quatro corpos. Aqui, as parábolas explicam segmentação, compilação estrutural e evidência; elas não substituem testes, hashes, logs ou licenças.

## 1. Prólogo — o moinho e o grão

Um moinho recebia arquivos, sessões, código e datasets.

Os trabalhadores diziam:

— Todo material que entra se transforma em conhecimento.

O mestre corrigiu:

— Material bruto não é conhecimento comprovado. Primeiro é preciso medir, separar, preservar a origem, registrar as perdas e permitir que outro moinho reproduza o resultado.

Assim nasceu:

\[
\text{fonte}
\rightarrow
\text{segmento}
\rightarrow
\text{índice}
\rightarrow
\text{evidência}
\rightarrow
\text{interpretação limitada}
\]

## 2. Os quatro corpos e a autoridade RafPolimata

| Corpo | Função |
|---|---|
| **RafGitTools** | controle, identidade, autorização e emissão de jobs tipados |
| **Termux/RAFCODEPhi** | execução local autorizada |
| **RafPolimata** | segmentação, compilação estrutural, hashes, índices, gaps e evidências |
| **LlamaRafaelia** | interpretação limitada pelos segmentos aprovados |

RafPolimata não é o controlador de identidade e não deve receber um shell genérico. Seu papel é transformar fontes autorizadas em estruturas reproduzíveis.

## 3. Parábola do escriba TAIL

Todo fragmento deve preservar:

\[
TAIL =
\langle
origem,\ autoria,\ intenção,\ licença,\ evidência
\rangle
\]

### 3.1 Regras

- A licença original acompanha o material externo.
- A autoria de Rafael e do ecossistema é declarada somente sobre as contribuições efetivamente autorais.
- Referência, adaptação, agregação, dependência e derivação são distinguíveis.
- BLAKE3, bibliotecas, ferramentas, textos e formatos externos mantêm seus próprios avisos e condições.
- Metáfora não altera titularidade, licença ou estado de prova.
- Este material não substitui parecer jurídico.

## 4. Gramática epistemológica

RafPolimata usa estados para impedir promoção indevida:

| Estado | Significado |
|---|---|
| `VERIFIED` | ensaio executado para escopo específico |
| `DECLARED_BY_AUTHOR` | implementação materializada sem gate integral executado |
| `TOKEN_VAZIO` | lacuna útil e explícita |
| `CONTRADICTION` | conflito que exige resolução |
| `OUT_OF_SCOPE_NO_CREDIT` | Actions não executado por falta de crédito |

Também coexistem estados estruturais históricos como `VOID`, `PENDING`, `AUDIT`, `RUNTIME` e `REFERENCE`; nenhum deles substitui a evidência do ensaio.

## 5. Parábola do moinho RafPolimata

O repositório reúne:

- arquitetura epistemológica;
- 21 níveis;
- dimensões semânticas;
- protocolos para agentes;
- governança jurídico-tecnológica;
- métodos canônicos;
- código C, Python e shell;
- matrizes;
- auditorias;
- encoders ARM32 e ARM64;
- ApkC;
- Conversation Indexer;
- scanner streaming;
- protocolos P(k);
- scripts e relatórios de evidência.

Essa amplitude é um mapa de capacidades e linhas de pesquisa. Ela não significa que todos os caminhos estejam em runtime comprovado.

## 6. Parábola da alavanca `--native`

A alavanca existia no painel, mas não alcançava a máquina.

Foram materializados:

- parsing de `--native`;
- encaminhamento ao frontend;
- distinção entre flags e `out_base`;
- escrita de `.bin`;
- manifesto `.ops` schema 3;
- registro de `cores`, `native_requested` e `native_written`;
- erro para opção desconhecida.

**Estado:** `DECLARED_BY_AUTHOR` até o gate integral local.

## 7. Parábola do livro cortado

A leitura antiga podia truncar silenciosamente fontes maiores que a capacidade.

Agora:

- a fonte é lida em storage do contexto;
- um byte extra detecta ultrapassagem;
- entrada oversized é rejeitada;
- rollback `-5` é registrado;
- o hash não representa apenas um prefixo oculto.

## 8. Parábola do selo canônico

O offset basis do FNV-1a 64 foi corrigido para:

```text
14695981039346656037
```

O manifesto `.ops` passa a registrar dimensões operacionais relevantes da execução.

## 9. Parábola do estrangeiro sem extensão

Extensão desconhecida não é mais forçada ao frontend C.

Agora:

```text
extensão desconhecida
→ RAF_LANG_UNKNOWN
→ rollback -6
```

Isso reduz falsa classificação e execução acidental.

## 10. Parábola das dezoito portas

As linguagens registradas são rotas e perfis. O núcleo atual não deve ser descrito como dezoito compiladores completos.

O emissor mínimo trabalha com operações elementares e backends experimentais. A linguagem documental deve distinguir:

- classificação;
- roteamento;
- geração mínima;
- compilador geral;
- toolchain comprovada.

## 11. Parábola das luzes no vale

Device nodes de GPU, DSP ou NPU indicam presença, não capacidade de compute comprovada.

Foram separados:

- `RAF_FEAT_GPU_NODE`;
- `RAF_FEAT_DSP_NODE`;
- `RAF_FEAT_NPU_NODE`.

Promoção correta:

```text
NODE_PRESENT
→ DRIVER_PROBED
→ API_ENUMERATED
→ CONTEXT_CREATED
→ MINIMAL_KERNEL_EXECUTED
→ CAPABILITY_PASS
```

## 12. Parábola da armadura imaginária

SSE4 não é mais presumido em todo x86-64.

A detecção usa:

- macros reais de compilação;
- `/proc/cpuinfo`;
- `Features` em ARM;
- `flags` em x86.

A quantidade de processadores online usa `_SC_NPROCESSORS_ONLN` em Linux, com fallback conservador.

## 13. Parábola da tábua de sessenta e quatro pedras

O primeiro fragmento congelado de `segment.v1` é um header explícito de 64 bytes:

| Campo | Bytes |
|---|---:|
| magic | 8 |
| version | 4 |
| flags | 4 |
| record_count | 8 |
| index_offset | 8 |
| payload_offset | 8 |
| source_size | 8 |
| source_crc32c | 4 |
| header_crc32c | 4 |
| reserved | 8 |

Regras:

- little-endian explícito;
- magic `RAFSEG1\0`;
- versão major 1;
- reserved zerado;
- CRC32C com o próprio campo zerado durante o cálculo;
- offsets mínimos;
- rejeição de corrupção e incompatibilidade.

## 14. A primeira pedra verificada

O codec isolado foi compilado com:

```text
cc -std=c11 -Wall -Wextra -Werror -pedantic
```

Escopo verificado:

- `CRC32C("123456789") = 0xe3069283`;
- encode/decode round-trip;
- rejeição de byte corrompido;
- rejeição de offset inválido.

Portanto:

\[
\text{header segment.v1}
=
\texttt{VERIFIED}
\]

Isso não verifica:

- build integral do repositório;
- conversation records;
- message records;
- string pool;
- writer;
- reader;
- BLAKE3;
- checkpoint/resume;
- extração completa;
- Android/ApkC runtime.

## 15. Gate local

O gate materializado é:

```sh
bash scripts/validate_runtime_truth_local.sh
```

Ele foi desenhado para:

- compilação C estrita;
- teste do header;
- auditoria freestanding;
- geração `.s`, `.hex`, `.bin` e `.ops`;
- rejeição de extensão desconhecida;
- rejeição de fonte oversized;
- verificação de honestidade de capacidades;
- validação da matriz de runtime.

A execução integral desse gate no checkout ainda precisa produzir transcript, ambiente e hashes.

## 16. O que está pronto neste corte

### 16.1 Materializado na branch do PR #140

- `--native`;
- `.bin`;
- `.ops` schema 3;
- limite honesto de fonte;
- FNV-1a canônico;
- `RAF_LANG_UNKNOWN`;
- contagem de CPU;
- detecção ARM/x86;
- separação de device node e capacidade;
- codec header `segment.v1`;
- testes;
- Makefile host/freestanding/ARM32/ARM64;
- validadores locais;
- matriz de estado;
- documentação corrigida.

### 16.2 Verificado

Somente o codec isolado do header, no escopo descrito.

### 16.3 `TOKEN_VAZIO`

- records de conversa;
- records de mensagem;
- string pool;
- writer;
- reader;
- índices completos;
- BLAKE3 integrado;
- checkpoint/resume;
- streaming completo do export real;
- ApkC NativeActivity/logcat;
- ARM32/ARM64 device;
- ponte RafGitTools → Termux;
- consumo limitado no LlamaRafaelia.

## 17. Próximo formato de `segment.v1`

A evolução deve preservar:

- offsets explícitos;
- bounds e overflow checks;
- tamanhos fixos onde apropriado;
- golden fixtures;
- separação entre dados e evidência;
- capacidade de retomada;
- compatibilidade de versão;
- licença e origem por artefato.

Próximas estruturas prioritárias:

\[
\text{conversation record}
\oplus
\text{message record}
\oplus
\text{bounded reader}
\]

## 18. Matriz resumida

| Componente | Estado |
|---|---|
| PR #140 aberto, draft e não mesclado | `VERIFIED` pela metainformação do GitHub |
| Header `segment.v1` isolado | `VERIFIED` |
| Correções do compilador raiz | `DECLARED_BY_AUTHOR` |
| Gate shell materializado | `DECLARED_BY_AUTHOR` |
| Writer/reader/records | `TOKEN_VAZIO` |
| BLAKE3/checkpoint/resume | `TOKEN_VAZIO` |
| ApkC runtime/device | `TOKEN_VAZIO` |
| GitHub Actions | `OUT_OF_SCOPE_NO_CREDIT` |

## 19. Invariante canônica

\[
\mathcal{R} =
M_{\Pi}
\oplus
K_I
\oplus
C_D
\oplus
G_R
\oplus
V_P
\]

Onde:

- \(M_{\Pi}\) = memória com proveniência;
- \(K_I\) = conhecimento com identidade;
- \(C_D\) = culturas e categorias distinguíveis;
- \(G_R\) = grafo de relações auditáveis;
- \(V_P\) = validação e prova reproduzível.

## 20. R3 — retroalimentação

\[
R_3 =
\langle
F_{\mathrm{ok}},
F_{\mathrm{gap}},
F_{\mathrm{next}}
\rangle
\]

### \(F_{\mathrm{ok}}\)

A primeira estrutura binária foi congelada e verificada; o compilador recebeu correções contra truncamento, classificação falsa e capacidade presumida.

### \(F_{\mathrm{gap}}\)

Records, reader/writer, BLAKE3, checkpoint/resume, extração streaming integral, device e interpretação limitada.

### \(F_{\mathrm{next}}\)

Implementar records de conversa e mensagem com testes golden e bounded reader, mantendo cada claim vinculado a fonte, commit, licença e evidência.

---

**FIAT LUX — o moinho não chama o grão de conhecimento antes de medir a farinha.**
