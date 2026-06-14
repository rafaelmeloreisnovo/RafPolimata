#!/usr/bin/env python3
import argparse
from pathlib import Path

STABLE_KEYS = ("arch", "lang", "opt", "feat", "flags", "src_len", "src_hash", "ir", "asm", "bin", "rollback_code")


def parse_ops(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        key, sep, value = line.partition("=")
        if sep:
            data[key] = value
    return data


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare deterministic .ops fields")
    parser.add_argument("left", type=Path)
    parser.add_argument("right", type=Path)
    args = parser.parse_args()
    left = parse_ops(args.left)
    right = parse_ops(args.right)
    diffs = [key for key in STABLE_KEYS if left.get(key) != right.get(key)]
    if diffs:
        for key in diffs:
            print(f"OPS DIFF {key}: {left.get(key)!r} != {right.get(key)!r}")
        return 1
    print(f"OPS SAME {args.left} {args.right}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
