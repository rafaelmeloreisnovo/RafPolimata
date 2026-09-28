#!/usr/bin/env python3
"""Deterministic exact-integer reference for Fibonacci, Tribonacci and Trinity633.

The recurrence matrices and Trinity633 are co-published for cross-language
equivalence. They are not fused into one mathematical object.
"""
from __future__ import annotations

import argparse
import json
from collections.abc import Sequence


Matrix = tuple[tuple[int, ...], ...]
Vector = tuple[int, ...]


FIBONACCI_COMPANION: Matrix = ((1, 1), (1, 0))
TRIBONACCI_COMPANION: Matrix = ((1, 1, 1), (1, 0, 0), (0, 1, 0))


def _square(matrix: Sequence[Sequence[int]]) -> Matrix:
    rows = tuple(tuple(int(x) for x in row) for row in matrix)
    if not rows or any(len(row) != len(rows) for row in rows):
        raise ValueError("matrix must be non-empty and square")
    return rows


def identity(size: int) -> Matrix:
    if size < 1:
        raise ValueError("size must be >= 1")
    return tuple(tuple(1 if r == c else 0 for c in range(size)) for r in range(size))


def matrix_mul(left: Sequence[Sequence[int]], right: Sequence[Sequence[int]]) -> Matrix:
    a = _square(left)
    b = _square(right)
    if len(a) != len(b):
        raise ValueError("matrix sizes differ")
    n = len(a)
    return tuple(
        tuple(sum(a[r][k] * b[k][c] for k in range(n)) for c in range(n))
        for r in range(n)
    )


def matrix_pow(matrix: Sequence[Sequence[int]], exponent: int) -> Matrix:
    base = _square(matrix)
    n = int(exponent)
    if n < 0:
        raise ValueError("exponent must be >= 0 for exact integer recurrence")
    result = identity(len(base))
    while n:
        if n & 1:
            result = matrix_mul(result, base)
        base = matrix_mul(base, base)
        n >>= 1
    return result


def matrix_vector_mul(matrix: Sequence[Sequence[int]], vector: Sequence[int]) -> Vector:
    m = _square(matrix)
    v = tuple(int(x) for x in vector)
    if len(m) != len(v):
        raise ValueError("matrix/vector sizes differ")
    return tuple(sum(row[c] * v[c] for c in range(len(v))) for row in m)


def fibonacci_matrix(n: int) -> int:
    """Return F_n for F_0=0, F_1=1 using Q^n * [F1,F0]."""
    idx = int(n)
    if idx < 0:
        raise ValueError("n must be >= 0")
    state = matrix_vector_mul(matrix_pow(FIBONACCI_COMPANION, idx), (1, 0))
    return state[1]


def tribonacci_matrix(n: int) -> int:
    """Return T_n for T_0=0,T_1=0,T_2=1 using C^n * [T2,T1,T0]."""
    idx = int(n)
    if idx < 0:
        raise ValueError("n must be >= 0")
    state = matrix_vector_mul(matrix_pow(TRIBONACCI_COMPANION, idx), (1, 0, 0))
    return state[2]


def trinity633(amor: int, luz: int, consciencia: int) -> int:
    """Exact scalar Trinity633 = Amor^6 * Luz^3 * Consciencia^3."""
    a, l, c = int(amor), int(luz), int(consciencia)
    return (a ** 6) * (l ** 3) * (c ** 3)


def reference_packet(n: int, amor: int, luz: int, consciencia: int) -> dict[str, object]:
    """Typed packet: same evidence bundle, deliberately separate semantics."""
    return {
        "schema": "RAFAELIA_RECURRENCE_MATRIX_REFERENCE_V1",
        "claim_allowed": False,
        "relation": "CO_PUBLISHED_NOT_FUSED",
        "index": int(n),
        "fibonacci": {
            "definition": "F0=0,F1=1; F(n+2)=F(n+1)+F(n)",
            "matrix": [list(row) for row in FIBONACCI_COMPANION],
            "value": fibonacci_matrix(n),
        },
        "tribonacci": {
            "definition": "T0=0,T1=0,T2=1; T(n+3)=T(n+2)+T(n+1)+T(n)",
            "matrix": [list(row) for row in TRIBONACCI_COMPANION],
            "value": tribonacci_matrix(n),
        },
        "trinity633": {
            "definition": "Amor^6 * Luz^3 * Consciencia^3",
            "inputs": {"amor": int(amor), "luz": int(luz), "consciencia": int(consciencia)},
            "value": trinity633(amor, luz, consciencia),
            "matrix_semantics": "NOT_APPLICABLE",
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int, default=10)
    parser.add_argument("--amor", type=int, default=2)
    parser.add_argument("--luz", type=int, default=3)
    parser.add_argument("--consciencia", type=int, default=5)
    args = parser.parse_args()
    print(json.dumps(reference_packet(args.n, args.amor, args.luz, args.consciencia),
                     ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
