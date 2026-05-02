# RafPolimata

RafPolimata é uma proposta de **arquitetura semântica-tecnológica-jurídica** para sistemas complexos que combinam:

- modelagem matemática (toro, dinâmica discreta, entropia/sintropia),
- engenharia de software e criptografia aplicada,
- semiótica e linguística comparada,
- governança de licenças e conformidade normativa em Estados Democráticos de Direito.

Este repositório passa a incluir uma documentação de referência de nível avançado (estilo pós-doc) para orientar implementação, validação e auditoria.

## Documentação principal

- [`docs/ARQUITETURA_21_NIVEIS.md`](docs/ARQUITETURA_21_NIVEIS.md): modelo em 21 níveis/camadas para contextualização e fluxo de sentido.
- [`docs/DEZ_DIMENSOES_SEMANTICAS.md`](docs/DEZ_DIMENSOES_SEMANTICAS.md): 10 dimensões semânticas e dinâmicas do sentido.
- [`docs/MATRIZ_JURIDICO_TECNOLOGICA.md`](docs/MATRIZ_JURIDICO_TECNOLOGICA.md): trilha jurídico-tecnológica com análise supralegal e risco.
- [`docs/LICENCAS_COMPARADAS.md`](docs/LICENCAS_COMPARADAS.md): comparação entre licenças/termos (incluindo referência a BLAKE3, e práticas de grandes fornecedores como Microsoft/Oracle/Google).
- [`docs/ATRATORES_42_JURIDICOS.md`](docs/ATRATORES_42_JURIDICOS.md): framework de 42 atratores jurídicos com 4 níveis por conteúdo e 7 direções antagônicas.
- [`docs/BASES_SUPRALEGAIS_E_PADROES.md`](docs/BASES_SUPRALEGAIS_E_PADROES.md): mapeamento de bases supralegais, constitucionais e padrões técnicos internacionais.
- [`docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md`](docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md): protocolo técnico para validação prévia, execução acoplada e auditoria IA↔humanos.
- [`docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md`](docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md): roadmap para evolução conjunta de código e documentação.
- [`docs/RELEASE_NOTES_PENDING.md`](docs/RELEASE_NOTES_PENDING.md): registro de pendências de release (pending only).

## Aviso importante

> Este material é técnico-acadêmico e **não substitui parecer jurídico profissional**.
> A aplicação em produção deve passar por validação de advogados habilitados em cada jurisdição relevante.


## Qualidade e automação

- Pipeline GitHub Actions em `.github/workflows/ci.yml` com build estrito (`-Wall -Wextra -Werror`) e smoke test do binário.
- Hotfix de compilação aplicado no núcleo C para restaurar uma base compilável e facilitar futuras refatorações incrementais.

- [`docs/PROTOCOLO_FALSIFICABILIDADE_PK.md`](docs/PROTOCOLO_FALSIFICABILIDADE_PK.md): protocolo mínimo de falsificabilidade em P(k) com execução automática.
