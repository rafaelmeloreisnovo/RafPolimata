import importlib.util
from pathlib import Path

P = Path(__file__).resolve().parents[1] / "scripts" / "audit_apkc_raw_source_paths.py"
spec = importlib.util.spec_from_file_location("rawgate", P)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

p = Path("scripts/demo.sh")
RAW = "Apkc/" + "apkc.c"

# Direct compile of canonical raw source is always a bypass, including implicit a.out.
state, _ = m.classify_text(p, f"clang {RAW} -o apkc\n")
assert state == "FAIL_RAW_BYPASS"
state, _ = m.classify_text(p, f"clang {RAW}\n")
assert state == "FAIL_RAW_BYPASS"
state, _ = m.classify_text(p, f'"$CC" {RAW}\n')
assert state == "FAIL_RAW_BYPASS"

# Mentioning a compiler in diagnostic text is not itself execution.
state, _ = m.classify_text(p, f'echo "clang {RAW}"\n')
assert state == "PASS_REFERENCE_ONLY"

# Multiline and variable-alias compiles must also fail.
state, _ = m.classify_text(
    p,
    f"SRC={RAW}\n"
    '"$CC" -std=c11 -nostdlib "$SRC" -o apkc\n',
)
assert state == "FAIL_RAW_BYPASS"

state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    "run: |\n  clang \\\n    -fsyntax-only \\\n    " + RAW + "\n",
)
assert state == "FAIL_RAW_BYPASS"

# Canonical hardening entrypoints authorize the raw source only as transformer input.
state, _ = m.classify_text(
    p,
    f"python3 scripts/patch_apkc_source_cap.py {RAW} '$TMP/apkc_hardened.c'\n"
    "clang '$TMP/apkc_hardened.c' -o apkc\n",
)
assert state == "PASS_HARDENED"

state, _ = m.classify_text(
    Path("tools/runtime.sh"),
    f"python3 scripts/patch_apkc_runtime_source.py --input {RAW} --output '$GEN'\n",
)
assert state == "PASS_HARDENED"

state, _ = m.classify_text(
    Path("tools/runtime-c-escape.sh"),
    f"python3 scripts/patch_apkc_runtime_source_c_escape.py --input {RAW} --output '$GEN'\n"
    "clang -nostdlib '$GEN' -o apkc\n",
)
assert state == "PASS_HARDENED"

# A hardening marker elsewhere in the file must never authorize a separate raw compile.
state, _ = m.classify_text(
    Path("tools/mixed.sh"),
    f"python3 scripts/patch_apkc_runtime_source_c_escape.py --input {RAW} --output '$GEN'\n"
    f"clang {RAW} -o raw-apkc\n",
)
assert state == "FAIL_RAW_BYPASS"

# Workflow compile remains fail-closed.
state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    f"run: clang -fsyntax-only {RAW}\n",
)
assert state == "FAIL_RAW_BYPASS"

state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    "run: make apkc-hardened-source && clang -fsyntax-only build/generated/Apkc/apkc.source-cap-hardened.c\n",
)
assert state == "NO_RAW_REF"

# Source inspection, fixtures and commentary are references, not build bypasses.
state, _ = m.classify_text(
    Path("scripts/preflight.py"),
    f'apkc_path = root / "{RAW}"\ntext = apkc_path.read_text()\n',
)
assert state == "PASS_REFERENCE_ONLY"

state, _ = m.classify_text(
    Path("tests/test_fixture.py"),
    f'(root / "{RAW}").write_text("fixture")\n',
)
assert state == "PASS_REFERENCE_ONLY"

state, _ = m.classify_text(
    Path("scripts/probe.sh"),
    f'echo "fork path in {RAW} remains TOKEN_VAZIO"\n',
)
assert state == "PASS_REFERENCE_ONLY"

# Files under docs remain documentation even when YAML-suffixed.
state, _ = m.classify_text(Path("docs/checklist.yml"), f'target: "{RAW}"\n')
assert state == "IGNORED_NON_EXEC"

state, _ = m.classify_text(Path("docs/note.md"), f"clang {RAW} -o apkc\n")
assert state == "IGNORED_NON_EXEC"

print("PASS raw-source gate classification")
