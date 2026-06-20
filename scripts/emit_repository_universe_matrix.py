#!/usr/bin/env python3
"""Emit repository evidence/governance universe matrix using stdlib only."""
from __future__ import annotations
import json, re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = ROOT / "results" / "repository_universe_matrix.json"
OUT_MD = ROOT / "docs" / "REPOSITORY_UNIVERSE_MATRIX.md"
EXPECTED = ["README.md","docs","configs","scripts","Benchmark","Apkc","data","results","tools",".github/workflows","RAF_INDEX.md","RAF_rafaelia_common.h","raf_compile.h","raf_precomp.c"]
ROOT_ALLOWED = {".gitignore","README.md","README_RAFAELIA_ROOT_OPTIMIZER.md","CHANGELOG.md","CLAUDE.md","RAFAELIA_MASTER_DOC.txt","RAFAELIA_COMPLETE_v4.zip","Arduíno.txt","Arm64 Mixer leve criptografia.md","L1.md","RASBERY.MD","big_test.sh"}
ROOT_PREFIXES = ("RAF_","raf_","raiz_")


def rel(p: Path) -> str: return p.relative_to(ROOT).as_posix()
def exists(path: str) -> bool: return (ROOT / path).exists()
def classify(path: str) -> str:
    p=ROOT/path
    if path.startswith(".github/workflows") or path.startswith("configs") or p.suffix in {".yml",".yaml"}: return "CONFIG"
    if path.startswith("data"): return "DATA"
    if path.startswith("results") or "proof" in path: return "RESULT"
    if path == "scripts" or path.startswith("scripts/") or path == "Apkc" or path.startswith("Apkc/") or path == "tools" or path.startswith("tools/") or p.suffix in {".c",".h",".py",".sh"}: return "RUNTIME"
    return "REFERENCE"
def state(path: str) -> str:
    if not exists(path): return "TOKEN_VAZIO"
    t=classify(path)
    if path.startswith("Apkc/proofs") or path.startswith("results"): return "AUDIT"
    return t
def gate(path: str) -> str:
    if path == "README.md": return "manual_review"
    if path.startswith("scripts/") and path.endswith(".py"): return f"python3 {path}"
    if path.startswith("scripts/") and path.endswith(".sh"): return f"sh {path}"
    if path.startswith(".github/workflows"): return "github_actions"
    if re.match(r"RAF_\d{3}_.*\.c", Path(path).name): return "bash RAF_host_syntax_check.sh"
    if path.startswith("Apkc"): return "sh scripts/apkc_validate.sh"
    if path.startswith("Benchmark"): return "TOKEN_VAZIO: benchmark requires baseline/hardware"
    return "review"
def invariant(path: str) -> str:
    if path.startswith("Apkc"): return "freestanding/no_false_PASS/android_runtime_requires_evidence"
    if path.startswith("Benchmark"): return "no_benchmark_without_baseline"
    if path.startswith("docs") or path == "README.md": return "claim_evidence_lock"
    if path.startswith("configs"): return "canonical_contract"
    if re.match(r"RAF_\d{3}_.*\.c", Path(path).name): return "RAF_INDEX_1_to_1_and_host_syntax"
    return "origin_to_structure_to_evidence_to_rollback"
def evidence(path: str) -> str:
    p=ROOT/path
    if not p.exists(): return "TOKEN_VAZIO"
    if p.is_dir(): return f"present; entries={len([x for x in p.iterdir() if x.name != '.git'])}"
    return f"present; bytes={p.stat().st_size}"
def gap(path: str) -> str:
    if not exists(path): return "TOKEN_VAZIO"
    if path.startswith("Apkc") and "proof" not in path: return "Android install+launch+logcat DEVICE_REQUIRED"
    if path.startswith("Benchmark"): return "baseline/p95/p99/raw log TOKEN_VAZIO unless present"
    if path.startswith("results") and not any((ROOT/path).glob("*")) if (ROOT/path).is_dir() else False: return "experiment outputs TOKEN_VAZIO"
    return "none_or_documented_in_matrix"
def next_action(path: str) -> str:
    if not exists(path): return "create_or_mark_TOKEN_VAZIO"
    if path.startswith("Apkc"): return "capture Android runtime proof when device exists"
    if path.startswith("Benchmark"): return "run benchmark with hardware+flags+raw logs"
    if re.match(r"RAF_\d{3}_.*\.c", Path(path).name): return "compile host syntax and classify hardware dependency"
    return "keep evidence gate current"
def rollback(path: str) -> str:
    if path.startswith("Apkc"): return "revert artifact; preserve TOKEN_VAZIO logs"
    if path.startswith(".github"): return "revert workflow step or mark manual/device-required"
    return "git revert scoped file; preserve audit trail"
def row(path: str, kind: str|None=None) -> dict[str,str]:
    return {"path":path,"type":kind or classify(path),"function":function(path),"state":state(path),"invariant":invariant(path),"gate":gate(path),"evidence":evidence(path),"gap":gap(path),"next_action":next_action(path),"risk":risk(path),"rollback":rollback(path)}
def function(path: str) -> str:
    if path == "README.md": return "project entry and evidence discipline summary"
    if path == "RAF_INDEX.md": return "index for RAF methods"
    if re.match(r"RAF_\d{3}_.*\.c", Path(path).name): return "RAF method implementation"
    if path.startswith("docs"): return "documentation/protocol/governance reference"
    if path.startswith("scripts"): return "automation/evidence gate"
    if path.startswith("Apkc"): return "Android/APKc toolchain/proof area"
    return "repository component"
def risk(path: str) -> str:
    if not exists(path): return "missing_expected_component"
    if path.startswith("Apkc"): return "false Android PASS without device/logcat"
    if path.startswith("Benchmark"): return "benchmark claim without reproducible baseline"
    if path.startswith("docs"): return "claim stronger than evidence"
    return "drift_without_gate"

def broken_links() -> list[str]:
    out=[]; pat=re.compile(r"\[[^\]]+\]\(([^)#:]+)(?:#[^)]+)?\)")
    for md in sorted((ROOT/"docs").glob("*.md")) + [ROOT/"README.md"]:
        text=md.read_text(encoding="utf-8", errors="ignore")
        for target in pat.findall(text):
            if "://" in target or target.startswith("mailto:"): continue
            cand=(md.parent/target).resolve()
            try: cand.relative_to(ROOT)
            except ValueError: out.append(f"{rel(md)} -> {target} (outside_root)"); continue
            if not cand.exists(): out.append(f"{rel(md)} -> {target}")
    return out

def main() -> int:
    rows=[]
    for item in EXPECTED: rows.append(row(item))
    for p in sorted(ROOT.glob("RAF_[0-9][0-9][0-9]_*.c")): rows.append(row(rel(p), "RUNTIME"))
    for extra in ["RAF_56_METHODS.md","RAF_BENCHMARK_MATRIX.md","RAF_benchmark_matrix.csv","RAF_host_syntax_check.sh","docs/PROTOCOLO_CANONICO_COHERENCIA.md","docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md","docs/MATRIZ_JURIDICO_TECNOLOGICA.md","Apkc/proofs/out","ci","tests","rafaelia"]:
        rows.append(row(extra))
    loose=[]
    for c in sorted(ROOT.iterdir(), key=lambda p:p.name):
        if c.is_file() and c.name not in ROOT_ALLOWED and not c.name.startswith(ROOT_PREFIXES): loose.append(c.name)
    report={"schema":"rafpolimata.repository_universe_matrix.v1","rows":rows,"summary":{"row_count":len(rows),"raf_method_files":len(list(ROOT.glob('RAF_[0-9][0-9][0-9]_*.c'))),"root_loose_files":loose,"broken_markdown_links":broken_links(),"data_files":[rel(p) for p in sorted((ROOT/'data').glob('*')) if p.is_file()],"result_files":[rel(p) for p in sorted((ROOT/'results').glob('*')) if p.is_file()],"workflows":[rel(p) for p in sorted((ROOT/'.github/workflows').glob('*')) if p.is_file()]}}
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True); OUT_JSON.write_text(json.dumps(report, indent=2, ensure_ascii=False)+"\n", encoding="utf-8")
    lines=["# Repository Universe Matrix", "", "Estado: `AUDIT`", "", "Regra: nenhum PASS sem evidência; lacunas ficam como `TOKEN_VAZIO`, `SKIPPED`, `PENDING`, `PASS_LIMITED` ou `DEVICE_REQUIRED`.", "", "| Caminho | Tipo | Função | Estado atual | Invariante protegida | Gate ou comando | Evidência | Lacuna | Próxima ação | Risco | Rollback/Mitigação |", "|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in rows:
        vals=[r[k].replace("|","/") for k in ("path","type","function","state","invariant","gate","evidence","gap","next_action","risk","rollback")]
        lines.append("| " + " | ".join(vals) + " |")
    lines += ["", "## Summary", "", f"- RAF method files: `{report['summary']['raf_method_files']}`", f"- Root loose files: `{len(loose)}`", f"- Broken markdown links: `{len(report['summary']['broken_markdown_links'])}`", f"- Workflows: `{len(report['summary']['workflows'])}`", "", "JSON source: `results/repository_universe_matrix.json`"]
    OUT_MD.write_text("\n".join(lines)+"\n", encoding="utf-8")
    print(f"wrote {OUT_JSON.relative_to(ROOT)} and {OUT_MD.relative_to(ROOT)} rows={len(rows)}")
    return 0
if __name__ == "__main__": raise SystemExit(main())
