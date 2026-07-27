# Science Learning Engine

> **Entrada canônica:** `docs/AGENTES.md §7` (governança documental) e `§6` (excelência operacional).  
> Script de aquisição de conhecimento científico — ORCID + Zenodo → 4 estágios evolutivos → `knowledge_base/` → `vv_scan_buf()`.

## Visão geral

`scripts/science_learning_engine.py` busca fenômenos científicos em ORCID e Zenodo,
classifica resultados em 4 estágios evolutivos e gera uma base de conhecimento
estruturada que pode alimentar o motor verbovivo/HDC via `vv_scan_buf()`.

## 4 Estágios evolutivos

| Estágio | Nome | Critério de promoção | Artefatos gerados |
|--------:|------|----------------------|-------------------|
| **1** | Descoberta | Qualquer hit com título (ORCID ou Zenodo) | `records.json`, `urls.txt` |
| **2** | Candidato | DOI presente + resumo ≥ 100 chars + keywords | `bibliography.bib`, `bibliography.md` |
| **3** | Validado | Zenodo curado + licença aberta + `download_url` | `.bib` completo, `metadata.json` |
| **4** | Canônico | ≥ 2 domínios cruzados com mesmo DOI | `synthesis.txt` — pronto para `vv_scan_buf()` |

## Domínios e queries padrão

| Domínio | Queries padrão |
|---------|----------------|
| `physics` | quantum mechanics, thermodynamics, electromagnetic fields, relativity |
| `chemistry` | molecular dynamics, reaction kinetics, chemical bonding, spectroscopy |
| `biology` | evolutionary biology, genetics, neuroscience, cellular biology |
| `mathematics` | differential equations, topology, number theory, complex analysis |

## APIs utilizadas

| API | Endpoint | Auth |
|-----|----------|------|
| ORCID | `https://pub.orcid.org/v3.0/search?q={query}&rows=N` | nenhuma (`Accept: application/json`) |
| Zenodo | `https://zenodo.org/api/records?q={query}&type=publication&access_right=open&size=N` | nenhuma |

## Estrutura de diretório

```
knowledge_base/
├── physics/
│   ├── stage_1_discovery/
│   │   ├── records.json
│   │   └── urls.txt
│   ├── stage_2_candidate/
│   │   ├── bibliography.bib
│   │   └── bibliography.md
│   ├── stage_3_validated/
│   │   ├── bibliography.bib
│   │   ├── bibliography.md
│   │   └── metadata.json
│   └── stage_4_canonical/
│       └── synthesis.txt    ← compatível com vv_scan_buf()
├── chemistry/   [mesma estrutura]
├── biology/     [mesma estrutura]
├── mathematics/ [mesma estrutura]
└── AQUISICAO_RESUMO.md
```

## Uso

```bash
# Busca padrão (todos os domínios):
python3 scripts/science_learning_engine.py --output knowledge_base/

# Domínio específico + query:
python3 scripts/science_learning_engine.py \
    --domains physics,mathematics \
    --query "quantum topology" \
    --max-per-stage 10

# Dry-run (não cria arquivos):
python3 scripts/science_learning_engine.py --dry-run
```

## Conexão com verbovivo/HDC

Os textos de `stage_4_canonical/synthesis.txt` são compatíveis com `vv_scan_buf()`:

```bash
# Compilar verbovivo:
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \
    rafaelia/verbovivo.c rafaelia/fiber_relmat.c -lm -o verbovivo

# Alimentar sínteses canônicas:
for domain in physics chemistry biology mathematics; do
    f="knowledge_base/${domain}/stage_4_canonical/synthesis.txt"
    [ -f "$f" ] && cat "$f" | ./verbovivo -s > "engram_${domain}.svg" || true
done
```

## Estado TOKEN_VAZIO

Um domínio fica em TOKEN_VAZIO no estágio 4 quando nenhum DOI aparece em ≥ 2 domínios
cruzados. O arquivo `synthesis.txt` é criado com o marcador explícito e instrução `PENDING`.
Isso não bloqueia os estágios 1-3 — eles são sempre materializados se houver hits.

O `AQUISICAO_RESUMO.md` gerado documenta o estado de cada domínio e contém a lista
de DOIs canônicos com instrução de uso para o modelo verbovivo.

## Invariantes de governança

- Arquivos em `knowledge_base/` são gerados — **nunca editar manualmente** (Regra 8 de `docs/AGENTES.md §4`)
- Para corrigir: ajustar queries em `DOMAINS` ou o critério de promoção no script e regenerar
- TOKEN_VAZIO explícito é preferível a silêncio (Regra 7 de `docs/AGENTES.md §4`)
