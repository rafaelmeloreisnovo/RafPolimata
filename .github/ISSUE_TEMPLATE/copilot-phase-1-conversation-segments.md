---
name: Copilot Fase 1 — Conversation Segments v1
about: Implementar segmentos determinísticos de conversas, mensagens e timeline
labels: ''
assignees: ''
---

# [Copilot Fase 1] Segmentos determinísticos de conversas, mensagens e timeline v1

## Ordem para o Copilot

Leia integralmente antes de editar:

1. `.github/copilot-instructions.md`
2. `docs/copilot/TASK_02_CONVERSATION_SEGMENTS_V1.md`
3. `docs/copilot/COPILOT_ASSIGNMENT_PHASE_1.md`
4. `docs/RAFAELIA_DATA_INGEST_INDEX_PROTOCOL.md`
5. `include/rafaelia_runtime_protocol.h`
6. todos os arquivos em `runtime/conversation_indexer/`
7. `.github/workflows/conversation-indexer-ci.yml`

Este issue é ordem de execução. Estenda o scanner já validado; não o substitua por DOM parser.

## Objetivo

Gerar, sem carregar o JSON inteiro em memória:

- [ ] `source.manifest.json`
- [ ] `conversations.segment`
- [ ] `messages.segment`
- [ ] `timeline.segment`
- [ ] `audit.jsonl`
- [ ] `checkpoint.state`
- [ ] `coverage_report.json`

## Requisitos obrigatórios

- [ ] parser/event stream bounded;
- [ ] source byte ranges exatos;
- [ ] extração de conversation/message IDs, títulos, roles, content types, parent/child;
- [ ] detecção de duplicate ID, dangling parent, cycle, multiple/no root, unreachable node e null message;
- [ ] timestamps em epoch microseconds assinados, com unknown diferente de zero;
- [ ] evidência temporal sem correção silenciosa;
- [ ] formato little-endian versionado;
- [ ] strings como offset+length;
- [ ] nenhum ponteiro, `size_t` ou enum nativo persistido;
- [ ] BLAKE3-256 real com implementação auditada/pinada e golden vectors;
- [ ] CRC32C somente para integridade rápida;
- [ ] reader/validator separado e sem alocação;
- [ ] checkpoint/resume em safe boundaries;
- [ ] hashes finais iguais para múltiplos chunk sizes e execução retomada.

## Núcleo low-level

- [ ] C;
- [ ] sem `malloc/calloc/realloc/free` no core freestanding;
- [ ] sem libc no core freestanding;
- [ ] sem recursão não limitada;
- [ ] capacidades e overflow checados;
- [ ] erro determinístico por classe e byte/estágio;
- [ ] output parcial nunca marcado `VERIFIED`.

## Testes obrigatórios

- [ ] Clang;
- [ ] GCC;
- [ ] ASan;
- [ ] UBSan;
- [ ] ARM32 freestanding;
- [ ] ARM64 freestanding;
- [ ] undefined-symbol audit;
- [ ] forbidden-allocation/libc audit;
- [ ] chunk size 1 e matriz de split points;
- [ ] strings/escapes/unicode/numbers cruzando chunks;
- [ ] truncamento, depth/capacity limits e malformed JSON;
- [ ] graph anomaly fixtures;
- [ ] corrupt header/sections/ranges/overflow;
- [ ] deterministic artifact hashes;
- [ ] interruption/resume equivalence;
- [ ] fuzz smoke com seed fixo.

## CI e evidência

- [ ] workflow focado atualizado sem enfraquecer gates atuais;
- [ ] artefatos sintéticos pequenos e tools publicados pela CI;
- [ ] nenhum corpus privado na CI;
- [ ] comandos realmente executados;
- [ ] layouts e tamanhos documentados;
- [ ] versão/licença/origem do BLAKE3;
- [ ] memória/limites documentados;
- [ ] hashes dos artefatos;
- [ ] gaps como `TOKEN_VAZIO`.

## Definition of Done

```text
streaming extraction
+ segmentos reais/versionados
+ reader seguro
+ timeline preservada
+ BLAKE3 real
+ resume determinístico
+ CI host/sanitizers/ARM verde
```

Não encerrar com plano, TODO, stubs, arquivos vazios ou falsa conclusão.