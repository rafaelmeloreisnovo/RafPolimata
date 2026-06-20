#!/usr/bin/env python3
"""Emit conservative status for RAF_001..RAF_056 methods (stdlib only)."""
from __future__ import annotations
import json, re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
OUT_JSON = ROOT / 'results' / 'raf_methods_status.json'
OUT_MD = ROOT / 'docs' / 'RAF_METHODS_STATUS.md'
RAF_RE = re.compile(r'^RAF_(\d{3})_(.+)\.c$')

def category(n:int, name:str)->str:
    if 1 <= n <= 20: return 'AVR/MCU'
    if 21 <= n <= 35: return 'Raspberry/Linux MMIO'
    if 24 <= n <= 28: return 'ARM64 counters/barriers'
    if 36 <= n <= 48: return 'Android/JNI/ABI'
    if n == 49: return 'Termux/CLI'
    if 50 <= n <= 51: return 'exportação JSON'
    if 52 <= n <= 53: return 'QEMU'
    if n == 54: return 'benchmark/cache/batching'
    if n == 55: return 'benchmark/cache/batching'
    if n == 56: return 'comparação baseline'
    return 'TOKEN_VAZIO'

def hardware(cat:str)->str:
    if cat == 'AVR/MCU': return 'AVR/MCU real para runtime completo'
    if cat == 'Raspberry/Linux MMIO': return 'Raspberry/Linux com /dev/mem ou /dev/gpiomem quando aplicável'
    if cat == 'ARM64 counters/barriers': return 'ARM64 real para semântica de contador/barreira'
    if cat == 'Android/JNI/ABI': return 'Android device/emulator + NDK/SDK/logcat'
    if cat == 'Termux/CLI': return 'Android/Termux ou host compatível'
    if cat == 'QEMU': return 'QEMU/TCG configurado'
    return 'host; baseline externo quando aplicável'

def title_from_index():
    d={}
    idx=ROOT/'RAF_INDEX.md'
    if idx.exists():
        for line in idx.read_text(encoding='utf-8', errors='replace').splitlines():
            m=re.match(r'- `(RAF_(\d{3})_[^`]+\.c)` — (.+)', line)
            if m: d[m.group(1)] = m.group(3)
    return d

def main():
    titles=title_from_index()
    rows=[]
    files=sorted(ROOT.glob('RAF_[0-9][0-9][0-9]_*.c'))
    for p in files:
        m=RAF_RE.match(p.name)
        if not m: continue
        n=int(m.group(1))
        if n>56: continue
        cat=category(n, m.group(2))
        text=p.read_text(encoding='utf-8', errors='replace')
        has_main='main(' in text
        has_bench=('benchmark' in text.lower()) or n in (39,40,49,52,53,54,56)
        needs_hw=cat in {'AVR/MCU','Raspberry/Linux MMIO','ARM64 counters/barriers','Android/JNI/ABI','Termux/CLI','QEMU'}
        rows.append({
            'method': f'{n:03d}', 'file': p.name, 'domain': titles.get(p.name, m.group(2).replace('_',' ')),
            'architecture': cat, 'hardware_required': hardware(cat),
            'host_compile': 'PENDING', 'host_run': 'TOKEN_VAZIO' if not has_main else 'PENDING',
            'return_zero_verdict': 'TOKEN_VAZIO', 'depends_on_hardware': needs_hw,
            'has_benchmark': has_bench, 'risk': 'DEVICE_REQUIRED' if needs_hw else 'PASS_LIMITED se apenas compilar',
            'next_proof': 'gcc -c -I. + execução em hardware/device quando aplicável; registrar logs brutos e rollback',
        })
    missing=[f'{i:03d}' for i in range(1,57) if not any(r['method']==f'{i:03d}' for r in rows)]
    data={'schema':'raf_methods_status.v1','generated_by':'scripts/emit_raf_methods_status.py','methods_count':len(rows),'missing_001_056':missing,'items':rows}
    OUT_JSON.parent.mkdir(exist_ok=True, parents=True); OUT_JSON.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    OUT_MD.parent.mkdir(exist_ok=True, parents=True)
    lines=['# RAF Methods Status','','Status conservador dos métodos RAF 001-056. `TOKEN_VAZIO` significa ausência honesta de prova runtime; retorno 0 só vira `EXECUTA_PASS` após execução registrada.','','| Método | Arquivo | Domínio | Arquitetura | Hardware necessário | Compila host | Executa host | Retorno 0 | Benchmark | Risco | Próxima prova |','|---|---|---|---|---|---|---|---|---|---|---|']
    for r in rows:
        lines.append(f"| {r['method']} | `{r['file']}` | {r['domain']} | {r['architecture']} | {r['hardware_required']} | {r['host_compile']} | {r['host_run']} | {r['return_zero_verdict']} | {str(r['has_benchmark']).lower()} | {r['risk']} | {r['next_proof']} |")
    if missing: lines += ['','## TOKEN_VAZIO', '- Métodos ausentes em 001-056: '+', '.join(missing)]
    OUT_MD.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    return 1 if missing else 0
if __name__=='__main__': raise SystemExit(main())
