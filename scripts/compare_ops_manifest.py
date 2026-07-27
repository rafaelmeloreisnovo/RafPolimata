#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

COMMON_STABLE_KEYS = (
    "ops_schema", "arch", "brand", "cores", "lang", "opt", "feat", "flags",
    "src_len", "src_hash", "omega_entropy_milli", "omega_phi_q16",
    "omega_attractor", "omega_flags", "omega_path", "omega_path_name",
    "ir", "asm", "bin", "native_requested", "native_written",
    "rollback_code", "ops_signature",
)
SCHEMA4_STABLE_KEYS = ("ir_value", "emitter_schema", "transaction_state")


def parse_ops(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line or line.startswith("#"):
            continue
        key, sep, value = line.partition("=")
        if not sep:
            raise SystemExit(f"{path}:{line_number}: line without '='")
        if key in data:
            raise SystemExit(f"{path}:{line_number}: duplicate key {key}")
        data[key] = value
    return data


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare deterministic .ops fields")
    parser.add_argument("left", type=Path)
    parser.add_argument("right", type=Path)
    args = parser.parse_args()
    left = parse_ops(args.left)
    right = parse_ops(args.right)

    if left.get("ops_schema") != right.get("ops_schema"):
        print(f"OPS DIFF ops_schema: {left.get('ops_schema')!r} != {right.get('ops_schema')!r}")
        return 1
    schema = int(left["ops_schema"])
    keys = COMMON_STABLE_KEYS + (SCHEMA4_STABLE_KEYS if schema >= 4 else ())
    diffs = [key for key in keys if left.get(key) != right.get(key)]
    if diffs:
        for key in diffs:
            print(f"OPS DIFF {key}: {left.get(key)!r} != {right.get(key)!r}")
        return 1
    print(f"OPS SAME {args.left} {args.right} schema={schema}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
