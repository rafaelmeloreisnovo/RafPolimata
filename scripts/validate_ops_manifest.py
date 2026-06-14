#!/usr/bin/env python3
import argparse
from pathlib import Path

REQUIRED = {
    "arch", "brand", "lang", "opt", "feat", "flags", "src_len", "src_hash",
    "ir", "asm", "bin", "elapsed_ns", "rollback_code",
}


def parse_ops(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        key, sep, value = line.partition("=")
        if not sep:
            raise SystemExit(f"invalid line without '=': {line}")
        data[key] = value
    return data


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate raf_compile .ops manifest")
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--expect-rollback", type=int)
    args = parser.parse_args()
    data = parse_ops(args.manifest)
    missing = sorted(REQUIRED - data.keys())
    if missing:
        raise SystemExit(f"missing keys: {', '.join(missing)}")
    for key in ("arch", "lang", "opt", "src_len", "ir", "asm", "bin", "elapsed_ns", "rollback_code"):
        int(data[key], 10)
    int(data["feat"], 16)
    int(data["src_hash"], 16)
    if args.expect_rollback is not None and int(data["rollback_code"]) != args.expect_rollback:
        raise SystemExit(f"rollback_code={data['rollback_code']} expected={args.expect_rollback}")
    print(f"OPS PASS {args.manifest} rollback_code={data['rollback_code']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
