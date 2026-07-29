#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from voynich_repro.core import write_report


def main() -> int:
    parser = argparse.ArgumentParser(description="Run deterministic synthetic Voynich experiment")
    parser.add_argument("--seed", type=int, default=144000)
    parser.add_argument("--steps", type=int, default=128)
    parser.add_argument("--alpha", type=float, default=0.25)
    parser.add_argument("--output", type=Path, default=Path("reports/synthetic_report.json"))
    args = parser.parse_args()
    print(write_report(args.output, args.seed, args.steps, args.alpha))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
