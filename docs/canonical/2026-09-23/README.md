# RafPolimata — corte documental 2026-09-23

**Base observada:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Árvore de origem:** ecd9731dba52cbc255972cfe2fa92e041f74c5b5  
**Tipo:** documentação de engenharia ligada a revisão  
**Estado:** CANONICAL_CANDIDATE / REVIEW_REQUIRED  
**claim_allowed:** false

> [!NOTE]
> Este corte descreve o que foi observado na revisão acima. Alterações posteriores exigem novo corte ou atualização explicitamente ligada a uma nova revisão.

## Objetivo

Tornar o RafPolimata navegável como sistema de engenharia e evidência, usando superfícies nativas do GitHub e mantendo a separação entre código, build, execução, evidência e claim.

## Mapa

| Documento | Pergunta respondida |
|---|---|
| [REPOSITORY_SURVEY.md](REPOSITORY_SURVEY.md) | O que existe fisicamente no repositório? |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Como os principais subsistemas se relacionam? |
| [LANGUAGES.md](LANGUAGES.md) | O que significa suporte a cada linguagem? |
| [BUILD_TEST_EVIDENCE.md](BUILD_TEST_EVIDENCE.md) | Como construir, testar e interpretar os gates? |
| [WORKFLOW_CATALOG.md](WORKFLOW_CATALOG.md) | Quais workflows existem e para que área roteiam? |
| [GITHUB_SURFACES.md](GITHUB_SURFACES.md) | Quais recursos nativos do GitHub estão presentes ou faltam? |
| [EVIDENCE_AND_CLAIMS.md](EVIDENCE_AND_CLAIMS.md) | Como promover um resultado sem confundir níveis de prova? |
| [DOCUMENTATION_STYLE.md](DOCUMENTATION_STYLE.md) | Como escrever documentação nova sem aumentar entropia? |
| [GAPS_AND_NEXT.md](GAPS_AND_NEXT.md) | O que ainda exige decisão, execução ou evidência? |
| [documentation-manifest.v1.json](documentation-manifest.v1.json) | Qual é o inventário legível por máquina deste corte? |

## Fontes de autoridade usadas

1. GitHub, revisão main@f22efc099ac530d946ff2ec34954455f75632e92.
2. START HERE — A-A auditar — RAFAELIA, no Google Drive, usado como router de governança.
3. RAFAELIA — Implementação Latentes e Papers — Drive GitHub V1, usado para delimitar o papel do RafPolimata como produtor de evidência.
4. Arquivos, workflows, receipts e contratos existentes no próprio repositório.
5. Documentação oficial do GitHub apenas para comportamento das superfícies GitHub.

## Invariantes

~~~text
SOURCE != ARTIFACT
ARTIFACT != EXECUTION
EXECUTION != EVIDENCE
EVIDENCE != CLAIM
TOKEN_VAZIO != FAIL
TOKEN_VAZIO != PASS
generated != hand-edited-current
historical-receipt != current-state
~~~

## Critério de parada deste corte

A documentação termina onde começa uma ação que exige:
- execução não realizada;
- configuração administrativa não alterada;
- decisão jurídica/autoral não fornecida;
- hardware/provider não observado;
- claim científico que exige falsificador ou reprodução.

Esses pontos ficam tipados como gaps; não são preenchidos por narrativa.
