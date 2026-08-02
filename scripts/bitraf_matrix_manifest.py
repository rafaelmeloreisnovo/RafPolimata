#!/usr/bin/env python3
"""Generate a deterministic BITRAF Matrix V1 manifest and optional CSV slices."""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
from dataclasses import asdict, dataclass
from pathlib import Path

B20 = "0123456789ABCDEFGHIJ"
TOTAL = 8000
CORE = 4096
SHELL = 3904


@dataclass(frozen=True)
class State:
    x: int
    y: int
    z: int
    vertex: int
    parity: int


def decode(index: int) -> State:
    if not 0 <= index < TOTAL:
        raise ValueError("index must be in 0..7999")
    parity = index & 1
    index >>= 1
    vertex = index & 3
    cell = index >> 2
    z = cell % 10
    cell //= 10
    y = cell % 10
    x = cell // 10
    return State(x, y, z, vertex, parity)


def encode(s: State) -> int:
    if not (
        0 <= s.x < 10
        and 0 <= s.y < 10
        and 0 <= s.z < 10
        and 0 <= s.vertex < 4
        and 0 <= s.parity < 2
    ):
        raise ValueError("invalid state")
    return (((s.x * 10 + s.y) * 10 + s.z) * 4 + s.vertex) * 2 + s.parity


def base20_3(index: int) -> str:
    if not 0 <= index < TOTAL:
        raise ValueError("index must be in 0..7999")
    a, rem = divmod(index, 400)
    b, c = divmod(rem, 20)
    return B20[a] + B20[b] + B20[c]


def is_core(s: State) -> bool:
    return 1 <= s.x <= 8 and 1 <= s.y <= 8 and 1 <= s.z <= 8


def core_index(s: State) -> int | None:
    if not is_core(s):
        return None
    cell = ((s.x - 1) * 8 + (s.y - 1)) * 8 + (s.z - 1)
    return (cell * 4 + s.vertex) * 2 + s.parity


def opposite(s: State) -> State:
    return State(9 - s.x, 9 - s.y, 9 - s.z, 3 - s.vertex, s.parity ^ 1)


def octant(s: State) -> int:
    return (
        ((1 if s.x >= 5 else 0) << 2)
        | ((1 if s.y >= 5 else 0) << 1)
        | (1 if s.z >= 5 else 0)
    )


def record(index: int) -> dict[str, object]:
    s = decode(index)
    return {
        "index8000": index,
        "base20": base20_3(index),
        **asdict(s),
        "region": "CORE4096" if is_core(s) else "SHELL3904",
        "core_index4096": core_index(s),
        "opposite_index8000": encode(opposite(s)),
        "octant": octant(s),
        "square_u": s.x * 10 + s.y,
        "square_v": s.z,
        "hex_q": s.x - s.y,
        "hex_r": s.z - ((s.x + s.y) // 2),
    }


def canonical_json(obj: object) -> bytes:
    return json.dumps(
        obj, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--out", type=Path, default=Path("output/bitraf_matrix_v1")
    )
    parser.add_argument(
        "--csv", choices=("none", "core", "shell", "all"), default="none"
    )
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    records = [record(i) for i in range(TOTAL)]
    core_count = sum(r["region"] == "CORE4096" for r in records)
    shell_count = TOTAL - core_count
    if (core_count, shell_count) != (CORE, SHELL):
        raise SystemExit("partition invariant failed")

    manifest = {
        "spec": "BITRAF_MATRIX_8000_4096_V1",
        "equations": {
            "total": "10^3*4*2=8000=20^3",
            "core": "8^3*4*2=4096=2^12",
            "shell": "8000-4096=3904",
        },
        "counts": {"total": TOTAL, "core": core_count, "shell": shell_count},
        "alphabet_base20": B20,
        "index_domain": [0, TOTAL - 1],
        "core_coordinate_domain": [1, 8],
        "shell_rule": "x,y,z outside the closed inner cube [1,8]^3",
        "relations": [
            "orthogonal_neighbor",
            "parity_twin",
            "vertex_sibling",
            "opposite",
            "rotate_z90",
            "rotate_z180",
            "rotate_z270",
        ],
        "epistemic_state": "COMPUTATIONAL_MODEL_NOT_PHYSICAL_CLAIM",
        "claim_allowed": False,
    }
    manifest_bytes = canonical_json(manifest)
    manifest["sha256_without_hash_field"] = hashlib.sha256(manifest_bytes).hexdigest()
    (args.out / "manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    if args.csv != "none":
        selected = records
        if args.csv == "core":
            selected = [r for r in records if r["region"] == "CORE4096"]
        elif args.csv == "shell":
            selected = [r for r in records if r["region"] == "SHELL3904"]
        path = args.out / f"states_{args.csv}.csv"
        with path.open("w", encoding="utf-8", newline="") as fh:
            writer = csv.DictWriter(fh, fieldnames=list(selected[0]))
            writer.writeheader()
            writer.writerows(selected)

    print(
        json.dumps(
            {"status": "PASS", "total": TOTAL, "core": core_count, "shell": shell_count},
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
