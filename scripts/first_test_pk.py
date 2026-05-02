#!/usr/bin/env python3
"""Teste mínimo de falsificabilidade: comparar P(k) observado vs modelo cru."""
from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path
from typing import List, Tuple


def load_csv(path: Path) -> Tuple[List[float], List[float], List[float]]:
    k, pk, sigma = [], [], []
    with path.open("r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        required = {"k", "Pk", "sigma"}
        if not required.issubset(set(reader.fieldnames or [])):
            raise ValueError(f"CSV deve conter colunas {required}, recebeu {reader.fieldnames}")
        for row in reader:
            k.append(float(row["k"]))
            pk.append(float(row["Pk"]))
            sigma.append(float(row["sigma"]))
    return k, pk, sigma


def model_pk(k: float, a: float, n: float, kd: float) -> float:
    # Modelo cru congelado (sem ajuste durante execução)
    return a * (k ** n) * math.exp(-k / kd)


def linear_slope(xs: List[float], ys: List[float]) -> float:
    n = len(xs)
    x_mean = sum(xs) / n
    y_mean = sum(ys) / n
    num = sum((x - x_mean) * (y - y_mean) for x, y in zip(xs, ys))
    den = sum((x - x_mean) ** 2 for x in xs)
    return 0.0 if den == 0 else num / den


def run(args: argparse.Namespace) -> dict:
    k, pk_obs, sigma = load_csv(Path(args.data))

    mask = [i for i, kv in enumerate(k) if args.kmin <= kv <= args.kmax]
    if not mask:
        raise ValueError("Sem pontos na janela de k selecionada")

    ks = [k[i] for i in mask]
    obs = [pk_obs[i] for i in mask]
    sig = [sigma[i] for i in mask]
    pred = [model_pk(ki, args.a, args.n, args.kd) for ki in ks]

    residuals = [(p - o) / s for p, o, s in zip(pred, obs, sig)]
    rrmse = math.sqrt(sum(r * r for r in residuals) / len(residuals))
    coverage = sum(1 for r in residuals if abs(r) <= 1.0) / len(residuals)
    slope = linear_slope(ks, residuals)

    passed = (rrmse <= args.rrmse_max and coverage >= args.coverage_min and abs(slope) <= args.slope_abs_max)

    report = {
        "input": {
            "data": args.data,
            "k_range": [args.kmin, args.kmax],
            "model_params": {"a": args.a, "n": args.n, "kd": args.kd},
            "criteria": {
                "rrmse_max": args.rrmse_max,
                "coverage_min": args.coverage_min,
                "slope_abs_max": args.slope_abs_max,
            },
        },
        "metrics": {"rrmse": rrmse, "coverage_1sigma": coverage, "residual_slope": slope},
        "verdict": "PASS" if passed else "FAIL",
    }
    return report


def main() -> None:
    parser = argparse.ArgumentParser(description="Primeiro teste de falsificabilidade em P(k)")
    parser.add_argument("--data", default="data/pk_observado.csv")
    parser.add_argument("--output", default="results/first_test_report.json")
    parser.add_argument("--kmin", type=float, default=0.02)
    parser.add_argument("--kmax", type=float, default=0.20)
    parser.add_argument("--a", type=float, default=2.0e4)
    parser.add_argument("--n", type=float, default=1.0)
    parser.add_argument("--kd", type=float, default=0.18)
    parser.add_argument("--rrmse_max", type=float, default=1.5)
    parser.add_argument("--coverage_min", type=float, default=0.60)
    parser.add_argument("--slope_abs_max", type=float, default=5.0)

    args = parser.parse_args()
    report = run(args)

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
