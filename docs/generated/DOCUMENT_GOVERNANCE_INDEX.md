# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `9f39e0489439482bc9462769b339846968970f54`
- Estado: `REVIEW_REQUIRED`
- Arquivos: **1303**
- Relações: **1206**
- Fila de revisão: **812**
- Bloqueadores: **0**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `DUPLICATE_REVIEW` | 23 |
| `INDEXED` | 485 |
| `LINK_REQUIRED` | 774 |
| `REFERENCE_REPAIR` | 1 |
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
