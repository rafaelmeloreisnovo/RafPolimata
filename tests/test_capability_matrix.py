#!/usr/bin/env python3
"""Validate stable RAF language ids, routes, extensions and M063 coverage."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent.parent
RAF_HEADER = ROOT / "raf_compile.h"
RAF_CPU = ROOT / "raf_cpu.c"
LP_HEADER = ROOT / "Apkc" / "lang_profile.h"
POLICY_HEADER = ROOT / "Apkc" / "lang_freestanding_policy.h"

raf_src = RAF_HEADER.read_text(encoding="utf-8")
cpu_src = RAF_CPU.read_text(encoding="utf-8")
lp_src = LP_HEADER.read_text(encoding="utf-8")
policy_src = POLICY_HEADER.read_text(encoding="utf-8")

constants = {
    name: int(value)
    for name, value in re.findall(r"#define\s+(RAF_LANG_[A-Z0-9_]+)\s+(\d+)", raf_src)
    if name not in {"RAF_LANG_COUNT", "RAF_RECOGNIZED_LANG_COUNT"}
}
count = int(re.search(r"#define\s+RAF_LANG_COUNT\s+(\d+)", raf_src).group(1))
recognized = int(
    re.search(r"#define\s+RAF_RECOGNIZED_LANG_COUNT\s+(\d+)", raf_src).group(1)
)

expected_names = [
    "C", "CPP", "S", "PY", "RS", "KT", "JAVA", "SH", "PL", "JS", "PHP", "JSX",
    "GLSL", "CL", "HLSL", "WGSL", "DSP", "TFLITE", "GO", "RB", "SWIFT", "GROOVY", "CLJ",
    "UNKNOWN",
]
expected_profiles = [
    "c", "cpp", "asm", "py", "rs", "kt", "java", "sh", "pl", "js", "php", "jsx",
    "glsl", "cl", "hlsl", "wgsl", "dsp", "tflite", "go", "rb", "swift", "groovy", "clj",
]
extension_contract = {
    ".c": "RAF_LANG_C",
    ".cpp": "RAF_LANG_CPP",
    ".s": "RAF_LANG_S",
    ".py": "RAF_LANG_PY",
    ".rs": "RAF_LANG_RS",
    ".kt": "RAF_LANG_KT",
    ".java": "RAF_LANG_JAVA",
    ".sh": "RAF_LANG_SH",
    ".pl": "RAF_LANG_PL",
    ".js": "RAF_LANG_JS",
    ".php": "RAF_LANG_PHP",
    ".jsx": "RAF_LANG_JSX",
    ".comp": "RAF_LANG_GLSL",
    ".cl": "RAF_LANG_CL",
    ".hlsl": "RAF_LANG_HLSL",
    ".wgsl": "RAF_LANG_WGSL",
    ".dsp": "RAF_LANG_DSP",
    ".tflite": "RAF_LANG_TFLITE",
    ".go": "RAF_LANG_GO",
    ".rb": "RAF_LANG_RB",
    ".swift": "RAF_LANG_SWIFT",
    ".groovy": "RAF_LANG_GROOVY",
    ".clj": "RAF_LANG_CLJ",
}

assert count == 24, f"RAF_LANG_COUNT={count}; expected 24 including UNKNOWN"
assert recognized == 23, f"RAF_RECOGNIZED_LANG_COUNT={recognized}; expected 23"
assert len(constants) == count, f"found {len(constants)} constants for count={count}"

for expected_id, suffix in enumerate(expected_names):
    key = f"RAF_LANG_{suffix}"
    assert constants.get(key) == expected_id, f"{key} must be stable id {expected_id}"
assert constants["RAF_LANG_UNKNOWN"] == count - 1

matrix_match = re.search(
    r"RAF_CAP_MATRIX\[RAF_LANG_COUNT\]\[5\]\s*=\s*\{(.*?)\};",
    raf_src,
    re.DOTALL,
)
assert matrix_match, "RAF_CAP_MATRIX not found"
rows = re.findall(
    r"\{\s*([01])\s*,\s*([01])\s*,\s*([01])\s*,\s*([01])\s*,\s*([01])\s*\}",
    matrix_match.group(1),
)
assert len(rows) == count, f"matrix rows={len(rows)}; expected {count}"
assert rows[constants["RAF_LANG_UNKNOWN"]] == ("0", "0", "0", "0", "0")
assert all(int(rows[i][1]) == 1 for i in range(recognized)), "all recognized routes need an ARM64 cell"

mapped_names = set(re.findall(r'return\s+"([a-z0-9_]+)";', raf_src))
for profile in expected_profiles:
    assert profile in mapped_names, f"raf_lang_to_apkc_name missing {profile}"

for extension, lang_id in extension_contract.items():
    pattern = rf'!strcmp\(dot,\s*"{re.escape(extension)}"\).*?return\s+{lang_id};'
    assert re.search(pattern, cpu_src), f"raf_lang_from_ext missing {extension} -> {lang_id}"
assert "if (path == (const char *)0) return RAF_LANG_UNKNOWN;" in cpu_src

lp_count = int(re.search(r"#define\s+LP_COUNT\s+(\d+)", lp_src).group(1))
assert lp_count == recognized == 23

policy_count = int(
    re.search(r"#define\s+RAF_M063_LANGUAGE_COUNT\s+(\d+)u?", policy_src).group(1)
)
assert policy_count == recognized
for profile in expected_profiles:
    assert f'{{"{profile}",' in policy_src, f"M063 policy missing {profile}"

print(
    f"RAF capability matrix: {count} constants / {recognized} recognized profiles / "
    f"{len(extension_contract)} extensions / 1 UNKNOWN: PASS"
)
