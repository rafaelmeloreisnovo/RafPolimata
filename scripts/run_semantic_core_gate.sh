#!/bin/sh
# Offline semantic compiler gate. No install, launch, clone or download.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=${1:-"$ROOT/build/semantic-core"}
PYTHON=${PYTHON:-python3}
CC_BIN=${CC:-cc}
FIXTURE="$ROOT/tests/fixtures/semantic_equivalence_mix32.v1.json"
mkdir -p "$OUT"

"$PYTHON" -m unittest discover -s "$ROOT/tests" -p 'test_raf_semantic_ir.py'
"$PYTHON" "$ROOT/scripts/raf_semantic_ir.py" validate "$FIXTURE" \
  --out "$OUT/semantic-receipt.json" >/dev/null
"$PYTHON" "$ROOT/scripts/raf_semantic_ir.py" emit-c "$FIXTURE" \
  --out "$OUT/raf_mix32.c" >/dev/null
"$CC_BIN" -std=c11 -O2 -ffreestanding -fno-builtin -fno-stack-protector \
  -Wall -Wextra -Werror -c "$OUT/raf_mix32.c" -o "$OUT/raf_mix32.o"

sha256_value() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  else
    "$PYTHON" - "$1" <<'PY'
import hashlib, pathlib, sys
print(hashlib.sha256(pathlib.Path(sys.argv[1]).read_bytes()).hexdigest())
PY
  fi
}

{
  printf 'schema=%s\n' 'rafaelia.semantic-core-gate.v1'
  printf 'state=%s\n' 'PASS'
  printf 'claim_allowed=%s\n' 'false'
  printf 'install_executed=%s\n' 'false'
  printf 'launch_executed=%s\n' 'false'
  printf 'semantic_receipt_sha256=%s\n' "$(sha256_value "$OUT/semantic-receipt.json")"
  printf 'generated_c_sha256=%s\n' "$(sha256_value "$OUT/raf_mix32.c")"
  printf 'object_sha256=%s\n' "$(sha256_value "$OUT/raf_mix32.o")"
} > "$OUT/semantic-core-gate.env"

printf 'semantic-core: PASS receipt=%s\n' "$OUT/semantic-core-gate.env"
