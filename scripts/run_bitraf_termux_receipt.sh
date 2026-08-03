#!/data/data/com.termux/files/usr/bin/bash
set -Eeuo pipefail

# BITRAF Matrix 8000/4096 — physical Termux receipt runner
# Non-destructive: it refuses dirty worktrees, does not reset main and writes only
# under auditoria/receipts/bitraf_termux before committing to the receipt branch.

REPO_EXPECTED="rafaelmeloreisnovo/RafPolimata"
BRANCH_EXPECTED="feature/bitraf-termux-receipt-v1-20260803"
SOURCE_MERGE_COMMIT="fd2b9925c6cbde730741117ec3c6dd979db9b25e"
CLAIM_ALLOWED="false"

fail() {
  printf 'BITRAF_TERMUX_RECEIPT=FAIL\nREASON=%s\n' "$1" >&2
  exit 1
}

need() {
  command -v "$1" >/dev/null 2>&1 || fail "missing_command:$1"
}

need git
need clang
need python3
need sha256sum
need uname
need date
need cmp
need grep

ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || fail "not_inside_git_repository"
cd "$ROOT"

ORIGIN="$(git remote get-url origin 2>/dev/null || true)"
case "$ORIGIN" in
  https://github.com/${REPO_EXPECTED}.git|https://github.com/${REPO_EXPECTED}|git@github.com:${REPO_EXPECTED}.git)
    ;;
  *) fail "unexpected_origin:$ORIGIN" ;;
esac

BRANCH="$(git branch --show-current)"
[ "$BRANCH" = "$BRANCH_EXPECTED" ] || fail "checkout_required:$BRANCH_EXPECTED"

[ -z "$(git status --porcelain)" ] || fail "dirty_worktree"

git cat-file -e "${SOURCE_MERGE_COMMIT}^{commit}" 2>/dev/null || fail "missing_source_merge_commit"
git merge-base --is-ancestor "$SOURCE_MERGE_COMMIT" HEAD || fail "source_merge_commit_not_ancestor"

# Exact bytes executed: these hashes are frozen by the local receipt merged in PR #194.
cat > /tmp/bitraf_expected_sha256.$$ <<'EOF_HASHES'
47e4237fd272aaf1e0bb96f28dcc138bf866d471c79ab8269d3d1f0da3965714  rafaelia/bitraf_matrix.h
2a4a43847d92ec21e16b534464d4819348b44e3cafc6415215e0bac4822423a8  tests/bitraf_matrix_test.c
c0c7199969c8dc4953f1915f4762dd88b248f8ab02248ba40d8d1c0e53ddb380  tests/test_bitraf_matrix.py
04ccd58a252d3cf921e748f8d42c094f9eb6dc35a3adac2e7116aa6e66718b64  scripts/bitraf_matrix_manifest.py
b0fb03c335fe023ee360555e3924da9ea334986594135672b3c597598f6d155c  data/encoding_timeline.v1.json
66e9423451171c7c775b1f1ff3bd804e04df46d60859ac74bf0771ebe262e4f6  docs/BITRAF_MATRIX_8000_4096_V1.md
EOF_HASHES
sha256sum -c /tmp/bitraf_expected_sha256.$$ || fail "source_hash_mismatch"
rm -f /tmp/bitraf_expected_sha256.$$

STAMP_UTC="$(date -u +%Y%m%dT%H%M%SZ)"
STAMP_LOCAL="$(date +%Y-%m-%dT%H:%M:%S%z)"
ARCH="$(uname -m | tr '/ ' '__')"
RECEIPT_DIR="auditoria/receipts/bitraf_termux/${STAMP_UTC}_${ARCH}"
BUILD_DIR="$RECEIPT_DIR/build"
mkdir -p "$BUILD_DIR"

HEAD_BEFORE="$(git rev-parse HEAD)"

{
  printf 'timestamp_utc=%s\n' "$STAMP_UTC"
  printf 'timestamp_local=%s\n' "$STAMP_LOCAL"
  printf 'repo=%s\n' "$REPO_EXPECTED"
  printf 'branch=%s\n' "$BRANCH"
  printf 'head_before=%s\n' "$HEAD_BEFORE"
  printf 'source_merge_commit=%s\n' "$SOURCE_MERGE_COMMIT"
  printf 'uname=%s\n' "$(uname -a)"
  printf 'clang=%s\n' "$(clang --version | head -n 1)"
  printf 'python=%s\n' "$(python3 --version 2>&1)"
  printf 'git=%s\n' "$(git --version)"
  if command -v getprop >/dev/null 2>&1; then
    printf 'android_release=%s\n' "$(getprop ro.build.version.release)"
    printf 'android_sdk=%s\n' "$(getprop ro.build.version.sdk)"
    printf 'device=%s\n' "$(getprop ro.product.manufacturer) $(getprop ro.product.model)"
    printf 'cpu_abilist=%s\n' "$(getprop ro.product.cpu.abilist)"
  else
    printf 'android_release=TOKEN_VAZIO_GETPROP\n'
    printf 'android_sdk=TOKEN_VAZIO_GETPROP\n'
    printf 'device=TOKEN_VAZIO_GETPROP\n'
    printf 'cpu_abilist=TOKEN_VAZIO_GETPROP\n'
  fi
} > "$RECEIPT_DIR/environment.txt"

python3 -m unittest tests/test_bitraf_matrix.py -v \
  > "$RECEIPT_DIR/python_tests.log" 2>&1

grep -Eq '(^|[[:space:]])OK($|[[:space:]])' "$RECEIPT_DIR/python_tests.log" \
  || fail "python_tests_without_OK"

clang -std=c11 -O2 -Wall -Wextra -Werror \
  tests/bitraf_matrix_test.c -o "$BUILD_DIR/bitraf-termux-clang"

"$BUILD_DIR/bitraf-termux-clang" > "$RECEIPT_DIR/bitraf.stdout"
"$BUILD_DIR/bitraf-termux-clang" > "$RECEIPT_DIR/bitraf.stdout.repeat"
cmp -s "$RECEIPT_DIR/bitraf.stdout" "$RECEIPT_DIR/bitraf.stdout.repeat" \
  || fail "nondeterministic_stdout"

grep -qx 'BITRAF_MATRIX_V1=PASS' "$RECEIPT_DIR/bitraf.stdout" \
  || fail "matrix_test_without_PASS"
grep -q 'TOTAL=8000 CORE=4096 SHELL=3904 BASE20=20\^3 CORE_BITS=12' \
  "$RECEIPT_DIR/bitraf.stdout" || fail "partition_output_mismatch"
grep -q 'PRIMES_0_7999=1007 MAX_NORM2_Q32=2378956800' \
  "$RECEIPT_DIR/bitraf.stdout" || fail "numeric_output_mismatch"

python3 scripts/bitraf_matrix_manifest.py \
  --out "$RECEIPT_DIR/generated_manifest" --csv core \
  > "$RECEIPT_DIR/manifest_command.log" 2>&1

sha256sum \
  rafaelia/bitraf_matrix.h \
  tests/bitraf_matrix_test.c \
  tests/test_bitraf_matrix.py \
  scripts/bitraf_matrix_manifest.py \
  data/encoding_timeline.v1.json \
  docs/BITRAF_MATRIX_8000_4096_V1.md \
  "$BUILD_DIR/bitraf-termux-clang" \
  "$RECEIPT_DIR/bitraf.stdout" \
  "$RECEIPT_DIR/generated_manifest/manifest.json" \
  > "$RECEIPT_DIR/SHA256SUMS"

export STAMP_UTC STAMP_LOCAL ARCH RECEIPT_DIR HEAD_BEFORE SOURCE_MERGE_COMMIT
export REPO_EXPECTED BRANCH_EXPECTED CLAIM_ALLOWED ORIGIN
python3 - <<'PY'
import hashlib
import json
import os
import pathlib
import platform
import subprocess

root = pathlib.Path.cwd()
receipt_dir = root / os.environ["RECEIPT_DIR"]

def run(*args):
    return subprocess.check_output(args, text=True, stderr=subprocess.STDOUT).strip()

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def prop(name):
    try:
        return run("getprop", name) or "TOKEN_VAZIO_EMPTY_PROPERTY"
    except Exception:
        return "TOKEN_VAZIO_GETPROP"

stdout_text = (receipt_dir / "bitraf.stdout").read_text(encoding="utf-8")
receipt = {
    "receipt_version": 1,
    "event_id": f"BITRAF-MATRIX-8000-4096-V1-TERMUX-{os.environ['STAMP_UTC']}-{os.environ['ARCH']}",
    "timestamp_utc": os.environ["STAMP_UTC"],
    "timestamp_local": os.environ["STAMP_LOCAL"],
    "repository": os.environ["REPO_EXPECTED"],
    "origin": os.environ["ORIGIN"],
    "branch": os.environ["BRANCH_EXPECTED"],
    "source_revision": os.environ["SOURCE_MERGE_COMMIT"],
    "runner_revision_before_receipt": os.environ["HEAD_BEFORE"],
    "environment": {
        "machine": platform.machine(),
        "platform": platform.platform(),
        "uname": " ".join(platform.uname()),
        "android_release": prop("ro.build.version.release"),
        "android_sdk": prop("ro.build.version.sdk"),
        "manufacturer": prop("ro.product.manufacturer"),
        "model": prop("ro.product.model"),
        "cpu_abilist": prop("ro.product.cpu.abilist"),
        "clang": run("clang", "--version").splitlines()[0],
        "python": run("python3", "--version"),
        "git": run("git", "--version"),
    },
    "commands": [
        "python3 -m unittest tests/test_bitraf_matrix.py -v",
        "clang -std=c11 -O2 -Wall -Wextra -Werror tests/bitraf_matrix_test.c",
        "bitraf-termux-clang (executed twice, stdout compared)",
        "python3 scripts/bitraf_matrix_manifest.py --csv core",
        "sha256sum frozen sources, binary, stdout and manifest",
    ],
    "results": {
        "python_tests": "PASS",
        "native_matrix_test": "PASS",
        "repeat_stdout_identical": True,
        "state_roundtrip": "8000/8000 PASS",
        "core_roundtrip": "4096/4096 PASS",
        "shell_partition": "3904 PASS",
        "base20_bijection": "8000/8000 PASS",
        "opposite_involution": "PASS",
        "rotation_order_4": "PASS",
        "bounded_b3_embedding": "PASS",
        "prime_indices_0_7999": 1007,
        "max_norm2_q32": 2378956800,
        "stdout": stdout_text.splitlines(),
    },
    "hashes_sha256": {
        "rafaelia/bitraf_matrix.h": sha256(root / "rafaelia/bitraf_matrix.h"),
        "tests/bitraf_matrix_test.c": sha256(root / "tests/bitraf_matrix_test.c"),
        "tests/test_bitraf_matrix.py": sha256(root / "tests/test_bitraf_matrix.py"),
        "scripts/bitraf_matrix_manifest.py": sha256(root / "scripts/bitraf_matrix_manifest.py"),
        "data/encoding_timeline.v1.json": sha256(root / "data/encoding_timeline.v1.json"),
        "docs/BITRAF_MATRIX_8000_4096_V1.md": sha256(root / "docs/BITRAF_MATRIX_8000_4096_V1.md"),
        "native_binary": sha256(receipt_dir / "build/bitraf-termux-clang"),
        "stdout": sha256(receipt_dir / "bitraf.stdout"),
        "generated_manifest": sha256(receipt_dir / "generated_manifest/manifest.json"),
    },
    "epistemic_state": {
        "computational_model": "TERMUX_PHYSICAL_PASS",
        "remote_hash_equivalence": "PASS_FROZEN_SOURCE_HASHES",
        "physical_claim": "NOT_CLAIMED",
        "claim_allowed": False,
    },
    "r3": {
        "F_ok": "Frozen source bytes executed natively in Termux; exhaustive invariants passed; repeat output identical.",
        "F_gap": "Independent second Android device and semantic AllStar adapter remain pending.",
        "F_next": "Review this receipt, verify its commit SHA remotely, then add the non-destructive AllStar adapter in a separate branch.",
    },
}
(receipt_dir / "receipt.json").write_text(
    json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
    encoding="utf-8",
)
PY

sha256sum "$RECEIPT_DIR/receipt.json" >> "$RECEIPT_DIR/SHA256SUMS"

# Local-only identity defaults; no token or credential is written.
if [ -z "$(git config user.name || true)" ]; then
  git config user.name "rafaelmeloreisnovo"
fi
if [ -z "$(git config user.email || true)" ]; then
  git config user.email "202743211+rafaelmeloreisnovo@users.noreply.github.com"
fi

git add "$RECEIPT_DIR"
git commit -m "test(bitraf): add Termux ${ARCH} physical receipt ${STAMP_UTC}"
git push -u origin "$BRANCH_EXPECTED"

FINAL_COMMIT="$(git rev-parse HEAD)"
printf 'BITRAF_TERMUX_RECEIPT=PASS\n'
printf 'SOURCE_REVISION=%s\n' "$SOURCE_MERGE_COMMIT"
printf 'RECEIPT_COMMIT=%s\n' "$FINAL_COMMIT"
printf 'RECEIPT_PATH=%s/receipt.json\n' "$RECEIPT_DIR"
printf 'BRANCH=%s\n' "$BRANCH_EXPECTED"
printf 'CLAIM_ALLOWED=%s\n' "$CLAIM_ALLOWED"
printf 'F_NEXT=review_remote_commit_then_second_device_or_allstar_adapter\n'
