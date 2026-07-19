# Log de Decisões entre Agentes

Este arquivo registra conflitos e decisões arquiteturais que envolvem mais
de um agente ou que precisam de rastreabilidade explícita. Foi criado para
preencher a lacuna referenciada em `docs/MULTI_AI_METHODOLOGY.md`:

> "Create `docs/DECISION_LOG.md` with both proposals, pros/cons,
> and a recommended resolution."

---

## Como usar

1. Quando dois agentes propõem mudanças incompatíveis, abrir uma entrada aqui.
2. Não mergear nenhuma das propostas até a resolução estar marcada como `RESOLVED`.
3. Se humano indisponível: criar branch `decision/<topic>` + PR + entrada aqui.
4. Após resolução, marcar a entrada como `RESOLVED` com data e decisão final.

---

## Template de entrada

```markdown
### DECISAO-NNN — <título curto descritivo>

- **Data de abertura**: YYYY-MM-DD
- **Agente A** (sessão/tipo): <Claude / Codex / ChatGPT / Humano + identificação da sessão>
- **Agente B** (sessão/tipo): <idem>
- **Contexto**: breve descrição do que motivou o conflito

#### Proposta A

<descrição técnica da proposta A>

**Prós:**
- ...

**Contras:**
- ...

#### Proposta B

<descrição técnica da proposta B>

**Prós:**
- ...

**Contras:**
- ...

#### Resolução recomendada

<qual proposta adotar e por quê — ou proposta híbrida>

#### Estado

`PENDING` | `RESOLVED` | `ESCALATED`

- **Data de resolução**: YYYY-MM-DD (quando RESOLVED)
- **Decidido por**: <agente ou humano que fechou>
- **Branch / PR**: `decision/<topic>` → PR #NNN (quando aplicável)
```

---

## Critérios de escalação obrigatória para humano

Qualquer decisão que envolva um dos itens abaixo **não pode ser resolvida por
agentes** — deve ser `ESCALATED` e aguardar revisão humana:

1. Mudança que viola um invariante de `CLAUDE.md`
2. Nova dependência externa (ferramenta, biblioteca, serviço)
3. Mudança de licença ou termos de redistribuição
4. Qualquer aspecto de segurança criptográfica
5. Mudança na estrutura de PR/merge (ex: remover proteção de branch Main)

---

## Decisões registradas

### DECISAO-001 — Campanha de callouts `> **Entrada canônica:** docs/AGENTES.md` em todos os docs

- **Data de abertura**: 2026-07-09
- **Agente A** (sessão/tipo): Claude — branch `claude/operational-excellence-agents-mabjye`
- **Agente B** (sessão/tipo): N/A — decisão unilateral do agente, validada pelo humano
- **Contexto**: Após criar a trilogia AGENTES (docs/AGENTES.md, docs/AGENTES_CHECKLIST.md,
  docs/AGENTES_DECISAO_LOG.md) nos PRs #108 e #109, identificou-se que todos os `.md`
  anteriores em `docs/` e `docs/arch/` referenciavam documentos de profundidade mas não
  tinham um ponteiro canônico para AGENTES.md. Sem esse ponteiro, um agente que abre um
  documento específico não descobre o ponto de entrada unificado.

#### Proposta adotada

Adicionar em cada arquivo `.md` preexistente (que não seja da trilogia AGENTES) um
blockquote de entrada imediatamente após o `# Título`:

```markdown
> **Entrada canônica:** docs/AGENTES.md §N (desc) e §N (desc). <frase contextualizando o doc>.
```

Regras de inserção:
- 1 linha, imediatamente após `# Título`, antes do primeiro parágrafo
- Seções referenciadas = as seções de AGENTES.md mais relevantes para o doc em questão
- Callouts não-padrão pré-existentes (`> **Agente novo?**`, `> Para o ciclo completo...`,
  `> **Ponto de entrada unificado:**`, `> **Resumo executivo em**`) convertidos para o
  formato uniforme `> **Entrada canônica:**`
- Zero alterações em código C/H

**Prós:**
- Qualquer agente que abra qualquer doc chega a AGENTES.md em ≤ 2 cliques
- Formato uniforme é parseável por automação futura
- Não remove nem substitui nenhum documento de profundidade

**Contras:**
- Adiciona 3–5 linhas por arquivo (visível no diff)
- Exige manutenção caso AGENTES.md mude as numerações de seção

#### Escopo executado

68 arquivos `.md` em `docs/` e `docs/arch/` cobertos em batches 1–11:

| Batch | PR | Arquivos |
|---|---|---|
| 1–9 | #110–#135 | docs/ (55 arquivos) |
| 10 | #139 | docs/arch/ (4 arquivos) |
| 11a–11f | #139 | docs/ remanescentes (13 arquivos — callouts não-padrão + arquivos adicionados após início da campanha) |

#### Estado

`RESOLVED`

- **Data de resolução**: 2026-07-19
- **Decidido por**: rafaelmeloreisnovo (aprovação tácita via merge dos PRs #110–#138)
- **Branch / PR**: `claude/operational-excellence-agents-mabjye` → PR #139

---

## Histórico de resolução (resumo executivo)

| ID | Título | Data | Estado | Decidido por |
|---|---|---|---|---|
| DECISAO-001 | Campanha callouts Entrada canônica AGENTES.md | 2026-07-19 | RESOLVED | rafaelmeloreisnovo |

*Atualizar esta tabela sempre que uma entrada muda de estado.*
