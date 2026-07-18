# RafPolimata

RafPolimata é uma proposta de **arquitetura semântica-tecnológica-jurídica** para sistemas complexos que combinam:

- modelagem matemática (toro, dinâmica discreta, entropia/sintropia),
- engenharia de software e criptografia aplicada,
- semiótica e linguística comparada,
- governança de licenças e conformidade normativa em Estados Democráticos de Direito.

Este repositório inclui documentação avançada, código C/ASM, scripts de auditoria, micro-toolchain Android e contratos de evidência. Documento, arquivo e workflow não são tratados automaticamente como prova de runtime.

## Fonte de verdade executável

- [`ECOSYSTEM_RUNTIME_STATE.json`](ECOSYSTEM_RUNTIME_STATE.json): estado material por componente, evidência, lacuna e próxima ação.
- [`contracts/ecosystem-runtime-state.schema.json`](contracts/ecosystem-runtime-state.schema.json): contrato da matriz.
- [`scripts/validate_runtime_truth_local.sh`](scripts/validate_runtime_truth_local.sh): build/teste local sem depender de GitHub Actions.
- [`scripts/validate_ecosystem_runtime_state.py`](scripts/validate_ecosystem_runtime_state.py): valida estados de evidência com Python stdlib.

> **GitHub Actions:** nesta revisão, execução de Actions está `OUT_OF_SCOPE_NO_CREDIT`. A ausência de run não vira PASS. O gate executável é local e deve registrar comando, ambiente, stdout/stderr e hashes.

## Documentação principal

### Colaboração entre agentes (AI e humanos)

- [`docs/AGENTES.md`](docs/AGENTES.md): guia operacional unificado — taxonomia, ciclo de sessão, regras de não-colisão, anti-padrões, CI gates e entradas canônicas por subsistema. **Leia antes de qualquer mudança de código.**
- [`docs/AGENTES_CHECKLIST.md`](docs/AGENTES_CHECKLIST.md): checklist executável por sessão.
- [`docs/AGENTES_DECISAO_LOG.md`](docs/AGENTES_DECISAO_LOG.md): log de conflitos e decisões.

### Arquitetura, semântica e governança

- [`docs/ARQUITETURA_21_NIVEIS.md`](docs/ARQUITETURA_21_NIVEIS.md)
- [`docs/DEZ_DIMENSOES_SEMANTICAS.md`](docs/DEZ_DIMENSOES_SEMANTICAS.md)
- [`docs/MATRIZ_JURIDICO_TECNOLOGICA.md`](docs/MATRIZ_JURIDICO_TECNOLOGICA.md)
- [`docs/LICENCAS_COMPARADAS.md`](docs/LICENCAS_COMPARADAS.md)
- [`docs/ATRATORES_42_JURIDICOS.md`](docs/ATRATORES_42_JURIDICOS.md)
- [`docs/BASES_SUPRALEGAIS_E_PADROES.md`](docs/BASES_SUPRALEGAIS_E_PADROES.md)
- [`docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md`](docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md)
- [`docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md`](docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md)
- [`docs/RELEASE_NOTES_PENDING.md`](docs/RELEASE_NOTES_PENDING.md)
- [`docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md`](docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md)
- [`docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md`](docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md)
- [`docs/DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md`](docs/DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md)
- [`docs/CONVERGENCIA_UNICA_METODOLOGICA.md`](docs/CONVERGENCIA_UNICA_METODOLOGICA.md)
- [`docs/CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md`](docs/CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md)
- [`docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md`](docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md)
- [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md`](docs/MAPA_ESTRUTURAL_REPOSITORIO.md)
- [`docs/CONCEPT_STRUCTURAL_AUDIT.md`](docs/CONCEPT_STRUCTURAL_AUDIT.md)

## Aviso importante

> Este material é técnico-acadêmico e **não substitui parecer jurídico profissional**. A aplicação em produção deve passar por validação qualificada nas jurisdições relevantes.

## Qualidade e automação

O repositório contém workflows e gates, mas a execução local abaixo é o caminho momentâneo:

```sh
bash scripts/validate_runtime_truth_local.sh
```

Esse comando:

1. compila o núcleo C com `-Wall -Wextra -Werror`;
2. executa o codec freestanding `segment.v1`;
3. verifica `.s`, `.hex`, `.bin` e `.ops`;
4. rejeita extensão desconhecida;
5. rejeita fonte maior que o limite em vez de truncar;
6. verifica distinção entre presença de device node e capacidade de acelerador;
7. valida a matriz de estados.

## Evidence Discipline

RafPolimata separa hipótese, analogia, prova parcial e prova runtime. `TOKEN_VAZIO` é estado válido quando falta ferramenta, device, dataset, log ou execução.

## Conversation Indexer — segment v1

`runtime/conversation_indexer/` contém o scanner streaming existente e agora também o primeiro contrato binário congelado:

- `raf_segment_v1.h`;
- `raf_segment_v1.c`;
- `test_segment_v1.c`.

O corte implementa o header explícito de **64 bytes**, little-endian, CRC32C e rejeição de corrupção. Ainda não declara concluídos o writer/reader de registros de conversas/mensagens, BLAKE3, checkpoint/resume ou extração integral.

## ApkC — micro-toolchain Android freestanding

`Apkc/` contém micro-toolchain experimental em C para gerar APK nativo mínimo por escrita direta de ZIP/APK, AXML, DEX e ELF, com backends ARM64 e ARM32.

| Gate | Status |
|---|---|
| Compile ApkC | `TOKEN_VAZIO` para transcript completo do commit |
| Generate APK | `PASS` documental/prova existente |
| ZIP parse | `PASS` |
| AXML parse | `PASS` |
| DEX SHA-1 | `PASS` |
| ELF readelf | `PASS arm32 / TOKEN_VAZIO arm64` |
| APK signing | `PASS debug`; release `PENDING` |
| adb install | `PASS_LIMITED` |
| NativeActivity runtime | `TOKEN_VAZIO` |

A prova de runtime Android exige install, launch e logcat filtrado sem crash; nenhum artefato estático substitui esse gate.
