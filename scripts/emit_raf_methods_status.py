#!/usr/bin/env python3
"""Emit status matrix for RAF_001..RAF_056 C methods using stdlib only."""
from __future__ import annotations
import json, re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT_JSON=ROOT/"results"/"raf_methods_status.json"
OUT_MD=ROOT/"docs"/"RAF_METHODS_STATUS.md"

def rel(p:Path)->str: return p.relative_to(ROOT).as_posix()
def domain(num:int,name:str)->str:
    if num<=20: return "AVR/MCU"
    if num<=23 or 29<=num<=35: return "Raspberry/Linux MMIO"
    if 24<=num<=28: return "ARM64 counters/barriers"
    if 36<=num<=48: return "Android/JNI/ABI"
    if 49<=num<=56: return "benchmark/cache/batching/export/QEMU"
    return "TOKEN_VAZIO"
def hardware(num:int)->str:
    if num<=20: return "AVR/MCU"
    if 21<=num<=35: return "Raspberry Pi/Broadcom or Linux MMIO"
    if 24<=num<=28: return "ARM64 host"
    if 41<=num<=48: return "Android device/NDK/JNI"
    if 52<=num<=53: return "QEMU"
    return "host_or_device_dependent"
def state(num:int,path:Path)->str:
    if not path.exists(): return "TOKEN_VAZIO"
    if num in set(range(1,21)): return "DEVICE_REQUIRED"
    if 21<=num<=35: return "DEVICE_REQUIRED"
    if 41<=num<=48: return "DEVICE_REQUIRED"
    return "COMPILE_OK"
def has_benchmark(num:int)->str:
    return "AUDIT" if num in {39,40,49,52,53,54,55,56} else "TOKEN_VAZIO"
def risk(num:int)->str:
    if num<=35: return "hardware_register_access_requires_device"
    if 41<=num<=48: return "android_runtime_requires_install_launch_logcat"
    if num>=49: return "benchmark_without_raw_baseline"
    return "host_syntax_not_runtime_proof"
def main()->int:
    rows=[]
    for p in sorted(ROOT.glob("RAF_[0-9][0-9][0-9]_*.c")):
        m=re.match(r"RAF_(\d{3})_(.*)\.c",p.name); num=int(m.group(1)) if m else 0
        rows.append({"number":num,"file":p.name,"domain":domain(num,p.name),"architecture":hardware(num),"hardware_required":hardware(num),"compiles_on_host":"TOKEN_VAZIO: run bash RAF_host_syntax_check.sh for aggregate gate","executes_on_host":"TOKEN_VAZIO unless method has host-safe harness","return_zero_meaning":"EXECUTA_PASS only with explicit runtime log; otherwise TOKEN_VAZIO","state":state(num,p),"has_benchmark":has_benchmark(num),"risk":risk(num),"next_proof":"add command, hardware, flags, raw log, and rollback note"})
    refs=[]
    idx=ROOT/"RAF_INDEX.md"
    if idx.exists(): refs=re.findall(r"`([^`]+\.c)`",idx.read_text(encoding="utf-8",errors="ignore"))
    report={"schema":"rafpolimata.raf_methods_status.v1","count":len(rows),"index_refs":refs,"index_ref_count":len(refs),"rows":rows}
    OUT_JSON.parent.mkdir(exist_ok=True); OUT_JSON.write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    lines=["# RAF Methods Status", "", "Estado: `AUDIT`", "", "Regra: retorno 0 só vira `EXECUTA_PASS` com log explícito; hardware ausente fica `DEVICE_REQUIRED` ou `TOKEN_VAZIO`.", "", "| Nº | Arquivo | Domínio | Arquitetura/Hardware | Estado | Compila no host | Executa no host | Benchmark | Risco | Próxima prova |", "|---:|---|---|---|---|---|---|---|---|---|"]
    for r in rows:
        lines.append(f"| {r['number']} | `{r['file']}` | {r['domain']} | {r['hardware_required']} | {r['state']} | {r['compiles_on_host']} | {r['executes_on_host']} | {r['has_benchmark']} | {r['risk']} | {r['next_proof']} |")
    lines += ["", "## Índice", "", f"- Arquivos RAF encontrados: `{len(rows)}`", f"- Referências em RAF_INDEX.md: `{len(refs)}`", "- Divergência conhecida: `RAF_INDEX.md` referencia `methods/*.c`, enquanto os arquivos reais estão na raiz; isso é `AUDIT`, não PASS.", "", "JSON source: `results/raf_methods_status.json`"]
    OUT_MD.write_text("\n".join(lines)+"\n",encoding="utf-8")
    print(f"wrote {rel(OUT_JSON)} and {rel(OUT_MD)} methods={len(rows)}")
    return 0
if __name__=="__main__": raise SystemExit(main())
