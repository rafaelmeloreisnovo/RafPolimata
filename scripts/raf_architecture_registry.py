#!/usr/bin/env python3
"""Validate and inspect the seven-architecture RAFAELIA registry."""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
from pathlib import Path
import shutil
from typing import Any

SCHEMA = "rafaelia.architecture-registry.v2"
EXPECTED_IDS = (
    "aarch64", "armv7a", "x86_64", "riscv64", "mips64r6el", "s390x", "loongarch64"
)
FORBIDDEN_ACTIVE = {"i386", "ia32", "x86-32", "80386"}


class RegistryError(RuntimeError):
    pass


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def load(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise RegistryError(f"cannot read registry: {exc}") from exc
    if not isinstance(value, dict):
        raise RegistryError("registry root must be an object")
    return value


def validate(registry: dict[str, Any]) -> list[dict[str, Any]]:
    if registry.get("schema") != SCHEMA:
        raise RegistryError("unsupported schema")
    if registry.get("claim_allowed") is not False:
        raise RegistryError("claim_allowed must remain false")
    items = registry.get("architectures")
    if not isinstance(items, list) or len(items) != 7 or registry.get("active_count") != 7:
        raise RegistryError("exactly seven active architectures are required")
    ids = [item.get("id") if isinstance(item, dict) else None for item in items]
    if tuple(ids) != EXPECTED_IDS:
        raise RegistryError(f"architecture order/set mismatch: {ids!r}")
    if len(set(ids)) != len(ids):
        raise RegistryError("duplicate architecture id")
    for item in items:
        if item["id"] in FORBIDDEN_ACTIVE:
            raise RegistryError("retired i386 family found in active matrix")
        if item.get("bits") not in {32, 64}:
            raise RegistryError(f"{item['id']}: invalid width")
        if item.get("endianness") not in {"little", "big"}:
            raise RegistryError(f"{item['id']}: invalid endianness")
        for field in ("display_name", "compiler_triple", "qemu_user", "android_abi", "state", "execution"):
            if not isinstance(item.get(field), str) or not item[field]:
                raise RegistryError(f"{item['id']}: missing {field}")
        if not str(item["execution"]).startswith("TOKEN_VAZIO"):
            raise RegistryError(f"{item['id']}: execution requires an observed receipt before promotion")
    retired = registry.get("retired")
    if not isinstance(retired, list) or not any(item.get("id") == "i386" for item in retired if isinstance(item, dict)):
        raise RegistryError("i386 retirement record is required")
    return items


def audit(path: Path) -> dict[str, Any]:
    registry = load(path)
    items = validate(registry)
    return {
        "schema": "rafaelia.architecture-registry-receipt.v2",
        "created_at": now_utc(),
        "state": "PASS_SEVEN_ARCHITECTURE_POLICY",
        "registry_sha256": sha256_file(path),
        "active_count": len(items),
        "active_ids": [item["id"] for item in items],
        "retired_i386_active_occurrences": 0,
        "toolchain_probe": [
            {
                "id": item["id"],
                "qemu_user": item["qemu_user"],
                "qemu_path": shutil.which(item["qemu_user"]) or "TOKEN_VAZIO_COMMAND_ABSENT",
                "execution": item["execution"],
            }
            for item in items
        ],
        "claim_allowed": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("registry", type=Path, nargs="?", default=Path("compiler/architectures.v2.json"))
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    try:
        receipt = audit(args.registry)
        code = 0
    except RegistryError as exc:
        receipt = {"state": "FAIL", "error": str(exc), "claim_allowed": False}
        code = 1
    text = json.dumps(receipt, indent=2, sort_keys=True, ensure_ascii=False) + "\n"
    if args.receipt:
        args.receipt.parent.mkdir(parents=True, exist_ok=True)
        args.receipt.write_text(text, encoding="utf-8")
    print(text, end="")
    return code


if __name__ == "__main__":
    raise SystemExit(main())
