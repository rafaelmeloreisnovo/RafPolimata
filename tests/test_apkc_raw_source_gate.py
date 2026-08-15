import importlib.util
from pathlib import Path

P = Path(__file__).resolve().parents[1] / "scripts" / "audit_apkc_raw_source_paths.py"
spec = importlib.util.spec_from_file_location("rawgate", P)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

p = Path("scripts/demo.sh")

# Direct compile of canonical raw source is always a bypass.
state, _ = m.classify_text(p, "clang Apkc/apkc.c -o apkc\n")
assert state == "FAIL_RAW_BYPASS"

# Multiline and variable-alias compiles must also fail.
state, _ = m.classify_text(
    p,
    "SRC=Apkc/apkc.c\n"
    '"$CC" -std=c11 -nostdlib "$SRC" -o apkc\n',
)
assert state == "FAIL_RAW_BYPASS"

state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    "run: |\n  clang \\\n    -fsyntax-only \\\n    Apkc/apkc.c\n",
)
assert state == "FAIL_RAW_BYPASS"

# Canonical hardening entrypoints authorize the raw source only as transformer input.
state, _ = m.classify_text(
    p,
    "python3 scripts/patch_apkc_source_cap.py Apkc/apkc.c '$TMP/apkc_hardened.c'\n"
    "clang '$TMP/apkc_hardened.c' -o apkc\n",
)
assert state == "PASS_HARDENED"

state, _ = m.classify_text(
    Path("tools/runtime.sh"),
    "python3 scripts/patch_apkc_runtime_source.py --input Apkc/apkc.c --output '$GEN'\n",
)
assert state == "PASS_HARDENED"

state, _ = m.classify_text(
    Path("tools/runtime-c-escape.sh"),
    "python3 scripts/patch_apkc_runtime_source_c_escape.py --input Apkc/apkc.c --output '$GEN'\n"
    "clang -nostdlib '$GEN' -o apkc\n",
)
assert state == "PASS_HARDENED"

# Workflow compile remains fail-closed.
state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    "run: clang -fsyntax-only Apkc/apkc.c\n",
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
    'apkc_path = root / "Apkc/apkc.c"\ntext = apkc_path.read_text()\n',
)
assert state == "PASS_REFERENCE_ONLY"

state, _ = m.classify_text(
    Path("tests/test_fixture.py"),
    '(root / "Apkc/apkc.c").write_text("fixture")\n',
)
assert state == "PASS_REFERENCE_ONLY"

state, _ = m.classify_text(
    Path("scripts/probe.sh"),
    'echo "fork path in Apkc/apkc.c remains TOKEN_VAZIO"\n',
)
assert state == "PASS_REFERENCE_ONLY"

# Files under docs remain documentation even when YAML-suffixed.
state, _ = m.classify_text(Path("docs/checklist.yml"), 'target: "Apkc/apkc.c"\n')
assert state == "IGNORED_NON_EXEC"

state, _ = m.classify_text(Path("docs/note.md"), "clang Apkc/apkc.c -o apkc\n")
assert state == "IGNORED_NON_EXEC"

print("PASS raw-source gate classification")
