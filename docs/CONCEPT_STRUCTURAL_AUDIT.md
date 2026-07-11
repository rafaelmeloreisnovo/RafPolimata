# Concept Structural Audit

> **Entrada canônica:** `docs/AGENTES.md` §5 descreve o pipeline VOID → VALIDATED e §3 define
> os estados canônicos. Este documento audita o uso de conceitos como unidades estruturais —
> conceito só avança quando possui arquivo-âncora, gate ou evidência, e lacuna explícita.

Auditoria reproduzivel do uso de conceitos como unidades estruturais do RafPolimata.

Regra: conceito so avanca quando possui arquivo ancora, uso estrutural, gate ou evidencia, lacuna explicita e proxima acao. Ausencia operacional permanece `TOKEN_VAZIO`, `PENDING` ou `DEVICE_REQUIRED`.

## Resumo

- Schema: `concept_structural_audit.v1`
- Gerado por: `scripts/emit_concept_structural_audit.py`
- Conceitos auditados: 12
- Caminhos obrigatorios faltantes: 0

| Estado | Quantidade |
|---|---|
| `AUDIT_PASS` | 1 |
| `AUDIT_READY` | 5 |
| `DEVICE_REQUIRED` | 2 |
| `PENDING_BY_DESIGN` | 1 |
| `REFERENCE_AUDIT` | 3 |

## Matriz

| ID | Conceito | Estado | Arquivos ancora | Lacuna | Proxima acao |
|---|---|---|---|---|---|
| C01 | Disciplina de evidencia e trava de claim | `AUDIT_READY` | `docs/CLAIM_EVIDENCE_LOCK.md`<br>`README.md` | Nao substitui revisao externa nem runtime; apenas trava linguagem e escopo. | Manter cada claim novo ligado a evidencia minima, limite e rollback. |
| C02 | Arquitetura de 21 camadas | `REFERENCE_AUDIT` | `docs/ARQUITETURA_21_NIVEIS.md` | Ainda falta matriz por camada com arquivo dono, teste e resultado para cada nivel. | Converter as 21 camadas em linhas auditaveis no JSON de universo ou em teste dedicado. |
| C03 | Dez dimensoes semanticas | `REFERENCE_AUDIT` | `docs/DEZ_DIMENSOES_SEMANTICAS.md` | Nem toda dimensao possui metrica executavel, dataset ou teste. | Promover dimensoes prioritarias para AUDIT/RUNTIME com metricas e fixtures. |
| C04 | Protocolo canonico F01-F50 | `AUDIT_READY` | `docs/PROTOCOLO_CANONICO_COHERENCIA.md`<br>`configs/semantic_coherence.yml` | Algumas formulas ainda apontam para documentacao, nao para implementacao executavel. | Transformar formulas documentais em testes unitarios/property tests quando houver codigo. |
| C05 | Dois ciclos omega | `AUDIT_READY` | `docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md`<br>`configs/two_cycle_omega.yml` | Nem toda unidade semantica do projeto ainda tem comando reproduzivel. | Exigir que cada conceito promovido tenha comando, artefato e rollback. |
| C06 | Mapa estrutural do repositorio | `AUDIT_READY` | `docs/MAPA_ESTRUTURAL_REPOSITORIO.md`<br>`scripts/audit_repository_structure.py` | Precisa manter o mapa sincronizado com novos diretorios e artefatos. | Rodar auditoria em CI e atualizar README/indices no mesmo commit das mudancas estruturais. |
| C07 | Excelencia operacional CPU/GPU/SIMD/cache | `REFERENCE_AUDIT` | `docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md` | Claims de desempenho exigem baseline, p95/p99, tamanho, energia ou raw logs. | Adicionar resultados JSON por arquitetura antes de promover SIMD/GPU para VALIDATED. |
| C08 | ApkC Android runtime proof | `DEVICE_REQUIRED` | `docs/APKC_ANDROID_RUNTIME_PROOF_PLAN.md`<br>`Apkc` | NativeActivity runtime e logcat sem crash ainda sao TOKEN_VAZIO ate execucao completa. | Coletar device-info, install completo, launch e logcat filtrado em Apkc/proofs/out/. |
| C09 | RAF methods 001-056 | `PENDING_BY_DESIGN` | `docs/RAF_METHODS_STATUS.md`<br>`results/raf_methods_status.json` | Retorno 0 so vira EXECUTA_PASS apos execucao registrada; muitos itens dependem de hardware/Android/QEMU. | Priorizar 3 metodos por dominio e anexar logs brutos de compilacao/execucao. |
| C10 | Matriz do universo do repositorio | `AUDIT_READY` | `docs/REPOSITORY_UNIVERSE_MATRIX.md`<br>`results/repository_universe_matrix.json` | Deve continuar ignorando outputs transitorios de CI e falhar quando broken links surgirem. | Adicionar drift check e cobertura de markdown_broken_links como gate bloqueante. |
| C11 | Falsificabilidade P(k) | `AUDIT_PASS` | `scripts/first_test_pk.py`<br>`results/first_test_report.json` | Ainda depende da qualidade/versionamento do dataset e do congelamento previo dos criterios. | Versionar datasets reais por hash e registrar baseline externo quando existir. |
| C12 | Android/JNI/ABI e runtime de device | `DEVICE_REQUIRED` | `docs/RAF_METHODS_STATUS.md`<br>`docs/APKC_ANDROID_RUNTIME_PROOF_PLAN.md` | Sem device/emulator e logcat completo, runtime final permanece TOKEN_VAZIO. | Executar prova minima Android e gerar runtime-verdict.json. |

## Criterio de lapidacao

1. `REFERENCE_AUDIT` deve ganhar arquivo dono, criterio de queda e teste quando virar comportamento.
2. `AUDIT_READY` deve manter comando reproduzivel e resultado versionado.
3. `PENDING_BY_DESIGN` deve priorizar poucos metodos por ciclo e anexar logs brutos.
4. `DEVICE_REQUIRED` so vira `EXEC_PASS` com device/emulator, comando, stdout/stderr/logcat e verdict.
5. Nenhum conceito pode pular direto para `PASS` sem evidencia minima.
