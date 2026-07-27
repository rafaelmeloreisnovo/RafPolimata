#!/usr/bin/env python3
"""Deterministic Bitraf loss-observation vector index.

This tool indexes *observations* of bit states and error surfaces. It does not
claim to read physical transistor state, repair hardware, or replace ECC/FEC.
Nearest-neighbour results are heuristic evidence only.

Stdlib-only by design for Termux/CI portability.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable

SCHEMA = "rafaelia.bitraf-loss-observation/v1"
INDEX_SCHEMA = "rafaelia.bitraf-loss-vector-index/v1"
RESULT_SCHEMA = "rafaelia.bitraf-loss-query-result/v1"
Q_HEX = math.sqrt(3.0) / 2.0
CLASSES = (
    "MATCH",
    "FLIP_0_TO_1",
    "FLIP_1_TO_0",
    "ERASURE",
    "TOKEN_VAZIO_EXPECTED",
    "TOKEN_VAZIO_OBSERVED",
)


class ObservationError(ValueError):
    """Invalid observation."""


def _bit_or_none(value: Any, name: str) -> int | None:
    if value is None:
        return None
    if value in (0, 1) and not isinstance(value, bool):
        return int(value)
    raise ObservationError(f"{name}:expected_0_1_or_null")


def _finite_or_none(value: Any, name: str) -> float | None:
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ObservationError(f"{name}:expected_number_or_null")
    value = float(value)
    if not math.isfinite(value):
        raise ObservationError(f"{name}:non_finite")
    return value


def _nonneg_int(value: Any, name: str, default: int = 0) -> int:
    if value is None:
        return default
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ObservationError(f"{name}:expected_nonnegative_integer")
    return value


def classify(expected: int | None, observed: int | None, known_erasure: bool) -> str:
    if expected is None:
        return "TOKEN_VAZIO_EXPECTED"
    if observed is None:
        return "ERASURE" if known_erasure else "TOKEN_VAZIO_OBSERVED"
    if expected == observed:
        return "MATCH"
    return "FLIP_0_TO_1" if expected == 0 else "FLIP_1_TO_0"


def _signed_unit(value: float) -> float:
    """Map any finite scalar to (-1, 1) without a fitted corpus scaler."""
    return value / (1.0 + abs(value))


def _presence_value(value: float | None) -> tuple[float, float]:
    return (0.0, 0.0) if value is None else (_signed_unit(value), 1.0)


def _nearest_fibonacci_index(n: int) -> int:
    """Index of the Fibonacci number nearest to non-negative n."""
    if n <= 1:
        return n
    a, b, idx = 0, 1, 1
    while b < n:
        a, b = b, a + b
        idx += 1
    if abs(n - a) <= abs(b - n):
        return max(0, idx - 1)
    return idx


def normalize_observation(raw: dict[str, Any]) -> dict[str, Any]:
    if raw.get("schema_version") != SCHEMA:
        raise ObservationError("schema_version")
    observation_id = raw.get("observation_id")
    matrix_id = raw.get("matrix_id")
    run_id = raw.get("run_id")
    if not isinstance(observation_id, str) or not observation_id:
        raise ObservationError("observation_id")
    if not isinstance(matrix_id, str) or not matrix_id:
        raise ObservationError("matrix_id")
    if not isinstance(run_id, str) or not run_id:
        raise ObservationError("run_id")

    coord = raw.get("coord")
    if not isinstance(coord, dict):
        raise ObservationError("coord")
    x = _nonneg_int(coord.get("x"), "coord.x")
    y = _nonneg_int(coord.get("y"), "coord.y")
    z = _nonneg_int(coord.get("z"), "coord.z")
    t = _nonneg_int(coord.get("t"), "coord.t")

    expected = _bit_or_none(raw.get("expected"), "expected")
    observed = _bit_or_none(raw.get("observed"), "observed")
    known_erasure = raw.get("known_erasure", False)
    if not isinstance(known_erasure, bool):
        raise ObservationError("known_erasure")

    syndrome = raw.get("syndrome", [])
    if not isinstance(syndrome, list) or any(
        v not in (0, 1) or isinstance(v, bool) for v in syndrome
    ):
        raise ObservationError("syndrome")
    parity_memberships = raw.get("parity_memberships", [])
    if (
        not isinstance(parity_memberships, list)
        or any(
            isinstance(v, bool) or not isinstance(v, int) or v < 0
            for v in parity_memberships
        )
    ):
        raise ObservationError("parity_memberships")

    source = raw.get("source", {})
    if not isinstance(source, dict):
        raise ObservationError("source")

    normalized = {
        "schema_version": SCHEMA,
        "observation_id": observation_id,
        "matrix_id": matrix_id,
        "run_id": run_id,
        "coord": {"x": x, "y": y, "z": z, "t": t},
        "linear_index": _nonneg_int(
            raw.get("linear_index"), "linear_index", default=0
        ),
        "expected": expected,
        "observed": observed,
        "known_erasure": known_erasure,
        "temperature_c": _finite_or_none(
            raw.get("temperature_c"), "temperature_c"
        ),
        "voltage_v": _finite_or_none(raw.get("voltage_v"), "voltage_v"),
        "latency_ns": _finite_or_none(raw.get("latency_ns"), "latency_ns"),
        "shard": _nonneg_int(raw.get("shard"), "shard", default=0),
        "stripe": _nonneg_int(raw.get("stripe"), "stripe", default=0),
        "syndrome": [int(v) for v in syndrome],
        "parity_memberships": parity_memberships,
        "source": source,
        "notes": raw.get("notes") if isinstance(raw.get("notes"), str) else "",
    }
    normalized["class"] = classify(expected, observed, known_erasure)
    return normalized


def vectorize(obs: dict[str, Any]) -> list[float]:
    c = obs["coord"]
    linear = obs["linear_index"]
    fib_idx = _nearest_fibonacci_index(linear)

    # Two explicit planar projections: hexagonal (60°) and octagonal (45°).
    theta_hex = linear * math.pi / 3.0
    theta_oct = linear * math.pi / 4.0
    radius = Q_HEX ** min(fib_idx, 256)

    temp, temp_present = _presence_value(obs["temperature_c"])
    voltage, voltage_present = _presence_value(obs["voltage_v"])
    latency, latency_present = _presence_value(obs["latency_ns"])

    expected = obs["expected"]
    observed = obs["observed"]
    expected_value = 0.0 if expected is None else (-1.0 if expected == 0 else 1.0)
    observed_value = 0.0 if observed is None else (-1.0 if observed == 0 else 1.0)
    expected_present = 0.0 if expected is None else 1.0
    observed_present = 0.0 if observed is None else 1.0

    syndrome = obs["syndrome"]
    syndrome_density = (sum(syndrome) / len(syndrome)) if syndrome else 0.0
    syndrome_present = 1.0 if syndrome else 0.0
    parity_count = len(obs["parity_memberships"])

    one_hot = [1.0 if obs["class"] == name else 0.0 for name in CLASSES]

    return [
        _signed_unit(float(c["x"])),
        _signed_unit(float(c["y"])),
        _signed_unit(float(c["z"])),
        _signed_unit(float(c["t"])),
        _signed_unit(float(linear)),
        _signed_unit(float(fib_idx)),
        radius * math.cos(theta_hex),
        radius * math.sin(theta_hex),
        radius * math.cos(theta_oct),
        radius * math.sin(theta_oct),
        expected_value,
        expected_present,
        observed_value,
        observed_present,
        1.0 if obs["known_erasure"] else 0.0,
        temp,
        temp_present,
        voltage,
        voltage_present,
        latency,
        latency_present,
        _signed_unit(float(obs["shard"])),
        _signed_unit(float(obs["stripe"])),
        syndrome_density,
        syndrome_present,
        _signed_unit(float(parity_count)),
        *one_hot,
    ]


def _canonical_hash(payload: Any) -> str:
    data = json.dumps(
        payload,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha3_256(data).hexdigest()


def build_index(observations: Iterable[dict[str, Any]]) -> dict[str, Any]:
    records = []
    seen = set()
    for raw in observations:
        obs = normalize_observation(raw)
        if obs["observation_id"] in seen:
            raise ObservationError(
                f"duplicate_observation_id:{obs['observation_id']}"
            )
        seen.add(obs["observation_id"])
        vector = vectorize(obs)
        records.append(
            {
                "observation": obs,
                "vector": [round(v, 12) for v in vector],
                "record_sha3_256": _canonical_hash(obs),
            }
        )
    records.sort(key=lambda r: r["observation"]["observation_id"])
    vector_dim = len(records[0]["vector"]) if records else 0
    return {
        "schema": INDEX_SCHEMA,
        "vector_semantics": "external_auditable_observation_features",
        "vector_dim": vector_dim,
        "records": records,
        "index_sha3_256": _canonical_hash(
            [
                {
                    "id": r["observation"]["observation_id"],
                    "hash": r["record_sha3_256"],
                }
                for r in records
            ]
        ),
        "claim_allowed": False,
    }


def cosine_similarity(a: list[float], b: list[float]) -> float:
    if len(a) != len(b):
        raise ObservationError("vector_dimension_mismatch")
    dot = sum(x * y for x, y in zip(a, b))
    na = math.sqrt(sum(x * x for x in a))
    nb = math.sqrt(sum(y * y for y in b))
    if na == 0.0 or nb == 0.0:
        return 0.0
    return dot / (na * nb)


def query_index(
    index: dict[str, Any], raw_query: dict[str, Any], top_k: int = 5
) -> dict[str, Any]:
    if index.get("schema") != INDEX_SCHEMA:
        raise ObservationError("index_schema")
    query = normalize_observation(raw_query)
    qv = vectorize(query)
    ranked = []
    for record in index.get("records", []):
        obs = record["observation"]
        similarity = cosine_similarity(qv, record["vector"])
        ranked.append(
            {
                "observation_id": obs["observation_id"],
                "matrix_id": obs["matrix_id"],
                "run_id": obs["run_id"],
                "class": obs["class"],
                "expected": obs["expected"],
                "observed": obs["observed"],
                "coord": obs["coord"],
                "similarity": round(similarity, 9),
                "record_sha3_256": record["record_sha3_256"],
            }
        )
    ranked.sort(key=lambda item: (-item["similarity"], item["observation_id"]))
    neighbours = ranked[: max(1, top_k)]

    votes: defaultdict[int, float] = defaultdict(float)
    for item in neighbours:
        value = item["expected"]
        if value in (0, 1):
            votes[int(value)] += max(0.0, item["similarity"])
    total = sum(votes.values())
    candidate = None
    confidence = 0.0
    if total > 0:
        candidate = max(votes, key=lambda v: (votes[v], -v))
        confidence = votes[candidate] / total

    return {
        "schema": RESULT_SCHEMA,
        "query_id": query["observation_id"],
        "neighbours": neighbours,
        "heuristic_candidate_expected_bit": candidate,
        "heuristic_confidence": round(confidence, 6),
        "recovery_status": "HEURISTIC_ONLY_NOT_ECC",
        "claim_allowed": False,
        "TOKEN_VAZIO": (
            [] if candidate is not None else ["no_historical_expected_bit_support"]
        ),
    }


def audit_index(index: dict[str, Any]) -> dict[str, Any]:
    records = index.get("records", [])
    classes = Counter(r["observation"]["class"] for r in records)
    total = len(records)
    observed_comparable = sum(
        classes[c] for c in ("MATCH", "FLIP_0_TO_1", "FLIP_1_TO_0")
    )
    flips = classes["FLIP_0_TO_1"] + classes["FLIP_1_TO_0"]
    erasures = classes["ERASURE"]
    return {
        "schema": "rafaelia.bitraf-loss-audit/v1",
        "record_count": total,
        "classes": dict(sorted(classes.items())),
        "bit_flip_rate_on_comparable": (
            flips / observed_comparable if observed_comparable else None
        ),
        "erasure_rate_on_all_records": erasures / total if total else None,
        "TOKEN_VAZIO": ["physical_cause"],
        "claim_allowed": False,
    }


def _read_jsonl(path: Path) -> list[dict[str, Any]]:
    rows = []
    for line_no, line in enumerate(
        path.read_text(encoding="utf-8").splitlines(), 1
    ):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        try:
            obj = json.loads(line)
        except json.JSONDecodeError as exc:
            raise ObservationError(f"{path}:{line_no}:{exc.msg}") from exc
        if not isinstance(obj, dict):
            raise ObservationError(f"{path}:{line_no}:expected_object")
        rows.append(obj)
    return rows


def _write_json(path: Path | None, payload: Any) -> None:
    text = json.dumps(
        payload,
        ensure_ascii=False,
        sort_keys=True,
        indent=2,
    ) + "\n"
    if path:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    print(text, end="")


def main() -> int:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)

    p_build = sub.add_parser("build")
    p_build.add_argument("observations", type=Path)
    p_build.add_argument("--out", type=Path)

    p_query = sub.add_parser("query")
    p_query.add_argument("index", type=Path)
    p_query.add_argument("observation", type=Path)
    p_query.add_argument("--top-k", type=int, default=5)
    p_query.add_argument("--out", type=Path)

    p_audit = sub.add_parser("audit")
    p_audit.add_argument("index", type=Path)
    p_audit.add_argument("--out", type=Path)

    args = parser.parse_args()
    try:
        if args.command == "build":
            result = build_index(_read_jsonl(args.observations))
        elif args.command == "query":
            index = json.loads(args.index.read_text(encoding="utf-8"))
            query = json.loads(args.observation.read_text(encoding="utf-8"))
            result = query_index(index, query, args.top_k)
        else:
            index = json.loads(args.index.read_text(encoding="utf-8"))
            result = audit_index(index)
        _write_json(args.out, result)
        return 0
    except (OSError, json.JSONDecodeError, ObservationError) as exc:
        _write_json(
            getattr(args, "out", None),
            {
                "status": "FAIL",
                "reason": str(exc),
                "claim_allowed": False,
            },
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
