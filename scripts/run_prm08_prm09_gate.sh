#!/data/data/com.termux/files/usr/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="${1:-$ROOT/build/prm08_prm09}"
PYTHON_BIN="${PYTHON_BIN:-}"

if [ -z "$PYTHON_BIN" ]; then
  if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN=python3
  elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN=python
  else
    echo "FAIL: python3/python not found" >&2
    exit 127
  fi
fi

mkdir -p "$OUT"
export PYTHONDONTWRITEBYTECODE=1

"$PYTHON_BIN" -m unittest \
  "$ROOT/tests/test_navier_stokes_triangle_mms.py" \
  "$ROOT/tests/test_yang_mills_u1_triangle.py" \
  -v 2>&1 | tee "$OUT/tests.log"

"$PYTHON_BIN" "$ROOT/experiments/navier_stokes_triangle_mms/mms2d.py" \
  > "$OUT/prm08_triangle_mms.json"

"$PYTHON_BIN" "$ROOT/experiments/yang_mills_u1_triangle/u1_triangle.py" \
  > "$OUT/prm09_u1_triangle.json"

"$PYTHON_BIN" - "$ROOT" "$OUT" <<'PY'
import hashlib
import json
import platform
import sys
from pathlib import Path

root = Path(sys.argv[1])
out = Path(sys.argv[2])
paths = [
    root / "experiments/navier_stokes_triangle_mms/mms2d.py",
    root / "experiments/yang_mills_u1_triangle/u1_triangle.py",
    root / "tests/test_navier_stokes_triangle_mms.py",
    root / "tests/test_yang_mills_u1_triangle.py",
    out / "prm08_triangle_mms.json",
    out / "prm09_u1_triangle.json",
]
hashes = {
    str(path.relative_to(root) if path.is_relative_to(root) else path.name):
    hashlib.sha256(path.read_bytes()).hexdigest()
    for path in paths
}
receipt = {
    "schema": "raf.prm08-prm09.runtime-receipt.v1",
    "environment": {
        "python": platform.python_version(),
        "platform": platform.platform(),
        "implementation": platform.python_implementation(),
    },
    "results": {
        "unit_tests": "17/17_EXPECTED",
        "prm08_output": "GENERATED",
        "prm09_output": "GENERATED",
        "claim_allowed": False,
    },
    "hashes_sha256": hashes,
    "boundaries": {
        "independent_replication": "TOKEN_VAZIO_UNTIL_SECOND_ENVIRONMENT",
        "navier_stokes_3d_solution": "NOT_CLAIMED",
        "yang_mills_mass_gap": "NOT_CLAIMED",
    },
}
(out / "receipt.json").write_text(
    json.dumps(receipt, indent=2, sort_keys=True) + "\n",
    encoding="utf-8",
)
PY

echo "PASS: PRM-08/PRM-09 gate"
echo "OUT=$OUT"
