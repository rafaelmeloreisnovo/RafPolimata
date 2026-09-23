# Superfícies GitHub — presença, uso e gaps

**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92

O objetivo desta página é separar recursos de documentação, colaboração, automação e política administrativa.

## 1. Matriz

| Superfície | Estado antes deste corte | Ação documental deste corte |
|---|---|---|
| README | presente, corte 2026-09-06 | adicionar rota corrente |
| docs/INDEX.md | presente, corte 2026-09-06 | adicionar rota corrente |
| SECURITY.md | presente | preservar |
| Pull request template | presente, simples | ampliar com evidência/claim/rollback |
| Issue templates Markdown | 2 especializadas | preservar |
| Issue Forms YAML | ausentes | adicionar formas gerais |
| CONTRIBUTING.md | ausente | adicionar |
| SUPPORT.md | ausente | adicionar |
| CODE_OF_CONDUCT.md | ausente | adicionar |
| CITATION.cff | ausente | adicionar sem inventar licença/DOI |
| CODEOWNERS | ausente | adicionar roteamento para owner atual |
| GitHub Actions | 50 workflows | catalogar |
| Discussions | habilitado | documentar; templates/categorias não auditados |
| Wiki | habilitado | documentar; docs canônicas permanecem no Git |
| Pages | desabilitado | não habilitar por documentação |
| Projects | habilitado | configuração interna não auditada |
| Releases | 1 prerelease observada | registrar estado |
| Topics | nenhum | gap de metadata |
| Repository description | ausente | gap de metadata |
| Repository license detection | ausente | gap jurídico/proveniência |
| Dependabot config | não observado | gap/opção, não ativado automaticamente |
| Ruleset | ativo: deletion + non-fast-forward | documentar |
| Required PR/status checks | não observados | gap de governança, decisão administrativa |

## 2. Community health files

GitHub usa arquivos como README, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, SUPPORT, templates e CITATION para orientar a interface de colaboração.

Este corte preenche os arquivos que podem ser adicionados com segurança documental. Ele **não** cria:
- licença global;
- política comercial;
- canal privado que não foi verificado;
- obrigação de SLA;
- configuração administrativa de branch;
- GitHub Pages;
- financiamento.

## 3. Issue Forms

As forms adicionadas se destinam a:
- bug reproduzível;
- gap de evidência;
- documentação.

Elas não substituem as issue templates especializadas existentes. config.yml mantém blank issues habilitados para não bloquear classes de trabalho ainda não modeladas.

## 4. CODEOWNERS

CODEOWNERS pode solicitar review automaticamente quando a regra correspondente do GitHub estiver ativa. O arquivo por si só não impõe aprovação.

O corte roteia as áreas ao proprietário GitHub observado, sem afirmar equipe inexistente.

## 5. Citation

CITATION.cff facilita a citação do software. Sem DOI/release acadêmica validada, a recomendação é citar:
- título;
- autor;
- URL do repositório;
- revisão/commit usado na pesquisa.

Não é adicionada licença ao CFF porque o repositório raiz não possui licença canônica observada.

## 6. Rules e branch governance

Estado observado:
- main sem proteção clássica reportada;
- ruleset ativo impede deletion e non-fast-forward;
- required review/status checks não observados.

Uma mudança desses controles altera a política operacional e deve ser deliberada separadamente; documentação não deve ativá-los implicitamente.

## 7. Pages, Wiki e Discussions

- Pages está desabilitado; os Markdown versionados são a fonte documental.
- Wiki está habilitado, mas conteúdo de Wiki não foi usado como autoridade neste corte.
- Discussions está habilitado; configuração de categorias/templates não foi auditada.

## 8. Links oficiais de referência

- README e community health: https://docs.github.com/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-readmes
- Contribution guidelines: https://docs.github.com/communities/setting-up-your-project-for-healthy-contributions/setting-guidelines-for-repository-contributors
- Issue forms: https://docs.github.com/communities/using-templates-to-encourage-useful-issues-and-pull-requests/syntax-for-issue-forms
- CODEOWNERS: https://docs.github.com/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-code-owners
- Citation files: https://docs.github.com/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/about-citation-files
- Protected branches: https://docs.github.com/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches

R3 = ⟨F_ok: superfícies documentais mapeadas e lacunas não destrutivas preenchidas; F_gap: license, metadata e políticas administrativas exigem decisão própria; F_next: revisar community files e depois decidir settings por PR/política separada⟩.
