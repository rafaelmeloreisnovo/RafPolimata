#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import math
import sys
import unicodedata
from pathlib import Path
from typing import Iterable, Sequence

ROOT = Path(__file__).resolve().parents[1]

SCHEMA = "raf.language-matrix-state.v1"
GATES = (
    "definition",
    "contract",
    "implementation",
    "local_test",
    "hashed_fixture",
    "validated_tokenizer",
    "out_of_sample",
    "independent_replication",
    "multi_repository",
    "domain_audit",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def is_tenth(value: float) -> bool:
    return 0.0 <= value <= 1.0 and math.isclose(
        value * 10.0, round(value * 10.0), abs_tol=1e-9
    )


def required_gates(score: float) -> list[str]:
    require(is_tenth(score), f"score must be in 0.1 steps: {score}")
    count = int(round(score * 10.0))
    return list(GATES[:count])


def validate_axis(axis: dict) -> None:
    identifier = axis["id"]
    score = axis["score"]
    evidence = axis["evidence"]
    status = axis["status"]
    token_vazio = axis["token_vazio"]

    require(isinstance(evidence, list), f"{identifier}: evidence must be list")
    require(bool(axis["next_gate"]), f"{identifier}: next_gate required")

    if score is None:
        require(status == "TOKEN_VAZIO", f"{identifier}: null score requires TOKEN_VAZIO")
        require(
            isinstance(token_vazio, str) and token_vazio.startswith("TOKEN_VAZIO"),
            f"{identifier}: TOKEN_VAZIO reason required",
        )
        return

    require(isinstance(score, (int, float)), f"{identifier}: score must be numeric or null")
    score = float(score)
    require(is_tenth(score), f"{identifier}: score must be a tenth between 0 and 1")
    require(token_vazio is None, f"{identifier}: scored axis cannot carry TOKEN_VAZIO")

    if math.isclose(score, 0.0):
        require(status == "MEASURED_ZERO", f"{identifier}: 0.0 is reserved for measured zero")
        require("measurement" in evidence, f"{identifier}: measured zero requires measurement")
        return

    required = required_gates(score)
    missing = [gate for gate in required if gate not in evidence]
    require(not missing, f"{identifier}: missing gates for score {score}: {missing}")


def validate_state(state: dict) -> None:
    require(state["schema"] == SCHEMA, "wrong schema")
    require(state["claim_allowed"] is False, "claim_allowed must remain false")
    require(
        state["score_semantics"] == "engineering_support_not_accuracy",
        "score must not be represented as accuracy",
    )
    axes = state["axes"]
    require(isinstance(axes, list) and axes, "axes must be a non-empty list")
    identifiers: set[str] = set()
    for axis in axes:
        identifier = axis["id"]
        require(identifier not in identifiers, f"duplicate axis: {identifier}")
        identifiers.add(identifier)
        validate_axis(axis)


def promote(score: float | None, evidence: Iterable[str]) -> float:
    evidence_set = set(evidence)
    if score is None:
        require("definition" in evidence_set, "TOKEN_VAZIO can start at 0.1 only with definition")
        return 0.1

    require(is_tenth(float(score)), "current score is invalid")
    require(score < 1.0, "score is already at maximum")
    next_score = round(float(score) + 0.1, 1)
    next_gate = GATES[int(round(float(score) * 10.0))]
    require(next_gate in evidence_set, f"promotion requires gate: {next_gate}")
    return next_score


def normalize_text(text: str, form: str = "NFC") -> str:
    require(form in {"NFC", "NFD", "NFKC", "NFKD"}, f"unsupported normalization: {form}")
    return unicodedata.normalize(form, text)


def tokenize_codepoints(text: str, normalization: str = "NFC") -> list[str]:
    normalized = normalize_text(text, normalization)
    return [char for char in normalized if not char.isspace()]


def tokenize_whitespace(text: str, normalization: str = "NFC") -> list[str]:
    return normalize_text(text, normalization).split()


def validate_square(matrix: Sequence[Sequence[float]]) -> int:
    n = len(matrix)
    require(all(len(row) == n for row in matrix), "matrix must be square")
    for row in matrix:
        for value in row:
            require(
                isinstance(value, (int, float)) and math.isfinite(value),
                "matrix values must be finite numbers",
            )
    return n


def direct_matrix(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    validate_square(matrix)
    return [[float(value) for value in row] for row in matrix]


def inverse_relational(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    n = validate_square(matrix)
    return [[float(matrix[j][i]) for j in range(n)] for i in range(n)]


def multiply_matrices(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> list[list[float]]:
    n = validate_square(left)
    require(validate_square(right) == n, "matrix sizes must match")
    return [
        [
            sum(float(left[i][k]) * float(right[k][j]) for k in range(n))
            for j in range(n)
        ]
        for i in range(n)
    ]


def indirect_two_step(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    return multiply_matrices(matrix, matrix)


def reciprocal_matrix(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    n = validate_square(matrix)
    for row in matrix:
        require(all(value >= 0 for value in row), "reciprocal matrix requires nonnegative weights")
    return [
        [min(float(matrix[i][j]), float(matrix[j][i])) for j in range(n)]
        for i in range(n)
    ]


def logarithmic_matrix(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    validate_square(matrix)
    output: list[list[float]] = []
    for row in matrix:
        require(all(value >= 0 for value in row), "logarithmic matrix requires nonnegative weights")
        output.append([math.log1p(float(value)) for value in row])
    return output


def inverse_logarithmic_matrix(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    validate_square(matrix)
    return [[math.expm1(float(value)) for value in row] for row in matrix]


def fibonacci_numbers(limit: int) -> list[int]:
    require(isinstance(limit, int) and limit >= 0, "limit must be a nonnegative integer")
    if limit == 0:
        return [0]
    values = [0, 1]
    while values[-1] < limit:
        values.append(values[-1] + values[-2])
    return values


def fibonacci_windows(length: int) -> list[int]:
    require(isinstance(length, int) and length >= 0, "length must be nonnegative")
    return sorted({value for value in fibonacci_numbers(length) if 0 < value <= length})


def fibonacci_inverse_index(value: int) -> int | None:
    require(isinstance(value, int) and value >= 0, "value must be nonnegative")
    sequence = fibonacci_numbers(value)
    for index, item in enumerate(sequence):
        if item == value:
            return index
    return None


def dyadic_partition(length: int) -> list[list[tuple[int, int]]]:
    require(isinstance(length, int) and length >= 0, "length must be nonnegative")
    if length == 0:
        return [[]]
    levels: list[list[tuple[int, int]]] = [[(0, length)]]
    while any(end - start > 1 for start, end in levels[-1]):
        next_level: list[tuple[int, int]] = []
        for start, end in levels[-1]:
            if end - start <= 1:
                next_level.append((start, end))
                continue
            middle = start + (end - start) // 2
            next_level.extend(((start, middle), (middle, end)))
        levels.append(next_level)
    return levels


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate RafPolimata language matrix state")
    parser.add_argument(
        "--state",
        type=Path,
        default=ROOT / "data/language/language-matrix-state.v1.json",
    )
    args = parser.parse_args(argv)
    state = load_json(args.state)
    validate_state(state)
    scored = [axis["score"] for axis in state["axes"] if axis["score"] is not None]
    empty = [axis["id"] for axis in state["axes"] if axis["score"] is None]
    print(
        json.dumps(
            {
                "schema": SCHEMA,
                "state": "PASS",
                "max_sustained_score": max(scored) if scored else None,
                "token_vazio_axes": empty,
                "claim_allowed": False,
            },
            ensure_ascii=False,
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, KeyError, TypeError, ValueError, OSError, json.JSONDecodeError) as error:
        print(f"FAIL language-matrix: {error}", file=sys.stderr)
        raise SystemExit(1)
