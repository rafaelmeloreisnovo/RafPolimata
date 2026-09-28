#!/usr/bin/env python3
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[1]

def require(cond, msg):
    if not cond:
        raise SystemExit(msg)

def main():
    contract = json.loads((ROOT / "contracts/rafpolimata_api_abi_v1.json").read_text())
    header = (ROOT / contract["public_header"]).read_text()
    require(contract["schema"] == "rafpolimata.api-abi.v1", "bad ABI schema")
    require(contract["abi_version"] == 1, "unexpected ABI version")
    require(contract["api_version"] == "0.1.0", "unexpected API version")
    require(contract["runtime_external_dependencies"] == [], "SDK runtime dependency contract changed")
    for symbol in contract["public_symbols"]:
        require(symbol in header, f"public symbol missing from header: {symbol}")
    require("RAFP_V1_ABI_VERSION 1u" in header, "header ABI macro mismatch")
    require((ROOT / "docs/canonical/2026-09-28/MATURITY_AND_PRODUCTIZATION_V1.md").is_file(), "maturity document missing")
    require((ROOT / "docs/security/THREAT_MODEL_RAFPOLIMATA_V1.md").is_file(), "threat model missing")
    require((ROOT / ".github/workflows/maturity-evidence.yml").is_file(), "maturity workflow missing")
    print("MATURITY_CONTRACT_PASS abi=1 api=0.1.0")

if __name__ == "__main__":
    main()
