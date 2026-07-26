#!/usr/bin/env python3
"""Validate lexical meaning and declared phoneme records without automatic G2P."""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
from pathlib import Path
import unicodedata
from typing import Any

SCHEMA = "rafaelia.semantic-lexeme.v1"


class LexicalError(RuntimeError):
    pass


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def normalize_surface(value: str) -> str:
    return unicodedata.normalize("NFC", value.strip()).casefold()


def load_jsonl(path: Path) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise LexicalError(f"cannot read lexeme registry: {exc}") from exc
    for line_no, raw in enumerate(lines, 1):
        if not raw.strip():
            continue
        try:
            value = json.loads(raw)
        except json.JSONDecodeError as exc:
            raise LexicalError(f"line {line_no}: invalid JSON: {exc}") from exc
        if not isinstance(value, dict):
            raise LexicalError(f"line {line_no}: record must be an object")
        records.append(value)
    if not records:
        raise LexicalError("lexeme registry is empty")
    return records


def validate_record(record: dict[str, Any]) -> dict[str, Any]:
    required = {
        "schema", "id", "language", "script", "surface", "normalized", "lemma",
        "part_of_speech", "morphemes", "senses", "phonology", "provenance",
        "limitations", "claim_allowed"
    }
    if set(record) != required:
        raise LexicalError(f"{record.get('id', 'unknown')}: field set mismatch")
    if record.get("schema") != SCHEMA or record.get("claim_allowed") is not False:
        raise LexicalError(f"{record.get('id', 'unknown')}: invalid schema or claim_allowed")
    for field in ("id", "language", "script", "surface", "normalized", "lemma", "part_of_speech"):
        if not isinstance(record.get(field), str) or not record[field]:
            raise LexicalError(f"{record.get('id', 'unknown')}: missing {field}")
    if normalize_surface(record["surface"]) != record["normalized"]:
        raise LexicalError(f"{record['id']}: normalized form mismatch")
    if not isinstance(record["morphemes"], list) or not all(isinstance(item, str) and item for item in record["morphemes"]):
        raise LexicalError(f"{record['id']}: invalid morphemes")
    senses = record["senses"]
    if not isinstance(senses, list) or not senses:
        raise LexicalError(f"{record['id']}: at least one sense is required")
    sense_ids: set[str] = set()
    for sense in senses:
        if not isinstance(sense, dict) or set(sense) != {"sense_id", "gloss", "semantic_tags", "domain", "state"}:
            raise LexicalError(f"{record['id']}: invalid sense object")
        if sense["sense_id"] in sense_ids:
            raise LexicalError(f"{record['id']}: duplicate sense id")
        sense_ids.add(sense["sense_id"])
        if sense["state"] not in {"CONTROLLED_CONVENTION", "HYPOTHESIS", "TOKEN_VAZIO"}:
            raise LexicalError(f"{record['id']}: invalid sense state")
        if not isinstance(sense["semantic_tags"], list):
            raise LexicalError(f"{record['id']}: semantic_tags must be a list")
    phonology = record["phonology"]
    expected_phonology = {"state", "system", "representation", "dialect", "ipa", "phonemes", "syllables", "stress_index"}
    if not isinstance(phonology, dict) or set(phonology) != expected_phonology:
        raise LexicalError(f"{record['id']}: invalid phonology object")
    if phonology["system"] != "IPA":
        raise LexicalError(f"{record['id']}: only explicit IPA declarations are accepted")
    if phonology["state"] == "DECLARED_BROAD_PHONEMIC":
        if phonology["representation"] != "PHONEMIC_BROAD" or not phonology["ipa"]:
            raise LexicalError(f"{record['id']}: broad phonemic declaration incomplete")
        if not phonology["phonemes"] or not phonology["syllables"]:
            raise LexicalError(f"{record['id']}: phonemes and syllables are required")
        if not 0 <= phonology["stress_index"] < len(phonology["syllables"]):
            raise LexicalError(f"{record['id']}: stress index out of range")
    elif phonology["state"] == "TOKEN_VAZIO_PHONEME_NOT_DECLARED":
        if phonology["ipa"] or phonology["phonemes"] or phonology["syllables"] or phonology["stress_index"] != -1:
            raise LexicalError(f"{record['id']}: TOKEN_VAZIO phonology must not contain invented values")
    else:
        raise LexicalError(f"{record['id']}: invalid phonology state")
    if not isinstance(record["limitations"], list) or not record["limitations"]:
        raise LexicalError(f"{record['id']}: limitations required")
    identity = {
        "language": record["language"],
        "normalized": record["normalized"],
        "lemma": record["lemma"],
        "senses": record["senses"],
        "phonology": record["phonology"],
    }
    return {
        "id": record["id"],
        "lexical_id": "raflx1-" + hashlib.sha256(canonical_bytes(identity)).hexdigest(),
        "surface": record["surface"],
        "language": record["language"],
        "sense_count": len(senses),
        "phoneme_count": len(phonology["phonemes"]),
        "phonology_state": phonology["state"],
        "claim_allowed": False,
    }


def validate_registry(path: Path) -> dict[str, Any]:
    records = load_jsonl(path)
    seen_ids: set[str] = set()
    seen_forms: set[tuple[str, str]] = set()
    compiled = []
    for record in records:
        if record.get("id") in seen_ids:
            raise LexicalError(f"duplicate lexeme id: {record.get('id')}")
        form_key = (str(record.get("language")), str(record.get("normalized")))
        if form_key in seen_forms:
            raise LexicalError(f"duplicate language/normalized form: {form_key}")
        seen_ids.add(str(record.get("id")))
        seen_forms.add(form_key)
        compiled.append(validate_record(record))
    return {
        "schema": "rafaelia.lexical-semantics-receipt.v1",
        "created_at": now_utc(),
        "state": "PASS_LEXICAL_SEMANTICS_AND_DECLARED_PHONEMES",
        "record_count": len(compiled),
        "sense_count": sum(item["sense_count"] for item in compiled),
        "phoneme_count": sum(item["phoneme_count"] for item in compiled),
        "automatic_grapheme_to_phoneme": False,
        "records": compiled,
        "claim_allowed": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("registry", type=Path, nargs="?", default=Path("data/semantics/lexemes.seed.v1.jsonl"))
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    try:
        receipt = validate_registry(args.registry)
        code = 0
    except LexicalError as exc:
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
