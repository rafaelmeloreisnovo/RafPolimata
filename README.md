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
- [`docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md`](docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md): metodologia operacional para governança, GPU/SIMD, cache, paralelismo, fail-safe/failover/rollback e evidência auditável.

- [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md`](docs/MAPA_ESTRUTURAL_REPOSITORIO.md): mapa profissional da disposição por tags/diretórios, estados (`VOID`/`PENDING`/`AUDIT`/`RUNTIME`/`REFERENCE`) e varredura estrutural em 5 níveis.

## Aviso importante

> Este material é técnico-acadêmico e **não substitui parecer jurídico profissional**.
> A aplicação em produção deve passar por validação de advogados habilitados em cada jurisdição relevante.


## Qualidade e automação

- Pipeline GitHub Actions em `.github/workflows/ci.yml` com validação do protocolo canônico, checagem C host, build estrito (`-Wall -Wextra -Werror`), smoke test do binário e relatório P(k).
- Hotfix de compilação aplicado no núcleo C para restaurar uma base compilável e facilitar futuras refatorações incrementais.

- [`docs/PROTOCOLO_FALSIFICABILIDADE_PK.md`](docs/PROTOCOLO_FALSIFICABILIDADE_PK.md): protocolo mínimo de falsificabilidade em P(k) com execução automática.
- [`docs/PROTOCOLO_CANONICO_COHERENCIA.md`](docs/PROTOCOLO_CANONICO_COHERENCIA.md): contrato canônico que liga as 50 sementes matemáticas a invariantes, arquivos e gates de prova/queda.
- [`docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md`](docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md): protocolo enterprise de dois ciclos para coerência semântica, execução técnica, fail-safe/failover/rollback e uso honesto de `TOKEN_VAZIO`.
- [`configs/semantic_coherence.yml`](configs/semantic_coherence.yml): YAML operacional para orquestração mutável de coerência, benchmark e falsificabilidade.
- [`configs/two_cycle_omega.yml`](configs/two_cycle_omega.yml): manifesto auditável dos campos canônicos, camadas linguísticas e invariantes técnicos dos dois ciclos.
- [`configs/operational_excellence.yml`](configs/operational_excellence.yml): manifesto verificável da rota de excelência operacional, estados, arquiteturas e controles de risco.
- [`docs/ROTINA_OPERACIONAL_BENCHMARKS.md`](docs/ROTINA_OPERACIONAL_BENCHMARKS.md): rotina operacional de auditoria, benchmark com prewarm/warmup e top 20 modelos.
- [`docs/LOGOTIPO_RAFAELIA_60COL.md`](docs/LOGOTIPO_RAFAELIA_60COL.md): logotipo ASCII de 60 colunas com variante colorida ANSI.
- [`assets/raf_operational_seal.svg`](assets/raf_operational_seal.svg): selo visual local dos gates operacionais, sem alegação de certificação.

## Evidence Discipline

RafPolimata separa hipótese, analogia, prova parcial, prova de compilação, prova runtime e prova em device real. `TOKEN_VAZIO` é estado válido quando falta ferramenta, dataset, hardware, log ou execução. O CI funciona como compilador de evidências: emite matrizes, executa gates e preserva lacunas sem promovê-las a `PASS`. A trilha auditável fica em [`docs/REPOSITORY_UNIVERSE_MATRIX.md`](docs/REPOSITORY_UNIVERSE_MATRIX.md) e [`docs/CLAIM_EVIDENCE_LOCK.md`](docs/CLAIM_EVIDENCE_LOCK.md).

## ApkC — micro-toolchain Android freestanding

Resumo: `Apkc/` contém um micro-toolchain experimental em C para gerar APK nativo mínimo por escrita direta de ZIP/APK, AndroidManifest AXML, DEX e ELF `.so`, com backends ARM64 e ARM32.

Status: experimental, com validação em andamento.

Ver:
- `docs/APKC_STRUCTURE.md`
- `docs/APKC_PROTOCOL.md`
- `docs/APKC_VALUE_AND_GAPS.md`
- `docs/CI_COMPILER_EXCELLENCE/README.md`
- `Apkc/proofs/`

| Gate | Status |
|---|---|
| Compile ApkC | NOT_RUN |
| Generate APK | NOT_RUN |
| ZIP parse | NOT_RUN |
| AXML parse | NOT_RUN |
| DEX SHA-1 | NOT_RUN |
| ELF readelf | NOT_RUN |
| APK signing | TOKEN_VAZIO |
| adb install | TOKEN_VAZIO |
| NativeActivity runtime | TOKEN_VAZIO |
