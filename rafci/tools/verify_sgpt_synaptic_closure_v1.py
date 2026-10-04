#!/usr/bin/env python3
"""Fail-closed validator for SGPT Synaptic Closure V1.

This tool validates cross-repository identity/evidence wiring only. It never
turns mathematical consistency, CI execution, synthetic falsifiers, provider
identity, or document presence into astrophysical validation.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import sys
from typing import Any, Callable

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CLOSURE = ROOT / "rafci" / "closures" / "sgpt_synaptic_closure_v1.json"
TOKEN_VAZIO = "TOKEN_VAZIO"
SCHEMA = "rafaelia.rafci.sgpt-synaptic-closure/v1"
RECEIPT_SCHEMA = "rafaelia.rafci.sgpt-synaptic-closure-receipt/v1"
HEX40 = re.compile(r"^[0-9a-f]{40}$")

EXPECTED_AUTHORITIES = {
    "science.rll.sgpt": {
        "role": "SCIENCE_PRODUCER",
        "repository": "instituto-Rafael/relativity-living-light",
        "commit": "e287ef13be4d5e6d366f29e234c8a5d113d21729",
        "source_head": "b7dd266ff6787696630ebe3cdd684905920e9854",
    },
    "math.world69.sgpt": {
        "role": "MATHEMATICAL_PROOF",
        "repository": "rafaelmeloreisnovo/Matem-tica-",
        "commit": "03deff5b68123406fcca100791d404fe18fda23d",
    },
    "theorems.sgpt": {
        "role": "THEOREM_CLASSIFICATION",
        "repository": "rafaelmeloreisnovo/TeoremasTesesTeorias",
        "commit": "49d542e6ea66d774485726a7fcc1666043538ed8",
    },
    "papers.sgpt": {
        "role": "SCHOLARLY_MANUSCRIPT",
        "repository": "rafaelmeloreisnovo/papers",
        "commit": "266675fb8ec346e05db87383ddc6b4234ce6b61b",
    },
    "papers.sgpt.fire": {
        "role": "ADVERSARIAL_TESTBED",
        "repository": "rafaelmeloreisnovo/papers",
        "commit": "801f95f0ced3baea54a3283eade757baf0365c70",
    },
    "rafci.sgpt.authority": {
        "role": "CLOSURE_AUTHORITY",
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "commit": "757dead806e9b91654553d5eebcda176f40d0473",
        "contract_origin": "f336ca78b00de3fe85cdfff020040f46616ecfed",
        "external_pin_hardening": "f4d1f162ebed36257fe41f89cb145e9006865112",
        "evidence_garden_hardening": "0fb8fcf50baf2471f8a22f9c85063cf6c4992a59",
    },
    "actions.sgpt.provider": {
        "role": "ACTION_PROVIDER",
        "repository": "rafaelmeloreisnovo/actions",
        "commit": "d2731ee6550eaa0d3939067d7a43d902b2f85c3e",
        "authority_transfer": False,
    },
}

EXPECTED_HYPOTHESES = {
    "H_down",
    "H_logistic_photonic",
    "H_temporal_ordering",
}

EXPECTED_BASELINES = [
    "ideal_GRMHD",
    "resistive_GRMHD",
    "two_temperature_GRRMHD",
    "GRPIC",
    "general_relativistic_radiative_transfer",
]

EXPECTED_ADVERSARIAL = [
    "drop_Theta",
    "drop_M_f",
    "drop_R_ram",
    "flip_Theta_sign",
    "shuffle_predictor_variables",
    "permute_temporal_targets",
    "baseline_only",
]

EXPECTED_GATES = [
    "G0_IDENTITY",
    "G1_MATHEMATICS",
    "G2_DETERMINISM",
    "G3_ADVERSARIAL",
    "G4_BASELINE",
    "G5_REPRODUCTION",
    "G6_PREDICTION",
    "G7_CLAIM_PROMOTION",
]

EXPECTED_UNRESOLVED = {
    "cross_repo_receipts",
    "calibrated_source_solution",
    "baseline_comparison",
    "independent_provider_reproduction",
    "held_out_prediction",
    "exclusive_RLL_observable",
}


class ClosureError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ClosureError(message)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def require_hex40(value: Any, label: str) -> None:
    require(isinstance(value, str) and bool(HEX40.fullmatch(value)), f"{label} must be exact lowercase 40-hex")


def load_closure(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fh:
        value = json.load(fh)
    require(isinstance(value, dict), "closure root must be an object")
    return value


def validate_authorities(closure: dict[str, Any]) -> None:
    authorities = closure.get("authorities")
    require(isinstance(authorities, list), "authorities must be an array")
    ids = [item.get("id") if isinstance(item, dict) else None for item in authorities]
    require(len(ids) == len(set(ids)), "duplicate authority id")
    require(set(ids) == set(EXPECTED_AUTHORITIES), "authority set drift")

    indexed = {item["id"]: item for item in authorities}
    for authority_id, expected in EXPECTED_AUTHORITIES.items():
        actual = indexed[authority_id]
        for key, expected_value in expected.items():
            require(actual.get(key) == expected_value, f"{authority_id}.{key} drift")
        for key in ("commit", "source_head", "contract_origin", "external_pin_hardening", "evidence_garden_hardening"):
            if key in actual:
                require_hex40(actual[key], f"{authority_id}.{key}")

    action = indexed["actions.sgpt.provider"]
    require(action.get("role") == "ACTION_PROVIDER", "actions provider cannot become epistemic authority")
    require(action.get("authority_transfer") is False, "actions authority transfer must remain false")

    closure_authorities = [item for item in authorities if item.get("role") == "CLOSURE_AUTHORITY"]
    require(len(closure_authorities) == 1, "exactly one closure authority required")
    require(closure_authorities[0].get("repository") == "rafaelmeloreisnovo/RafPolimata", "RafPolimata must remain closure authority")


def validate_hypotheses(closure: dict[str, Any]) -> None:
    hypotheses = closure.get("hypotheses")
    require(isinstance(hypotheses, list), "hypotheses must be an array")
    ids = {item.get("id") for item in hypotheses if isinstance(item, dict)}
    require(ids == EXPECTED_HYPOTHESES, "hypothesis set drift")
    require(len(hypotheses) == len(EXPECTED_HYPOTHESES), "duplicate hypothesis id")
    for item in hypotheses:
        require(item.get("class") == "HYPOTHESIS", f"{item.get('id')} cannot be promoted to theorem in this closure")
        require(isinstance(item.get("equation"), str) and item["equation"], f"{item.get('id')} equation missing")
        require(isinstance(item.get("falsifier"), str) and item["falsifier"], f"{item.get('id')} falsifier missing")


def validate_gates(closure: dict[str, Any]) -> None:
    require(closure.get("gate_order") == EXPECTED_GATES, "gate order drift")
    gates = closure.get("gates")
    require(isinstance(gates, list), "gates must be an array")
    ids = [item.get("id") if isinstance(item, dict) else None for item in gates]
    require(ids == EXPECTED_GATES, "gate array must preserve G0..G7 order")
    for gate in gates:
        require(gate.get("state") == TOKEN_VAZIO, f"source closure gate must remain TOKEN_VAZIO: {gate.get('id')}")
        requires = gate.get("requires")
        require(isinstance(requires, list) and requires and all(isinstance(x, str) and x for x in requires), f"{gate.get('id')} requires evidence rules")

    promotion = closure.get("promotion")
    require(isinstance(promotion, dict), "promotion block missing")
    require(promotion.get("gate") == "G7_CLAIM_PROMOTION", "unexpected promotion gate")
    require(promotion.get("state") == TOKEN_VAZIO, "promotion source state must remain TOKEN_VAZIO")
    require(promotion.get("human_authorization_required") is True, "human authorization must remain required")
    require(promotion.get("automatic_promotion_forbidden") is True, "automatic promotion must remain forbidden")


def validate_unresolved(closure: dict[str, Any]) -> None:
    unresolved = closure.get("unresolved")
    require(isinstance(unresolved, dict), "unresolved block missing")
    require(set(unresolved) == EXPECTED_UNRESOLVED, "unresolved key set drift")
    for key, value in unresolved.items():
        require(value == TOKEN_VAZIO, f"unresolved evidence cannot be source-promoted: {key}")


def validate_closure(closure: dict[str, Any]) -> None:
    require(closure.get("schema") == SCHEMA, "unexpected closure schema")
    require(closure.get("id") == "SGPT_SYNAPTIC_CLOSURE_V1", "unexpected closure id")
    require(closure.get("authority") == "rafaelmeloreisnovo/RafPolimata", "unexpected closure authority")
    require(closure.get("claim_allowed") is False, "claim_allowed must remain false")

    invariants = closure.get("truth_invariants")
    require(isinstance(invariants, list), "truth_invariants must be an array")
    for invariant in (
        "SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM",
        "TOKEN_VAZIO != 0",
        "IMPLEMENTED_UNTESTED != PASS",
    ):
        require(invariant in invariants, f"missing truth invariant: {invariant}")

    validate_authorities(closure)
    validate_hypotheses(closure)
    require(closure.get("mandatory_baselines") == EXPECTED_BASELINES, "mandatory baseline set/order drift")
    require(closure.get("adversarial_controls") == EXPECTED_ADVERSARIAL, "adversarial control set/order drift")
    validate_gates(closure)
    validate_unresolved(closure)
    require(isinstance(closure.get("rollback"), str) and closure["rollback"], "rollback rule missing")


def expect_rejected(base: dict[str, Any], mutate: Callable[[dict[str, Any]], None], label: str) -> None:
    candidate = copy.deepcopy(base)
    mutate(candidate)
    try:
        validate_closure(candidate)
    except ClosureError:
        return
    raise ClosureError(f"selftest mutation was not rejected: {label}")


def selftest(closure: dict[str, Any]) -> list[str]:
    rejected: list[str] = []

    cases: list[tuple[str, Callable[[dict[str, Any]], None]]] = [
        ("claim-promotion", lambda x: x.__setitem__("claim_allowed", True)),
        ("malformed-rll-pin", lambda x: x["authorities"][0].__setitem__("commit", "deadbeef")),
        ("missing-baseline", lambda x: x["mandatory_baselines"].pop()),
        ("missing-adversarial-control", lambda x: x["adversarial_controls"].pop()),
        ("hypothesis-to-theorem", lambda x: x["hypotheses"][0].__setitem__("class", "THEOREM")),
        ("claim-gate-source-pass", lambda x: x["gates"][-1].__setitem__("state", "PASS")),
        ("actions-to-authority", lambda x: x["authorities"][-1].__setitem__("role", "CLOSURE_AUTHORITY")),
        ("actions-authority-transfer", lambda x: x["authorities"][-1].__setitem__("authority_transfer", True)),
        ("unresolved-to-pass", lambda x: x["unresolved"].__setitem__("baseline_comparison", "PASS")),
        ("gate-reorder", lambda x: x["gate_order"].reverse()),
    ]

    for label, mutate in cases:
        expect_rejected(closure, mutate, label)
        rejected.append(label)
    return rejected


def write_receipt(path: Path, closure_path: Path, rejected: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    source_sha = os.environ.get("RAFCI_SOURCE_SHA") or os.environ.get("GITHUB_SHA") or TOKEN_VAZIO
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "source_commit": source_sha,
        "closure_path": str(closure_path.relative_to(ROOT)) if closure_path.is_relative_to(ROOT) else str(closure_path),
        "closure_sha256": sha256_file(closure_path),
        "status": "PASS_CONTRACT_ONLY",
        "structural_contract": "PASS",
        "selftest_rejected_mutations": rejected,
        "cross_repo_exact_pin_readback": TOKEN_VAZIO,
        "mathematical_execution_receipt": TOKEN_VAZIO,
        "sgpt_deterministic_execution_receipt": TOKEN_VAZIO,
        "adversarial_execution_receipt": TOKEN_VAZIO,
        "baseline_comparison": TOKEN_VAZIO,
        "independent_provider_reproduction": TOKEN_VAZIO,
        "held_out_prediction": TOKEN_VAZIO,
        "scientific_validation": TOKEN_VAZIO,
        "claim_allowed": False,
        "scope": "source-contract identity, gate topology and fail-closed mutation rejection only",
    }
    path.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--closure", type=Path, default=DEFAULT_CLOSURE)
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()

    try:
        closure_path = args.closure.resolve()
        closure = load_closure(closure_path)
        validate_closure(closure)
        rejected = selftest(closure) if args.selftest else []
        if args.receipt:
            write_receipt(args.receipt.resolve(), closure_path, rejected)
    except (ClosureError, OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"SGPT_SYNAPTIC_CLOSURE_FAIL: {exc}", file=sys.stderr)
        return 1

    print(
        "SGPT_SYNAPTIC_CLOSURE_PASS_CONTRACT_ONLY "
        f"authorities={len(closure['authorities'])} "
        f"hypotheses={len(closure['hypotheses'])} "
        f"baselines={len(closure['mandatory_baselines'])} "
        f"adversarial_controls={len(closure['adversarial_controls'])} "
        f"selftest_rejections={len(rejected)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
