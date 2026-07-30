#!/usr/bin/env python3
import json
import pathlib
import sys


def validate(path):
    data = json.loads(pathlib.Path(path).read_text(encoding="utf-8"))
    errors = []
    if data.get("schema") != "orquestrador_omega_excellence_v1":
        errors.append("schema")
    needs = data.get("needs", [])
    vectors = data.get("vectors", [])
    if len(needs) != 12:
        errors.append(f"needs={len(needs)}")
    if len(vectors) != 12:
        errors.append(f"vectors={len(vectors)}")
    need_ids = [item.get("id") for item in needs]
    vector_ids = [item.get("id") for item in vectors]
    if len(set(need_ids)) != 12:
        errors.append("need_ids")
    if len(set(vector_ids)) != 12:
        errors.append("vector_ids")
    expected = {f"{need}-{vector}" for need in need_ids for vector in vector_ids}
    contract = data.get("matrix_contract", {})
    if contract.get("expected_cells") != len(expected) or len(expected) != 144:
        errors.append("matrix_coverage")
    if contract.get("generation") != "cartesian_product(needs.id, vectors.id)":
        errors.append("generation")
    defaults = data.get("cell_defaults", {})
    if defaults.get("authority") != "HUMAN_REVIEW_REQUIRED":
        errors.append("authority")
    if defaults.get("state") != "TOKEN_VAZIO_UNASSESSED":
        errors.append("default_state")
    if data.get("global_state", {}).get("claim_allowed") is not False:
        errors.append("claim_gate")
    if contract.get("non_compensatory") is not True:
        errors.append("non_compensatory")
    return errors


if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else pathlib.Path(__file__).with_name("orquestrador-omega-excellence-v1.json")
    found = validate(target)
    if found:
        print("FAIL", *sorted(set(found)))
        raise SystemExit(1)
    print("PASS needs=12 vectors=12 generated_cells=144 claim_allowed=false")
