# Índice gerado de governança documental

> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo
> versionado; não promove implementação ou prova apenas pela existência.

- Commit: `08b71841a7832195ee9e6cf0c7f190ab3933baba`
- Estado: `REVIEW_REQUIRED`
- Arquivos: **605**
- Relações: **792**
- Fila de revisão: **302**
- Bloqueadores: **0**

## Distribuição por rota

| Rota | Quantidade |
|---|---:|
| `CANONICAL` | 6 |
| `INDEXED` | 297 |
| `LINK_REQUIRED` | 290 |
| `ROOT_REVIEW` | 10 |
| `SENSITIVITY_REVIEW` | 2 |

## Entradas canônicas

| Arquivo | Área | Evidência | Qualidade | Risco |
|---|---|---|---:|---:|
| `docs/AGENTES.md` | documentation | E3 | 100 | 0 |
| `docs/DOCUMENT_GOVERNANCE.md` | documentation | E2 | 90 | 0 |
| `docs/INDEX.md` | documentation | E2 | 90 | 0 |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | documentation | E2 | 90 | 0 |
| `ECOSYSTEM_RUNTIME_STATE.json` | canonical | E2 | 80 | 0 |
| `README.md` | canonical | E3 | 100 | 0 |

## Contrato operacional

```text
arquivo → identidade SHA-256 → área → dono lógico → relações → evidência
       → temporalidade → risco → rota → revisão/promoção
```

O catálogo completo está em `results/document-governance/catalog.jsonl`.
