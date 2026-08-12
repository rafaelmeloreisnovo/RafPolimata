import importlib.util
from pathlib import Path

P = Path(__file__).resolve().parents[1] / "scripts" / "audit_apkc_raw_source_paths.py"
spec = importlib.util.spec_from_file_location("rawgate", P)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

p = Path("scripts/demo.sh")
state, _ = m.classify_text(p, "clang Apkc/apkc.c -o apkc\n")
assert state == "FAIL_RAW_BYPASS"

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
    Path(".github/workflows/ci.yml"),
    "run: clang -fsyntax-only Apkc/apkc.c\n",
)
assert state == "FAIL_RAW_BYPASS"

state, _ = m.classify_text(
    Path(".github/workflows/ci.yml"),
    "run: make apkc-hardened-source && clang -fsyntax-only build/generated/Apkc/apkc.source-cap-hardened.c\n",
)
assert state == "NO_RAW_REF"

state, _ = m.classify_text(Path("docs/note.md"), "clang Apkc/apkc.c -o apkc\n")
assert state == "IGNORED_NON_EXEC"

print("PASS raw-source gate classification")
