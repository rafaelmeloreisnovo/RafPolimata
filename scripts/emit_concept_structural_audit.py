#!/usr/bin/env python3
"""Emit a structural audit for RafPolimata concept usage.

The audit converts high-level concepts into an auditable matrix:
concept -> required paths -> structural use -> gate -> evidence -> gap -> next action.

It does not promote runtime claims. Missing hardware/device/runtime evidence
remains TOKEN_VAZIO, PENDING or DEVICE_REQUIRED.
"""
from __future__ import annotations

import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = ROOT / "results" / "concept_structural_audit.json"
OUT_MD = ROOT / "docs" / "CONCEPT_STRUCTURAL_AUDIT.md"

CONCEPTS = [
    {
        "id": "C01",
        "concept": "Disciplina de evidencia e trava de claim",
        "state": "AUDIT_READY",
        "required_paths": ["docs/CLAIM_EVIDENCE_LOCK.md", "README.md"],
        "structural_use": "Impede transformar ausencia de ferramenta, device, dataset, log ou execucao em PASS inventado.",
        "gate": "python3 scripts/emit_concept_structural_audit.py",
        "evidence": "Estados e niveis de claim documentados; auditor passa se os arquivos ancora existirem.",
        "gap": "Nao substitui revisao externa nem runtime; apenas trava linguagem e escopo.",
        "next_action": "Manter cada claim novo ligado a evidencia minima, limite e rollback.",
    },
    {
        "id": "C02",
        "concept": "Arquitetura de 21 camadas",
        "state": "REFERENCE_AUDIT",
        "required_paths": ["docs/ARQUITETURA_21_NIVEIS.md"],
        "structural_use": "Organiza decisao em camadas tecnico-formais, semantico-cognitivas e juridico-institucionais.",
        "gate": "Arquivo rastreavel + auditor de conceito",
        "evidence": "Camadas e regra de coerencia transversal documentadas.",
        "gap": "Ainda falta matriz por camada com arquivo dono, teste e resultado para cada nivel.",
        "next_action": "Converter as 21 camadas em linhas auditaveis no JSON de universo ou em teste dedicado.",
    },
    {
        "id": "C03",
        "concept": "Dez dimensoes semanticas",
        "state": "REFERENCE_AUDIT",
        "required_paths": ["docs/DEZ_DIMENSOES_SEMANTICAS.md"],
        "structural_use": "Separa topologia, dinamica, informacao, integridade, prosodia, linguagem, cognicao, analogia fisica, computacao e norma.",
        "gate": "Arquivo rastreavel + auditor de conceito",
        "evidence": "Dimensoes descritas e limites de interpretacao registrados.",
        "gap": "Nem toda dimensao possui metrica executavel, dataset ou teste.",
        "next_action": "Promover dimensoes prioritarias para AUDIT/RUNTIME com metricas e fixtures.",
    },
    {
        "id": "C04",
        "concept": "Protocolo canonico F01-F50",
        "state": "AUDIT_READY",
        "required_paths": ["docs/PROTOCOLO_CANONICO_COHERENCIA.md", "configs/semantic_coherence.yml"],
        "structural_use": "Liga sementes matematico-semanticas a invariantes, arquivos canonicos e gates.",
        "gate": "python3 scripts/validate_coherence_protocol.py",
        "evidence": "Registro F01-F50 e gates G1-G7 preservam rastreabilidade.",
        "gap": "Algumas formulas ainda apontam para documentacao, nao para implementacao executavel.",
        "next_action": "Transformar formulas documentais em testes unitarios/property tests quando houver codigo.",
    },
    {
        "id": "C05",
        "concept": "Dois ciclos omega",
        "state": "AUDIT_READY",
        "required_paths": ["docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md", "configs/two_cycle_omega.yml"],
        "structural_use": "Separa sintese semantica de execucao tecnica com rollback.",
        "gate": "python3 scripts/validate_two_cycle_omega.py",
        "evidence": "Estados SEMANTIC_READY/TOKEN_VAZIO/SEMANTIC_FAIL e EXEC_PASS/EXEC_FAIL/EXEC_SKIPPED/ROLLBACK_READY documentados.",
        "gap": "Nem toda unidade semantica do projeto ainda tem comando reproduzivel.",
        "next_action": "Exigir que cada conceito promovido tenha comando, artefato e rollback.",
    },
    {
        "id": "C06",
        "concept": "Mapa estrutural do repositorio",
        "state": "AUDIT_READY",
        "required_paths": ["docs/MAPA_ESTRUTURAL_REPOSITORIO.md", "scripts/audit_repository_structure.py"],
        "structural_use": "Classifica diretorios e arquivos em VOID, PENDING, AUDIT, RUNTIME e REFERENCE.",
        "gate": "python3 scripts/audit_repository_structure.py --depth 5",
        "evidence": "Criterios de saida por estado e auditoria de cinco niveis documentados.",
        "gap": "Precisa manter o mapa sincronizado com novos diretorios e artefatos.",
        "next_action": "Rodar auditoria em CI e atualizar README/indices no mesmo commit das mudancas estruturais.",
    },
    {
        "id": "C07",
        "concept": "Excelencia operacional CPU/GPU/SIMD/cache",
        "state": "REFERENCE_AUDIT",
        "required_paths": ["docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md"],
        "structural_use": "Define quando usar C generico, branchless, NEON, GPU, syscall direta, storage/buffer e rollback.",
        "gate": "tools/raf_validate_operational_excellence.c + benchmarks quando disponiveis",
        "evidence": "Matriz de decisao e criterios enterprise-ready documentados.",
        "gap": "Claims de desempenho exigem baseline, p95/p99, tamanho, energia ou raw logs.",
        "next_action": "Adicionar resultados JSON por arquitetura antes de promover SIMD/GPU para VALIDATED.",
    },
    {
        "id": "C08",
        "concept": "ApkC Android runtime proof",
        "state": "DEVICE_REQUIRED",
        "required_paths": ["docs/APKC_ANDROID_RUNTIME_PROOF_PLAN.md", "Apkc"],
        "structural_use": "Define caminho source->build->APK->sign->install->launch->logcat->verdict.",
        "gate": "bash scripts/apkc_validate.sh; bash scripts/apkc_sign_debug.sh; device/logcat para runtime",
        "evidence": "Plano de artefatos esperados existe; runtime final depende de device/emulator.",
        "gap": "NativeActivity runtime e logcat sem crash ainda sao TOKEN_VAZIO ate execucao completa.",
        "next_action": "Coletar device-info, install completo, launch e logcat filtrado em Apkc/proofs/out/.",
    },
    {
        "id": "C09",
        "concept": "RAF methods 001-056",
        "state": "PENDING_BY_DESIGN",
        "required_paths": ["docs/RAF_METHODS_STATUS.md", "results/raf_methods_status.json"],
        "structural_use": "Transforma metodos low-level em catalogo com dominio, arquitetura, risco, proxima prova e exigencia de device quando aplicavel.",
        "gate": "python3 scripts/emit_raf_methods_status.py",
        "evidence": "Status conservador dos metodos RAF registrado em Markdown e JSON.",
        "gap": "Retorno 0 so vira EXECUTA_PASS apos execucao registrada; muitos itens dependem de hardware/Android/QEMU.",
        "next_action": "Priorizar 3 metodos por dominio e anexar logs brutos de compilacao/execucao.",
    },
    {
        "id": "C10",
        "concept": "Matriz do universo do repositorio",
        "state": "AUDIT_READY",
        "required_paths": ["docs/REPOSITORY_UNIVERSE_MATRIX.md", "results/repository_universe_matrix.json"],
        "structural_use": "Gera visao de origem, estrutura, integridade, execucao, metrica, evidencia, governanca e rollback.",
        "gate": "python3 scripts/emit_repository_universe_matrix.py",
        "evidence": "JSON registra contagens RAF, diretorios esperados, links quebrados e itens de auditoria.",
        "gap": "Deve continuar ignorando outputs transitorios de CI e falhar quando broken links surgirem.",
        "next_action": "Adicionar drift check e cobertura de markdown_broken_links como gate bloqueante.",
    },
    {
        "id": "C11",
        "concept": "Falsificabilidade P(k)",
        "state": "AUDIT_PASS",
        "required_paths": ["scripts/first_test_pk.py", "results/first_test_report.json"],
        "structural_use": "Permite resultado PASS/FAIL por metrica, sem proteger a teoria por ajuste posterior.",
        "gate": "python3 scripts/first_test_pk.py --output results/first_test_report.json",
        "evidence": "Relatorio JSON versionado registra criterios, metricas e verdict.",
        "gap": "Ainda depende da qualidade/versionamento do dataset e do congelamento previo dos criterios.",
        "next_action": "Versionar datasets reais por hash e registrar baseline externo quando existir.",
    },
    {
        "id": "C12",
        "concept": "Android/JNI/ABI e runtime de device",
        "state": "DEVICE_REQUIRED",
        "required_paths": ["docs/RAF_METHODS_STATUS.md", "docs/APKC_ANDROID_RUNTIME_PROOF_PLAN.md"],
        "structural_use": "Conecta C nativo, ABI, JNI, Termux/Android, benchmark e logcat.",
        "gate": "device/emulator + NDK/SDK/adb/logcat",
        "evidence": "Rotas e artefatos esperados estao documentados.",
        "gap": "Sem device/emulator e logcat completo, runtime final permanece TOKEN_VAZIO.",
        "next_action": "Executar prova minima Android e gerar runtime-verdict.json.",
    },
]


def rel_missing(paths: list[str]) -> list[str]:
    return [path for path in paths if not (ROOT / path).exists()]


def build_summary() -> dict:
    concepts = []
    missing_required_paths: list[dict] = []
    for item in CONCEPTS:
        concept = dict(item)
        missing = rel_missing(concept["required_paths"])
        concept["missing_required_paths"] = missing
        if missing:
            missing_required_paths.append({"id": concept["id"], "missing": missing})
        concepts.append(concept)
    return {
        "schema": "concept_structural_audit.v1",
        "generated_by": "scripts/emit_concept_structural_audit.py",
        "concepts_count": len(concepts),
        "states": dict(Counter(c["state"] for c in concepts)),
        "missing_required_paths": missing_required_paths,
        "concepts": concepts,
    }


def emit_markdown(summary: dict) -> str:
    lines: list[str] = [
        "# Concept Structural Audit",
        "",
        "Auditoria reproduzivel do uso de conceitos como unidades estruturais do RafPolimata.",
        "",
        "Regra: conceito so avanca quando possui arquivo ancora, uso estrutural, gate ou evidencia, lacuna explicita e proxima acao. Ausencia operacional permanece `TOKEN_VAZIO`, `PENDING` ou `DEVICE_REQUIRED`.",
        "",
        "## Resumo",
        "",
        f"- Schema: `{summary['schema']}`",
        f"- Gerado por: `{summary['generated_by']}`",
        f"- Conceitos auditados: {summary['concepts_count']}",
        f"- Caminhos obrigatorios faltantes: {len(summary['missing_required_paths'])}",
        "",
        "| Estado | Quantidade |",
        "|---|---|",
    ]
    for state, count in sorted(summary["states"].items()):
        lines.append(f"| `{state}` | {count} |")
    lines.extend([
        "",
        "## Matriz",
        "",
        "| ID | Conceito | Estado | Arquivos ancora | Lacuna | Proxima acao |",
        "|---|---|---|---|---|---|",
    ])
    for concept in summary["concepts"]:
        paths = "<br>".join(f"`{path}`" for path in concept["required_paths"])
        lines.append(
            f"| {concept['id']} | {concept['concept']} | `{concept['state']}` | "
            f"{paths} | {concept['gap']} | {concept['next_action']} |"
        )
    lines.extend([
        "",
        "## Criterio de lapidacao",
        "",
        "1. `REFERENCE_AUDIT` deve ganhar arquivo dono, criterio de queda e teste quando virar comportamento.",
        "2. `AUDIT_READY` deve manter comando reproduzivel e resultado versionado.",
        "3. `PENDING_BY_DESIGN` deve priorizar poucos metodos por ciclo e anexar logs brutos.",
        "4. `DEVICE_REQUIRED` so vira `EXEC_PASS` com device/emulator, comando, stdout/stderr/logcat e verdict.",
        "5. Nenhum conceito pode pular direto para `PASS` sem evidencia minima.",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    summary = build_summary()
    OUT_JSON.write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    OUT_MD.write_text(emit_markdown(summary), encoding="utf-8")
    return 1 if summary["missing_required_paths"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
