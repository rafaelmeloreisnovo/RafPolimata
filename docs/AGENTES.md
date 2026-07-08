# AGENTES — Guia Operacional Unificado

> Leia este documento antes de escrever qualquer linha de código.
> Tempo estimado: 8–10 minutos. Economiza horas de retrabalho.

Este documento é a entrada canônica para **qualquer agente** (Claude, Codex,
Copilot, ChatGPT, humano) que abra uma sessão neste repositório. Documentos
de profundidade estão em `docs/MULTI_AI_METHODOLOGY.md` e
`docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md` — leia-os depois.

---

## 1. O que este repositório faz (leitura em 2 minutos)

**RafPolimata** tem três camadas integradas:

| Camada | Arquivo principal | Entrada |
|---|---|---|
| Pipeline de alto nível | `raf_compile.h` | `raf_compile_file()` |
| Compilador APK bare-metal | `Apkc/apkc.c` | `apkc_main()` |
| Motor cognitivo T^7 | `rafaelia/verbovivo.c` | `verbovivo_main()` |

**Invariantes que NUNCA podem ser quebrados:**

```
1. Zero malloc/calloc/free em Apkc/ e hot paths (freestanding)
2. Zero includes de libc em Apkc/ (syscalls via sys.h svc/swi)
3. Adicionar linguagem = 1 linha em _lang_table[] em Apkc/lang_profile.h
4. Adicionar instrução ARM64 = 1 inline em arch_arm64.h + 1 case em asm_insn64()
5. lang_profile_find() retorna NULL — callers DEVEM fazer guard
6. Todos os CI gates devem passar antes do merge
```

**O único comando a rodar antes de qualquer PR:**
```bash
clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
  -ffreestanding -I Apkc Apkc/apkc.c
```
Se falhar, a mudança não é mergeável.

---

## 2. Taxonomia de agentes

| Tipo | Responsabilidade primária | Escopo de decisão | Pode mergear? |
|---|---|---|---|
| **Claude** | Arquitetura, freestanding C, ARM64 ISA, ELF/AXML, CI | Alta | Não sem review humano |
| **Codex/Copilot** | Implementação dirigida, validação de encoders | Média — implementa sob direção | Não |
| **ChatGPT** | Revisão semântica, lookup de spec, documentação | Baixa — referência e revisão | Não |
| **Agente humano** | Decisão final, escalação, segurança, merge | Total | Sim |

**Regra de prioridade**: quando dois agentes propõem mudanças incompatíveis,
o agente humano decide. Se indisponível, use o protocolo de escalação (seção 6).

### Capacidades por tipo de tarefa

| Tarefa | Claude | Codex | ChatGPT | Humano |
|---|---|---|---|---|
| Decisão de arquitetura | primário | suporte | consulta | valida |
| C freestanding (sem libc) | primário | ✓ | revisão | audita |
| Encoders ARM64 | primário | valida | referência | audita |
| Extensão lang_profile | primário | ✓ | revisão | audita |
| Formatos ELF/AXML binários | primário | ✓ | referência | audita |
| CI / workflows | ✓ | ✓ | suporte | define |
| Documentação (MD) | ✓ | ✓ | ✓ | revisa |
| Segurança / criptografia | revisão | — | consulta | **primário** |
| Semântica / filosófico | revisão | — | primário | valida |

---

## 3. Ciclo de sessão

### 3.1 Startup — 5 passos antes de escrever código

```
[ ] 1. Ler CLAUDE.md (invariantes, buffer limits, extensão protocol)
[ ] 2. Rodar o syntax check freestanding (comando acima)
[ ] 3. Confirmar branch: deve ser claude/operational-excellence-agents-mabjye
        git branch --show-current
[ ] 4. Grep para malloc/calloc/free em Apkc/ — deve retornar vazio:
        grep -r "malloc\|calloc\|free(" Apkc/*.c Apkc/*.h
[ ] 5. Identificar a tarefa e classificá-la:
        documentação | implementação | refatoração | conformidade | pesquisa
```

Ver checklist completo em `docs/AGENTES_CHECKLIST.md`.

### 3.2 Execução — estados canônicos

Todo campo, comentário ou arquivo deve ter um estado explícito:

| Estado | Significado | Critério de saída |
|---|---|---|
| `VOID` | Placeholder — não implementado | Criar artefato mínimo ou remover |
| `PENDING` | Em progresso ou sem gate automatizado | Adicionar teste ou validação documental |
| `AUDIT` | Evidência ou relatório — verificar reprodutibilidade | Manter hash/data quando aplicável |
| `RUNTIME` | Conhecido só em runtime | Documentar origem |
| `REFERENCE` | Spec externa (RFC/ISO/ARM ISA) | Linkar ao código que a implementa |

**TOKEN_VAZIO**: quando não há evidência suficiente para uma afirmação, declare
`TOKEN_VAZIO` em vez de inventar. É o estado mais honesto disponível.

### 3.3 Shutdown — antes do push

```
[ ] Código e documentação na mesma unidade de trabalho (mesmo commit)
[ ] VOID/PENDING explícitos para tudo que ficou incompleto
[ ] Commit message no formato:  type(scope): o que + por quê
    Exemplo: feat(Apkc): add PHP to _lang_table — completes OS coverage
[ ] PR criado como DRAFT se CI não está verde
[ ] Pendências e riscos publicados no body do PR
```

---

## 4. Regras de não-colisão

Estas 6 regras impedem que dois agentes quebrem o trabalho um do outro:

### Regra 1 — Syntax check após cada edição em `Apkc/*.h`
Um `;` faltando em um header freestanding cascateia para 100+ erros.
Rodar `clang -fsyntax-only` após **cada** edição, não só antes do commit.

### Regra 2 — Nunca adicionar linguagem sem linha em `_lang_table[]`
O pipeline inteiro é dirigido por `Apkc/lang_profile.h`. Adicionar suporte a
uma linguagem em outro lugar sem adicionar a entrada na tabela = código morto.

### Regra 3 — Nunca adicionar encoder ARM64 incompleto
Exige **os dois** juntos:
- `static inline u32` em `Apkc/arch_arm64.h`
- `case` correspondente em `asm_insn64()` em `Apkc/apkc.c`

### Regra 4 — Nunca commitar caminho de NULL-deref
`lang_profile_find()` retorna NULL para nomes desconhecidos.
Guard obrigatório:
```c
if (!prof) { pr_err("unknown language\n"); return 1; }
```

### Regra 5 — Nunca mudar índices de seção ELF sem atualizar cross-refs
Layout atual (ARM64):
```
0=null  1=.text  2=.rodata  3=.hash  4=.dynsym  5=.dynstr
6=.dynamic  7=.shstrtab  8=.ARM.attributes   e_shstrndx=7
```
Qualquer adição/remoção exige atualizar `sh_link` e `e_shstrndx` manualmente.

### Regra 6 — Buffer limits são hard limits
Todos estão documentados em `CLAUDE.md`. Ultrapassar silenciosamente trunca —
sem alocação, sem realloc.

### Anti-padrões: o que parece certo mas destrói o repo

| Anti-padrão | Consequência | Alternativa correta |
|---|---|---|
| `#include <stdlib.h>` em `Apkc/` | Quebra freestanding, CI falha | Usar `sys.h` — syscalls via `svc`/`swi` |
| `malloc()` em qualquer hot path | Viola invariante central | Buffer estático pré-alocado |
| Editar `_lang_table[]` sem syntax check | Cascata de 100+ erros silenciosa | Rodar clang após cada linha |
| `pr->name` sem guard de NULL | NULL-deref em runtime | Sempre: `if (!prof) return err;` |
| Mudar `e_shstrndx` sem atualizar `sh_link` | ELF inválido — Android rejeita .apk | Reler `Apkc/PROTOCOL.md` seção ELF |
| Commitar com `/* VOID */` removido sem implementar | Sinaliza implementação que não existe | Manter VOID até implementar de verdade |
| "documentar depois" | Lacuna permanente — próximo agente perde contexto | Código + doc no mesmo commit |
| Inverter flag `jsx_node`/`use_d8` na tabela | Caminho errado de pipeline — APK inválido | Verificar tabela de dispatch em `CLAUDE.md` |

---

## 5. Protocolo de excelência operacional

Baseado em `docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md` e
`docs/ROTINA_OPERACIONAL_BENCHMARKS.md`.

### Pipeline de promoção de código

```
VOID → BASELINE → CANDIDATE → VALIDATED → ROLLBACK (se regredir)
```

| Estado | Critério de entrada | Ação do agente |
|---|---|---|
| `VOID` | Placeholder sem artefato | Declarar explicitamente — não presumir |
| `BASELINE` | Compilável, correto, medido | Medir compile_ns + .text bytes antes de otimizar |
| `CANDIDATE` | Otimização aplicada | Comparar contra baseline com hash, p95, p99 |
| `VALIDATED` | Gates CI passam + benchmark melhor que baseline | Promover — atualizar matriz |
| `ROLLBACK` | Regressão detectada | Reverter para BASELINE, registrar causa |

### Rotina de auditoria obrigatória

Antes de promover qualquer otimização, verificar:

1. **Dados**: entrada versionada, formato declarado, hash/reprodutibilidade
2. **Qualidade**: requisito → execução → evidência → não-conformidade → ação
3. **Segurança**: menor privilégio, integridade, rollback, trilha de comando
4. **Benchmark**: prewarm → warmup → mediana → p95/p99 → comparação com baseline
5. **TOKEN_VAZIO**: quando falta evidência, registrar VOID/SKIPPED — nunca fabricar PASS

### Critério de parada da rotina

A sessão só encerra quando:
- Gates bloqueantes passam
- Relatório P(k) em PASS
- Runtime router tem fail-safe/failover/rollback testado
- Nenhuma mudança não auditada no índice de arquivos críticos

### Matriz de decisão por caminho de execução

| Caminho | Usar quando | Rejeitar quando |
|---|---|---|
| Genérico C | baseline, portabilidade | gargalo comprovado |
| Branchless inteiro | decisões pequenas, previsíveis | aumenta instruções ou cache miss |
| ARM64 NEON | hot path vetorial estável | dados não contíguos/alinháveis |
| GPU batch | lote grande, paralelismo massivo | transferência RAM/GPU domina |
| Syscall direta | caminho mínimo controlado | ABI/segurança/manutenção piora |

**Fallback obrigatório**: `generic_c` — GPU/NEON/syscall são acelerações
condicionais, nunca dependência única.

---

## 6. Escalação e conflito

### Quando escalar para humano (4 critérios)

1. A mudança **quebra um invariante** listado em `CLAUDE.md`
2. Escopo **arquitetural**: novo subsistema, mudança de API pública, novo formato binário
3. Implicações de **licença ou segurança**
4. Dois agentes propõem mudanças **incompatíveis sem resolução clara**

### Protocolo de escalação sem humano disponível

```
1. NÃO mergear
2. Criar branch:  decision/<topic-curto>
3. Abrir PR com:
   - Resumo do conflito
   - Proposta A (com prós/contras)
   - Proposta B (com prós/contras)
   - Recomendação do agente atual
4. Registrar em docs/AGENTES_DECISAO_LOG.md
5. Aguardar revisão humana
```

### Resolução de conflito entre agentes

Ver template em `docs/AGENTES_DECISAO_LOG.md`. Sempre registrar:
- Qual sessão gerou qual proposta
- Critério de escolha
- Estado final: `RESOLVED` ou `ESCALATED`

---

## 7. Gates CI obrigatórios

O workflow `.github/workflows/ci.yml` tem 15+ gates. Todos devem passar para
que o PR saia de DRAFT:

| Gate | Arquivo/script | O que valida |
|---|---|---|
| Validate canonical coherence protocol | `scripts/validate_coherence_protocol.py` | 50 fórmulas e protocolo canônico |
| Validate two-cycle omega protocol | `scripts/validate_two_cycle_omega.py` | Protocolo Ω de dois ciclos |
| Host C syntax check | `RAF_host_syntax_check.sh` | Sintaxe C no host |
| Build compiler (strict warnings) | gcc `-Wall -Wextra -Werror` | Compila `raf_compile` sem warnings |
| Smoke test compiler | `./raf_compile --help` | Binário funcional |
| Validate operational manifest | `scripts/test_ops_manifest.sh` | Manifesto de excelência |
| Audit repository structure | `scripts/audit_repository_structure.py` | Estrutura e links quebrados |
| Emit repository universe matrix | `scripts/emit_repository_universe_matrix.py` | JSON de estado do repo |
| Emit RAF methods status | `scripts/emit_raf_methods_status.py` | Estado dos 56 métodos |
| Emit concept structural audit | `scripts/emit_concept_structural_audit.py` | Auditoria conceitual |
| Validate Android ABI plan | `scripts/android_build_matrix.sh --plan` | Plano ABI Android |
| Audit ApkC freestanding invariants | `scripts/ci_freestanding_audit.sh` | Zero malloc/libc em Apkc/ |
| RAF TraceBuf freestanding smoke gate | `tools/raf_tracebuf_smoke.c` | TraceBuf freestanding |
| ARM64 encoder golden tests | `tests/test_arm64_encoders.py` | Codificação ARM64 bit-a-bit |
| ARM32 encoder golden tests | `tests/test_arm32_encoders.py` | Codificação ARM32/Thumb2 |
| APKc assembler roundtrip smoke test | `tests/test_asm_roundtrip.sh` | Montador 2-pass roundtrip |
| ApkC compile transcript | gcc cross-freestanding | Apkc/apkc.c compila sem erros |
| Validate ApkC proof protocol | `scripts/apkc_validate.sh` | Cadeia de custódia F1 |
| ApkC language coverage | `scripts/apkc_lang_coverage.sh` | Modos asm+script+fork |
| ApkC API/ABI matrix | `scripts/apkc_api_abi_matrix.sh` | minSdkVersion no AXML binário |
| Build and smoke-test verbovivo | `rafaelia/verbovivo.c` + Fiber-H | T^7 + Fiber-H + SVG engram |
| Codegen equivalence-family test | `tools/raf_codegen_select_test.c` | MOV family determinístico |
| Bus throughput test | `Benchmark/raf_bus_throughput_test.c` | S23 throughput |
| Processing-per-watt proxy | `scripts/raf_watt_proxy_probe.sh` | S25 (TOKEN_VAZIO permitido) |
| Falsificabilidade P(k) gate | `scripts/first_test_pk.py` | RRMSE gate A9 |
| Generate P(k) falsifiability report | `scripts/first_test_pk.py` | Veredicto PASS/FAIL |

**O que fazer quando um gate falha:**

- **Syntax / compiler / freestanding**: inspecionar a saída do gate, corrigir C,
  rodar syntax check local antes de novo push
- **ARM64/ARM32 encoder**: inspecionar `tests/` para ver qual golden falhou — o
  encoding esperado está comentado no test
- **P(k) FAIL**: verificar `data/pk_observado.csv` — os dados mudaram ou o modelo
  saiu da tolerância; não ajustar o modelo para fazer passar
- **TOKEN_VAZIO (permitido)**: gates com `|| true` ou `TOKEN_VAZIO allowed` podem
  falhar sem bloquear CI — mas devem ser documentados

---

## 8. Entradas canônicas por subsistema

| Subsistema | Arquivo de entrada | Função principal |
|---|---|---|
| APKc pipeline | `Apkc/lang_profile.h` | `lang_profile_from_path()` |
| ARM64 assembly | `Apkc/arch_arm64.h` | `asm_insn64()` em `apkc.c` |
| ARM32/Thumb2 assembly | `Apkc/arch_arm32.h` | `asm_insn32()` em `apkc.c` |
| ELF64/ELF32 builder | `Apkc/fmt_elf.h` | `elf64_build_so()` |
| AXML binary manifest | `Apkc/fmt_axml.h` | `axml_build()` |
| ZIP/APK writer | `Apkc/fmt_zip.h` | `zip_open()` / `zip_add()` |
| Execve bootstrap | `Apkc/lang_script.h` | `gen_script_code64()` |
| Freestanding syscalls | `Apkc/sys.h` | `os_read()`, `os_fork()`, … |
| Geometric coherence | `Apkc/coherence.h` | `phi_fst()`, `phi_attractor()` |
| Codegen determinístico | `Apkc/codegen_select.h` | `codegen_select()` |
| High-level pipeline | `raf_compile.h` | `raf_compile_file()` |
| Motor cognitivo T^7 | `rafaelia/verbovivo.c` | `verbovivo_main()` |
| Fiber-H hash + recall | `rafaelia/verbovivo.h` | `vv_scan()`, `vv_audit()`, `vv_svg()` |
| Runtime router | `Benchmark/raf_runtime_router.h` | seleção de backend por capacidade |

---

## Referências de profundidade

| Documento | Conteúdo |
|---|---|
| `CLAUDE.md` | Invariantes, buffer limits, extensão protocol, build commands |
| `Apkc/PROTOCOL.md` | APKc: dispatch modes, ELF layout, AXML pool, erros comuns |
| `docs/MULTI_AI_METHODOLOGY.md` | Handoff protocol detalhado, non-collision rules |
| `docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md` | Protocolo formal humano-agente |
| `docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md` | Governança de execução e SIMD |
| `docs/ROTINA_OPERACIONAL_BENCHMARKS.md` | Rotina de auditoria e benchmark |
| `docs/PROTOCOLO_CANONICO_COHERENCIA.md` | 50 sementes e gates de prova |
| `docs/AGENTES_CHECKLIST.md` | Checklist executável por sessão |
| `docs/AGENTES_DECISAO_LOG.md` | Log de conflitos e decisões entre agentes |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | Mapa em 5 níveis de profundidade |
