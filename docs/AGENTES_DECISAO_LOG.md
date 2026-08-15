# Log de Decisões entre Agentes

Este arquivo registra conflitos e decisões arquiteturais que envolvem mais de um agente ou precisam de rastreabilidade explícita.

> **Política atual:** `AGENTS.md` é o roteador comum; `docs/AGENTES.md` é o protocolo detalhado. Referências históricas a `CLAUDE.md` como autoridade primária, a `Main` com M maiúsculo ou a números antigos de seção permanecem apenas como histórico quando aparecem em decisões antigas.

---

## Como usar

1. Quando agentes/humanos propõem mudanças incompatíveis, abrir uma entrada aqui.
2. Não promover a proposta conflitante até existir resolução reproduzível ou decisão humana quando exigida.
3. Preservar evidência de ambos os lados; não apagar o caso negativo.
4. Registrar branch/PR/commit quando existirem.
5. Uma decisão histórica pode ser `SUPERSEDED` por nova entrada sem apagar a anterior.

---

## Template de entrada

```markdown
### DECISAO-NNN — <título>

- **Data de abertura**: YYYY-MM-DD
- **Participantes/superfícies**: <humano/agentes>
- **Base**: <repo/branch/commit>
- **Contexto**: <conflito>

#### Propostas / evidências

<proposta A, B, ... + evidência>

#### Critério de decisão

<teste, spec, receipt ou autoridade humana>

#### Resolução

<decisão e limites>

#### Estado

`PENDING` | `RESOLVED` | `ESCALATED` | `SUPERSEDED`

- **Data de resolução**: YYYY-MM-DD
- **Decidido por**: <humano/teste/spec>
- **Branch / PR / commit**: <referência>
```

---

## Critérios de escalação obrigatória para humano

Escalar quando a mudança envolver:

1. quebra ou alteração de invariante canônico em `AGENTS.md` / `docs/AGENTES.md`;
2. API/ABI/formato persistido público com migração incompatível;
3. nova dependência externa de risco material;
4. licença, privacidade, segredo ou segurança criptográfica;
5. exclusão/movimentação/quarentena com perda potencial de provenance;
6. alteração de política de branch/merge;
7. promoção de claim científico/externo que exceda a evidência disponível;
8. merge com gate material aberto.

---

## Decisões registradas

### DECISAO-001 — Campanha de callouts `> **Entrada canônica:** docs/AGENTES.md` em todos os docs

- **Data de abertura**: 2026-07-09
- **Agente A** (sessão/tipo): Claude — branch `claude/operational-excellence-agents-mabjye`
- **Agente B** (sessão/tipo): N/A — decisão unilateral do agente, validada pelo humano
- **Contexto**: Após criar a trilogia AGENTES (`docs/AGENTES.md`, `docs/AGENTES_CHECKLIST.md`, `docs/AGENTES_DECISAO_LOG.md`) nos PRs #108 e #109, identificou-se que documentos anteriores não tinham um ponteiro operacional unificado.

#### Proposta adotada

Adicionar callout de entrada canônica nos documentos aplicáveis para permitir navegação até a governança de agentes.

Regras históricas da campanha:

- callout imediatamente após o título;
- apontar para a seção então relevante de `docs/AGENTES.md`;
- não alterar código C/H;
- preservar documentos de profundidade.

#### Escopo executado

A campanha registrou 68 arquivos `.md` em `docs/` e `docs/arch/`, distribuídos em batches/PRs históricos.

#### Estado

`RESOLVED`

- **Data de resolução**: 2026-07-19
- **Decidido por**: rafaelmeloreisnovo via merges da campanha
- **Observação de 2026-08-15**: números de seção e a noção de uma única entrada `docs/AGENTES.md` foram posteriormente refinados por `DECISAO-002`; os callouts históricos continuam sendo rotas válidas, mas agentes novos devem iniciar em `AGENTS.md`.

---

### DECISAO-002 — Unificar Codex, Copilot, ChatGPT e Claude Code sem duplicar a verdade

- **Data de abertura**: 2026-08-15
- **Participantes/superfícies**: humano + ChatGPT/GitHub; afeta Codex, GitHub Copilot e Claude Code
- **Base**: `rafaelmeloreisnovo/RafPolimata@265baae3f0c1d8b2763e2eac24286e3a25dd8ace`
- **Branch**: `docs/unify-ai-agent-instructions-20260815`
- **Contexto**: os arquivos de onboarding divergiram entre si. `.github/copilot-instructions.md` estava excessivamente especializado no conversation indexer; `CLAUDE.md` continha afirmações históricas sobre “42 attractors” e no-libc global; `CODEX_FIX_PROTOCOL.md` tratava `TOKEN_VAZIO` como se tivesse codificação C universal; `MULTI_AI_METHODOLOGY.md` atribuía autoridade por fornecedor e citava branch `Main` com capitalização incorreta.

#### Evidências observadas

- `main` usa `main` como default branch.
- o caminho hosted x86/x86_64 do ApkC pode usar libc de desenvolvimento, enquanto targets freestanding preservam seus gates próprios;
- `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md` registra o claim forte de fixed-point convergence como falsificado na formulação anterior;
- GitHub Copilot possui mecanismo repo-wide e path-specific, permitindo retirar regras do conversation indexer do arquivo global;
- Codex usa `AGENTS.md` como instrução de repositório;
- `CLAUDE.md` pode funcionar como adaptador do Claude Code, sem duplicar o protocolo completo.

#### Resolução

Adotar arquitetura:

```text
AGENTS.md                       = roteador comum curto
  -> docs/AGENTES.md            = protocolo detalhado
  -> .github/copilot-instructions.md = adaptador Copilot repo-wide
  -> .github/instructions/*     = regras por caminho
  -> CLAUDE.md                  = adaptador Claude Code
  -> docs/CODEX_FIX_PROTOCOL.md = protocolo cirúrgico subordinado
  -> docs/MULTI_AI_METHODOLOGY.md = handoff por papel/evidência
```

Regras derivadas:

1. adaptadores não podem enfraquecer o protocolo canônico;
2. autoridade é por escopo/evidência, não por marca/modelo;
3. `TOKEN_VAZIO` é estado epistemológico e não possui codificação inteira universal;
4. hosted != freestanding;
5. `42` como range/index não implica 42 fixed-point attractors;
6. merge continua decisão humana explícita.

#### Estado

`PENDING_HUMAN_REVIEW`

- **Data de resolução**: TOKEN_VAZIO até revisão/merge
- **Decidido por**: TOKEN_VAZIO
- **Branch / PR**: `docs/unify-ai-agent-instructions-20260815` → PR a ser associado

---

## Histórico de resolução — resumo

| ID | Título | Data | Estado | Autoridade |
|---|---|---|---|---|
| DECISAO-001 | Callouts de entrada canônica | 2026-07-19 | RESOLVED | humano/merges históricos |
| DECISAO-002 | Unificação multiagente | 2026-08-15 | PENDING_HUMAN_REVIEW | TOKEN_VAZIO |
