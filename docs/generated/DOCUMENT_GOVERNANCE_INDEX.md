# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `4ffb1293ae90be7dab8f8ef3727d9e338972ce9b`
- Estado: `FAIL`
- Arquivos: **553**
- Relações: **789**
- Fila de revisão: **252**
- Bloqueadores: **1**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `INDEXED` | 295 |
| `LINK_REQUIRED` | 237 |
| `QUARANTINE_REVIEW` | 1 |
| `REFERENCE_REPAIR` | 2 |
| `ROOT_REVIEW` | 10 |
| `SENSITIVITY_REVIEW` | 2 |

## Entradas canônicas

| Arquivo | Área | Evidência | Qualidade | Risco |
|---|---|---|---:|---:|
| `docs/AGENTES.md` | documentation | E3 | 100 | 0 |
| `docs/DOCUMENT_GOVERNANCE.md` | documentation | E2 | 90 | 0 |
| `docs/INDEX.md` | documentation | E3 | 100 | 0 |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | documentation | E2 | 90 | 0 |
| `ECOSYSTEM_RUNTIME_STATE.json` | canonical | E2 | 80 | 0 |
| `README.md` | canonical | E3 | 100 | 0 |

## Contrato operacional

```text
arquivo → identidade SHA-256 → área → dono lógico → relações → evidência
       → temporalidade → risco → rota → revisão/promoção
```

O catálogo completo está em `results/document-governance/catalog.jsonl`.
