#!/usr/bin/env python3
"""Synthetic forensic-deception laboratory.

This module is defensive and intentionally isolated from production data.
It models:
- canonical records;
- station-specific projected identifiers;
- synthetic decoy records;
- a repeated fingerprint codeword distributed across records;
- signed epoch manifests;
- bounded attack simulations (drop/reorder/strip);
- source-candidate scoring.

It does not implement production cryptography, format-preserving encryption,
database triggers, credential traps, or automatic retaliation.
"""

from __future__ import annotations

import argparse
import dataclasses
import hashlib
import hmac
import json
import random
from typing import Iterable, Sequence


STATUS = "REFERENCE_IMPLEMENTATION"
CLAIM_ALLOWED = False
SCHEMA_VERSION = "forensic-deception-lab.v1"


@dataclasses.dataclass(frozen=True)
class StationContext:
    tenant_id: str
    station_id: str
    user_scope: str
    epoch_id: str

    def canonical_bytes(self) -> bytes:
        return "|".join(
            (self.tenant_id, self.station_id, self.user_scope, self.epoch_id)
        ).encode("utf-8")


@dataclasses.dataclass(frozen=True)
class CanonicalRecord:
    canonical_id: int
    parent_id: int | None
    name: str
    cep: str
    amount_cents: int


@dataclasses.dataclass(frozen=True)
class ProjectedRecord:
    canonical_id: int
    projected_id: int
    projected_parent_id: int | None
    name: str
    cep: str
    amount_cents: int
    block_id: int
    cadence_symbol: int
    decoy_token: str | None = None

    @property
    def is_decoy(self) -> bool:
        return self.decoy_token is not None


def _canonical_json(value: object) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def derive_seed(master_key: bytes, context: StationContext, purpose: str) -> bytes:
    return hmac.new(
        master_key,
        context.canonical_bytes() + b"|" + purpose.encode("utf-8"),
        hashlib.sha256,
    ).digest()


def station_fingerprint(context: StationContext, bits: int = 16) -> list[int]:
    if bits <= 0 or bits % 8:
        raise ValueError("bits must be a positive multiple of 8")
    digest = hashlib.sha256(context.canonical_bytes()).digest()
    raw = int.from_bytes(digest[: bits // 8], "big")
    return [(raw >> shift) & 1 for shift in range(bits - 1, -1, -1)]


def repetition_encode(bits: Sequence[int], repetitions: int = 3) -> list[int]:
    if repetitions < 3 or repetitions % 2 == 0:
        raise ValueError("repetitions must be odd and >= 3")
    if any(bit not in (0, 1) for bit in bits):
        raise ValueError("bits must contain only 0/1")
    return [bit for bit in bits for _ in range(repetitions)]


def repetition_decode(
    symbols: Sequence[int | None], repetitions: int = 3
) -> tuple[list[int | None], float]:
    if len(symbols) % repetitions:
        raise ValueError("symbol count must be divisible by repetitions")
    decoded: list[int | None] = []
    resolved = 0
    for offset in range(0, len(symbols), repetitions):
        group = [v for v in symbols[offset : offset + repetitions] if v in (0, 1)]
        if not group:
            decoded.append(None)
            continue
        ones = sum(group)
        zeros = len(group) - ones
        if ones == zeros:
            decoded.append(None)
            continue
        decoded.append(1 if ones > zeros else 0)
        resolved += 1
    confidence = resolved / len(decoded) if decoded else 0.0
    return decoded, confidence


def _ranked_projection(
    records: Sequence[CanonicalRecord], seed: bytes, start: int = 100_000
) -> dict[int, int]:
    """Return a deterministic bijection for the supplied canonical ID set.

    This is a keyed permutation of a finite observed set, not FPE.
    """
    scored: list[tuple[bytes, int]] = []
    for record in records:
        score = hmac.new(
            seed, str(record.canonical_id).encode("ascii"), hashlib.sha256
        ).digest()
        scored.append((score, record.canonical_id))
    scored.sort()
    return {
        canonical_id: start + rank
        for rank, (_, canonical_id) in enumerate(scored, start=1)
    }


def synthetic_records(count: int = 64) -> list[CanonicalRecord]:
    if count < 48:
        raise ValueError("count must be >= 48 for the default codeword")
    records: list[CanonicalRecord] = []
    for idx in range(1, count + 1):
        parent = None if idx <= 8 else ((idx - 1) % 8) + 1
        records.append(
            CanonicalRecord(
                canonical_id=idx,
                parent_id=parent,
                name=f"TESTE-ENTIDADE-{idx:04d}",
                cep=f"99{idx % 1000:03d}-{(idx * 17) % 1000:03d}",
                amount_cents=10_000 + idx * 137,
            )
        )
    return records


def project_records(
    records: Sequence[CanonicalRecord],
    context: StationContext,
    master_key: bytes,
    decoy_count: int = 4,
) -> list[ProjectedRecord]:
    if decoy_count < 0:
        raise ValueError("decoy_count must be >= 0")

    seed = derive_seed(master_key, context, "id-projection")
    mapping = _ranked_projection(records, seed)
    codeword = repetition_encode(station_fingerprint(context), repetitions=3)

    projected: list[ProjectedRecord] = []
    for index, record in enumerate(records):
        projected.append(
            ProjectedRecord(
                canonical_id=record.canonical_id,
                projected_id=mapping[record.canonical_id],
                projected_parent_id=(
                    mapping[record.parent_id] if record.parent_id is not None else None
                ),
                name=record.name,
                cep=record.cep,
                amount_cents=record.amount_cents,
                block_id=index,
                cadence_symbol=codeword[index % len(codeword)],
            )
        )

    decoy_seed = int.from_bytes(
        derive_seed(master_key, context, "decoys")[:8], "big"
    )
    rng = random.Random(decoy_seed)
    next_canonical = max(record.canonical_id for record in records) + 1
    next_projected = max(mapping.values()) + 10_000

    for offset in range(decoy_count):
        token = hmac.new(
            derive_seed(master_key, context, "decoy-token"),
            str(offset).encode("ascii"),
            hashlib.sha256,
        ).hexdigest()[:20]
        projected.append(
            ProjectedRecord(
                canonical_id=next_canonical + offset,
                projected_id=next_projected + rng.randrange(1_000, 9_999),
                projected_parent_id=None,
                name=f"PROJETO-RESERVADO-{token[:6].upper()}",
                cep=f"98{rng.randrange(0, 999):03d}-{rng.randrange(0, 999):03d}",
                amount_cents=rng.randrange(250_000, 9_000_000),
                block_id=len(records) + offset,
                cadence_symbol=codeword[(len(records) + offset) % len(codeword)],
                decoy_token=token,
            )
        )

    return projected


def canonical_business_root(records: Iterable[CanonicalRecord]) -> str:
    payload = [dataclasses.asdict(record) for record in records]
    return hashlib.sha256(_canonical_json(payload)).hexdigest()


def projected_root(records: Iterable[ProjectedRecord]) -> str:
    payload = [dataclasses.asdict(record) for record in records]
    return hashlib.sha256(_canonical_json(payload)).hexdigest()


def make_manifest(
    canonical: Sequence[CanonicalRecord],
    projected: Sequence[ProjectedRecord],
    context: StationContext,
    master_key: bytes,
) -> dict[str, object]:
    body: dict[str, object] = {
        "schema_version": SCHEMA_VERSION,
        "status": STATUS,
        "claim_allowed": CLAIM_ALLOWED,
        "context": dataclasses.asdict(context),
        "canonical_root_sha256": canonical_business_root(canonical),
        "projected_root_sha256": projected_root(projected),
        "canonical_record_count": len(canonical),
        "projected_record_count": len(projected),
        "decoy_record_count": sum(record.is_decoy for record in projected),
        "fingerprint_bits": station_fingerprint(context),
        "encoding": {"kind": "repetition", "repetitions": 3},
        "limitations": [
            "synthetic data only",
            "HMAC manifest is not a digital signature",
            "finite-set ID projection is not format-preserving encryption",
            "attribution requires independent custody evidence",
        ],
    }
    body["manifest_hmac_sha256"] = hmac.new(
        derive_seed(master_key, context, "manifest"),
        _canonical_json(body),
        hashlib.sha256,
    ).hexdigest()
    return body


def verify_manifest(manifest: dict[str, object], master_key: bytes) -> bool:
    supplied = str(manifest.get("manifest_hmac_sha256", ""))
    body = dict(manifest)
    body.pop("manifest_hmac_sha256", None)
    context_dict = body.get("context")
    if not isinstance(context_dict, dict):
        return False
    try:
        context = StationContext(**context_dict)
    except TypeError:
        return False
    expected = hmac.new(
        derive_seed(master_key, context, "manifest"),
        _canonical_json(body),
        hashlib.sha256,
    ).hexdigest()
    return hmac.compare_digest(supplied, expected)


def simulate_leak(
    projected: Sequence[ProjectedRecord],
    seed: int = 7,
    drop_fraction: float = 0.15,
    strip_decoy_labels: bool = False,
) -> list[dict[str, object]]:
    if not 0.0 <= drop_fraction < 1.0:
        raise ValueError("drop_fraction must be in [0, 1)")
    rng = random.Random(seed)
    retained = [record for record in projected if rng.random() >= drop_fraction]
    rng.shuffle(retained)
    leak: list[dict[str, object]] = []
    for record in retained:
        item = dataclasses.asdict(record)
        if strip_decoy_labels:
            item.pop("decoy_token", None)
        leak.append(item)
    return leak


def recover_fingerprint(
    leak: Sequence[dict[str, object]], bit_count: int = 16, repetitions: int = 3
) -> tuple[list[int | None], float]:
    total = bit_count * repetitions
    symbols: list[int | None] = [None] * total
    for item in leak:
        block = item.get("block_id")
        symbol = item.get("cadence_symbol")
        if isinstance(block, int) and symbol in (0, 1):
            position = block % total
            current = symbols[position]
            if current is None:
                symbols[position] = int(symbol)
            elif current != symbol:
                symbols[position] = None
    return repetition_decode(symbols, repetitions=repetitions)


def score_candidate(recovered: Sequence[int | None], context: StationContext) -> float:
    expected = station_fingerprint(context, bits=len(recovered))
    observed = [(a, b) for a, b in zip(recovered, expected) if a is not None]
    if not observed:
        return 0.0
    return sum(a == b for a, b in observed) / len(observed)


def detect_decoy_touch(leak: Sequence[dict[str, object]]) -> list[str]:
    return sorted(
        {
            str(item["decoy_token"])
            for item in leak
            if item.get("decoy_token")
        }
    )


def run_demo() -> dict[str, object]:
    master_key = b"LAB-ONLY-REPLACE-IN-REAL-DEPLOYMENT"
    context = StationContext(
        tenant_id="TENANT-LAB",
        station_id="ESTACAO-07",
        user_scope="FISCAL-OPERADOR",
        epoch_id="2026-07-26T00",
    )
    canonical = synthetic_records()
    projected = project_records(canonical, context, master_key)
    manifest = make_manifest(canonical, projected, context, master_key)
    leak = simulate_leak(projected, drop_fraction=0.10)
    recovered, confidence = recover_fingerprint(leak)

    candidates = [
        context,
        dataclasses.replace(context, station_id="ESTACAO-08"),
        dataclasses.replace(context, user_scope="FISCAL-GERENTE"),
    ]
    ranking = sorted(
        (
            {
                "context": dataclasses.asdict(candidate),
                "score": score_candidate(recovered, candidate),
            }
            for candidate in candidates
        ),
        key=lambda item: item["score"],
        reverse=True,
    )

    canonical_total = sum(record.amount_cents for record in canonical)
    projected_total = sum(
        record.amount_cents for record in projected if not record.is_decoy
    )

    return {
        "schema_version": SCHEMA_VERSION,
        "status": STATUS,
        "claim_allowed": CLAIM_ALLOWED,
        "manifest_valid": verify_manifest(manifest, master_key),
        "business_invariant": {
            "canonical_total_cents": canonical_total,
            "projected_total_cents": projected_total,
            "preserved": canonical_total == projected_total,
        },
        "leak_record_count": len(leak),
        "fingerprint_recovery_confidence": confidence,
        "candidate_ranking": ranking,
        "decoy_tokens_observed": detect_decoy_touch(leak),
        "manifest": manifest,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", help="write JSON report to this path")
    args = parser.parse_args()
    report = run_demo()
    text = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True)
    if args.output:
        with open(args.output, "w", encoding="utf-8") as handle:
            handle.write(text + "\n")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
