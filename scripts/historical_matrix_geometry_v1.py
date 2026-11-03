#!/usr/bin/env python3
"""Bounded March-July 2025 8x3x2 geometric matrix composition.

Extends the existing Matrix Compose V1 producer; does not replace its codec,
serialization, source-root protection or truth boundaries. Hosted adapter only.
48 source cells must be supplied: TOKEN_VAZIO is not synthesized as zero.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any

from scripts.matrix_compose_v1 import (
    SCHEMA_IN, canonical_bytes, compose, infer_shape, sha256_hex,
)

SCHEMA = "rafaelia.historical-matrix-geometry.input.v1"
TOPOLOGY = {"positions": 8, "layers": 3, "oppositions": 2, "basis": "cube_parity_tetra"}
OUTPUT_SCHEMA = "rafaelia.historical-matrix-geometry.output.v1"
RECEIPT_SCHEMA = "rafaelia.historical-matrix-geometry.receipt.v1"
MAX_TOTAL_INPUT_CELLS = 65536
SLOTS = 8 * 3 * 2
Q16 = 56756
OPS = frozenset(("xor", "sum_mod_256", "weighted_mod_256", "majority_bit"))


def _shape(cell: dict[str, Any]) -> tuple[int, ...]:
    if ("matrix" in cell) == ("source" in cell):
        raise ValueError("each cell requires exactly one of matrix or source")
    if "matrix" in cell:
        value = cell["matrix"]
        shape = infer_shape(value)
        if not shape:
            raise ValueError("matrix must have at least one dimension")
        # bool is a subtype of int in Python; do not treat it as a byte.
        stack = [value]
        while stack:
            node = stack.pop()
            if isinstance(node, list):
                stack.extend(node)
            elif type(node) is not int:
                raise ValueError("only integral uint8 cells are supported")
    else:
        src = cell["source"]
        if not isinstance(src, dict):
            raise ValueError("source must be an object")
        dims = src.get("shape")
        if (not isinstance(dims, list) or not 1 <= len(dims) <= 7
            or any(type(x) is not int or x <= 0 for x in dims)):
            raise ValueError("source.shape must have 1..7 positive dimensions")
        shape = tuple(dims)
    if len(shape) > 7:
        raise ValueError("at most seven matrix dimensions are supported")
    return shape


def _slot(position: int, layer: int, opposition: int) -> int:
    return (position * 3 + layer) * 2 + opposition


def build(spec: dict[str, Any], source_root: Path) -> dict[str, Any]:
    if not isinstance(spec, dict) or spec.get("schema") != SCHEMA:
        raise ValueError("incorrect historical geometry schema")
    if spec.get("topology") != TOPOLOGY:
        raise ValueError("topology must be the exact versioned 8x3x2 cube profile")
    op = spec.get("operator")
    if op not in OPS:
        raise ValueError("operator must be one of the Matrix Compose V1 operators")
    axis = spec.get("mirror_axis", None)
    if axis is not None and (type(axis) is not int or not 0 <= axis <= 2):
        raise ValueError("mirror_axis must be null or 0..2")
    cells = spec.get("cells")
    if not isinstance(cells, list) or len(cells) != SLOTS:
        raise ValueError("all 48 typed cells required; missing is TOKEN_VAZIO, not zero")

    seen: dict[int, dict[str, Any]] = {}
    expected_shape: tuple[int, ...] | None = None
    for cell in cells:
        if not isinstance(cell, dict) or set(cell) - {
            "position", "layer", "opposition", "matrix", "source"
        }:
            raise ValueError("unknown or malformed cell fields")
        p, l, o = (cell.get("position"), cell.get("layer"), cell.get("opposition"))
        if any(type(v) is not int for v in (p, l, o)):
            raise ValueError("position/layer/opposition must be integers")
        if not (0 <= p < 8 and 0 <= l < 3 and 0 <= o < 2):
            raise ValueError("geometry coordinate out of range")
        target_p = p ^ (1 << axis) if axis is not None else p
        key = _slot(target_p, l, o)
        if key in seen:
            raise ValueError("duplicate canonical slot")
        current_shape = _shape(cell)
        if expected_shape is None:
            expected_shape = current_shape
        elif expected_shape != current_shape:
            raise ValueError("cell matrix shapes must match")
        seen[key] = cell

    if len(seen) != SLOTS or expected_shape is None:
        raise ValueError("missing canonical slot; no silent filling")
    cells_each = math.prod(expected_shape)
    if cells_each * SLOTS > MAX_TOTAL_INPUT_CELLS:
        raise ValueError("materialization budget exceeded; use symbolic grid indexing")

    ordered = [seen[i] for i in range(SLOTS)]
    layers: list[dict[str, Any]] = []
    for idx, cell in enumerate(ordered):
        entry: dict[str, Any] = {"id": f"G{idx:02d}"}
        entry["matrix" if "matrix" in cell else "source"] = (
            cell["matrix"] if "matrix" in cell else cell["source"]
        )
        layers.append(entry)

    weights = spec.get("weights")
    if op == "weighted_mod_256":
        if (not isinstance(weights, list) or len(weights) != SLOTS
            or any(type(w) is not int for w in weights)):
            raise ValueError("weighted mode requires 48 exact integer weights")
    elif weights is not None:
        raise ValueError("weights only allowed with weighted_mod_256")

    base_spec: dict[str, Any] = {
        "schema": SCHEMA_IN, "operator": op, "layers": layers,
    }
    if weights is not None:
        base_spec["weights"] = weights
    core = compose(base_spec, source_root)

    # Context/layer projections are independent aggregates, not an inverse
    # transform. For majority_bit, combining partial folds is not associative.
    by_opposition = []
    for o in range(2):
        selected = [i for i in range(SLOTS) if (i % 2) == o]
        projection = {"schema": SCHEMA_IN, "operator": op,
                      "layers": [layers[i] for i in selected]}
        if weights is not None:
            projection["weights"] = [weights[i] for i in selected]
        out = compose(projection, source_root)
        by_opposition.append({
            "opposition": o,
            "matrix": out["matrix"],
            "sha256": out["ifdex"]["output"]["matrix_sha256"],
        })

    by_layer = []
    for level in range(3):
        selected = [i for i in range(SLOTS) if ((i // 2) % 3) == level]
        projection = {"schema": SCHEMA_IN, "operator": op,
                      "layers": [layers[i] for i in selected]}
        if weights is not None:
            projection["weights"] = [weights[i] for i in selected]
        out = compose(projection, source_root)
        by_layer.append({
            "layer": level,
            "matrix": out["matrix"],
            "sha256": out["ifdex"]["output"]["matrix_sha256"],
        })

    corners = []
    for p in range(8):
        corner = [(-1 if p & (1 << bit) else 1) for bit in range(3)]
        parity = ((p & 1) ^ ((p >> 1) & 1) ^ ((p >> 2) & 1))
        corners.append({"position": p, "xyz": corner, "parity": parity})
    scales = [65536]
    for _ in range(2):
        scales.append((scales[-1] * Q16 + 32768) >> 16)

    metadata = {
        "schema": OUTPUT_SCHEMA,
        "source_profile": "historical_2025_mar_jul_source_qualified",
        "topology": TOPOLOGY,
        "mirror_axis": axis if axis is not None else "TOKEN_VAZIO_NOT_REQUESTED",
        "canonical_slots": [
            {"slot": i, "position": i // 6,
             "layer": (i // 2) % 3, "opposition": i % 2,
             "source_position": ordered[i]["position"]}
            for i in range(SLOTS)
        ],
        "corners": corners,
        "q16_scales": scales,
        "central_matrix": core["matrix"],
        "by_opposition": by_opposition,
        "by_layer": by_layer,
        "source_spec_sha256": sha256_hex(canonical_bytes(spec)),
        "ifdex_sha256": sha256_hex(canonical_bytes(core["ifdex"])),
        "projection_is_reversible": False,
        "reconstruction_requires_original_48_cells": True,
        "quantum_or_spacetime_model": "TOKEN_VAZIO_NOT_DERIVED",
        "claim_allowed": False,
    }
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "source_spec_sha256": metadata["source_spec_sha256"],
        "geometry_sha256": sha256_hex(canonical_bytes(metadata)),
        "matrix_sha256": core["ifdex"]["output"]["matrix_sha256"],
        "packed_cell10_sha256": core["ifdex"]["output"]["packed_cell10_sha256"],
        "slots": SLOTS,
        "cells_per_slot": cells_each,
        "total_input_cells": cells_each * SLOTS,
        "operator": op,
        "state": "EXECUTED_WHEN_THIS_RUNS",
        "claim_allowed": False,
        "boundaries": [
            "USER_INPUT!=ASSISTANT_OUTPUT",
            "SOURCE!=ARTIFACT!=EXECUTION!=EVIDENCE!=CLAIM",
            "MATRIX!=PHYSICAL_SPACETIME",
            "XOR_PROJECTION!=LOSSLESS_RECONSTRUCTION",
        ],
    }
    return {"core": core, "geometry": metadata, "receipt": receipt}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("--source-root", default=".")
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()
    spec = json.loads(Path(args.input).read_text(encoding="utf-8"))
    out = build(spec, Path(args.source_root))
    directory = Path(args.out_dir)
    directory.mkdir(parents=True, exist_ok=True)
    files = {
        "matrix.output.json": canonical_bytes(out["core"]["matrix"]) + b"\n",
        "compose.output.hex": (out["core"]["packed"].hex() + "\n").encode("ascii"),
        "ifdex.v1.json": canonical_bytes(out["core"]["ifdex"]) + b"\n",
        "receipt.v1.json": canonical_bytes(out["core"]["receipt"]) + b"\n",
        "geometry.output.v1.json": canonical_bytes(out["geometry"]) + b"\n",
        "geometry.receipt.v1.json": canonical_bytes(out["receipt"]) + b"\n",
    }
    for name, data in files.items():
        (directory / name).write_bytes(data)
    print(canonical_bytes(out["receipt"]).decode("utf-8"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
