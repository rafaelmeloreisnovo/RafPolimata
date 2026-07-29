# `build/` — outputs efêmeros e reproduzíveis

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `ci-governance`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · ci`](../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)

Este diretório é a raiz canônica dos artefatos gerados localmente e pela CI.

## Repository tracker

```text
build/repository-tracker/
├── manifest.json
├── state-sanitized.json
├── repository-tracker-snapshot.zip
├── shards/
│   └── <commit-or-fork-shard-id>.json
├── pr-shards/
│   └── <pull-request-shard-id>.json
└── learning/
    ├── pr-context.json
    └── pr-context.md
```

A estrutura é recriada em cada execução. O estado longitudinal não é versionado em commits automáticos; ele permanece em:

```text
.tracker-state/
├── state.json
└── pr-context-state.json
```

Na GitHub Actions, `.tracker-state/` é restaurado e salvo por cache. Isso evita:

- um commit automático a cada quinze minutos;
- poluição do histórico;
- loops em que o tracker detecta o próprio commit;
- gravação acidental de token;
- crescimento ilimitado da branch principal.

## Invariantes

1. `build/` nunca é fonte de verdade autoral.
2. Todo shard possui payload SHA-256 e hash encadeado.
3. A primeira observação de repositório, fork ou PR é baseline, não evento novo.
4. Commits/forks e PRs usam cadeias separadas.
5. O snapshot ZIP usa ordem e timestamps determinísticos.
6. Nenhum código externo é clonado ou executado.
7. Falta de acesso permanece `TOKEN_VAZIO_AUTH_SCOPE_OR_NOT_FOUND`.
8. O cache é estado operacional; artifacts são evidência de execução.
9. Índice semântico não é inferência de intenção ou autoria.

## Limpeza

```sh
rm -rf build/repository-tracker .tracker-state
```

A remoção reinicia as linhas de base e os contadores de shards. Ela não altera nenhum repositório observado.
