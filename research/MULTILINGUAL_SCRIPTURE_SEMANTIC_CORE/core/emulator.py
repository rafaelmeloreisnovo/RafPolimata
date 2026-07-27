#!/usr/bin/env python3
"""Deterministic, disabled-by-default multilingual scripture core emulator.

The emulator validates annotated fixtures. It does not import corpora, infer
brain states, reconstruct historical pronunciation, or claim authorial intent.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
import unicodedata
from collections import Counter, defaultdict, deque
from pathlib import Path
from typing import Any, Iterable

CORE_SCHEMA = "rafpolimata.multilingual-scripture-core.config.v1"
FIXTURE_SCHEMA = "rafpolimata.multilingual-scripture-fixture.v1"
RECEIPT_SCHEMA = "rafpolimata.multilingual-scripture-receipt.v1"
TOKEN_VAZIO = "TOKEN_VAZIO"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def sha256_json(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def normalize_nfc(text: str) -> str:
    return unicodedata.normalize("NFC", text)


def grapheme_clusters_approx(text: str) -> list[str]:
    """Approximate grapheme clusters with stdlib only.

    This is not a full implementation of Unicode UAX #29. It is intentionally
    named and reported as an approximation.
    """
    clusters: list[str] = []
    for char in normalize_nfc(text):
        if unicodedata.combining(char) and clusters:
            clusters[-1] += char
        else:
            clusters.append(char)
    return clusters


def tokenize_annotated(unit: dict[str, Any]) -> list[dict[str, Any]]:
    return list(unit["grammar"]["tokens"])


def entropy(values: Iterable[str]) -> float:
    items = list(values)
    if not items:
        return 0.0
    counts = Counter(items)
    total = len(items)
    return -sum((count / total) * math.log2(count / total) for count in counts.values())


def script_votes(text: str) -> Counter[str]:
    votes: Counter[str] = Counter()
    for char in text:
        if char.isspace() or unicodedata.category(char).startswith("P"):
            continue
        name = unicodedata.name(char, "")
        if "HEBREW" in name:
            votes["Hebr"] += 1
        elif "GREEK" in name:
            votes["Grek"] += 1
        elif "LATIN" in name:
            votes["Latn"] += 1
        elif char.isdigit():
            votes["Zyyy"] += 1
        else:
            votes["Unknown"] += 1
    return votes


def dominant_script(text: str) -> str:
    votes = script_votes(text)
    if not votes:
        return "Zyyy"
    return votes.most_common(1)[0][0]


def dependency_distances(tokens: list[dict[str, Any]]) -> list[int]:
    distances: list[int] = []
    for index, token in enumerate(tokens):
        head = token.get("head")
        require(isinstance(head, int), f"token {index}: head must be integer")
        require(0 <= head < len(tokens), f"token {index}: head outside token range")
        if head != index:
            distances.append(abs(index - head))
    return distances


def validate_config(config: dict[str, Any]) -> dict[str, dict[str, Any]]:
    require(config.get("schema") == CORE_SCHEMA, "wrong core schema")
    require(config.get("enabled") is False, "core must remain disabled")
    require(config.get("execution_mode") == "EMULATION_ONLY", "runtime activation forbidden")
    require(config.get("claim_allowed") is False, "claim_allowed must remain false")
    require(config.get("corpus_import_allowed") is False, "corpus import must remain disabled")
    require(config.get("network_access_allowed") is False, "network access must remain disabled")
    require(config["normalization"]["storage"] == "NFC", "NFC storage is required")
    require(config["normalization"]["compatibility_folding"] is False, "NFKC folding forbidden")

    languages = config.get("languages")
    require(isinstance(languages, list) and len(languages) == 4, "exactly four language profiles required")
    by_tag: dict[str, dict[str, Any]] = {}
    for language in languages:
        tag = language["tag"]
        require(tag not in by_tag, f"duplicate language tag: {tag}")
        by_tag[tag] = language
    require(set(by_tag) == {"pt-BR", "hbo", "arc", "grc"}, "language registry mismatch")
    return by_tag


def validate_unit(unit: dict[str, Any], languages: dict[str, dict[str, Any]]) -> list[str]:
    warnings: list[str] = []
    unit_id = unit["id"]
    tag = unit["language_tag"]
    require(tag in languages, f"{unit_id}: unsupported language tag {tag}")
    require(unit["script"] == languages[tag]["script"], f"{unit_id}: configured script mismatch")
    require(unit["text"] == normalize_nfc(unit["text"]), f"{unit_id}: text must be NFC")
    detected = dominant_script(unit["text"])
    require(detected == unit["script"], f"{unit_id}: detected script {detected} != {unit['script']}")

    tokens = tokenize_annotated(unit)
    require(tokens, f"{unit_id}: annotated tokens required")
    surfaces = [token["surface"] for token in tokens]
    for index, token in enumerate(tokens):
        require(bool(token.get("surface")), f"{unit_id}: token {index} surface required")
        require(bool(token.get("lemma")), f"{unit_id}: token {index} lemma required")
        require(bool(token.get("pos")), f"{unit_id}: token {index} pos required")
        require(isinstance(token.get("morphology"), list), f"{unit_id}: token {index} morphology list required")
    dependency_distances(tokens)

    phonetics = unit["phonetics"]
    require(phonetics.get("approximate") is True, f"{unit_id}: pronunciation must be explicitly approximate")
    require(bool(phonetics.get("profile")), f"{unit_id}: pronunciation profile required")
    if languages[tag]["historical_pronunciation"]:
        require(
            TOKEN_VAZIO in phonetics.get("ipa", "") or "RECONSTRUCTION" in phonetics.get("profile", "") or "PROFILE_REQUIRED" in phonetics.get("profile", ""),
            f"{unit_id}: historical pronunciation certainty is not permitted",
        )

    if tag == "arc":
        dialect = unit["grammar"].get("dialect")
        require(bool(dialect) and not str(dialect).startswith(TOKEN_VAZIO), f"{unit_id}: Aramaic dialect required")

    source = unit["source"]
    for field in ("edition", "license", "status"):
        require(bool(source.get(field)), f"{unit_id}: source.{field} required")
    if source["status"] == TOKEN_VAZIO:
        warnings.append(f"{unit_id}: source corpus remains TOKEN_VAZIO")

    normalized_text = normalize_nfc(unit["text"])
    for surface in surfaces:
        require(surface in normalized_text or TOKEN_VAZIO in surface, f"{unit_id}: token surface absent from text: {surface}")
    return warnings


def validate_relations(
    relations: list[dict[str, Any]],
    unit_ids: set[str],
    allowed_types: set[str],
) -> list[str]:
    warnings: list[str] = []
    relation_ids: set[str] = set()
    for relation in relations:
        relation_id = relation["id"]
        require(relation_id not in relation_ids, f"duplicate relation id: {relation_id}")
        relation_ids.add(relation_id)
        require(relation["source"] in unit_ids, f"{relation_id}: unknown source")
        require(relation["target"] in unit_ids, f"{relation_id}: unknown target")
        require(relation["type"] in allowed_types, f"{relation_id}: unsupported relation type")
        require(relation.get("authorial_intent_claimed") is False, f"{relation_id}: authorial intent claim forbidden")
        require(isinstance(relation.get("evidence"), list), f"{relation_id}: evidence list required")
        require(isinstance(relation.get("limitations"), list), f"{relation_id}: limitations list required")
        if relation["type"] in {"thematic", "allusion", "hypothesis"}:
            require(relation["status"] in {"HYPOTHESIS", "TOKEN_VAZIO", "DERIVED"}, f"{relation_id}: soft relation overclaimed")
        if relation["status"] in {"HYPOTHESIS", "TOKEN_VAZIO"}:
            warnings.append(f"{relation_id}: relation remains {relation['status']}")
    return warnings


def connected_components(unit_ids: set[str], relations: list[dict[str, Any]]) -> int:
    adjacency: dict[str, set[str]] = {unit_id: set() for unit_id in unit_ids}
    for relation in relations:
        source = relation["source"]
        target = relation["target"]
        adjacency[source].add(target)
        adjacency[target].add(source)
    unseen = set(unit_ids)
    components = 0
    while unseen:
        components += 1
        root = unseen.pop()
        queue: deque[str] = deque([root])
        while queue:
            node = queue.popleft()
            for neighbor in adjacency[node]:
                if neighbor in unseen:
                    unseen.remove(neighbor)
                    queue.append(neighbor)
    return components


def unit_metrics(unit: dict[str, Any]) -> dict[str, Any]:
    tokens = tokenize_annotated(unit)
    clusters = [cluster for cluster in grapheme_clusters_approx(unit["text"]) if not cluster.isspace()]
    distances = dependency_distances(tokens)
    morphology_items = sum(len(token["morphology"]) for token in tokens)
    ipa = unit["phonetics"]["ipa"]
    phonetic_coverage = 0.0 if TOKEN_VAZIO in ipa else 1.0
    return {
        "unit_id": unit["id"],
        "language_tag": unit["language_tag"],
        "canonical_passage": unit["canonical_passage"],
        "text_sha256": hashlib.sha256(unit["text"].encode("utf-8")).hexdigest(),
        "token_count": len(tokens),
        "grapheme_count_approx": len(clusters),
        "mean_grapheme_length_per_token": round(len(clusters) / len(tokens), 6),
        "clause_count": unit["grammar"]["clause_count"],
        "max_dependency_distance": max(distances, default=0),
        "mean_dependency_distance": round(sum(distances) / len(distances), 6) if distances else 0.0,
        "morphological_density": round(morphology_items / len(tokens), 6),
        "phonetic_annotation_coverage": phonetic_coverage,
        "character_entropy": round(entropy(clusters), 6),
        "token_entropy": round(entropy(token["lemma"] for token in tokens), 6),
        "pos_signature": dict(sorted(Counter(token["pos"] for token in tokens).items())),
        "neurocognitive_interpretation": "OBSERVABLE_PROXY_ONLY_NO_BRAIN_STATE_INFERENCE"
    }


def build_receipt(config: dict[str, Any], fixture: dict[str, Any]) -> dict[str, Any]:
    languages = validate_config(config)
    require(fixture.get("schema") == FIXTURE_SCHEMA, "wrong fixture schema")
    require(fixture.get("claim_allowed") is False, "fixture claim_allowed must remain false")
    units = fixture.get("units")
    relations = fixture.get("relations")
    require(isinstance(units, list) and units, "non-empty units required")
    require(isinstance(relations, list), "relations list required")

    unit_ids: set[str] = set()
    warnings: list[str] = []
    passage_languages: dict[str, set[str]] = defaultdict(set)
    metrics: list[dict[str, Any]] = []
    for unit in units:
        unit_id = unit["id"]
        require(unit_id not in unit_ids, f"duplicate unit id: {unit_id}")
        unit_ids.add(unit_id)
        warnings.extend(validate_unit(unit, languages))
        passage_languages[unit["canonical_passage"]].add(unit["language_tag"])
        metrics.append(unit_metrics(unit))

    warnings.extend(validate_relations(relations, unit_ids, set(config["relation_types"])))
    alignment = {
        passage: {
            "languages_present": sorted(tags),
            "coverage_of_four_languages": round(len(tags) / 4.0, 6),
            "full_four_language_alignment": len(tags) == 4,
        }
        for passage, tags in sorted(passage_languages.items())
    }
    for passage, state in alignment.items():
        if not state["full_four_language_alignment"]:
            warnings.append(f"{passage}: TOKEN_VAZIO_FULL_FOUR_LANGUAGE_ALIGNMENT")

    relation_counts = dict(sorted(Counter(relation["type"] for relation in relations).items()))
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "state": "PASS_WITH_TOKEN_VAZIO" if warnings else "PASS",
        "enabled": False,
        "execution_mode": "EMULATION_ONLY",
        "claim_allowed": False,
        "config_sha256": sha256_json(config),
        "fixture_sha256": sha256_json(fixture),
        "language_registry": sorted(languages),
        "unit_count": len(units),
        "relation_count": len(relations),
        "connected_components": connected_components(unit_ids, relations),
        "relation_counts": relation_counts,
        "alignment": alignment,
        "unit_metrics": sorted(metrics, key=lambda item: item["unit_id"]),
        "warnings": sorted(set(warnings)),
        "proof_boundary": {
            "validated": [
                "configuration_invariants",
                "NFC_storage",
                "annotated_token_structure",
                "script_profile",
                "relation_graph_integrity",
                "explicit_pronunciation_approximation",
                "deterministic_hash_receipt"
            ],
            "not_validated": [
                "corpus_authenticity",
                "edition_license_completeness",
                "historical_pronunciation_accuracy",
                "translation_equivalence",
                "authorial_intent",
                "human_comprehension",
                "brain_state_or_neural_activation",
                "doctrinal_truth"
            ]
        }
    }
    receipt["receipt_sha256"] = sha256_json(receipt)
    return receipt


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def main(argv: list[str] | None = None) -> int:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Emulate the disabled multilingual scripture semantic core")
    parser.add_argument("--config", type=Path, default=root / "core_config.v1.json")
    parser.add_argument("--fixture", type=Path, default=root / "fixtures/demo_units.v1.json")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)

    receipt = build_receipt(load_json(args.config), load_json(args.fixture))
    encoded = json.dumps(receipt, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        sys.stdout.write(encoded)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, KeyError, TypeError, ValueError, OSError, json.JSONDecodeError) as error:
        print(f"FAIL multilingual-scripture-core: {error}", file=sys.stderr)
        raise SystemExit(1)
