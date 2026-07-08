# Checklist operacional por sessão de agente

Este checklist é executável — marque cada item antes de prosseguir.
Para contexto completo de cada item, consulte `docs/AGENTES.md`.

---

## Startup — antes de qualquer mudança de código

```
[ ] 1. Ler CLAUDE.md (invariantes, buffer limits, extensão protocol)

[ ] 2. Confirmar branch destino:
       git branch --show-current
       → deve ser: claude/operational-excellence-agents-mabjye

[ ] 3. Rodar syntax check freestanding:
       clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
         -ffreestanding -I Apkc Apkc/apkc.c
       → deve retornar sem erros antes e depois de qualquer edição em Apkc/

[ ] 4. Grep de invariante freestanding em Apkc/:
       grep -rn "malloc\|calloc\|free(\|#include <stdlib" Apkc/*.c Apkc/*.h
       → deve retornar vazio

[ ] 5. Identificar e classificar a tarefa:
       [ ] documentação
       [ ] implementação de feature
       [ ] refatoração / limpeza
       [ ] conformidade / auditoria
       [ ] pesquisa / investigação
```

---

## Pré-código — por tipo de mudança

### Se for adicionar uma linguagem à tabela

```
[ ] Adicionar exatamente 1 linha em Apkc/lang_profile.h → _lang_table[]
[ ] Definir flags: use_asm | use_script | use_fork
[ ] Se DEX: use_d8=1
[ ] Se JSX: jsx_node=1
[ ] Rodar syntax check freestanding após a adição
[ ] Nenhuma outra mudança necessária para o pipeline
```

### Se for adicionar encoder ARM64

```
[ ] Escrever static inline u32 em Apkc/arch_arm64.h
[ ] Adicionar case correspondente em asm_insn64() em Apkc/apkc.c
[ ] Se for família de equivalência: verificar semântica idêntica para TODOS os encodings
[ ] Adicionar golden test em tests/test_arm64_encoders.py
[ ] Rodar: python3 tests/test_arm64_encoders.py
```

### Se for adicionar seção ELF

```
[ ] Estender Apkc/fmt_elf.h com nova seção
[ ] Atualizar sh_link de .hash, .dynsym, .dynamic (todos apontam para .dynstr)
[ ] Atualizar e_shstrndx se .shstrtab mudou de índice
[ ] Conferir contagem de seções em todos os cross-refs
[ ] Rodar syntax check freestanding
```

### Se for documentação apenas

```
[ ] Linkar o documento a um arquivo de código, gate ou script executável
[ ] Não remover TOKEN_VAZIO sem implementar o que ele sinaliza
[ ] Atualizar MAPA_ESTRUTURAL_REPOSITORIO.md se novo arquivo muda a estrutura
```

---

## Durante a execução

```
[ ] Syntax check freestanding após CADA edição em Apkc/*.h ou Apkc/*.c
[ ] Estados VOID/PENDING/AUDIT/RUNTIME/REFERENCE explícitos em tudo incompleto
[ ] TOKEN_VAZIO quando falta evidência — nunca fabricar PASS
[ ] Nenhuma alocação dinâmica em hot paths (nem mesmo alloca)
[ ] Commits atômicos: código + documentação correspondente no mesmo commit
```

---

## Shutdown — antes do push

```
[ ] Syntax check freestanding passa limpo

[ ] Grep de malloc/calloc/free em Apkc/ retorna vazio:
    grep -rn "malloc\|calloc\|free(\|#include <stdlib" Apkc/*.c Apkc/*.h

[ ] Commit message no formato:
    type(scope): o que foi feito + por quê
    ✓  feat(Apkc): add PHP to _lang_table — completes OS coverage
    ✓  fix(arch_arm64): guard NULL return in lang_profile_find callers
    ✗  update lang_profile.h

[ ] Todo trabalho incompleto tem VOID ou PENDING explícito com descrição

[ ] Riscos e pendências listados no body do PR

[ ] PR aberto como DRAFT se CI não está completamente verde

[ ] Se houve conflito com outro agente: entrada criada em docs/AGENTES_DECISAO_LOG.md
```

---

## Critério de encerramento de sessão

A sessão só é encerrada com PR pronto quando:

1. Gates bloqueantes do CI passam
2. Relatório P(k) em PASS (`results/first_test_report.json` → `"verdict": "PASS"`)
3. Nenhum arquivo crítico (listado em `CLAUDE.md → Key files`) foi alterado sem documentação correspondente
4. Todas as pendências estão declaradas como PENDING no PR ou em código

Se qualquer item acima estiver incompleto, deixar PR como DRAFT com comentário
explicativo — não fingir que está pronto.

---

## Referência rápida — buffer limits (nunca ultrapassar)

| Buffer | Tamanho | Uso |
|---|---|---|
| `_code64` | 64 KB | Código ARM64 montado |
| `_so64_buf` | 32 KB | Saída ELF64 .so |
| `_fork_out` | 1 MB | Saída de compilador externo (fork+exec) |
| `_dex_buf` | 200 B | Stub classes.dex mínimo |
| dynstr (stack) | 512 B | Nomes de símbolos ELF dinâmicos |
| elfhash (stack) | 64 B | Hash table ELF (nbucket=1) |
| AXML sv[] | 50 entradas | Pool de strings do AXML builder |
