#!/usr/bin/env python3
"""Validate the RAFAELIA formal-science orchestrator without PyYAML.

The file format intentionally uses one-line registry records so a stdlib-only
validator can enforce the scientific contract in CI.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CONFIG = ROOT / "configs" / "formal_science_orchestrator.yml"
DEFAULT_OUTPUT = ROOT / "results" / "formal_science_orchestrator_report.json"

REQUIRED_DOMAINS = {
    "dimensional_analysis",
    "classical_mechanics",
    "electrical_machines",
    "control_and_ai",
    "thermodynamics_combustion",
    "electrochemistry",
    "hall_thruster",
    "quantum_mechanics",
    "dynamical_systems",
    "fluid_dynamics",
    "graph_number_semantics",
    "statistics_and_time",
}

REQUIRED_GATES = {
    "G-DIM", "G-DOM", "G-NUM", "G-TIME", "G-DATA", "G-UNC",
    "G-STAT", "G-PHYS", "G-SAFE", "G-SEM", "G-REP", "G-CLAIM",
}

EXPECTED_DIMENSIONS = {
    "E01": ("M1L2T-3", "M1L2T-3"),
    "E02": ("M1L1T-2", "M1L1T-2"),
    "E03": ("M1L2T-3", "M1L2T-3"),
    "E06": ("M1L2T-2", "M1L2T-2"),
    "E11": ("N1", "N1"),
    "E12": ("D0", "D0"),
    "E15": ("M1L2T-3", "M1L2T-3"),
    "E18": ("M1L1T-2", "M1L1T-2"),
    "E19": ("D0", "D0"),
    "E24": ("M1L-2T-2", "M1L-2T-2"),
    "E25": ("D0", "D0"),
    "E26": ("D0", "D0"),
    "E30": ("D0", "D0"),
    "E31": ("T1", "T1"),
    "E32": ("T1", "T1"),
}

RECORD_RE = re.compile(
    r"^\s*- id:\s*(E\d{2});\s*domain:\s*([^;]+);\s*symbol:\s*([^;]+);"
    r"\s*expression:\s*([^;]+);\s*lhs_dim:\s*([^;]+);\s*rhs_dim:\s*([^;]+);"
    r"\s*claim_state:\s*([^;]+);\s*falsifier:\s*([^;]+)\s*$",
    re.MULTILINE,
)


def scalar(text: str, key: str) -> str:
    match = re.search(rf"^\s*{re.escape(key)}:\s*([^#\n]+)", text, re.MULTILINE)
    if not match:
        raise SystemExit(f"missing_scalar:{key}")
    return match.group(1).strip()


def section_items(text: str, section: str) -> list[str]:
    lines = text.splitlines()
    start = next((i + 1 for i, line in enumerate(lines) if line == f"{section}:"), None)
    if start is None:
        raise SystemExit(f"missing_section:{section}")
    result: list[str] = []
    for line in lines[start:]:
        if line and not line.startswith((" ", "\t")):
            break
        stripped = line.strip()
        if stripped.startswith("- "):
            result.append(stripped[2:].strip())
    return result


def validate(config: Path) -> dict[str, object]:
    if not config.exists():
        raise SystemExit(f"config_not_found:{config}")
    text = config.read_text(encoding="utf-8")

    equation_count = int(scalar(text, "equation_count"))
    domain_count = int(scalar(text, "domain_count"))
    gate_count = int(scalar(text, "proof_gate_count"))
    direct_promotion = scalar(text, "semantic_to_physical_direct_promotion")
    time_provenance = scalar(text, "time_provenance")

    states = set(section_items(text, "allowed_claim_states"))
    domains = set(section_items(text, "domain_registry"))
    gates = set(section_items(text, "proof_gates"))

    required_states = {
        "METAPHOR", "FORMALIZED", "METHOD_DEFINED", "SIMULATED",
        "EVIDENCE_LINKED", "REPLICATED", "CLAIM_ALLOWED", "TOKEN_VAZIO",
        "CONTRADICTION", "DECLARED_BY_AUTHOR",
    }
    if states != required_states:
        raise SystemExit(f"invalid_claim_states:{sorted(states)}")
    if domains != REQUIRED_DOMAINS or len(domains) != domain_count:
        raise SystemExit(f"invalid_domains:{sorted(domains)}")
    if gates != REQUIRED_GATES or len(gates) != gate_count:
        raise SystemExit(f"invalid_proof_gates:{sorted(gates)}")
    if direct_promotion != "forbidden":
        raise SystemExit("semantic_physical_promotion_must_be_forbidden")
    if time_provenance != "utc_ntp_plus_monotonic":
        raise SystemExit("invalid_time_provenance")

    records = [dict(zip(
        ["id", "domain", "symbol", "expression", "lhs_dim", "rhs_dim", "claim_state", "falsifier"],
        groups,
    )) for groups in RECORD_RE.findall(text)]

    expected_ids = [f"E{i:02d}" for i in range(1, equation_count + 1)]
    ids = [record["id"].strip() for record in records]
    if ids != expected_ids:
        raise SystemExit(f"invalid_equation_ids:expected={expected_ids},got={ids}")

    for record in records:
        for key, value in record.items():
            record[key] = value.strip()
        if record["domain"] not in domains:
            raise SystemExit(f"unknown_domain:{record['id']}:{record['domain']}")
        if record["claim_state"] not in {"ESTABLISHED", "METHOD_DEFINED"}:
            raise SystemExit(f"invalid_equation_claim_state:{record['id']}")
        if not record["falsifier"]:
            raise SystemExit(f"missing_falsifier:{record['id']}")
        if record["lhs_dim"] != record["rhs_dim"]:
            raise SystemExit(
                f"dimension_mismatch:{record['id']}:{record['lhs_dim']}!={record['rhs_dim']}"
            )

    for equation_id, expected in EXPECTED_DIMENSIONS.items():
        record = next(item for item in records if item["id"] == equation_id)
        actual = (record["lhs_dim"], record["rhs_dim"])
        if actual != expected:
            raise SystemExit(f"canonical_dimension_changed:{equation_id}:{actual}!={expected}")

    required_phrases = {
        "forbidden_edge: SEMANTICALLY_RELATED=>CAUSES_PHYSICAL_EFFECT",
        "forbidden_claim: prime_number_is_physical_mechanism_without_adapter",
        "duration_clock: MONOTONIC",
        "wall_clock: UTC_NTP",
        "secure_sync: NTS_WHEN_AVAILABLE",
    }
    missing = sorted(phrase for phrase in required_phrases if phrase not in text)
    if missing:
        raise SystemExit("missing_safety_contract:" + ",".join(missing))

    by_domain: dict[str, int] = {domain: 0 for domain in sorted(domains)}
    for record in records:
        by_domain[record["domain"]] += 1

    empty_domains = [domain for domain, count in by_domain.items() if count == 0]
    if empty_domains:
        raise SystemExit("domains_without_equations:" + ",".join(empty_domains))

    return {
        "protocol": scalar(text, "id"),
        "version": int(scalar(text, "version")),
        "equation_count": len(records),
        "domain_count": len(domains),
        "proof_gate_count": len(gates),
        "dimensional_checks": len(EXPECTED_DIMENSIONS),
        "equations_by_domain": by_domain,
        "semantic_physical_direct_promotion": direct_promotion,
        "time_provenance": time_provenance,
        "verdict": "PASS",
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    config = args.config if args.config.is_absolute() else ROOT / args.config
    output = args.output if args.output.is_absolute() else ROOT / args.output
    result = validate(config)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))


if __name__ == "__main__":
    main()
