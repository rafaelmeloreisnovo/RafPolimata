# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `1b6d6cfe51f5c827201d45f196d8c2bb7478bb19`
- Estado: `REVIEW_REQUIRED`
- Arquivos: **1669**
- Relações: **1472**
- Fila de revisão: **1056**
- Bloqueadores: **0**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `DUPLICATE_REVIEW` | 25 |
| `INDEXED` | 607 |
| `LINK_REQUIRED` | 1012 |
| `REFERENCE_REPAIR` | 2 |
| `ROOT_REVIEW` | 15 |
| `SENSITIVITY_REVIEW` | 2 |

## Entradas canônicas

| Arquivo | Área | Evidência | Qualidade | Risco |
|---|---|---|---:|---:|
| `docs/AGENTES.md` | documentation | E2 | 90 | 0 |
| `docs/DOCUMENT_GOVERNANCE.md` | documentation | E2 | 90 | 0 |
| `docs/INDEX.md` | documentation | E2 | 90 | 0 |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | documentation | E3 | 100 | 0 |
| `ECOSYSTEM_RUNTIME_STATE.json` | canonical | E2 | 80 | 0 |
| `README.md` | canonical | E2 | 90 | 0 |

## Contrato operacional

```text
arquivo → identidade SHA-256 → área → dono lógico → relações → evidência
       → temporalidade → risco → rota → revisão/promoção
```

O catálogo completo está em `results/document-governance/catalog.jsonl`.
