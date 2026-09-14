#!/usr/bin/env python3
"""RAFAELIA Matrix Compose V1.

Deterministic N-dimensional uint8 matrix composition.
SOURCE != EXECUTION != EVIDENCE != CLAIM.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from pathlib import Path
from typing import Any

SCHEMA_IN = "rafaelia.matrix-compose.input.v1"
SCHEMA_RECEIPT = "rafaelia.matrix-compose.receipt.v1"
SCHEMA_IFDEX = "rafaelia.ifdex.v1"

def canonical_bytes(obj: Any) -> bytes:
    return json.dumps(obj, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")

def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def infer_shape(node: Any) -> tuple[int, ...]:
    if isinstance(node, int):
        if not 0 <= node <= 255:
            raise ValueError("matrix values must be uint8")
        return ()
    if not isinstance(node, list) or not node:
        raise ValueError("matrix dimensions must be non-empty lists")
    child_shapes = [infer_shape(x) for x in node]
    if any(s != child_shapes[0] for s in child_shapes[1:]):
        raise ValueError("ragged matrix is forbidden")
    return (len(node),) + child_shapes[0]

def flatten(node: Any) -> list[int]:
    if isinstance(node, int):
        return [node]
    out: list[int] = []
    for item in node:
        out.extend(flatten(item))
    return out

def reshape(values: list[int], shape: tuple[int, ...]) -> Any:
    if not shape:
        if len(values) != 1:
            raise ValueError("scalar reshape mismatch")
        return values[0]
    expected = math.prod(shape)
    if len(values) != expected:
        raise ValueError("reshape size mismatch")

    def rec(offset: int, dims: tuple[int, ...]):
        if len(dims) == 1:
            return values[offset:offset + dims[0]], offset + dims[0]
        rows = []
        for _ in range(dims[0]):
            part, offset = rec(offset, dims[1:])
            rows.append(part)
        return rows, offset

    result, end = rec(0, shape)
    if end != expected:
        raise AssertionError("internal reshape mismatch")
    return result

def _resolve_source(path: str, source_root: Path) -> Path:
    root = source_root.resolve()
    resolved = (root / path).resolve()
    if root != resolved and root not in resolved.parents:
        raise ValueError("source path escapes source_root")
    return resolved

def load_layer(layer: dict[str, Any], source_root: Path):
    layer_id = layer.get("id")
    if not isinstance(layer_id, str) or not layer_id:
        raise ValueError("layer.id required")
    has_matrix = "matrix" in layer
    has_source = "source" in layer
    if has_matrix == has_source:
        raise ValueError("layer must contain exactly one of matrix or source")

    if has_matrix:
        shape = infer_shape(layer["matrix"])
        values = flatten(layer["matrix"])
        identity = {
            "id": layer_id,
            "kind": "inline_matrix",
            "shape": list(shape),
            "source_sha256": sha256_hex(canonical_bytes(layer["matrix"])),
            "offset": 0,
            "length": len(values),
        }
        return shape, values, identity

    source = layer["source"]
    if not isinstance(source, dict):
        raise ValueError("source must be object")
    rel = source.get("path")
    shape_raw = source.get("shape")
    offset = source.get("offset", 0)
    if not isinstance(rel, str) or not rel:
        raise ValueError("source.path required")
    if not isinstance(shape_raw, list) or not shape_raw or any(not isinstance(x, int) or x <= 0 for x in shape_raw):
        raise ValueError("source.shape must be positive integer array")
    if not isinstance(offset, int) or offset < 0:
        raise ValueError("source.offset must be non-negative integer")

    shape = tuple(shape_raw)
    count = math.prod(shape)
    source_path = _resolve_source(rel, source_root)
    with source_path.open("rb") as handle:
        handle.seek(offset)
        data = handle.read(count)
    if len(data) != count:
        raise ValueError("source does not contain enough bytes for declared shape")

    identity = {
        "id": layer_id,
        "kind": "binary_source",
        "path": rel,
        "shape": list(shape),
        "source_sha256": sha256_hex(data),
        "offset": offset,
        "length": count,
    }
    return shape, list(data), identity

def _xor(values) -> int:
    acc = 0
    for value in values:
        acc ^= value
    return acc

def overlay(vectors: list[list[int]], operator: str, weights: list[int] | None = None) -> list[int]:
    if not vectors:
        raise ValueError("at least one layer required")
    length = len(vectors[0])
    if any(len(vector) != length for vector in vectors):
        raise ValueError("layer lengths differ")

    if operator == "xor":
        return [_xor(values) for values in zip(*vectors)]
    if operator == "sum_mod_256":
        return [sum(values) & 0xFF for values in zip(*vectors)]
    if operator == "weighted_mod_256":
        if weights is None or len(weights) != len(vectors) or any(not isinstance(weight, int) for weight in weights):
            raise ValueError("integer weights required for every layer")
        return [sum(weight * value for weight, value in zip(weights, values)) & 0xFF for values in zip(*vectors)]
    if operator == "majority_bit":
        threshold = len(vectors) // 2 + 1
        output = []
        for values in zip(*vectors):
            byte = 0
            for bit in range(8):
                if sum((value >> bit) & 1 for value in values) >= threshold:
                    byte |= 1 << bit
            output.append(byte)
        return output
    raise ValueError(f"unsupported operator: {operator}")

def encode_cell10(byte: int) -> int:
    if not 0 <= byte <= 255:
        raise ValueError("byte out of range")
    payload = byte & 0x7F
    extension = (byte >> 7) & 1
    p0 = extension
    p1 = extension
    for bit in (0, 2, 4, 6):
        p0 ^= (payload >> bit) & 1
    for bit in (1, 3, 5):
        p1 ^= (payload >> bit) & 1
    return payload | (p0 << 7) | (p1 << 8) | (extension << 9)

def decode_cell10(cell: int) -> int:
    if not 0 <= cell < 1024:
        raise ValueError("cell10 out of range")
    payload = cell & 0x7F
    p0 = (cell >> 7) & 1
    p1 = (cell >> 8) & 1
    extension = (cell >> 9) & 1
    expected0 = extension
    expected1 = extension
    for bit in (0, 2, 4, 6):
        expected0 ^= (payload >> bit) & 1
    for bit in (1, 3, 5):
        expected1 ^= (payload >> bit) & 1
    if p0 != expected0 or p1 != expected1:
        raise ValueError("cell10 parity mismatch")
    return payload | (extension << 7)

def pack_cells10(cells: list[int]) -> bytes:
    bits = "".join(f"{cell:010b}" for cell in cells)
    if len(bits) % 8:
        bits += "0" * (8 - len(bits) % 8)
    return bytes(int(bits[index:index + 8], 2) for index in range(0, len(bits), 8))

def compose(spec: dict[str, Any], source_root: Path) -> dict[str, Any]:
    if spec.get("schema") != SCHEMA_IN:
        raise ValueError(f"schema must be {SCHEMA_IN}")
    layers = spec.get("layers")
    if not isinstance(layers, list) or not layers:
        raise ValueError("layers must be a non-empty array")

    loaded = [load_layer(layer, source_root) for layer in layers]
    shape = loaded[0][0]
    if any(item[0] != shape for item in loaded[1:]):
        raise ValueError("all layer shapes must match")

    vectors = [item[1] for item in loaded]
    output = overlay(vectors, spec.get("operator"), spec.get("weights"))
    cells = [encode_cell10(value) for value in output]
    packed = pack_cells10(cells)
    output_matrix = reshape(output, shape)
    identities = [item[2] for item in loaded]

    ifdex = {
        "schema": SCHEMA_IFDEX,
        "definition": "Index of Files, Digests, Extents and Cross-layer composition",
        "shape": list(shape),
        "cell_count": len(output),
        "layers": identities,
        "operator": spec["operator"],
        "output": {
            "packed_cell10_sha256": sha256_hex(packed),
            "packed_cell10_bytes": len(packed),
            "hex": packed.hex(),
            "matrix_sha256": sha256_hex(canonical_bytes(output_matrix)),
        },
    }
    return {
        "matrix": output_matrix,
        "packed": packed,
        "ifdex": ifdex,
        "receipt": {
            "schema": SCHEMA_RECEIPT,
            "state": "EXECUTED_LOCAL_OR_CI",
            "claim_allowed": False,
            "source_spec_sha256": sha256_hex(canonical_bytes(spec)),
            "ifdex_sha256": sha256_hex(canonical_bytes(ifdex)),
            "packed_cell10_sha256": sha256_hex(packed),
            "shape": list(shape),
            "operator": spec["operator"],
            "boundary": "SOURCE!=EXECUTION!=EVIDENCE!=CLAIM",
        },
    }

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--source-root", default=".")
    args = parser.parse_args()

    spec = json.loads(Path(args.input).read_text(encoding="utf-8"))
    result = compose(spec, Path(args.source_root))
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "compose.output.hex").write_text(result["packed"].hex() + "\n", encoding="ascii")
    (out_dir / "ifdex.v1.json").write_bytes(canonical_bytes(result["ifdex"]) + b"\n")
    (out_dir / "receipt.v1.json").write_bytes(canonical_bytes(result["receipt"]) + b"\n")
    (out_dir / "matrix.output.json").write_bytes(canonical_bytes(result["matrix"]) + b"\n")
    print(json.dumps(result["receipt"], sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
