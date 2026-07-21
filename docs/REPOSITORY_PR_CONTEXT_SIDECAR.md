# Repository PR Context Sidecar

> **Entrada:** `scripts/repo_pr_context_tracker.py`  
> **Estado:** `AUDIT / CANDIDATE`  
> **Modo:** somente leitura  
> **Saída:** `build/repository-tracker/learning/`  
> **Claims:** `claim_allowed=false`

## Finalidade

O sidecar complementa o rastreador de commits e forks com o estado recente dos pull requests dos mesmos repositórios permitidos.

Para cada PR, registra:

- número;
- título normalizado;
- estado aberto ou fechado;
- estado draft;
- data de criação e atualização;
- data de merge, quando existente;
- branch de origem;
- branch de destino;
- SHA da cabeça;
- autor GitHub quando disponível;
- etiquetas semânticas determinísticas;
- impressão digital da mudança.

## Baseline

A primeira consulta estabelece a linha de base:

```text
primeira leitura de PR ≠ PR recém-criado
```

Um evento só nasce quando a impressão digital do PR muda entre dois ciclos.

A impressão digital inclui:

```text
number
title
state
draft
merged_at
updated_at
head_sha
head_ref
base_ref
```

## Cadeia própria

Cada alteração de PR recebe:

\[
p_n=SHA256(JSON_{canônico}(PR_n))
\]

\[
h_n=SHA256(h_{n-1}\parallel shard_n\parallel p_n)
\]

Os shards ficam em:

```text
build/repository-tracker/pr-shards/
```

A cadeia de PRs é separada da cadeia de commits para evitar que duas unidades epistemicamente diferentes sejam contadas como a mesma coisa.

## Relatório evolutivo

O sidecar gera:

```text
build/repository-tracker/learning/
├── pr-context.json
└── pr-context.md
```

O relatório agrega:

- número de repositórios acessíveis;
- PRs alterados no ciclo;
- histórico limitado;
- estados de PR;
- famílias de repositórios;
- etiquetas semânticas;
- próximo shard;
- cabeça da cadeia.

## Semântica permitida

As etiquetas são produzidas por regras visíveis, compartilhadas com o rastreador de commits:

```text
build
ci
test
docs
security
performance
android
virtualization
kernel
crypto
governance
fix
feature
```

Logo:

\[
\text{etiqueta semântica}
\neq
\text{compreensão humana}
\]

\[
\text{mudança de PR}
\neq
\text{qualidade, autoria transferida ou causalidade}
\]

Os pesos permanecem:

```text
TOKEN_VAZIO_CALIBRATION
```

## Limites

- lê no máximo seis PRs recentes por repositório;
- usa no máximo quarenta requisições por execução;
- não comenta em PRs;
- não altera labels;
- não fecha ou reabre PRs;
- não solicita revisão;
- não executa código das branches observadas;
- não calcula similaridade autoral probatória.

## Testes

```sh
python3 -m unittest tests.test_repo_pr_context_tracker
```

Os testes cobrem:

1. baseline inicial;
2. mudança posterior detectada;
3. relatório com `claim_allowed=false`;
4. histórico limitado.

## Fechamento

```text
PR observado
→ estado congelado
→ comparação temporal
→ delta
→ shard
→ cadeia
→ relatório semântico
→ TOKEN_VAZIO onde não existe prova
```
