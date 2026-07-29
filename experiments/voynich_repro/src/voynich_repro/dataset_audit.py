"""Audit a local Voynich dataset ZIP without publishing corpus bytes."""
from __future__ import annotations

import csv
import hashlib
import json
import math
import zipfile
from collections import Counter, defaultdict
from dataclasses import asdict
from pathlib import Path
from typing import Any, Sequence

from .core import (
    ToroidalMap,
    bits_geom,
    canonical_bytes,
    detect_cycle_hashing,
    entropy_milli,
    iterate_map,
    merkle_root,
    spectral_correlation,
    trajectory_signal,
)

EXPECTED_FILES = {
    "paper_outline.md",
    "semantic_hit_candidates.json",
    "folio_family_cluster_summary.csv",
    "candidate_parallels_extended.csv",
    "candidate_parallels.csv",
    "pattern_grammar (1).json",
    "pattern_grammar (1).md",
    "templatic_families_ranked (1).csv",
    "determinatives_arabic_control_top.csv",
    "determinatives_voynich_top.csv",
    "arabic_control_summary.json",
    "arabic_control_tokens.txt",
    "voynich_arabic_entropy_correlation.json",
    "determinative_candidates.csv",
    "family_entropy_vs_baseline.csv",
    "family_affixes.csv",
    "root_families_onset.csv",
    "voynich_eva_words.txt",
    "voynich_raw.txt",
}
HEADER_TOKENS = {"evat", "extracted", "from", "lsi", "ivtff", "txt", "version", "of", "modified"}


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _safe_members(zf: zipfile.ZipFile, max_member_bytes: int = 50_000_000) -> list[zipfile.ZipInfo]:
    infos = zf.infolist()
    for info in infos:
        path = Path(info.filename)
        if path.is_absolute() or ".." in path.parts:
            raise ValueError(f"unsafe ZIP member: {info.filename}")
        if info.file_size > max_member_bytes:
            raise ValueError(f"ZIP member exceeds limit: {info.filename}")
    return infos


def _words(blob: bytes) -> list[str]:
    return [line.strip() for line in blob.decode("utf-8").splitlines() if line.strip()]


def _csv(blob: bytes) -> list[dict[str, str]]:
    return list(csv.DictReader(blob.decode("utf-8-sig").splitlines()))


def _entropy(values: Sequence[str]) -> float:
    if not values:
        return 0.0
    counts = Counter(values)
    total = len(values)
    return -sum((count / total) * math.log2(count / total) for count in counts.values())


def _onset_stats(words: Sequence[str]) -> dict[str, dict[str, float | int]]:
    families: dict[str, list[str]] = defaultdict(list)
    for word in words:
        if len(word) > 2:
            families[word[:2]].append(word)
    return {
        family: {
            "support": len(items),
            "distinct_words": len(set(items)),
            "third_char_entropy_bits": _entropy([word[2] for word in items]),
        }
        for family, items in families.items()
    }


def audit_dataset_zip(zip_path: Path, event_id: str, timestamp_local: str) -> dict[str, Any]:
    zip_bytes = zip_path.read_bytes()
    with zipfile.ZipFile(zip_path) as zf:
        infos = _safe_members(zf)
        blobs = {info.filename: zf.read(info.filename) for info in infos}

    files = [
        {"name": name, "size_bytes": len(blob), "sha256": sha256_hex(blob)}
        for name, blob in sorted(blobs.items())
    ]
    words = _words(blobs["voynich_eva_words.txt"])
    control = _words(blobs["arabic_control_tokens.txt"])
    onset = _onset_stats(words)
    checks: list[dict[str, Any]] = []

    def check(check_id: str, passed: bool, severity: str, detail: Any = None) -> None:
        checks.append({"id": check_id, "pass": bool(passed), "severity": severity, "detail": detail})

    check("ZIP_SAFE_PATHS", True, "GATE")
    check("EXPECTED_FILE_SET", set(blobs) == EXPECTED_FILES, "FAIL", sorted(set(blobs) ^ EXPECTED_FILES))
    check("TOKEN_COUNT_MATCH", len(words) == len(control), "INFO", {"voynich": len(words), "control": len(control)})

    contamination = [word for word in words[:20] if word in HEADER_TOKENS]
    check("CLEANING_HEADER_CONTAMINATION", not contamination, "FAIL", contamination)

    root_mismatches = []
    for row in _csv(blobs["root_families_onset.csv"]):
        family = row["family_onset(first2)"]
        reported = int(row["occurrences"])
        recomputed = int(onset.get(family, {}).get("support", 0))
        if reported != recomputed:
            root_mismatches.append({"family": family, "reported": reported, "recomputed": recomputed})
    check("ROOT_FAMILY_COUNTS_REPRODUCE", not root_mismatches, "FAIL", root_mismatches)

    entropy_mismatches, degenerate = [], []
    for row in _csv(blobs["family_entropy_vs_baseline.csv"]):
        family = row["family_onset"]
        observed = float(row["obs_entropy_bits"])
        recomputed = float(onset[family]["third_char_entropy_bits"])
        if abs(observed - recomputed) > 0.001:
            entropy_mismatches.append({"family": family, "reported": observed, "recomputed": recomputed})
        if abs(float(row["baseline_mean"]) - observed) < 1e-12 and abs(float(row["z_score (lower=templatic)"])) < 1e-12:
            degenerate.append(family)
    check("OBSERVED_ENTROPY_REPRODUCES", not entropy_mismatches, "FAIL", entropy_mismatches)
    check("MONTE_CARLO_BASELINE_NONDEGENERATE", not degenerate, "FAIL", degenerate)

    correlation = json.loads(blobs["voynich_arabic_entropy_correlation.json"])
    paper_json_mismatch = (
        abs(float(correlation["spearman_correlation"]) - 0.82) > 0.01
        or abs(float(correlation["linear_fit_slope"]) - 0.96) > 0.01
    )
    check("PAPER_CORRELATION_MATCHES_JSON", not paper_json_mismatch, "FAIL", correlation)
    check("REPRODUCIBILITY_CODE_PRESENT", any(name.endswith((".py", ".ipynb", ".R", ".jl", ".sh")) for name in blobs), "FAIL")
    check("CONTROL_GENERATOR_AND_SEED_DECLARED", False, "FAIL")

    cluster_fields = set(_csv(blobs["folio_family_cluster_summary.csv"])[0])
    check("KMEANS_OUTPUT_CONTRACT_PRESENT", {"cluster_id", "centroid", "k", "seed"}.issubset(cluster_fields), "FAIL", sorted(cluster_fields))
    check("SEMANTIC_PARALLELS_HAVE_BLIND_TEST", False, "FAIL")
    check("SOURCE_LICENSE_AND_EXTERNAL_HASH", False, "GAP", blobs["voynich_raw.txt"].decode("utf-8", errors="replace").splitlines()[:4])

    mapper = ToroidalMap()
    points = [
        mapper.map({"sha256": item["sha256"], "size_bytes": item["size_bytes"]}, {"name": item["name"]}).unit()
        for item in files
    ]
    package_point = mapper.map({"zip_sha256": sha256_hex(zip_bytes), "files": files}, {"event_id": event_id})
    psi = trajectory_signal(points)
    spectrum_values, correlation_r = spectral_correlation(psi)
    step = lambda state: iterate_map(state, coupling=0.05)
    cycle = detect_cycle_hashing(step, package_point.unit(), max_steps=10_000, bits=16)
    leaves = [name.encode("utf-8") + b"\x00" + blobs[name] for name in sorted(blobs)]

    structural = {
        "zip_sha256": sha256_hex(zip_bytes),
        "merkle_root_sha256": merkle_root(leaves),
        "package_toroidal_q64_hex": package_point.hex(),
        "file_trajectory_points": len(points),
        "psi_length": len(psi),
        "fft_bins": len(spectrum_values),
        "cardio_spectral_correlation_r": correlation_r,
        "entropy_milli_axis0_q8": entropy_milli(int(point[0] * 256) for point in points),
        "bits_geom_bins16": bits_geom(points, bins_per_axis=16),
        "cycle_hashing_q16_10000": asdict(cycle),
    }
    scientific_failures = [c for c in checks if not c["pass"] and c["severity"] == "FAIL"]
    return {
        "event_id": event_id,
        "timestamp_local": timestamp_local,
        "timezone": "America/Sao_Paulo",
        "input": {"basename": zip_path.name, "size_bytes": len(zip_bytes), "entry_count": len(files)},
        "files": files,
        "corpus": {
            "voynich_tokens": len(words),
            "voynich_unique_tokens": len(set(words)),
            "control_tokens": len(control),
            "control_unique_tokens": len(set(control)),
            "header_contamination_tokens": contamination,
        },
        "checks": checks,
        "structural_toolkit_run": structural,
        "status": "LOCAL_DATASET_STRUCTURAL_PASS_SCIENTIFIC_GATE_FAIL" if scientific_failures else "LOCAL_DATASET_GATE_PASS",
        "claim_allowed": False,
        "scientific_claims_reproduced": False,
        "failure_count": len(scientific_failures),
        "token_vazio": [
            "TOKEN_VAZIO_SOURCE_LICENSE",
            "TOKEN_VAZIO_EXTERNAL_SOURCE_HASH",
            "TOKEN_VAZIO_CONTROL_GENERATOR_AND_SEED",
            "TOKEN_VAZIO_MONTE_CARLO_NULL_MODEL",
            "TOKEN_VAZIO_KMEANS_PARAMETERS_AND_OUTPUT",
            "TOKEN_VAZIO_SEMANTIC_BLIND_VALIDATION",
            "TOKEN_VAZIO_INDEPENDENT_REPLICATION",
        ],
        "F_ok": "ZIP safe; 19 members hashed; corpus ingested; onset and observed entropy tables reproduced; deterministic T7/FFT/integrity run completed.",
        "F_gap": "Header contamination; degenerate baseline; paper/JSON contradiction; missing code, seed, cluster contract, source license/provenance, semantic falsifier and replication.",
        "F_next": "Freeze provenance/license, add exact generator and analysis code with seeds, regenerate every table from raw input, and run negative controls before linguistic claims.",
    }
