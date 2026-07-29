"""Consolidated deterministic toolkit for toroidal mapping, integrity, dynamics and reports."""
from __future__ import annotations

import base64
import cmath
import dataclasses
import hashlib
import json
import math
import random
import struct
import zlib
from collections import Counter
from collections.abc import Callable, Iterable, Sequence
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

UINT64_MOD = 1 << 64
TAU = 2.0 * math.pi
DEFAULT_ALPHA = 0.25
State = tuple[float, ...]


def _normalize(value: Any) -> Any:
    if dataclasses.is_dataclass(value):
        return _normalize(dataclasses.asdict(value))
    if isinstance(value, Path):
        return {"$path": value.as_posix()}
    if isinstance(value, bytes):
        return {"$bytes_b64": base64.b64encode(value).decode("ascii")}
    if isinstance(value, bytearray):
        return {"$bytes_b64": base64.b64encode(bytes(value)).decode("ascii")}
    if isinstance(value, tuple):
        return [_normalize(item) for item in value]
    if isinstance(value, list):
        return [_normalize(item) for item in value]
    if isinstance(value, dict):
        return {str(key): _normalize(item) for key, item in sorted(value.items(), key=lambda pair: str(pair[0]))}
    if isinstance(value, float):
        if not math.isfinite(value):
            raise ValueError("non-finite floats are forbidden in canonical input")
        return {"$float_hex": value.hex()}
    if value is None or isinstance(value, (str, int, bool)):
        return value
    raise TypeError(f"unsupported canonical type: {type(value).__name__}")


def canonical_bytes(value: Any) -> bytes:
    normalized = _normalize(value)
    return json.dumps(normalized, ensure_ascii=False, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


@dataclass(frozen=True, slots=True)
class ToroidalPoint:
    q64: tuple[int, int, int, int, int, int, int]

    def __post_init__(self) -> None:
        if len(self.q64) != 7:
            raise ValueError("ToroidalPoint requires exactly seven coordinates")
        if any(not 0 <= value < UINT64_MOD for value in self.q64):
            raise ValueError("coordinates must be uint64 values")

    def unit(self) -> tuple[float, ...]:
        return tuple(value / UINT64_MOD for value in self.q64)

    def radians(self) -> tuple[float, ...]:
        return tuple(TAU * value / UINT64_MOD for value in self.q64)

    def hex(self) -> tuple[str, ...]:
        return tuple(f"{value:016x}" for value in self.q64)


class ToroidalMap:
    """Domain-separated SHA-256 map from canonical data and metadata to T^7."""

    def __init__(self, namespace: str = "voynich-repro:toroidal-map:v1") -> None:
        if not namespace:
            raise ValueError("namespace cannot be empty")
        self.namespace = namespace

    def map(self, data: Any, metadata: Any) -> ToroidalPoint:
        payload = canonical_bytes({"namespace": self.namespace, "data": data, "metadata": metadata})
        coords = []
        for axis in range(7):
            digest = hashlib.sha256(b"TORO7\x00" + axis.to_bytes(1, "big") + payload).digest()
            coords.append(int.from_bytes(digest[:8], "big"))
        return ToroidalPoint(tuple(coords))  # type: ignore[arg-type]

    def map_many(self, samples: Iterable[tuple[Any, Any]]) -> list[ToroidalPoint]:
        return [self.map(data, metadata) for data, metadata in samples]


@dataclass(frozen=True, slots=True)
class CHState:
    c: float = 0.0
    h: float = 0.0


def _validate_unit(name: str, value: float) -> None:
    if not 0.0 <= value <= 1.0:
        raise ValueError(f"{name} must be within [0, 1]")


def update_ch(state: CHState, c_observation: float, h_observation: float, alpha: float = DEFAULT_ALPHA) -> CHState:
    _validate_unit("state.c", state.c)
    _validate_unit("state.h", state.h)
    _validate_unit("c_observation", c_observation)
    _validate_unit("h_observation", h_observation)
    if not 0.0 < alpha <= 1.0:
        raise ValueError("alpha must be within (0, 1]")
    return CHState(c=(1.0 - alpha) * state.c + alpha * c_observation, h=(1.0 - alpha) * state.h + alpha * h_observation)


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def crc32_hex(data: bytes) -> str:
    return f"{zlib.crc32(data) & 0xFFFFFFFF:08x}"


def fnv1a_64(data: bytes) -> int:
    value = 0xCBF29CE484222325
    for byte in data:
        value ^= byte
        value = (value * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def fnv1a_64_hex(data: bytes) -> str:
    return f"{fnv1a_64(data):016x}"


def merkle_root(leaves: Iterable[bytes]) -> str:
    level = [hashlib.sha256(b"LEAF\x00" + leaf).digest() for leaf in leaves]
    if not level:
        return hashlib.sha256(b"EMPTY\x00").hexdigest()
    while len(level) > 1:
        if len(level) % 2:
            level.append(level[-1])
        level = [hashlib.sha256(b"NODE\x00" + level[i] + level[i + 1]).digest() for i in range(0, len(level), 2)]
    return level[0].hex()


def entropy_milli(values: Iterable[object]) -> int:
    items = list(values)
    if not items:
        return 0
    counts = Counter(items)
    total = len(items)
    entropy_bits = -sum((count / total) * math.log2(count / total) for count in counts.values())
    return int(round(entropy_bits * 1000.0))


def bits_geom(points: Sequence[Sequence[float]], bins_per_axis: int = 16) -> float:
    """log2 occupied quantized cells: an explicit proxy, not a universal theorem."""
    if bins_per_axis < 2:
        raise ValueError("bins_per_axis must be >= 2")
    if not points:
        return 0.0
    width = len(points[0])
    if width == 0:
        return 0.0
    cells: set[tuple[int, ...]] = set()
    for point in points:
        if len(point) != width:
            raise ValueError("all points must have equal dimensionality")
        quantized = []
        for value in point:
            if not 0.0 <= value < 1.0:
                raise ValueError("geometric coordinates must be in [0, 1)")
            quantized.append(min(int(value * bins_per_axis), bins_per_axis - 1))
        cells.add(tuple(quantized))
    return math.log2(len(cells)) if cells else 0.0


def trajectory_signal(points: Sequence[Sequence[float]]) -> list[float]:
    """Psi(t): mean of seven phase sines for coordinates in [0,1)."""
    signal = []
    for point in points:
        if len(point) != 7:
            raise ValueError("each trajectory point must have seven coordinates")
        if any(not 0.0 <= value < 1.0 for value in point):
            raise ValueError("trajectory coordinates must be in [0,1)")
        signal.append(sum(math.sin(TAU * value) for value in point) / 7.0)
    return signal


def h_cardio(length: int) -> list[float]:
    """Declared cardioid window H[n]=0.5*(1-cos(2*pi*n/(N-1)))."""
    if length <= 0:
        raise ValueError("length must be positive")
    if length == 1:
        return [1.0]
    return [0.5 * (1.0 - math.cos(TAU * n / (length - 1))) for n in range(length)]


def _next_power_of_two(value: int) -> int:
    return 1 if value <= 1 else 1 << (value - 1).bit_length()


def fft(values: Sequence[complex]) -> list[complex]:
    """Iterative Cooley-Tukey FFT with power-of-two input length."""
    n = len(values)
    if n == 0 or n & (n - 1):
        raise ValueError("FFT length must be a non-zero power of two")
    output = list(values)
    j = 0
    for i in range(1, n):
        bit = n >> 1
        while j & bit:
            j ^= bit
            bit >>= 1
        j ^= bit
        if i < j:
            output[i], output[j] = output[j], output[i]
    size = 2
    while size <= n:
        root = cmath.exp(-2j * math.pi / size)
        half = size // 2
        for start in range(0, n, size):
            factor = 1.0 + 0.0j
            for offset in range(half):
                even = output[start + offset]
                odd = output[start + offset + half] * factor
                output[start + offset] = even + odd
                output[start + offset + half] = even - odd
                factor *= root
        size <<= 1
    return output


def spectrum(signal: Sequence[float], apply_cardio: bool = True) -> list[float]:
    if not signal:
        return []
    window = h_cardio(len(signal)) if apply_cardio else [1.0] * len(signal)
    mean = sum(signal) / len(signal)
    centered = [(value - mean) * weight for value, weight in zip(signal, window)]
    n_fft = _next_power_of_two(len(centered))
    padded = centered + [0.0] * (n_fft - len(centered))
    transformed = fft([complex(value, 0.0) for value in padded])
    return [abs(value) ** 2 for value in transformed[: n_fft // 2 + 1]]


def pearson_correlation(left: Sequence[float], right: Sequence[float]) -> float:
    if len(left) != len(right) or not left:
        raise ValueError("vectors must be non-empty and have equal length")
    mean_l = sum(left) / len(left)
    mean_r = sum(right) / len(right)
    dl = [value - mean_l for value in left]
    dr = [value - mean_r for value in right]
    denom = math.sqrt(sum(value * value for value in dl) * sum(value * value for value in dr))
    if denom == 0.0:
        return 0.0
    return sum(a * b for a, b in zip(dl, dr)) / denom


def cardio_reference_spectrum(length: int) -> list[float]:
    return spectrum(h_cardio(length), apply_cardio=False)


def spectral_correlation(signal_values: Sequence[float]) -> tuple[list[float], float]:
    spec = spectrum(signal_values, apply_cardio=True)
    reference = cardio_reference_spectrum(len(signal_values))
    if len(reference) != len(spec):
        raise AssertionError("internal spectral length mismatch")
    return spec, pearson_correlation(spec, reference)


@dataclass(frozen=True, slots=True)
class CycleResult:
    found: bool
    mu: int | None = None
    period: int | None = None
    witness_hash: str | None = None


def iterate_map(state: Sequence[float], drift: Sequence[float] | None = None, coupling: float = 0.05) -> State:
    """One nearest-neighbour coupled circle-map step on T^n."""
    if not state:
        raise ValueError("state cannot be empty")
    if any(not 0.0 <= value < 1.0 for value in state):
        raise ValueError("state values must be in [0,1)")
    if not 0.0 <= coupling <= 1.0:
        raise ValueError("coupling must be in [0,1]")
    if drift is None:
        drift = tuple((index + 1) * 0.6180339887498948 % 1.0 for index in range(len(state)))
    if len(drift) != len(state):
        raise ValueError("drift dimensionality mismatch")
    output = []
    for index, value in enumerate(state):
        previous = state[index - 1]
        correction = coupling * math.sin(TAU * (previous - value)) / TAU
        output.append((value + drift[index] + correction) % 1.0)
    return tuple(output)


def quantized_state(state: Sequence[float], bits: int = 16) -> tuple[int, ...]:
    if not 1 <= bits <= 32:
        raise ValueError("bits must be within [1,32]")
    scale = 1 << bits
    return tuple(int(value * scale) % scale for value in state)


def state_hash(state: Sequence[float], bits: int = 16) -> str:
    payload = b"".join(struct.pack(">I", value) for value in quantized_state(state, bits))
    return hashlib.sha256(b"STATE\x00" + bytes([bits]) + payload).hexdigest()


def detect_cycle_hashing(step: Callable[[State], State], initial: State, max_steps: int = 10000, bits: int = 16) -> CycleResult:
    seen: dict[str, int] = {}
    state = initial
    for index in range(max_steps + 1):
        digest = state_hash(state, bits)
        if digest in seen:
            mu = seen[digest]
            return CycleResult(True, mu, index - mu, digest)
        seen[digest] = index
        state = step(state)
    return CycleResult(False)


def detect_cycle_floyd(step: Callable[[State], State], initial: State, max_steps: int = 10000, bits: int = 16) -> CycleResult:
    def equal(left: State, right: State) -> bool:
        return quantized_state(left, bits) == quantized_state(right, bits)
    tortoise = step(initial)
    hare = step(step(initial))
    iterations = 0
    while not equal(tortoise, hare) and iterations < max_steps:
        tortoise = step(tortoise)
        hare = step(step(hare))
        iterations += 1
    if iterations >= max_steps:
        return CycleResult(False)
    mu = 0
    tortoise = initial
    while not equal(tortoise, hare) and mu < max_steps:
        tortoise = step(tortoise)
        hare = step(hare)
        mu += 1
    if mu >= max_steps:
        return CycleResult(False)
    period = 1
    hare = step(tortoise)
    while not equal(tortoise, hare) and period < max_steps:
        hare = step(hare)
        period += 1
    if period >= max_steps:
        return CycleResult(False)
    return CycleResult(True, mu, period, state_hash(tortoise, bits))


def chunk_conversation(messages: Iterable[dict[str, Any]], output_dir: Path, max_bytes: int = 1_000_000, repository_map: dict[str, str] | None = None) -> Path:
    if max_bytes <= 0:
        raise ValueError("max_bytes must be positive")
    output_dir.mkdir(parents=True, exist_ok=True)
    repository_map = repository_map or {}
    chunks: list[list[dict[str, Any]]] = []
    current: list[dict[str, Any]] = []
    current_size = 2
    for ordinal, raw in enumerate(messages):
        message = dict(raw)
        message.setdefault("ordinal", ordinal)
        encoded = canonical_bytes(message)
        if len(encoded) + 2 > max_bytes:
            raise ValueError(f"message {ordinal} exceeds max_bytes")
        separator = 1 if current else 0
        if current and current_size + separator + len(encoded) > max_bytes:
            chunks.append(current)
            current = []
            current_size = 2
        current.append(message)
        current_size += separator + len(encoded)
    if current:
        chunks.append(current)
    index: dict[str, Any] = {"version": 1, "chunks": [], "repository_map": repository_map}
    for chunk_number, chunk in enumerate(chunks):
        filename = f"conversation_{chunk_number:04d}.json"
        payload = canonical_bytes({"messages": chunk})
        (output_dir / filename).write_bytes(payload)
        tags = sorted({str(tag) for item in chunk for tag in item.get("tags", [])})
        repositories = sorted({repository_map[tag] for tag in tags if tag in repository_map})
        index["chunks"].append({"file": filename, "first_ordinal": chunk[0]["ordinal"], "last_ordinal": chunk[-1]["ordinal"], "messages": len(chunk), "bytes": len(payload), "sha256": sha256_hex(payload), "tags": tags, "repositories": repositories})
    index_path = output_dir / "index.json"
    index_path.write_bytes(canonical_bytes(index))
    return index_path


def run_synthetic(seed: int = 144000, steps: int = 128, alpha: float = 0.25) -> dict[str, Any]:
    if steps < 8:
        raise ValueError("steps must be >= 8")
    rng = random.Random(seed)
    mapper = ToroidalMap()
    points = []
    ch = CHState(0.5, 0.5)
    leaves: list[bytes] = []
    for index in range(steps):
        data = {"sample": index, "value": rng.randrange(0, 1 << 31)}
        metadata = {"seed": seed, "stream": "synthetic-v1"}
        point = mapper.map(data, metadata)
        points.append(point.unit())
        leaves.append(canonical_bytes({"data": data, "metadata": metadata}))
        ch = update_ch(ch, c_observation=points[-1][1], h_observation=points[-1][0], alpha=alpha)
    psi = trajectory_signal(points)
    spec, correlation = spectral_correlation(psi)
    initial = tuple(points[0])
    step_fn = lambda state: iterate_map(state, coupling=0.05)
    floyd = detect_cycle_floyd(step_fn, initial, max_steps=2048, bits=12)
    hashed = detect_cycle_hashing(step_fn, initial, max_steps=2048, bits=12)
    report = {
        "status": "SYNTHETIC_LOCAL_MODEL_ONLY",
        "claim_allowed": False,
        "parameters": {"seed": seed, "steps": steps, "alpha": alpha},
        "toroidal_first_q64": mapper.map({"sample": 0, "value": random.Random(seed).randrange(0, 1 << 31)}, {"seed": seed, "stream": "synthetic-v1"}).hex(),
        "final_ch": asdict(ch),
        "psi": {"count": len(psi), "min": min(psi), "max": max(psi)},
        "spectrum": {"bins": len(spec), "cardio_correlation_r": correlation},
        "integrity": {"sha256": sha256_hex(b"".join(leaves)), "crc32": crc32_hex(b"".join(leaves)), "fnv1a64": fnv1a_64_hex(b"".join(leaves)), "merkle_root": merkle_root(leaves)},
        "metrics": {"entropy_milli_axis0_q8": entropy_milli(int(point[0] * 256) for point in points), "bits_geom_bins16": bits_geom(points, bins_per_axis=16)},
        "cycles": {"floyd": asdict(floyd), "hashing": asdict(hashed)},
        "remaining_gaps": ["TOKEN_VAZIO_REAL_VOYNICH_DATASET_CONTRACT", "TOKEN_VAZIO_H_CARDIO_DOMAIN_JUSTIFICATION", "TOKEN_VAZIO_CYCLE_ATTRACTOR_EXTERNAL_VALIDATION", "TOKEN_VAZIO_BITS_GEOM_SCIENTIFIC_CALIBRATION", "TOKEN_VAZIO_INDEPENDENT_REPLICATION"],
    }
    report["report_sha256"] = sha256_hex(canonical_bytes(report))
    return report


def write_report(path: Path, seed: int = 144000, steps: int = 128, alpha: float = 0.25) -> Path:
    report = run_synthetic(seed=seed, steps=steps, alpha=alpha)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True) + "\n", encoding="utf-8")
    return path
