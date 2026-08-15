# Checklist operacional por sessão de agente

> Entrada curta: `AGENTS.md`. Protocolo detalhado: `docs/AGENTES.md`.

Use este checklist de forma proporcional à tarefa. Nem todo gate pertence a todo PR.

## Startup

```text
[ ] 1. Ler AGENTS.md e docs/AGENTES.md.
[ ] 2. Ler README/índice e instruções específicas do caminho alterado.
[ ] 3. Registrar branch, HEAD e working tree.
[ ] 4. Confirmar branch dedicada; não trabalhar em main por padrão.
[ ] 5. Classificar tarefa: code | docs | data | compliance | research.
[ ] 6. Identificar fonte canônica, derivados, invariantes e gates aplicáveis.
[ ] 7. Registrar tool/device/dataset ausente como TOKEN_VAZIO.
```

Comandos de identidade:

```sh
git branch --show-current
git rev-parse HEAD
git status --short
```

## Antes de editar

```text
[ ] Existe AGENTS.md mais próximo do arquivo?
[ ] Existe .github/instructions/*.instructions.md aplicável?
[ ] O texto que vou usar é atual ou histórico?
[ ] Há receipt/closure mais recente que contradiz onboarding antigo?
[ ] O baseline mínimo pode ser executado neste ambiente?
```

## Se tocar ApkC

```text
[ ] Classificar caminho: hosted | freestanding | shared.
[ ] Não aplicar “no libc em todo ApkC” indiscriminadamente.
[ ] Preservar no-libc/no-heap apenas onde o target/gate freestanding exige.
[ ] Validar capacidade/offset/alinhamento antes de alterar buffers/formats.
[ ] Guardar NULL/erro de lookup conforme a API atual.
[ ] Rodar o menor gate atual aplicável.
```

Entrada recomendada para syntax hardened atual:

```sh
make syntax
```

Quando aplicável:

```sh
make compiler-contract
make compiler-selftest
make hotfix-audit
```

Não copie comandos históricos se o Makefile atual já representa o caminho canônico.

## Se tocar linguagem ou encoder

```text
[ ] Verificar tabela/dispatch/artefato/ABI exigidos pelo contrato atual.
[ ] Encoder novo tem implementação + dispatch + golden/roundtrip aplicável.
[ ] Família de equivalência preserva semântica e efeitos colaterais relevantes.
[ ] Nenhum caminho desconhecido virou sucesso silencioso.
```

## Se tocar ELF / DEX / AXML / ZIP / segment / schema

```text
[ ] Avaliar versão e compatibilidade.
[ ] Atualizar todos os cross-references, offsets, contagens e alinhamentos.
[ ] Preservar endianness/tamanho explícito.
[ ] Rodar leitor/validador independente aplicável.
[ ] Corrupção/truncamento continuam rejeitados onde o contrato exige.
```

## Se tocar T^7 / Verbovivo

```text
[ ] Ler docs/closures/CLOSURE_L9_T7_CONVERGENCE.md.
[ ] Não restaurar “42 fixed-point attractors” como claim provado.
[ ] Distinguir índice/range de propriedade dinâmica.
[ ] Se houver claim novo: definição + falsificador + execução + receipt.
```

## Se tocar Android/runtime

```text
[ ] Não colapsar source/ELF/APK/signature/install/launch/runtime.
[ ] Current-commit/current-artifact evidence está explicitamente ligada ao claim?
[ ] Device ausente = TOKEN_VAZIO, não PASS.
```

## Se for documentação

```text
[ ] Distinguir REFERENCE / IMPLEMENTED / PASS / FAIL / TOKEN_VAZIO.
[ ] Não transformar receipt histórico em estado atual.
[ ] Novo documento tem área, lifecycle, responsável lógico e rota de descoberta.
[ ] Não editar docs/generated/ ou results/document-governance/ manualmente.
[ ] Se a mudança for canônica, executar governance check quando disponível.
```

Comandos típicos:

```sh
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --check --print-summary
```

## Durante a execução

```text
[ ] Commits/diffs permanecem no escopo.
[ ] Falha não foi escondida por || true, skip silencioso ou retorno sempre sucesso.
[ ] Resultados negativos/falsificadores foram preservados.
[ ] Segredos não foram reproduzidos em logs/docs.
[ ] Estado desconhecido continua TOKEN_VAZIO.
[ ] Otimização não substituiu referência/golden sem equivalência.
```

## Shutdown

```text
[ ] Código/docs/estado permanecem coerentes.
[ ] Listei apenas comandos realmente executados como execução.
[ ] Cada gate observado tem PASS / FAIL / TOKEN_VAZIO explícito.
[ ] Riscos e rollback estão no handoff/PR.
[ ] PR permanece draft se um gate material para o claim ainda está aberto.
[ ] Conflito entre agentes foi registrado em docs/AGENTES_DECISAO_LOG.md.
[ ] Merge não foi feito sem autorização humana explícita.
```

## Critério de prontidão

Não existe gate universal como P(k), Android físico ou CI completo para todo tipo de mudança.

Um PR está pronto para revisão quando:

1. os gates **aplicáveis ao escopo declarado** foram executados ou marcados `TOKEN_VAZIO` com razão;
2. nenhum claim excede a evidência observada;
3. não há regressão conhecida escondida;
4. documentação/rollback/provenance necessários estão presentes;
5. o humano pode entender claramente o que falta antes de merge.

## Fechamento R3

```text
F_ok   = alterado/executado/demonstrado
F_gap  = não executado, indisponível, contraditório ou pendente
F_next = menor ação reproduzível seguinte
```
