#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import math
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
LEDGER_SCHEMA = "raf.language-matrix-commit-ledger.v1"
FIXTURE_SCHEMA = "raf.language-matrix-fixture.v1"
SHA40 = re.compile(r"^[0-9a-f]{40}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
ALLOWED_GATES = {
    "definition", "contract", "implementation", "local_test",
    "hashed_fixture", "validated_tokenizer", "out_of_sample",
    "independent_replication", "multi_repository", "domain_audit",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(65536), b""):
            digest.update(block)
    return digest.hexdigest()


def parse_sha256_file(path: Path) -> tuple[str, str]:
    parts = path.read_text(encoding="utf-8").strip().split(maxsplit=1)
    require(len(parts) == 2, "sha256 file must contain digest and filename")
    digest, filename = parts
    filename = filename.lstrip("* ")
    require(bool(SHA256.fullmatch(digest)), "invalid sha256 digest")
    require(bool(filename), "missing sha256 filename")
    return digest, filename


def validate_fixture_hash(fixture_path: Path, digest_path: Path) -> str:
    expected, filename = parse_sha256_file(digest_path)
    require(filename == fixture_path.name, "sha256 filename does not match fixture")
    actual = sha256_file(fixture_path)
    require(actual == expected, "fixture sha256 mismatch")
    return actual


def load_language_module(path: Path):
    spec = importlib.util.spec_from_file_location("language_matrix_runtime", path)
    require(spec is not None and spec.loader is not None, "cannot load language module")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def nested_close(expected: Any, actual: Any, tolerance: float = 1e-12) -> bool:
    if isinstance(expected, list) and isinstance(actual, list):
        return len(expected) == len(actual) and all(
            nested_close(left, right, tolerance)
            for left, right in zip(expected, actual)
        )
    if isinstance(expected, (int, float)) and isinstance(actual, (int, float)):
        return math.isclose(
            float(expected), float(actual), rel_tol=tolerance, abs_tol=tolerance
        )
    return expected == actual


def validate_fixture(fixture: dict[str, Any], language: Any) -> None:
    require(fixture["schema"] == FIXTURE_SCHEMA, "wrong fixture schema")
    require(fixture["claim_allowed"] is False, "fixture claim must remain false")
    require(
        fixture["scope"] == "deterministic_engineering_fixture_not_linguistic_corpus",
        "fixture scope drift",
    )
    matrix = fixture["matrix"]
    expected = fixture["expected"]
    observed = {
        "direct": language.direct_matrix(matrix),
        "inverse_relational": language.inverse_relational(matrix),
        "indirect_two_step": language.indirect_two_step(matrix),
        "reciprocal": language.reciprocal_matrix(matrix),
    }
    for key, actual in observed.items():
        require(nested_close(expected[key], actual), f"fixture mismatch: {key}")

    source = fixture["logarithmic_source"]
    restored = language.inverse_logarithmic_matrix(language.logarithmic_matrix(source))
    require(nested_close(source, restored), "logarithmic roundtrip mismatch")

    fibonacci = fixture["fibonacci"]
    require(
        language.fibonacci_windows(fibonacci["limit"]) == fibonacci["windows"],
        "fibonacci windows mismatch",
    )
    for raw_value, expected_index in fibonacci["inverse"].items():
        require(
            language.fibonacci_inverse_index(int(raw_value)) == expected_index,
            f"fibonacci inverse mismatch: {raw_value}",
        )

    levels = language.dyadic_partition(fixture["dyadic"]["length"])
    require(
        len(levels[-1]) == fixture["dyadic"]["expected_leaf_count"],
        "dyadic leaf count mismatch",
    )
    tokens = language.tokenize_whitespace(
        fixture["multiscript_text"], fixture["normalization"]
    )
    require(tokens == fixture["expected_whitespace_tokens"], "token mismatch")


def validate_ledger(ledger: dict[str, Any], fixture_digest: str) -> None:
    require(ledger["schema"] == LEDGER_SCHEMA, "wrong ledger schema")
    require(ledger["repository"] == "rafaelmeloreisnovo/RafPolimata", "wrong repo")
    require(ledger["claim_allowed"] is False, "claim_allowed must remain false")
    require(bool(SHA40.fullmatch(ledger["base_sha"])), "invalid base sha")
    require(bool(SHA40.fullmatch(ledger["observed_head_sha"])), "invalid head sha")
    require(ledger["branch_relation"]["ahead_by"] >= 1, "branch is not ahead")
    require(ledger["branch_relation"]["behind_by"] == 0, "branch drift")
    require(ledger["fixture"]["sha256"] == fixture_digest, "fixture digest drift")

    shas: set[str] = set()
    for sequence, record in enumerate(ledger["commits"], start=1):
        require(record["sequence"] == sequence, "non-contiguous sequence")
        sha = record["sha"]
        require(bool(SHA40.fullmatch(sha)), f"invalid commit sha: {sha}")
        require(sha not in shas, f"duplicate commit sha: {sha}")
        shas.add(sha)
        require(record["existence"] == "VERIFIED_GITHUB_API", "unverified commit")
        require(bool(record["message"].strip()), "missing commit message")
        require(bool(record["artifacts"]), "missing artifacts")
        for artifact in record["artifacts"]:
            path = Path(artifact)
            require(not path.is_absolute(), "artifact path must be relative")
            require(".." not in path.parts, "artifact path escapes repository")
        gates = set(record["gate_contribution"])
        require(gates <= ALLOWED_GATES, f"unknown gates: {gates - ALLOWED_GATES}")

    require(
        ledger["evidence_scope"]["runtime_reexecution"]
        == "TOKEN_VAZIO_CURRENT_CHECKOUT",
        "runtime reexecution cannot be promoted without a clean-checkout run",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate language commit evidence")
    parser.add_argument(
        "--ledger", type=Path,
        default=ROOT / "data/language/language-matrix-commit-ledger.v1.json",
    )
    parser.add_argument(
        "--fixture", type=Path,
        default=ROOT / "data/language/fixtures/language-matrix-fixture.v1.json",
    )
    parser.add_argument(
        "--fixture-sha256", type=Path,
        default=ROOT / "data/language/fixtures/language-matrix-fixture.v1.sha256",
    )
    parser.add_argument(
        "--language-module", type=Path,
        default=ROOT / "scripts/language_matrix.py",
    )
    args = parser.parse_args(argv)

    digest = validate_fixture_hash(args.fixture, args.fixture_sha256)
    fixture = read_json(args.fixture)
    ledger = read_json(args.ledger)
    validate_fixture(fixture, load_language_module(args.language_module))
    validate_ledger(ledger, digest)
    print(json.dumps({
        "schema": LEDGER_SCHEMA,
        "state": "PASS",
        "fixture_sha256": digest,
        "commit_records": len(ledger["commits"]),
        "max_material_gate": "hashed_fixture",
        "runtime_reexecution": "TOKEN_VAZIO_CURRENT_CHECKOUT",
        "claim_allowed": False,
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, KeyError, TypeError, ValueError, OSError, json.JSONDecodeError) as error:
        print(f"FAIL language-commit-evidence: {error}", file=sys.stderr)
        raise SystemExit(1)
