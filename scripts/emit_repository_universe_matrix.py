#!/usr/bin/env python3
"""Emit an evidence-first repository universe matrix for RafPolimata.

Stdlib-only scanner. It does not promote runtime/benchmark/device claims; missing
preconditions are represented as TOKEN_VAZIO/SKIPPED/DEVICE_REQUIRED.
"""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Dict, List

ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = ROOT / "results" / "repository_universe_matrix.json"
MAX_DEPTH = 5
IGNORED_DIRS = {".git", "__pycache__", "build_host_check"}
EXPECTED_DIRS = ["docs", "configs", "scripts", "Benchmark", "Apkc", "data", "results", "tools", ".github/workflows"]
ALLOWED_ROOT_FILES = {
    ".gitignore", "README.md", "CHANGELOG.md", "Makefile", "RAF_INDEX.md", "RAF_56_METHODS.md",
    "RAF_40_STRATEGIES.md", "RAF_BENCHMARK_MATRIX.md", "RAF_VALIDATION_PROTOCOL.md",
    "RAF_CODEX_INTEGRATE_96_56_BENCH.md", "RAF_CHECKLIST_96_ITEMS.md", "RAF_host_syntax_check.sh",
    "RAF_list_tree.sh", "RAF_rafaelia_common.h", "raf_compile.h", "raf_precomp.c", "raf_main.c",
    "raf_frontend.c", "raf_cpu.c", "raf_asm_emit.c", "raiz_example.c", "raf_c_to_asm_root_optimizer.py",
    "README_RAFAELIA_ROOT_OPTIMIZER.md", "RELEASE_NOTES.md", "CLAUDE.md", "RAFAELIA_MASTER_DOC.txt",
    "big_test.sh", "RAF_benchmark_matrix.csv", "RAF_avr_regs_generated.h", "RASBERY.MD",
    "Arduíno.txt", "Arm64 Mixer leve criptografia.md", "L1.md", "RAFAELIA_COMPLETE_v4.zip",
    "raiz_audit_arm64.json", "raiz_audit_x86_64.json", "raiz_output_arm64.s", "raiz_output_x86_64.asm",
}
RAF_RE = re.compile(r"^RAF_(\d{3})_.*\.c$")
MD_LINK_RE = re.compile(r"\[[^\]]+\]\((?!https?://|mailto:|#)([^)]+)\)")



def visible_children(path: Path) -> List[Path]:
    return sorted(
        (child for child in path.iterdir() if child.name not in IGNORED_DIRS),
        key=lambda item: item.as_posix(),
    )


def walk_depth(max_depth: int = MAX_DEPTH) -> List[Path]:
    paths: List[Path] = []
    stack = [(ROOT, 0)]
    while stack:
        current, depth = stack.pop()
        if depth > max_depth:
            continue
        if current != ROOT:
            paths.append(current)
        if current.is_dir() and depth < max_depth:
            for child in reversed(visible_children(current)):
                stack.append((child, depth + 1))
    return paths

def rel(p: Path) -> str:
    return p.relative_to(ROOT).as_posix()


def read_index_files() -> List[str]:
    idx = ROOT / "RAF_INDEX.md"
    if not idx.exists():
        return []
    return sorted(set(re.findall(r"`(RAF_\d{3}_[^`]+?\.c)`", idx.read_text(encoding="utf-8", errors="replace"))))


def state_for(path: str) -> str:
    p = ROOT / path
    if path.startswith(".github/workflows") or path.startswith("configs/"):
        return "CONFIG"
    if path.startswith("data/"):
        return "DATA"
    if path.startswith("results/") or "proof" in path.lower() or path.endswith(".json"):
        return "RESULT"
    if path.startswith("docs/") or path.endswith(".md") or path.endswith(".txt"):
        return "REFERENCE"
    if path.startswith("Apkc/"):
        return "DEVICE_REQUIRED" if ("android" in path.lower() or "proof" in path.lower()) else "RUNTIME"
    if path.endswith((".c", ".h", ".sh", ".py")):
        return "RUNTIME"
    if not p.exists():
        return "TOKEN_VAZIO"
    return "AUDIT"


def type_for(path: str) -> str:
    if path.endswith(".md"):
        return "documento"
    if path.endswith(".yml") or path.endswith(".yaml"):
        return "configuração"
    if path.endswith(".py") or path.endswith(".sh"):
        return "script"
    if path.endswith(".c") or path.endswith(".h"):
        return "código C"
    if path.endswith(".json"):
        return "resultado JSON"
    if path.endswith(".csv"):
        return "dado CSV"
    if (ROOT / path).is_dir():
        return "diretório"
    return "artefato"


def row(path: str, function: str, invariant: str, gate: str, evidence: str, gap: str, next_action: str, risk: str, rollback: str) -> Dict[str, str]:
    return {
        "origin": "repository-scan-depth-5",
        "path": path,
        "type": type_for(path),
        "function": function,
        "state": state_for(path),
        "invariant": invariant,
        "gate": gate,
        "evidence": evidence,
        "gap": gap,
        "next_action": next_action,
        "risk": risk,
        "rollback": rollback,
    }


def markdown_link_issues() -> List[str]:
    issues: List[str] = []
    for md in ROOT.rglob("*.md"):
        if ".git" in md.parts:
            continue
        text = md.read_text(encoding="utf-8", errors="replace")
        for match in MD_LINK_RE.finditer(text):
            target = match.group(1).split()[0].split("#", 1)[0]
            if not target:
                continue
            t = (md.parent / target).resolve()
            try:
                t.relative_to(ROOT)
            except ValueError:
                issues.append(f"{rel(md)} -> {target} (fora do repositório)")
                continue
            if not t.exists():
                issues.append(f"{rel(md)} -> {target}")
    return issues



def function_for_scanned_path(path: str) -> str:
    if path.startswith("docs/"):
        return "Documento/protocolo dentro da varredura estrutural em 5 níveis"
    if path.startswith("scripts/"):
        return "Script operacional dentro da varredura estrutural em 5 níveis"
    if path.startswith("Benchmark/"):
        return "Benchmark ou suporte de medição dentro da varredura estrutural em 5 níveis"
    if path.startswith("Apkc/"):
        return "Artefato ApkC dentro da varredura estrutural em 5 níveis"
    if path.startswith("configs/"):
        return "Configuração canônica dentro da varredura estrutural em 5 níveis"
    if path.startswith("data/"):
        return "Entrada versionada dentro da varredura estrutural em 5 níveis"
    if path.startswith("results/"):
        return "Resultado versionado dentro da varredura estrutural em 5 níveis"
    if path.startswith("tools/"):
        return "Ferramenta auxiliar dentro da varredura estrutural em 5 níveis"
    if path.startswith(".github/workflows/"):
        return "Workflow de CI dentro da varredura estrutural em 5 níveis"
    return "Arquivo/diretório detectado na varredura estrutural em 5 níveis"


def generic_row(path: str) -> Dict[str, str]:
    return row(
        path,
        function_for_scanned_path(path),
        "origem→estrutura→integridade→evidência",
        "python3 scripts/emit_repository_universe_matrix.py",
        "detectado por varredura depth=5" if (ROOT / path).exists() else "TOKEN_VAZIO",
        "prova runtime não inferida pela presença do arquivo",
        "Vincular claim específico a comando, log, dataset, hardware ou rollback",
        "claim sem evidência se promovido manualmente",
        "Reverter arquivo ou rebaixar claim para TOKEN_VAZIO/PASS_LIMITED",
    )

def main() -> int:
    index_files = read_index_files()
    raf_files = sorted(p.name for p in ROOT.glob("RAF_[0-9][0-9][0-9]_*.c"))
    workflows = sorted(rel(p) for p in (ROOT / ".github/workflows").glob("*")) if (ROOT / ".github/workflows").exists() else []
    root_unexpected = sorted(p.name for p in ROOT.iterdir() if p.is_file() and not RAF_RE.match(p.name) and p.name not in ALLOWED_ROOT_FILES)
    md_issues = markdown_link_issues()

    rows: List[Dict[str, str]] = []
    for d in EXPECTED_DIRS:
        exists = (ROOT / d).exists()
        rows.append(row(d, "Bloco esperado do universo do repositório", "origem→estrutura→integridade", "test -d " + d, "presente" if exists else "TOKEN_VAZIO", "TOKEN_VAZIO" if not exists else "nenhuma lacuna estrutural básica", "Criar diretório ou documentar remoção" if not exists else "Manter varredura em 5 níveis", "perda de cobertura" if not exists else "baixo", "Restaurar diretório a partir do histórico ou criar marcador auditável"))

    fixed = ["README.md", "RAF_INDEX.md", "RAF_rafaelia_common.h", "raf_compile.h", "raf_precomp.c", "RAF_VALIDATION_PROTOCOL.md", "RAF_BENCHMARK_MATRIX.md"]
    for f in fixed:
        rows.append(row(f, "Arquivo canônico de navegação, protocolo, código ou benchmark", "claim↔evidência; índice 1:1; sem heap em hot path", "test -e " + f, "presente" if (ROOT / f).exists() else "TOKEN_VAZIO", "runtime/claim forte exige prova específica" if (ROOT / f).exists() else "TOKEN_VAZIO", "Cruzar com scripts de emissão e gates CI", "claim excessivo se não houver artefato", "Reverter alteração documental ou restaurar arquivo canônico"))

    for f in raf_files:
        n = int(RAF_RE.match(f).group(1)) if RAF_RE.match(f) else 0
        gap = "fora do índice RAF_INDEX.md" if f not in index_files else "runtime/hardware não provado por este scanner"
        rows.append(row(f, f"Método RAF {n:03d}", "1:1 RAF_INDEX.md↔RAF_###; compilação separada; sem sucesso runtime inventado", f"gcc -c -I. {f}", "listado em RAF_INDEX.md" if f in index_files else "TOKEN_VAZIO", gap, "Executar status por método e prova de hardware quando aplicável", "hardware/device pode ser requerido", "Manter arquivo e índice sincronizados; rollback por git"))

    rows.extend([
        row("data/", "Entradas versionadas", "dataset real com hash quando claim científico existir", "find data -type f", "arquivos presentes" if any((ROOT/"data").glob("**/*")) else "TOKEN_VAZIO", "hash/baseline por dataset pode faltar", "Registrar hashes e métodos por experimento", "claim científico sem dataset", "Marcar claim como TOKEN_VAZIO ou restaurar dataset"),
        row("results/", "Saídas de experimentos e auditorias", "resultado não substitui comando/raw log", "find results -type f", "arquivos presentes" if any((ROOT/"results").glob("**/*")) else "TOKEN_VAZIO", "raw log/hardware/flags podem faltar", "Acoplar resultados aos comandos de origem", "PASS falso por artefato órfão", "Invalidar resultado sem cadeia de custódia"),
        row("Apkc/proofs/", "Provas Android/ApkC", "Android runtime PASS exige install+launch+logcat", "bash scripts/apkc_validate.sh", "diretório presente" if (ROOT/"Apkc/proofs").exists() else "TOKEN_VAZIO", "device/logcat podem faltar", "Executar plano manual device-required", "claim runtime sem device", "Rebaixar para DEVICE_REQUIRED/TOKEN_VAZIO"),
    ])

    seen = {item["path"] for item in rows}
    for scanned in walk_depth(MAX_DEPTH):
        scanned_rel = rel(scanned)
        if scanned_rel not in seen:
            rows.append(generic_row(scanned_rel))
            seen.add(scanned_rel)

    summary = {
        "schema": "repository_universe_matrix.v1",
        "generated_by": "scripts/emit_repository_universe_matrix.py",
        "depth_policy": MAX_DEPTH,
        "raf_index_entries": len(index_files),
        "raf_c_files": len(raf_files),
        "raf_c_files_001_056": len([f for f in raf_files if 1 <= int(RAF_RE.match(f).group(1)) <= 56]),
        "raf_index_missing_files": sorted(set(index_files) - set(raf_files)),
        "raf_files_not_in_index": sorted(f for f in set(raf_files) - set(index_files) if 1 <= int(RAF_RE.match(f).group(1)) <= 56),
        "raf_extension_files_not_in_index": sorted(f for f in set(raf_files) - set(index_files) if int(RAF_RE.match(f).group(1)) > 56),
        "expected_dirs_missing": [d for d in EXPECTED_DIRS if not (ROOT / d).exists()],
        "unexpected_root_files": root_unexpected,
        "markdown_broken_links": md_issues,
        "data_files_present": any(p.is_file() for p in (ROOT / "data").glob("**/*")) if (ROOT / "data").exists() else False,
        "result_files_present": any(p.is_file() for p in (ROOT / "results").glob("**/*")) if (ROOT / "results").exists() else False,
        "workflows": workflows,
        "items": rows,
    }
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return 1 if summary["expected_dirs_missing"] or summary["raf_index_missing_files"] or summary["raf_files_not_in_index"] else 0

if __name__ == "__main__":
    raise SystemExit(main())
