# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `c2e4279930fd9402dfc2d5b601dd8ac00c3cb992`
- Estado: `FAIL`
- Arquivos: **608**
- Relações: **795**
- Fila de revisão: **306**
- Bloqueadores: **1**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `INDEXED` | 296 |
| `LINK_REQUIRED` | 290 |
| `QUARANTINE_REVIEW` | 1 |
| `REFERENCE_REPAIR` | 3 |
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
