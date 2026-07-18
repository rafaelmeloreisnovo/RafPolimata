# RafPolimata

RafPolimata é uma proposta de **arquitetura semântica-tecnológica-jurídica** para sistemas complexos que combinam:

- modelagem matemática (toro, dinâmica discreta, entropia/sintropia),
- engenharia de software e criptografia aplicada,
- semiótica e linguística comparada,
- governança de licenças e conformidade normativa em Estados Democráticos de Direito.

Este repositório passa a incluir uma documentação de referência de nível avançado (estilo pós-doc) para orientar implementação, validação e auditoria.

## Fonte de verdade executável desta revisão

- [`ECOSYSTEM_RUNTIME_STATE.json`](ECOSYSTEM_RUNTIME_STATE.json): estado material por componente, evidência, lacuna e próxima ação.
- [`contracts/ecosystem-runtime-state.schema.json`](contracts/ecosystem-runtime-state.schema.json): contrato da matriz.
- [`scripts/validate_runtime_truth_local.sh`](scripts/validate_runtime_truth_local.sh): build/teste local sem depender de GitHub Actions.
- [`scripts/validate_ecosystem_runtime_state.py`](scripts/validate_ecosystem_runtime_state.py): valida estados de evidência com Python stdlib.

> **GitHub Actions:** nesta revisão, a execução de Actions está `OUT_OF_SCOPE_NO_CREDIT`. A ausência de run não vira PASS. O gate executável momentâneo é local e deve registrar comando, ambiente, stdout/stderr e hashes.

## Documentação principal

### Colaboração entre agentes (AI e humanos)

- [`docs/AGENTES.md`](docs/AGENTES.md): guia operacional unificado — taxonomia, ciclo de sessão, regras de não-colisão, anti-padrões, CI gates e entradas canônicas por subsistema. **Leia antes de qualquer mudança de código.**
- [`docs/AGENTES_CHECKLIST.md`](docs/AGENTES_CHECKLIST.md): checklist executável por sessão (startup 5 passos, execução, shutdown, critério de encerramento).
- [`docs/AGENTES_DECISAO_LOG.md`](docs/AGENTES_DECISAO_LOG.md): log de conflitos e decisões entre agentes, com template e critérios de escalação obrigatória.

### Arquitetura, semântica e governança

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
- [`docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md`](docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md): tradução mercadológica e técnica do paper RAFAELIA em 7 vetores, com gates de claim, monetização responsável e próximos experimentos reproduzíveis.
- [`docs/DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md`](docs/DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md): protocolo transversal que liga claim, fonte, modelo, ensaio, incerteza, IP, valuation, roteamento de repositório e retroalimentação. Configuração: [`configs/deeprafa2_evidence.yml`](configs/deeprafa2_evidence.yml).
- [`docs/CONVERGENCIA_UNICA_METODOLOGICA.md`](docs/CONVERGENCIA_UNICA_METODOLOGICA.md) e [`docs/CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md`](docs/CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md): mapa auditável de convergência entre RafPolimata e os repositórios irmãos (ChipQuantum, Vectras, llamaRafaelia, GAIA_phi, CONVERSATIONS_CHUNKS_PRIVATE, BLAKE3).
- [`docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md`](docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md): orquestrador de física clássica/quântica, máquinas elétricas, combustão, eletrólise, fluidos, grafos, primos, estatística e NTP, com 12 gates contra sobreclaim e inconsistência dimensional.

- [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md`](docs/MAPA_ESTRUTURAL_REPOSITORIO.md): mapa profissional da disposição por tags/diretórios, estados (`VOID`/`PENDING`/`AUDIT`/`RUNTIME`/`REFERENCE`) e varredura estrutural em 5 níveis.
- [`docs/CONCEPT_STRUCTURAL_AUDIT.md`](docs/CONCEPT_STRUCTURAL_AUDIT.md): auditoria reproduzível que liga conceitos a arquivos âncora, estado, lacuna e próxima ação.

## Aviso importante

> Este material é técnico-acadêmico e **não substitui parecer jurídico profissional**.
> A aplicação em produção deve passar por validação de advogados habilitados em cada jurisdição relevante.

## Qualidade e automação

- Pipeline GitHub Actions em `.github/workflows/ci.yml` com validação do protocolo canônico, checagem C host, build estrito (`-Wall -Wextra -Werror`), smoke test do binário e relatório P(k).
- Workflow `.github/workflows/formal-science.yml` com registro de 32 equações, 12 domínios, 12 gates de prova, verificação dimensional e relatório auditável em JSON.
- Hotfix de compilação aplicado no núcleo C para restaurar uma base compilável e facilitar futuras refatorações incrementais.

- [`docs/PROTOCOLO_FALSIFICABILIDADE_PK.md`](docs/PROTOCOLO_FALSIFICABILIDADE_PK.md): protocolo mínimo de falsificabilidade em P(k) com execução automática.
- [`docs/PROTOCOLO_CANONICO_COHERENCIA.md`](docs/PROTOCOLO_CANONICO_COHERENCIA.md): contrato canônico que liga as 50 sementes matemáticas a invariantes, arquivos e gates de prova/queda.
- [`docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md`](docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md): protocolo enterprise de dois ciclos para coerência semântica, execução técnica, fail-safe/failover/rollback e uso honesto de `TOKEN_VAZIO`.
- [`configs/semantic_coherence.yml`](configs/semantic_coherence.yml): YAML operacional para orquestração mutável de coerência, benchmark e falsificabilidade.
- [`configs/formal_science_orchestrator.yml`](configs/formal_science_orchestrator.yml): registro de equações, dimensões, estados de claim, falsificadores e contratos de tempo/semântica.
- [`configs/two_cycle_omega.yml`](configs/two_cycle_omega.yml): manifesto auditável dos campos canônicos, camadas linguísticas e invariantes técnicos dos dois ciclos.
- [`configs/operational_excellence.yml`](configs/operational_excellence.yml): manifesto verificável da rota de excelência operacional, estados, arquiteturas e controles de risco.
- [`docs/ROTINA_OPERACIONAL_BENCHMARKS.md`](docs/ROTINA_OPERACIONAL_BENCHMARKS.md): rotina operacional de auditoria, benchmark com prewarm/warmup e top 20 modelos.
- [`docs/LOGOTIPO_RAFAELIA_60COL.md`](docs/LOGOTIPO_RAFAELIA_60COL.md): logotipo ASCII de 60 colunas com variante colorida ANSI.
- [`assets/raf_operational_seal.svg`](assets/raf_operational_seal.svg): selo visual local dos gates operacionais, sem alegação de certificação.

### Gate local momentâneo

Enquanto GitHub Actions estiver fora do escopo por falta de crédito, o caminho executável é:

```sh
bash scripts/validate_runtime_truth_local.sh
```

Esse comando compila o núcleo C, executa o codec `segment.v1`, verifica `.s/.hex/.bin/.ops`, rejeita extensão desconhecida e fonte oversized, e valida a matriz de estados. Seu resultado só pode ser promovido com stdout/stderr, versão da ferramenta, commit e hashes.

## Evidence Discipline

RafPolimata separa hipótese, analogia, prova parcial e prova runtime. `TOKEN_VAZIO` é um estado válido quando falta ferramenta, device, dataset, log ou execução. O CI atua como compilador de evidências estruturais, enquanto a matriz do universo registra estado, lacunas, próximos passos e rollback sem transformar ausência em PASS.

## Conversation Indexer — `segment.v1`

`runtime/conversation_indexer/` mantém o scanner streaming existente e inclui agora:

- header explícito de 64 bytes;
- conversation record explícito de 96 bytes;
- message record explícito de 128 bytes;
- leitor limitado com iteração tipada;
- CRC32C de header, records, título, autor e conteúdo;
- rejeição de corrupção, truncamento, roles inválidos e ranges fora do buffer.

O ensaio isolado host, a auditoria freestanding e a compilação cruzada ARM32/ARM64 foram executados e estão documentados em:

- [`docs/RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md`](docs/RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md);
- [`docs/MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md`](docs/MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md).

Streaming extractor, writer atômico, checkpoint/resume, BLAKE3 e execução em device continuam pendentes e não são declarados concluídos.

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
| Compile ApkC | TOKEN_VAZIO (binário local verificado; falta transcript source→binary do commit) |
| Generate APK | PASS (SHA-256 a331d024…; 39 avisos ARM32 mnemonic) |
| ZIP parse | PASS (unzip lista entradas; container ZIP/APK válido) |
| AXML parse | PASS (aapt: package com.rafael.teste, NativeActivity, lib hello) |
| DEX SHA-1 | PASS (SHA-1 interno 9ea7c008… confere) |
| ELF readelf | PASS arm32 / SKIP arm64 (APK não contém lib/arm64-v8a/*.so) |
| APK signing | PASS (apksigner v1/v2/v3 true; debug/self-signed; v4 false) |
| adb install | PASS_LIMITED (package:com.rafael.teste visível; stdout de adb install -r incompleto) |
| NativeActivity runtime | TOKEN_VAZIO (falta logcat/lançamento sem crash) |
