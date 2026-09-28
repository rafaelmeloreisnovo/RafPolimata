#!/usr/bin/env bash
set -euo pipefail
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$ROOT"

PROVIDER="${RAFP_REPRO_PROVIDER:-UNDECLARED}"
EXECUTOR="${RAFP_REPRO_EXECUTOR:-UNDECLARED}"
OUT="build/maturity"
mkdir -p "$OUT"

make maturity-all
SANITIZE=1 bash tests/maturity/run_million_properties.sh
python3 scripts/generate_maturity_sbom.py --out "$OUT/sbom.spdx.json"
python3 scripts/generate_supply_chain_receipt.py   --artifact build/sdk/rafpolimata_v1/librafpolimata_v1.a   --out "$OUT/supply-chain.json"

RAFP_REPRO_PROVIDER="$PROVIDER" RAFP_REPRO_EXECUTOR="$EXECUTOR" python3 - <<'PY'
import json, os, pathlib, platform, subprocess
out = pathlib.Path("build/maturity/external-reproduction.json")
data = {
    "schema": "rafpolimata.external-reproduction.v1",
    "source_sha": subprocess.check_output(["git","rev-parse","HEAD"], text=True).strip(),
    "provider": os.environ["RAFP_REPRO_PROVIDER"],
    "executor": os.environ["RAFP_REPRO_EXECUTOR"],
    "host": {"system": platform.system(), "release": platform.release(), "machine": platform.machine()},
    "maturity_bundle": "PASS",
    "independent_provider_claim_allowed": False,
    "promotion_note": "External identity/provenance must be reviewed before independent-provider promotion."
}
out.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
PY

printf 'EXTERNAL_REPRO_BUNDLE_PASS provider=%s executor=%s sha=%s\n'   "$PROVIDER" "$EXECUTOR" "$(git rev-parse HEAD)"
