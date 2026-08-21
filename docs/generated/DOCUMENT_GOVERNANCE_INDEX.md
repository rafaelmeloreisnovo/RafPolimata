# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `6d0f84a7073723c2f13a137c1235565943779139`
- Estado: `REVIEW_REQUIRED`
- Arquivos: **1347**
- Relações: **1257**
- Fila de revisão: **839**
- Bloqueadores: **0**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `DUPLICATE_REVIEW` | 23 |
| `INDEXED` | 502 |
| `LINK_REQUIRED` | 800 |
| `REFERENCE_REPAIR` | 2 |
| `ROOT_REVIEW` | 12 |
| `SENSITIVITY_REVIEW` | 2 |

## Entradas canônicas

| Arquivo | Área | Evidência | Qualidade | Risco |
|---|---|---|---:|---:|
| `docs/AGENTES.md` | documentation | E2 | 90 | 0 |
| `docs/DOCUMENT_GOVERNANCE.md` | documentation | E2 | 90 | 0 |
| `docs/INDEX.md` | documentation | E2 | 90 | 0 |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | documentation | E3 | 100 | 0 |
| `ECOSYSTEM_RUNTIME_STATE.json` | canonical | E2 | 80 | 0 |
| `README.md` | canonical | E3 | 100 | 0 |

## Contrato operacional

```text
arquivo → identidade SHA-256 → área → dono lógico → relações → evidência
       → temporalidade → risco → rota → revisão/promoção
```

O catálogo completo está em `results/document-governance/catalog.jsonl`.
