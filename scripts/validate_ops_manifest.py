#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

BASE_REQUIRED = {
    "ops_schema", "arch", "brand", "cores", "lang", "opt", "feat", "flags",
    "src_len", "src_hash", "ir", "asm", "bin", "native_requested",
    "native_written", "elapsed_ns", "rollback_code", "ops_signature",
}
OMEGA_REQUIRED = {
    "omega_entropy_milli", "omega_phi_q16", "omega_attractor", "omega_flags",
    "omega_path", "omega_path_name",
}
SCHEMA4_REQUIRED = {"ir_value", "emitter_schema", "transaction_state"}
OMEGA_PATH_NAMES = {
    0: "PROCESSUAL",
    1: "VOID",
    2: "FORGOTTEN",
    3: "MENOSPREZADO",
    4: "URGENT",
}

# Canonical FNV-1a 64-bit constants. The previous validator accidentally used
# a truncated offset basis and omitted fields hashed by the producer.
FNV_OFFSET = 14695981039346656037
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
    h = fnv_u64(h, int(data["cores"]))
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
    if schema >= 4:
        h = fnv_u64(h, int(data["ir_value"]))
        h = fnv_u64(h, int(data["emitter_schema"]))
    h = fnv_u64(h, int(data["native_requested"]))
    h = fnv_u64(h, int(data["native_written"]))
    h = fnv_u64(h, int(data["rollback_code"]))
    if schema >= 4:
        h = fnv_update(h, data["transaction_state"].encode("utf-8"))
    return h


def parse_ops(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line or line.startswith("#"):
            continue
        key, sep, value = line.partition("=")
        if not sep:
            raise SystemExit(f"line {line_number}: invalid line without '=': {line}")
        if key in data:
            raise SystemExit(f"line {line_number}: duplicate key: {key}")
        data[key] = value
    return data


def require_integer(data: dict[str, str], key: str, base: int = 10) -> int:
    try:
        return int(data[key], base)
    except ValueError as exc:
        raise SystemExit(f"{key} is not a valid base-{base} integer: {data[key]!r}") from exc


def validate_invariants(data: dict[str, str], schema: int) -> None:
    rollback = require_integer(data, "rollback_code")
    native_requested = require_integer(data, "native_requested")
    native_written = require_integer(data, "native_written")
    ir_count = require_integer(data, "ir")
    asm_count = require_integer(data, "asm")
    bin_count = require_integer(data, "bin")

    if native_requested not in {0, 1} or native_written not in {0, 1}:
        raise SystemExit("native_requested/native_written must be 0 or 1")
    if native_written > native_requested:
        raise SystemExit("native_written cannot exceed native_requested")

    if rollback == 0:
        if ir_count != 2 or asm_count == 0 or bin_count == 0:
            raise SystemExit("committed compile requires IR=2 and nonempty asm/bin")
        if native_written != native_requested:
            raise SystemExit("committed native request must be fully written")
    elif native_written != 0:
        raise SystemExit("rolled-back compile cannot retain a committed native artifact")

    if schema >= 4:
        state = data["transaction_state"]
        wanted_state = "COMMITTED" if rollback == 0 else "ROLLED_BACK"
        if state != wanted_state:
            raise SystemExit(f"transaction_state={state} expected={wanted_state}")
        ir_value = require_integer(data, "ir_value")
        if not 0 <= ir_value <= 0xFFFFFFFF:
            raise SystemExit(f"ir_value out of u32 range: {ir_value}")
        if require_integer(data, "emitter_schema") < 1:
            raise SystemExit("emitter_schema must be positive")
        if rollback != 0 and ir_count == 0 and ir_value != 0:
            raise SystemExit("empty IR cannot claim a nonzero ir_value")


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate raf_compile .ops manifest")
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--expect-rollback", type=int)
    args = parser.parse_args()
    data = parse_ops(args.manifest)

    missing = sorted(BASE_REQUIRED - data.keys())
    if missing:
        raise SystemExit(f"missing keys: {', '.join(missing)}")

    schema = require_integer(data, "ops_schema")
    if schema not in {2, 3, 4}:
        raise SystemExit(f"unsupported ops_schema: {schema}")
    if schema >= 2:
        missing = sorted(OMEGA_REQUIRED - data.keys())
        if missing:
            raise SystemExit(f"missing Omega keys: {', '.join(missing)}")
    if schema >= 4:
        missing = sorted(SCHEMA4_REQUIRED - data.keys())
        if missing:
            raise SystemExit(f"missing schema-4 keys: {', '.join(missing)}")

    for key in (
        "arch", "cores", "lang", "opt", "src_len", "ir", "asm", "bin",
        "native_requested", "native_written", "elapsed_ns", "rollback_code",
    ):
        require_integer(data, key)
    require_integer(data, "feat", 16)
    require_integer(data, "src_hash", 16)

    entropy = require_integer(data, "omega_entropy_milli")
    phi = require_integer(data, "omega_phi_q16")
    attractor = require_integer(data, "omega_attractor")
    path = require_integer(data, "omega_path")
    require_integer(data, "omega_flags", 16)
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
            f"omega_path_name={data['omega_path_name']} expected={OMEGA_PATH_NAMES[path]}"
        )

    validate_invariants(data, schema)
    actual_signature = require_integer(data, "ops_signature", 16)
    wanted_signature = expected_signature(data)
    if actual_signature != wanted_signature:
        raise SystemExit(
            f"ops_signature={actual_signature:016x} expected={wanted_signature:016x}"
        )
    if args.expect_rollback is not None and require_integer(data, "rollback_code") != args.expect_rollback:
        raise SystemExit(
            f"rollback_code={data['rollback_code']} expected={args.expect_rollback}"
        )
    print(
        f"OPS PASS {args.manifest} schema={schema} "
        f"state={data.get('transaction_state', 'LEGACY')} rollback={data['rollback_code']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
