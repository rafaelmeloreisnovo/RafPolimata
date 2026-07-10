#!/usr/bin/env python3
import argparse
from pathlib import Path

BASE_REQUIRED = {
    "ops_schema", "arch", "brand", "lang", "opt", "feat", "flags", "src_len",
    "src_hash", "ir", "asm", "bin", "elapsed_ns", "rollback_code",
    "ops_signature",
}

OMEGA_REQUIRED = {
    "omega_entropy_milli", "omega_phi_q16", "omega_attractor", "omega_flags",
    "omega_path", "omega_path_name",
}

OMEGA_PATH_NAMES = {
    0: "PROCESSUAL",
    1: "VOID",
    2: "FORGOTTEN",
    3: "MENOSPREZADO",
    4: "URGENT",
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
    schema = int(data["ops_schema"])
    h = FNV_OFFSET
    h = fnv_u64(h, int(data["arch"]))
    h = fnv_update(h, data["brand"].encode("utf-8"))
    h = fnv_u64(h, int(data["lang"]))
    h = fnv_u64(h, int(data["opt"]))
    h = fnv_u64(h, int(data["feat"], 16))
    h = fnv_update(h, data["flags"].encode("utf-8"))
    h = fnv_u64(h, int(data["src_len"]))
    h = fnv_u64(h, int(data["src_hash"], 16))
    if schema >= 2:
        h = fnv_u64(h, int(data["omega_entropy_milli"]))
        h = fnv_u64(h, int(data["omega_phi_q16"]))
        h = fnv_u64(h, int(data["omega_attractor"]))
        h = fnv_u64(h, int(data["omega_flags"], 16))
        h = fnv_u64(h, int(data["omega_path"]))
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

    missing = sorted(BASE_REQUIRED - data.keys())
    if missing:
        raise SystemExit(f"missing keys: {', '.join(missing)}")

    schema = int(data["ops_schema"])
    if schema >= 2:
        missing = sorted(OMEGA_REQUIRED - data.keys())
        if missing:
            raise SystemExit(f"missing Omega keys: {', '.join(missing)}")

    for key in (
        "ops_schema", "arch", "lang", "opt", "src_len", "ir", "asm", "bin",
        "elapsed_ns", "rollback_code"
    ):
        int(data[key], 10)
    int(data["feat"], 16)
    int(data["src_hash"], 16)

    if schema >= 2:
        for key in (
            "omega_entropy_milli", "omega_phi_q16", "omega_attractor",
            "omega_path"
        ):
            int(data[key], 10)
        int(data["omega_flags"], 16)
        entropy = int(data["omega_entropy_milli"])
        phi = int(data["omega_phi_q16"])
        attractor = int(data["omega_attractor"])
        path = int(data["omega_path"])
        if not 0 <= entropy <= 8000:
            raise SystemExit(f"omega_entropy_milli out of range: {entropy}")
        if not 0 <= phi <= 65536:
            raise SystemExit(f"omega_phi_q16 out of range: {phi}")
        if not 0 <= attractor < 42:
            raise SystemExit(f"omega_attractor out of range: {attractor}")
        if path not in OMEGA_PATH_NAMES:
            raise SystemExit(f"unknown omega_path: {path}")
        if data["omega_path_name"] != OMEGA_PATH_NAMES[path]:
            raise SystemExit(
                f"omega_path_name={data['omega_path_name']} "
                f"expected={OMEGA_PATH_NAMES[path]}"
            )

    actual_signature = int(data["ops_signature"], 16)
    wanted_signature = expected_signature(data)
    if actual_signature != wanted_signature:
        raise SystemExit(
            f"ops_signature={actual_signature:016x} expected={wanted_signature:016x}"
        )
    if args.expect_rollback is not None and int(data["rollback_code"]) != args.expect_rollback:
        raise SystemExit(
            f"rollback_code={data['rollback_code']} expected={args.expect_rollback}"
        )
    print(
        f"OPS PASS {args.manifest} schema={schema} "
        f"rollback_code={data['rollback_code']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
