#!/usr/bin/env python3
"""test_capability_matrix.py — validate RAF_CAP_MATRIX and lang constants in raf_compile.h"""
import re, sys
from pathlib import Path

ROOT = Path(__file__).parent.parent
HDR  = ROOT / "raf_compile.h"

src = HDR.read_text()

# 1. Extract all RAF_LANG_* constants
langs = {k: v for k, v in re.findall(r'#define\s+(RAF_LANG_\w+)\s+(\d+)', src)
         if not k.endswith('_COUNT')}
count_def = int(re.search(r'#define\s+RAF_LANG_COUNT\s+(\d+)', src).group(1))

print(f"[1] Language constants found: {len(langs)}")
for name, val in sorted(langs.items(), key=lambda x: int(x[1])):
    print(f"    {int(val):2d}  {name}")

assert len(langs) == count_def, f"RAF_LANG_COUNT={count_def} but found {len(langs)} RAF_LANG_* constants"
print(f"    -> RAF_LANG_COUNT={count_def} matches constant count: PASS\n")

# 2. Verify expected 12 languages all present
EXPECTED = ['C','CPP','S','PY','RS','KT','JAVA','SH','PL','JS','PHP','JSX']
for e in EXPECTED:
    key = f'RAF_LANG_{e}'
    assert key in langs, f"MISSING: {key}"
print(f"[2] All 12 expected languages present: PASS\n")

# 3. Extract RAF_CAP_MATRIX rows
matrix_block = re.search(r'RAF_CAP_MATRIX\[.*?\]\[.*?\]\s*=\s*\{(.*?)\};', src, re.DOTALL)
assert matrix_block, "RAF_CAP_MATRIX not found"
rows = re.findall(r'\{\s*(\d)\s*,\s*(\d)\s*,\s*(\d)\s*,\s*(\d)\s*,\s*(\d)\s*\}', matrix_block.group(1))
print(f"[3] RAF_CAP_MATRIX rows found: {len(rows)}")
assert len(rows) == count_def, f"Expected {count_def} rows, got {len(rows)}"

# Columns: x86_64, arm64, arm32, rv64, unknown
archs = ['x86_64','arm64','arm32','rv64','unknown']
lang_names = ['C','CPP','ASM','PY','RS','KT','JAVA','SH','PL','JS','PHP','JSX']
print(f"\n    {'lang':6}  {'x86_64':6}  {'arm64':5}  {'arm32':5}  {'rv64':4}  {'unk':3}")
for i, row in enumerate(rows):
    cells = [int(v) for v in row]
    caps = ['  1  ' if c else '  0  ' for c in cells]
    print(f"    {lang_names[i]:6}  {''.join(caps)}")

# Invariant: arm64 must be 1 for ALL languages (apkc is ARM64-first)
arm64_col = [int(r[1]) for r in rows]
assert all(v == 1 for v in arm64_col), "BUG: some language has arm64=0"
print(f"\n    -> arm64=1 for all 12 languages: PASS")

# Invariant: use_fork langs (C/CPP/RS/KT/JAVA/JSX) must have x86_64=0
fork_idxs = [0,1,4,5,6,11]  # C,CPP,RS,KT,JAVA,JSX
for i in fork_idxs:
    assert int(rows[i][0]) == 0, f"BUG: {lang_names[i]} use_fork but x86_64=1"
print(f"    -> use_fork langs x86_64=0: PASS")

# Invariant: use_script langs (PY/SH/PL/JS/PHP) must have x86_64=1
script_idxs = [3,7,8,9,10]
for i in script_idxs:
    assert int(rows[i][0]) == 1, f"BUG: {lang_names[i]} use_script but x86_64=0"
print(f"    -> use_script langs x86_64=1: PASS\n")

# 4. raf_lang_to_apkc_name covers all 12
apkc_names = re.findall(r'return\s+"(\w+)";', src)
print(f"[4] raf_lang_to_apkc_name() return values: {apkc_names}")
EXPECTED_APKC = ['c','cpp','asm','py','rs','kt','java','sh','pl','js','php','jsx']
for name in EXPECTED_APKC:
    assert name in apkc_names, f"MISSING apkc name: {name}"
print(f"    -> all 12 apkc names present: PASS\n")

print("=== test_capability_matrix: ALL PASS ===")
