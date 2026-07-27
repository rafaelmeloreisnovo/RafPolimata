# AGENTES — Guia Operacional Unificado

> Leia antes de modificar código, documentação, configuração ou evidência.

Este documento é a entrada operacional para agentes humanos e IA. A navegação documental começa em `docs/INDEX.md`; a governança de arquivos está em `docs/DOCUMENT_GOVERNANCE.md`.

## 1. Arquitetura essencial

| Camada | Arquivo principal | Entrada |
|---|---|---|
| Pipeline de alto nível | `raf_compile.h` | `raf_compile_file()` |
| Micro-toolchain Android | `Apkc/apkc.c` | `apkc_main()` |
| Motor cognitivo T^7 | `rafaelia/verbovivo.c` | `verbovivo_main()` |
| Runtime router | `Benchmark/raf_runtime_router.h` | seleção por capacidade |
| Conversation indexer | `runtime/conversation_indexer/` | codec `segment.v1` |
| Governança documental | `scripts/document_governance.py` | `build_catalog()` |

### Invariantes

```text
1. Zero malloc/calloc/free em Apkc/ e hot paths declarados freestanding.
2. Zero dependência de libc em Apkc/ quando a rota exige syscall direta.
3. Perfil de linguagem pertence à tabela Apkc/lang_profile.h.
4. Encoder ARM exige implementação + dispatch + teste golden.
5. lang_profile_find()/from_path() podem retornar NULL; guard é obrigatório.
6. Existência de arquivo não equivale a PASS.
7. Código, documento, teste e estado devem permanecer coerentes.
8. Nenhum arquivo é movido ou apagado sem hash, relações e PR dedicado.
9. Nenhum segredo detectado é reproduzido em relatório.
10. Merge exige gates materiais aplicáveis e revisão humana.
```

Syntax check mínimo para mudanças no ApkC:

```sh
clang --target=aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
  -ffreestanding -I Apkc Apkc/apkc.c
```

Falha bloqueia promoção.

Rota local sem Gradle/SDK/rede para a entrada assembly interna:
`sh scripts/apkc_termux_hermetic_build.sh --abi both`. O contrato e os limites estão em `docs/APKC_HERMETIC_TERMUX.md`.

## 2. Papéis funcionais

| Papel | Responsabilidade | Autoriza merge? |
|---|---|---|
| arquiteto técnico | invariantes, APIs e formatos | não sozinho |
| implementador | mudança dirigida e testes | não |
| revisor semântico | coerência, documentação e limites | não |
| segurança/licença | risco, dados, criptografia e termos | recomenda/escalona |
| qualidade | regressão positiva e negativa | não |
| humano autorizador | decisão final, exceção e merge | sim |

Papéis não ficam presos a uma ferramenta ou fornecedor. Um agente deve declarar o papel exercido na sessão.

## 3. Ciclo de sessão

### 3.1 Startup

```text
[ ] 1. Ler README.md, docs/INDEX.md e este guia.
[ ] 2. Registrar branch e commit atual.
[ ] 3. Confirmar que a branch de trabalho não é main/master.
[ ] 4. Classificar a tarefa: código | docs | dados | conformidade | pesquisa.
[ ] 5. Identificar invariantes, arquivos canônicos e testes afetados.
[ ] 6. Rodar o baseline aplicável antes da primeira alteração.
[ ] 7. Declarar TOKEN_VAZIO para ferramenta, device ou evidência ausente.
```

Comandos de identidade:

```sh
git branch --show-current
git rev-parse HEAD
git status --short
```

Não existe branch histórica obrigatória. O requisito estável é trabalhar em branch não protegida, com escopo explícito e PR revisável.

### 3.2 Estados

| Estado | Significado | Saída |
|---|---|---|
| `VOID` | referência sem artefato | criar ou remover de forma dedicada |
| `PENDING` | conteúdo sem gate suficiente | adicionar validação |
| `REFERENCE` | explicação ou especificação | ligar à implementação/evidência |
| `AUDIT` | contrato, matriz ou relatório | preservar origem e hash |
| `RUNTIME` | depende do ambiente de execução | registrar ambiente e saída |
| `IMPLEMENTED` | código presente | executar teste específico |
| `PASS` | gate definido passou | preservar artefato e comando |
| `FAIL` | gate definido falhou | corrigir ou fazer rollback |
| `TOKEN_VAZIO` | evidência insuficiente | executar ou manter lacuna explícita |

### 3.3 Shutdown

```text
[ ] Código e documentação coerentes no mesmo PR.
[ ] Novos arquivos aparecem no índice ou na fila de governança.
[ ] VOID/PENDING/TOKEN_VAZIO permanecem explícitos.
[ ] Testes aplicáveis foram executados e registrados.
[ ] Riscos e rollback aparecem no body do PR.
[ ] PR continua DRAFT enquanto gate material estiver aberto.
[ ] Nenhum dado sensível foi exposto em log ou relatório.
```

## 4. Regras de não-colisão

### Regra 1 — Header freestanding exige syntax check imediato

Um erro em `Apkc/*.h` pode produzir cascata de diagnósticos. Rodar o compilador após cada alteração estrutural.

### Regra 2 — Linguagem nova exige tabela e pipeline completo

Não criar suporte paralelo fora de `Apkc/lang_profile.h`. Verificar família de execução, compilador, artefato, ABI, D8 e guard de erro.

### Regra 3 — Encoder exige duas pontas e golden

```text
arch_arm{32,64}.h
+ asm_insn{32,64}()
+ teste golden/roundtrip
```

Sem os três corpos, o estado é `PENDING`.

### Regra 4 — NULL é caminho normal de erro

```c
if (!prof) {
    pr_err("unknown language\n");
    return 1;
}
```

### Regra 5 — Layout binário exige cross-reference

Alterar índices ELF, offsets DEX/AXML ou estrutura ZIP exige atualizar todos os links, tamanhos, alinhamentos e validador independente.

### Regra 6 — Limite de buffer é contrato

Sem heap, truncamento silencioso é falha. Toda escrita precisa validar capacidade antes de avançar posição.

### Regra 7 — Documento novo exige rota

Todo novo arquivo deve receber:

```text
área + responsável lógico + ciclo de vida + referência de entrada
```

Rodar:

```sh
python3 scripts/document_governance.py --write --print-summary
```

### Regra 8 — Resultado gerado não é editado manualmente

Arquivos em `docs/generated/` e `results/document-governance/` são derivados. Corrigir política ou executor e regenerar.

## 5. Anti-padrões

| Anti-padrão | Risco | Alternativa |
|---|---|---|
| incluir libc em rota freestanding | quebra ABI/invariante | usar camada permitida ou separar backend |
| `malloc()` em hot path sem revisão | imprevisibilidade e violação | arena/buffer explícito |
| extensão desconhecida virar ASM | execução incorreta silenciosa | retornar erro |
| JAR tratado como DEX | APK inválido | classes → JAR → D8 → DEX |
| caminho de toolchain presumido | `execve` falha | resolução determinística e log |
| documento declara PASS sem artefato | falso verde | reconciliar documento↔prova |
| mover arquivos para “organizar” | links e proveniência quebrados | catalogar e mover em PR dedicado |
| apagar duplicidade por hash | perda de contexto/licença | revisar origem e dependências |
| `|| true` em gate bloqueante | falha escondida | exit code real e estado explícito |
| segredo copiado para relatório | incidente de dados | registrar somente flag de detector |
| editar saída gerada | drift irreproduzível | corrigir fonte e regenerar |

## 6. Excelência operacional

### 6.1 Promoção técnica

```text
VOID → BASELINE → CANDIDATE → VALIDATED
                     ↘ FAIL → ROLLBACK
```

| Estado | Critério |
|---|---|
| `BASELINE` | compilável, correto e medido |
| `CANDIDATE` | mudança aplicada com hipótese explícita |
| `VALIDATED` | testes e métricas superam ou preservam baseline |
| `ROLLBACK` | regressão ou risco não resolvido |

### 6.2 Auditoria mínima

1. **Dados:** origem, formato, classificação e hash.
2. **Qualidade:** requisito → execução → evidência → ação.
3. **Segurança:** menor privilégio, integridade e rollback.
4. **Desempenho:** prewarm, warmup, mediana, p95/p99 e baseline.
5. **Documentação:** índice, relações, responsável e revisão temporal.
6. **Lacuna:** TOKEN_VAZIO em vez de inferência.

### 6.3 Caminhos de execução

| Caminho | Usar | Rejeitar |
|---|---|---|
| C genérico | baseline e portabilidade | gargalo comprovado com alternativa melhor |
| branchless | decisão pequena e previsível | aumenta instruções/cache miss |
| ARM NEON | dados contíguos e hot path | alinhamento/volume inadequado |
| GPU batch | lote grande | transferência domina |
| syscall direta | ABI controlada | segurança/manutenção piora |

Fallback genérico permanece obrigatório quando a aceleração é opcional.

## 7. Governança documental

A política está em `configs/document-governance.v1.json`.

### Níveis

```text
L0 estrutura
L1 identidade
L2 grafo
L3 responsável/temporalidade
L4 evidência/risco
L5 revisão/promoção
```

### Gates

```sh
python3 -m json.tool configs/document-governance.v1.json >/dev/null
python3 -m json.tool schemas/document-record.v1.schema.json >/dev/null
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

Rotas críticas:

- `QUARANTINE_REVIEW` bloqueia;
- `SENSITIVITY_REVIEW`, `REFERENCE_REPAIR`, `ROOT_REVIEW`, `DUPLICATE_REVIEW`, `LINK_REQUIRED` e `REVIEW_STALE` entram em fila;
- `CANONICAL` e `INDEXED` permanecem monitorados.

## 8. Escalação

Escalar ao humano quando houver:

1. quebra de invariante;
2. mudança de API ou formato público;
3. licença, segurança, privacidade ou dado sensível;
4. exclusão, fusão, movimentação ou quarentena;
5. propostas incompatíveis;
6. evidência contraditória sem resolução reproduzível.

Sem humano disponível:

```text
1. não mergear;
2. manter PR draft;
3. registrar alternativas e riscos;
4. atualizar docs/AGENTES_DECISAO_LOG.md;
5. preservar rollback;
6. aguardar decisão.
```

## 9. Gates principais

| Gate | Executor | Valida |
|---|---|---|
| coerência canônica | `scripts/validate_coherence_protocol.py` | fórmulas e protocolo |
| dois ciclos Ω | `scripts/validate_two_cycle_omega.py` | estados e invariantes |
| estrutura L0 | `scripts/audit_repository_structure.py` | diretórios, raiz e links |
| governança L1–L5 | `scripts/document_governance.py` | catálogo, grafo e fila |
| runtime truth local | `scripts/validate_runtime_truth_local.sh` | build/teste local |
| freestanding ApkC | `scripts/ci_freestanding_audit.sh` | ausência de heap/libc proibidos |
| encoders ARM64/32 | `tests/test_arm64_encoders.py`, `tests/test_arm32_encoders.py` | palavras de instrução |
| assembler roundtrip | `tests/test_asm_roundtrip.sh` | parser/emissão/backpatch |
| formatos APK/DEX/ELF | `scripts/validate_apkc_formats.py` | bytes e ABI |
| prova source→binary | `tools/raf_source_to_binary_proof.sh` | build, identidade e reprodução |
| linguagem/API/ABI | scripts `apkc_*` | dispatch e matriz Android |
| verbovivo/Fiber-H | workflow/testes do módulo | execução e saída |
| P(k) | `scripts/first_test_pk.py` | falsificabilidade |

Status remoto, arquivo YAML ou checkbox não substituem a execução do gate.

## 10. Entradas canônicas por subsistema

| Subsistema | Entrada |
|---|---|
| documentação | `docs/INDEX.md` |
| governança documental | `docs/DOCUMENT_GOVERNANCE.md` |
| política documental | `configs/document-governance.v1.json` |
| APKc | `Apkc/lang_profile.h` |
| ARM64 | `Apkc/arch_arm64.h` + `asm_insn64()` |
| ARM32 | `Apkc/arch_arm32.h` + `asm_insn32()` |
| ELF | `Apkc/fmt_elf.h` |
| DEX | `Apkc/fmt_dex.h` |
| AXML | `Apkc/fmt_axml.h` |
| ZIP/APK | `Apkc/fmt_zip.h` |
| syscalls/toolchain | `Apkc/sys.h` |
| pipeline alto nível | `raf_compile.h` |
| T^7/Fiber-H | `rafaelia/verbovivo.c`, `rafaelia/verbovivo.h` |
| runtime router | `Benchmark/raf_runtime_router.h` |
| segmentação | `runtime/conversation_indexer/` |
| aprendizado científico | `scripts/science_learning_engine.py` → `knowledge_base/` |

## 11. Referências

| Documento | Conteúdo |
|---|---|
| `docs/INDEX.md` | índice curado |
| `docs/DOCUMENT_GOVERNANCE.md` | método de catálogo e promoção |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | estrutura e rotas |
| `docs/MULTI_AI_METHODOLOGY.md` | handoff e colaboração |
| `docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md` | protocolo formal humano-agente |
| `docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md` | execução, otimização e rollback |
| `docs/ROTINA_OPERACIONAL_BENCHMARKS.md` | benchmark e estatística |
| `docs/PROTOCOLO_CANONICO_COHERENCIA.md` | coerência e prova |
| `docs/AGENTES_CHECKLIST.md` | checklist de sessão |
| `docs/AGENTES_DECISAO_LOG.md` | decisões e conflitos |

## 12. Fechamento

A sessão termina com:

```text
F_ok   = o que foi executado e demonstrado
F_gap  = o que continua aberto ou contraditório
F_next = a menor ação reproduzível seguinte
```

Nenhum desses campos deve conter mérito inventado para aumentar aparência de completude.
