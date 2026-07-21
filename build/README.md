# `build/` — outputs efêmeros e reproduzíveis

Este diretório é a raiz canônica dos artefatos gerados localmente e pela CI.

## Repository tracker

```text
build/repository-tracker/
├── manifest.json
├── state-sanitized.json
├── repository-tracker-snapshot.zip
└── shards/
    └── <shard-id>.json
```

A estrutura é recriada em cada execução. O estado longitudinal não é versionado em commits automáticos; ele permanece em:

```text
.tracker-state/state.json
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
3. A primeira observação de um repositório é baseline, não evento novo.
4. O snapshot ZIP usa ordem e timestamps determinísticos.
5. Nenhum código externo é clonado ou executado.
6. Falta de acesso permanece `TOKEN_VAZIO_AUTH_SCOPE_OR_NOT_FOUND`.
7. O cache é estado operacional; artifacts são evidência de execução.

## Limpeza

```sh
rm -rf build/repository-tracker .tracker-state
```

A remoção reinicia a linha de base e o contador de shards. Ela não altera nenhum repositório observado.
