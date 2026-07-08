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

*Nenhuma decisão registrada ainda. Esta seção será preenchida conforme conflitos
surgem entre sessões e agentes.*

---

## Histórico de resolução (resumo executivo)

| ID | Título | Data | Estado | Decidido por |
|---|---|---|---|---|
| — | — | — | — | — |

*Atualizar esta tabela sempre que uma entrada muda de estado.*
