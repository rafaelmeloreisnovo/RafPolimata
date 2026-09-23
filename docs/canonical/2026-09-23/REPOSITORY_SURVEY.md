# Levantamento estrutural — RafPolimata

**Observed base:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Método:** árvore Git recursiva + metadados do repositório + leitura dirigida dos routers, contratos e workflows.

## 1. Escala observada

A árvore recursiva contém **1.752 entradas**. Entre os maiores domínios por número de entradas estão:

| Região | Entradas |
|---|---:|
| docs | 284 |
| Apkc | 227 |
| tests | 173 |
| scripts | 141 |
| tools | 91 |
| research | 74 |
| knowledge_base | 70 |
| .github | 66 |
| experiments | 48 |
| data | 47 |
| freestanding | 39 |
| native | 38 |
| Benchmark | 35 |
| COMPILA | 31 |
| configs | 28 |
| fractal_core | 28 |

O diretório docs contém **264 arquivos**; a diferença para 284 decorre de entradas de diretório.

## 2. Tipos de arquivo mais frequentes

| Extensão | Arquivos |
|---|---:|
| .md | 397 |
| .py | 214 |
| .h | 207 |
| .json | 182 |
| .c | 176 |
| .sh | 120 |
| .txt | 87 |
| .yml | 56 |
| .s | 16 |
| .bib | 8 |
| .svg | 6 |
| .jsonl | 6 |
| .yaml | 3 |
| .csv | 3 |
| .cpp | 2 |
| .rs | 1 |
| .toml | 1 |

> [!WARNING]
> Contagem de extensão prova presença de fonte, não compilabilidade, cobertura, maturidade ou execução.

## 3. Regiões funcionais

- **Apkc/** — compilação/empacotamento Android, perfis de linguagem, validação e provas.
- **freestanding/** — núcleo L0 sem syscall e sem runtime hospedado no contrato.
- **syscall/** — bindings opcionais de ABI/OS, deliberadamente separados do L0.
- **compiler/** — registros de alvo e arquitetura.
- **scripts/** — validadores, geração, auditoria, compilação e orquestração.
- **tests/** — unitários, fixtures, falsificadores e smoke tests.
- **ci/** — contratos e relatórios consumidos por gates.
- **evidence/**, **proofs/**, **receipts/** — níveis distintos de prova e cadeia de custódia.
- **docs/** — documentação manual, histórica, de evidência e gerada.
- **native/** — módulos nativos específicos, incluindo RAF Hash Fabric.
- **research/** e **experiments/** — pesquisa e experimentação; presença não promove claim.

## 4. Estado do repositório no GitHub

| Superfície | Estado observado |
|---|---|
| Visibilidade | public |
| Default branch | main |
| GitHub language principal | C |
| Issues | habilitado |
| Projects | habilitado |
| Wiki | habilitado |
| Discussions | habilitado |
| Pages | desabilitado |
| Topics | nenhum observado |
| Description | ausente |
| Homepage | ausente |
| Licença reconhecida no nível do repositório | ausente |
| Auto-merge | desabilitado |
| Delete branch on merge | desabilitado |
| Web commit signoff required | desabilitado |

A ausência de licença reconhecida no nível do repositório **não** autoriza inferir ausência de direitos, domínio público ou aplicar a licença de um submódulo ao conjunto inteiro.

## 5. Branch/rules observadas

A API de branch não reportou proteção clássica em main nem required status checks. Existe, porém, um ruleset ativo de repositório que bloqueia:

- deletion;
- non-fast-forward.

Não foi observado, nesse ruleset, requisito de review, CODEOWNERS ou status checks. Isso é estado de configuração observado, não recomendação automática de política.

## 6. Release observada

Há uma prerelease:

- tag: V1.0.0;
- nome: Compilador de APK em C;
- publicada em 2026-06-14;
- sem assets;
- corpo de release vazio.

Release histórica não equivale a estado atual de main.

## 7. Comunidade e navegação antes deste corte

Já existiam:
- .github/SECURITY.md;
- .github/pull_request_template.md;
- duas issue templates Markdown especializadas;
- Copilot instructions e instruções por área;
- GitHub Skill;
- 50 workflows;
- routers documentais e governança L0–L5.

Não existiam na árvore observada:
- CONTRIBUTING.md;
- SUPPORT.md;
- CODE_OF_CONDUCT.md;
- CITATION.cff;
- .github/CODEOWNERS;
- issue forms YAML;
- .github/ISSUE_TEMPLATE/config.yml.

## 8. Fronteira

Este levantamento é uma fotografia de estrutura. Ele não é:
- certificado de segurança;
- auditoria jurídica final;
- prova de execução física;
- cobertura integral de cada documento;
- resultado científico.

R3 = ⟨F_ok: superfície estrutural inventariada; F_gap: settings administrativos e resultados futuros mudam fora deste snapshot; F_next: ligar navegação ao sistema de contribuição, evidência e CI sem promover claims⟩.
