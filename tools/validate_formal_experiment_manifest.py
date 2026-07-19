#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path

REQUIRED_TOP = {"schema","manifest_id","owner_repository","claim_allowed","claim","formalization","experiment","evidence","falsifier","decision"}
ALLOWED_EVIDENCE = {"VERIFIED","TESTED","PARTIAL","TOKEN_VAZIO","BLOCKED","CONTRADICTION"}


def load(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise ValueError("root must be object")
    return data


def digest(data: dict) -> str:
    payload = json.dumps(data, sort_keys=True, ensure_ascii=False, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def validate(data: dict) -> list[str]:
    errors: list[str] = []
    missing = REQUIRED_TOP - set(data)
    if missing:
        errors.append("missing top fields: " + ", ".join(sorted(missing)))
    if data.get("schema") != "rafaelia.formal_experiment_manifest.v1":
        errors.append("invalid schema")
    if data.get("owner_repository") != "rafaelmeloreisnovo/RafPolimata":
        errors.append("invalid owner_repository")
    if data.get("claim_allowed") is not False:
        errors.append("claim_allowed must be false")

    claim = data.get("claim", {})
    if claim.get("claim_gate") not in {"BLOCKED", "REVIEW_REQUIRED"}:
        errors.append("claim gate must stay blocked/review")

    formal = data.get("formalization", {})
    for field in ("definitions", "equations", "proof_obligations"):
        if not isinstance(formal.get(field), list) or not formal[field]:
            errors.append(f"formalization.{field} required")

    experiment = data.get("experiment", {})
    if not experiment.get("target_repository") or not experiment.get("command"):
        errors.append("experiment target and command required")

    evidence = data.get("evidence", {})
    if evidence.get("status") not in ALLOWED_EVIDENCE:
        errors.append("invalid evidence status")
    artifact_sha = evidence.get("artifact_sha256")
    if artifact_sha != "TOKEN_VAZIO" and (
        not isinstance(artifact_sha, str)
        or len(artifact_sha) != 64
        or any(c not in "0123456789abcdef" for c in artifact_sha)
    ):
        errors.append("artifact_sha256 must be TOKEN_VAZIO or lowercase sha256")

    if evidence.get("status") in {"TOKEN_VAZIO", "BLOCKED", "PARTIAL"}:
        decision = data.get("decision", {})
        if not decision.get("next_action") or not decision.get("rollback"):
            errors.append("unresolved evidence requires next_action and rollback")

    forbidden = {"password", "secret", "api_key", "access_token", "private_key"}
    def walk(value: object) -> None:
        if isinstance(value, dict):
            for key, child in value.items():
                if key.lower() in forbidden:
                    errors.append(f"forbidden sensitive key: {key}")
                walk(child)
        elif isinstance(value, list):
            for child in value:
                walk(child)
    walk(data)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    try:
        data = load(args.manifest)
        errors = validate(data)
    except Exception as exc:
        print(f"BLOCKED: {exc}", file=sys.stderr)
        return 2
    report = {
        "schema": "rafaelia.formal_experiment_manifest.report.v1",
        "status": "PASS" if not errors else "FAIL",
        "manifest": str(args.manifest),
        "semantic_digest": digest(data),
        "errors": errors,
        "claim_allowed": False,
        "boundary": "structural validation only",
    }
    encoded = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    print(encoded, end="")
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(encoded, encoding="utf-8")
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
