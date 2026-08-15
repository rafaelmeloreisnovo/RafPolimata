# AGENTES — Guia Operacional Unificado

> Protocolo canônico detalhado para agentes humanos e IA no RafPolimata.

A entrada curta e interoperável é `AGENTS.md` na raiz. Este documento contém o protocolo detalhado. A navegação documental começa em `docs/INDEX.md`; a governança de arquivos está em `docs/DOCUMENT_GOVERNANCE.md`.

## 0. Precedência e compatibilidade entre agentes

### 0.1 Precedência operacional

Quando houver várias instruções no repositório, aplicar nesta ordem:

1. instrução explícita do humano para a tarefa atual;
2. `AGENTS.md` mais próximo do arquivo alterado, quando existir;
3. `AGENTS.md` da raiz;
4. este `docs/AGENTES.md`;
5. adaptadores específicos de ferramenta, somente de forma aditiva.

Um adaptador não pode enfraquecer evidência, segurança, provenance ou gates definidos aqui.

### 0.2 Adaptadores reconhecidos

| Superfície | Arquivo de entrada | Regra |
|---|---|---|
| OpenAI Codex | `AGENTS.md` | roteador principal; respeitar `AGENTS.md` mais próximo |
| GitHub Copilot | `.github/copilot-instructions.md` + `.github/instructions/*.instructions.md` | instruções repo-wide + path-specific |
| Claude Code | `CLAUDE.md` | adaptador curto; remete a `AGENTS.md` e a este protocolo |
| ChatGPT com contexto GitHub | `AGENTS.md` + este documento | contrato documental do repositório; não presume carregamento automático fora do contexto fornecido |
| Humano | `README.md`, `AGENTS.md`, `docs/INDEX.md` | autoridade final de merge/exceção |

Ferramenta não define autoridade epistemológica. Evidência material prevalece sobre texto de onboarding.

## 1. Invariante epistemológica

```text
conceito != implementação != execução != evidência != validação runtime != claim externo
```

Consequências:

- arquivo existente não é `PASS`;
- workflow YAML existente não prova que um job executou;
- commit ou PR merged não prova runtime físico;
- receipt histórico não prova o commit atual;
- hash prova identidade/integridade do byte observado, não verdade semântica;
- resultado sintético não valida automaticamente dado real;
- índice numérico não é automaticamente um atrator dinâmico;
- ausência de evidência é `TOKEN_VAZIO`, não zero, sucesso ou fracasso inferido.

## 2. Arquitetura essencial

| Camada | Arquivo principal | Entrada |
|---|---|---|
| Pipeline de alto nível | `raf_compile.h` | `raf_compile_file()` |
| Micro-toolchain Android | `Apkc/apkc.c` | `apkc_main()` |
| Motor T^7 / Verbovivo | `rafaelia/verbovivo.c` | `verbovivo_main()` |
| Runtime router | `Benchmark/raf_runtime_router.h` | seleção por capacidade |
| Conversation indexer | `runtime/conversation_indexer/` | codecs/parsers versionados |
| Governança documental | `scripts/document_governance.py` | catálogo/grafo/fila |
| Estado material | `ECOSYSTEM_RUNTIME_STATE.json` | snapshot versionado, não substitui verificação do HEAD |

## 3. Invariantes técnicas

1. Rotas declaradas freestanding devem continuar sem heap/libc proibidos conforme o gate daquela rota.
2. Não usar a regra obsoleta “zero libc em todo `Apkc/`”. Caminhos hosted de desenvolvimento podem declarar libc; isso não relaxa ARM/freestanding.
3. Perfil de linguagem pertence à tabela canônica de `Apkc/lang_profile.h` quando o pipeline aplicável usa essa tabela.
4. Encoder ARM exige implementação, dispatch e teste golden/roundtrip aplicável.
5. Retorno NULL/erro de lookup é caminho normal e deve ser tratado explicitamente.
6. Limite de buffer é contrato; truncamento silencioso é falha.
7. Alteração de layout binário exige cross-reference, versionamento e validador independente.
8. Código, documento, teste e estado precisam permanecer semanticamente coerentes.
9. Nenhum arquivo é movido/apagado por “organização” sem provenance, impacto e rollback.
10. Segredo detectado não é reproduzido em relatório.
11. Merge é decisão humana explícita; agente pode preparar branch/PR, não promover sozinho.
12. Saída gerada não é fonte autoral: corrigir gerador/política e regenerar.

## 4. Correções de verdade que todo agente deve conhecer

### 4.1 T^7 e o número 42

Não afirmar “42 fixed-point attractors” como resultado demonstrado.

A closure atual em `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` registra que a alegação forte de convergência a fixed points foi falsificada como formulada. O valor 42 permanece válido onde for explicitamente parte de um intervalo/índice/construção, mas isso não prova 42 atratores físicos ou matemáticos.

Antes de promover novo claim:

```text
definição precisa
-> hipótese/H0
-> falsificador
-> execução
-> receipt
-> revisão
```

### 4.2 ApkC: hosted versus freestanding

Separar os domínios:

```text
ARM/freestanding target -> contrato no-libc/no-heap conforme gate
host x86/x86_64 dev     -> libc permitida quando declarada pelo caminho hosted
```

Nunca usar a existência do host wrapper como justificativa para introduzir libc na rota freestanding.

### 4.3 Android current-commit

Para claim de runtime Android, exigir cadeia contínua do mesmo artefato/commit quando o claim assim requer:

```text
source
-> ARM artifact
-> APK atual
-> identidade do ELF dentro do APK
-> assinatura/verificação atual
-> install
-> launch/dlopen
-> runtime/ANativeActivity
-> logcat/exit/receipt
```

Cada elo ausente preserva `TOKEN_VAZIO` para a conclusão correspondente.

### 4.4 Estado material e temporalidade

`ECOSYSTEM_RUNTIME_STATE.json` é uma fonte importante, mas é um snapshot. Se `observed_at` ou commit referenciado for anterior ao HEAD, não promover seu conteúdo automaticamente para “estado atual”. Registrar a diferença e reconciliar por delta.

## 5. Papéis funcionais

| Papel | Responsabilidade | Autoriza merge? |
|---|---|---|
| arquiteto técnico | invariantes, APIs, formatos | não sozinho |
| implementador | mudança dirigida e testes | não |
| revisor semântico | coerência, limites e linguagem | não |
| segurança/licença | risco, dados, criptografia, termos | recomenda/escalona |
| qualidade | regressão positiva/negativa | não |
| evidence custodian | receipts, hashes, provenance | não |
| humano autorizador | decisão final, exceção e merge | sim |

Agente deve agir por papel, não por marca/modelo. Não existe “Claude manda no C” ou “ChatGPT manda na semântica”. Competência é demonstrada pela tarefa e evidência.

## 6. Ciclo de sessão

### 6.1 Startup

```text
[ ] Ler AGENTS.md, este documento e o README/índice aplicável.
[ ] Registrar branch e commit de base.
[ ] Confirmar branch de trabalho não protegida.
[ ] Classificar a tarefa: code | docs | data | compliance | research.
[ ] Identificar arquivo canônico e derivados.
[ ] Identificar invariantes e testes afetados.
[ ] Rodar baseline aplicável, se o ambiente permitir.
[ ] Marcar ferramentas/device/dados ausentes como TOKEN_VAZIO.
```

Comandos de identidade:

```sh
git branch --show-current
git rev-parse HEAD
git status --short
```

### 6.2 Estados

| Estado | Significado |
|---|---|
| `VOID` | referência/placeholder sem corpo suficiente |
| `PENDING` | conteúdo existe, gate insuficiente |
| `REFERENCE` | especificação/explicação |
| `AUDIT` | relatório, contrato ou trilha |
| `RUNTIME` | depende de ambiente observado |
| `IMPLEMENTED` | código existe |
| `PASS` | gate nomeado executou e passou no escopo declarado |
| `FAIL` | gate nomeado executou e falhou |
| `TOKEN_VAZIO` | evidência ausente/insuficiente/inaplicável no corte |

Não codificar `TOKEN_VAZIO` como um valor numérico universal. Em APIs concretas, usar o status definido pelo contrato daquela API.

### 6.3 Shutdown

```text
[ ] Código e documentação coerentes.
[ ] Novos docs/configs têm rota de descoberta.
[ ] TOKEN_VAZIO/FAIL/resultados negativos preservados.
[ ] Comandos realmente executados separados dos recomendados.
[ ] Riscos e rollback registrados.
[ ] PR permanece draft quando gate material para o claim está aberto.
[ ] Nenhum dado sensível foi exposto.
```

## 7. Regras de não-colisão

### Regra 1 — Não editar `main` por padrão

Trabalhar em branch dedicada e PR revisável. Exceção exige pedido humano explícito.

### Regra 2 — Não esconder falha

Proibido transformar gate bloqueante em verde por `|| true`, skip silencioso, retorno sempre zero ou mudança semântica equivalente.

### Regra 3 — Header/compilador exige validação imediata

Em mudança estrutural do ApkC, usar os targets/scripts canônicos existentes. Exemplo:

```sh
make syntax
```

Não perpetuar comandos históricos se o Makefile atual já aponta para a fonte hardened.

### Regra 4 — Linguagem/encoder exige pipeline completo

Mudança de linguagem ou ISA precisa atualizar as pontas e os testes exigidos pelo contrato atual, não apenas tabela ou comentário.

### Regra 5 — Layout persistido exige versão

Alterar ELF/DEX/AXML/ZIP/segment/schema exige avaliar compatibilidade, offsets, tamanhos, endianness, leitor e fixtures.

### Regra 6 — Documento novo exige rota

Todo documento novo precisa de pelo menos:

```text
área
responsável lógico
status/lifecycle
entrada/relação canônica
```

### Regra 7 — Gerado não é editado manualmente

`docs/generated/` e `results/document-governance/` são derivados. Corrigir fonte/política e regenerar.

### Regra 8 — Conflito entre agentes vira dado

Não escolher silenciosamente uma versão. Registrar a divergência, evidência de cada lado e a menor experiência/gate que decide.

## 8. Anti-padrões

| Anti-padrão | Correção |
|---|---|
| “arquivo existe, portanto PASS” | executar gate ou manter IMPLEMENTED/TOKEN_VAZIO |
| “PR merged, portanto validado” | verificar checks/receipts/escopo |
| “42 = 42 atratores” | distinguir índice/construção de teorema dinâmico |
| “no libc em qualquer build” | separar hosted de freestanding |
| “hardware ausente = sucesso” | usar estado/API explícito; não falsificar PASS |
| “histórico = atual” | ligar evidence ao commit/artefato corrente |
| “gerado corrigido à mão” | corrigir gerador/política |
| “duplicado deve ser apagado” | preservar contexto/provenance e revisar |
| “otimização antes de golden” | manter referência portátil e equivalência |
| “claim científico por analogia” | marcar modelo/hipótese e construir falsificador |

## 9. Gates e comandos principais

A escolha deve ser proporcional ao arquivo alterado.

| Área | Gate/entrada típica |
|---|---|
| compiler station | `make compiler-contract`, `make compiler-selftest` |
| ApkC hardened syntax | `make syntax` |
| hotfix compiler | `make hotfix-audit` |
| Verbovivo smoke | `make verbovivo-demo` |
| formatos/ELF | scripts/targets específicos e validadores independentes |
| document governance | `python3 -m unittest tests.test_document_governance` + `document_governance.py --check` |
| runtime truth | `scripts/validate_runtime_truth_local.sh` quando aplicável |
| Android físico | somente gates/receipts que realmente observem device atual |
| T^7 | falsificador/closure atuais antes de claim |

Status remoto, nome do workflow ou checkbox não substitui a execução observada.

## 10. Entradas canônicas por subsistema

| Subsistema | Entrada |
|---|---|
| agentes | `AGENTS.md` -> `docs/AGENTES.md` |
| documentação | `docs/INDEX.md` |
| governança | `docs/DOCUMENT_GOVERNANCE.md` |
| APKc | `Apkc/PROTOCOL.md`, `docs/APKC_PROTOCOL.md` |
| linguagem | `Apkc/lang_profile.h` + contratos/testes atuais |
| ELF | `Apkc/fmt_elf.h` |
| DEX | `Apkc/fmt_dex.h` |
| AXML | `Apkc/fmt_axml.h` |
| ZIP/APK | `Apkc/fmt_zip.h` |
| syscalls | `Apkc/sys.h` |
| pipeline alto nível | `raf_compile.h` |
| T^7/Fiber-H | `rafaelia/verbovivo.c`, `rafaelia/t7_toroid.h`, closure L9 |
| runtime router | `Benchmark/raf_runtime_router.h` |
| segmentação | `runtime/conversation_indexer/` |
| aprendizado científico | `scripts/science_learning_engine.py` -> `knowledge_base/` |

## 11. Evidência e receipts

Quando aplicável, receipt técnico deve registrar:

```text
repository + commit
input/hash
comando executado
toolchain/versões
ambiente/arquitetura
exit code
stdout/stderr ou hashes
output/hash
escopo
limitações/TOKEN_VAZIO
```

A promoção é sempre limitada ao que o receipt realmente demonstra.

## 12. PR e handoff

PR deve separar:

```text
WHY
SCOPE
INVARIANTS
EXECUTED
OBSERVED
TOKEN_VAZIO
RISKS
ROLLBACK
F_next
```

Não escrever “tests pass” se os testes não foram executados. Usar “not executed in this environment” quando for o caso.

## 13. Escalação

Escalar ao humano quando houver:

1. quebra/mudança de invariante;
2. alteração de API/formato público;
3. exclusão, movimentação ou quarentena relevante;
4. segurança, licença, privacidade ou segredo;
5. claims científicos/externos incompatíveis com evidência;
6. conflito sem falsificador reproduzível;
7. pedido de merge quando gates materiais continuam abertos.

Sem decisão humana, preservar branch/PR draft e rollback.

## 14. Referências

- `AGENTS.md` — roteador comum entre agentes.
- `.github/copilot-instructions.md` — adaptador Copilot.
- `CLAUDE.md` — adaptador Claude Code.
- `docs/MULTI_AI_METHODOLOGY.md` — colaboração/handoff.
- `docs/CODEX_FIX_PROTOCOL.md` — diagnóstico de compilação, subordinado a este documento.
- `docs/AGENTES_CHECKLIST.md` — checklist de sessão.
- `docs/AGENTES_DECISAO_LOG.md` — conflitos/decisões.
- `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` — fronteira T^7 atual.

## 15. Fechamento R3

Toda sessão termina com:

```text
F_ok   = o que foi realmente alterado/executado/demonstrado
F_gap  = o que permanece aberto, contraditório ou não executado
F_next = a menor ação reproduzível seguinte
```

Nenhum campo recebe mérito inventado para aumentar aparência de completude.
