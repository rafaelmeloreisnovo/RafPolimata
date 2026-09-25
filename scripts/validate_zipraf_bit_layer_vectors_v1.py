#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "canonical/zipraf-hw-v1/vectors/bit_layer_reference_vectors_phase_a_v1.json"

def planes(value: int):
    return [(value >> k) & 1 for k in range(8)]

def qmask(q: int):
    if not 1 <= q <= 8:
        raise ValueError(q)
    return (0xFF << (8 - q)) & 0xFF

def main():
    data = json.loads(PATH.read_text(encoding="utf-8"))
    assert data["contract"] == "ZIPRAF-BIT-LAYER-REFERENCE-VECTORS-V1"
    assert data["parent_contract"] == "ZIPRAF-BIT-LAYER-PROGRESSIVE-RASTER-V1"
    assert data["phase"] == "A_BITPLANES_ONLY"
    assert data["claim_allowed"] is False
    assert data["widths"] == [30, 60]
    assert data["q_values"] == [1, 2, 4, 8]
    assert data["geometry"]["M"].startswith("TOKEN_VAZIO")
    assert data["geometry"]["G(M)"].startswith("TOKEN_VAZIO")
    assert data["geometry"]["T-BL-010"] == "TOKEN_VAZIO"

    ids = set()
    for sample in data["samples"]:
        assert sample["id"] not in ids
        ids.add(sample["id"])
        value = sample["byte"]
        assert 0 <= value <= 255
        assert sample["planes_lsb_to_msb"] == planes(value)
        for q in data["q_values"]:
            assert sample["q_reconstruction"][str(q)] == (value & qmask(q))

    for item in data["arrival_order_vectors"]:
        order = item["order"]
        assert sorted(order) == list(range(8))
        value = item["byte"]
        acc = 0
        seen = 0
        p = planes(value)
        for k in order:
            seen |= 1 << k
            acc |= p[k] << k
        assert seen == 0xFF
        assert acc == item["expected_final"] == value

    neg = {x["id"]: x for x in data["negative_vectors"]}
    assert neg["HDR-VERSION"]["expected"] == "ZBL_E_VERSION"
    assert neg["HDR-WIDTH"]["expected"] == "ZBL_E_WIDTH"
    assert neg["HDR-Q0"]["expected"] == "ZBL_E_Q"
    assert neg["DUP-CONFLICT"]["expected"] == "ZBL_E_DUPLICATE_CONFLICT"

    print("PASS ZIPRAF Bit Layer reference vectors Phase A:", len(data["samples"]), "samples")

if __name__ == "__main__":
    main()
