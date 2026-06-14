#!/usr/bin/env python3
import argparse
from pathlib import Path

REQUIRED = {
    "ops_schema", "arch", "brand", "lang", "opt", "feat", "flags", "src_len",
    "src_hash", "ir", "asm", "bin", "elapsed_ns", "rollback_code",
    "ops_signature",
}

FNV_OFFSET = 1469598103934665603
FNV_PRIME = 1099511628211


def fnv_update(h: int, data: bytes) -> int:
    for byte in data:
        h ^= byte
        h = (h * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return h


def fnv_u64(h: int, value: int) -> int:
    return fnv_update(h, int(value & 0xFFFFFFFFFFFFFFFF).to_bytes(8, "little"))


def expected_signature(data: dict[str, str]) -> int:
    h = FNV_OFFSET
    h = fnv_u64(h, int(data["arch"]))
    h = fnv_update(h, data["brand"].encode("utf-8"))
    h = fnv_u64(h, int(data["lang"]))
    h = fnv_u64(h, int(data["opt"]))
    h = fnv_u64(h, int(data["feat"], 16))
    h = fnv_update(h, data["flags"].encode("utf-8"))
    h = fnv_u64(h, int(data["src_len"]))
    h = fnv_u64(h, int(data["src_hash"], 16))
    h = fnv_u64(h, int(data["ir"]))
    h = fnv_u64(h, int(data["asm"]))
    h = fnv_u64(h, int(data["bin"]))
    h = fnv_u64(h, int(data["rollback_code"]))
    return h


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
    for key in ("ops_schema", "arch", "lang", "opt", "src_len", "ir", "asm", "bin", "elapsed_ns", "rollback_code"):
        int(data[key], 10)
    int(data["feat"], 16)
    int(data["src_hash"], 16)
    actual_signature = int(data["ops_signature"], 16)
    wanted_signature = expected_signature(data)
    if actual_signature != wanted_signature:
        raise SystemExit(f"ops_signature={actual_signature:016x} expected={wanted_signature:016x}")
    if args.expect_rollback is not None and int(data["rollback_code"]) != args.expect_rollback:
        raise SystemExit(f"rollback_code={data['rollback_code']} expected={args.expect_rollback}")
    print(f"OPS PASS {args.manifest} rollback_code={data['rollback_code']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
