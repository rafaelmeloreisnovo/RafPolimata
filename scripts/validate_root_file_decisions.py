#!/usr/bin/env python3
"""Valida decisões de governança para arquivos soltos da raiz.

A regra é fechada: todo arquivo versionado na raiz que não pertence à política
canônica/prefixos deve possuir decisão explícita. O validador não move nem apaga.

O manifesto V1 principal permanece imutável quando possível. Decisões novas podem
ser anexadas em ``configs/root-file-decisions.d/*.json``; o carregamento é
lexicograficamente determinístico e rejeita paths duplicados no bundle final.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

POLICY_SCHEMA = "raf.document-governance-policy.v1"
DECISION_SCHEMA = "raf.root-file-decisions.v1"
DEFAULT_SUPPLEMENT_DIR = "configs/root-file-decisions.d"
ROUTES = {
    "MOVE_PROPOSED",
    "ARCHIVE_PROPOSED",
    "BINARY_ARTIFACT_REVIEW",
    "MOVE_AND_REFACTOR_PROPOSED",
    "SPLIT_AND_REFACTOR_PROPOSED",
    "SPLIT_REQUIRED",
    "CONVERT_TO_TYPED_BACKLOG",
    "FIX_THEN_MOVE_PROPOSED",
    "KEEP_AT_ROOT",
    "QUARANTINE_REVIEW",
}
RISKS = {"LOW", "MEDIUM", "HIGH", "CRITICAL"}


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"root-decisions: JSON inválido em {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise SystemExit(f"root-decisions: objeto JSON esperado em {path}")
    return value


def load_manifest_bundle(root: Path, primary: Path, supplement_dir: Path | None = None) -> dict[str, Any]:
    """Load primary + append-only supplements in deterministic path order."""
    base = load_json(primary)
    if base.get("schema") != DECISION_SCHEMA:
        raise SystemExit("root-decisions: schema do manifesto principal incompatível")
    decisions = list(base.get("decisions", []))
    sources = [primary.relative_to(root).as_posix() if primary.is_relative_to(root) else str(primary)]

    directory = supplement_dir or (root / DEFAULT_SUPPLEMENT_DIR)
    if not directory.is_absolute():
        directory = root / directory
    if directory.is_dir():
        for path in sorted(directory.glob("*.json"), key=lambda item: item.name.casefold()):
            supplement = load_json(path)
            if supplement.get("schema") != DECISION_SCHEMA:
                raise SystemExit(f"root-decisions: schema incompatível em suplemento {path}")
            extra = supplement.get("decisions", [])
            if not isinstance(extra, list):
                raise SystemExit(f"root-decisions: decisions deve ser lista em suplemento {path}")
            decisions.extend(extra)
            sources.append(path.relative_to(root).as_posix())

    merged = dict(base)
    merged["decisions"] = decisions
    merged["bundle_sources"] = sources
    return merged


def git_paths(root: Path) -> list[str]:
    try:
        raw = subprocess.check_output(
            ["git", "ls-files", "-z"], cwd=root, stderr=subprocess.DEVNULL
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise SystemExit(f"root-decisions: git ls-files falhou: {exc}") from exc
    return sorted(
        item.decode("utf-8", errors="surrogateescape")
        for item in raw.split(b"\0")
        if item
    )


def git_blob_sha(root: Path, path: str) -> str:
    try:
        return subprocess.check_output(
            ["git", "hash-object", "--", path],
            cwd=root,
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "TOKEN_VAZIO"


def loose_root_files(paths: list[str], policy: dict[str, Any]) -> list[str]:
    root_policy = policy.get("root_policy", {})
    allowed = set(root_policy.get("allowed_files", []))
    prefixes = tuple(root_policy.get("allowed_prefixes", []))
    result = []
    for path in paths:
        if "/" in path:
            continue
        if path in allowed or path.startswith(prefixes):
            continue
        result.append(path)
    return sorted(result, key=str.casefold)


def validate_decision(decision: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    required = {
        "path", "git_blob_sha", "kind", "content_state", "evidence_state",
        "route", "target", "area", "owner_role", "risk", "findings",
        "required_gates", "delete_allowed", "human_approval_required",
    }
    missing = sorted(required - set(decision))
    if missing:
        errors.append(f"campos ausentes: {missing}")
        return errors
    path = str(decision["path"])
    if not path or "/" in path:
        errors.append("path deve identificar arquivo da raiz")
    sha = str(decision["git_blob_sha"])
    if len(sha) != 40 or any(ch not in "0123456789abcdef" for ch in sha):
        errors.append("git_blob_sha deve ser SHA-1 Git de 40 hex")
    if decision["route"] not in ROUTES:
        errors.append(f"route inválida: {decision['route']}")
    if decision["risk"] not in RISKS:
        errors.append(f"risk inválido: {decision['risk']}")
    if decision["delete_allowed"] is not False:
        errors.append("delete_allowed deve permanecer false nesta fase")
    if decision["human_approval_required"] is not True:
        errors.append("human_approval_required deve ser true")
    if not isinstance(decision["findings"], list) or not decision["findings"]:
        errors.append("findings deve ser lista não vazia")
    if not isinstance(decision["required_gates"], list) or not decision["required_gates"]:
        errors.append("required_gates deve ser lista não vazia")
    target = str(decision["target"])
    if not target:
        errors.append("target vazio")
    if decision["route"] in {
        "MOVE_PROPOSED", "ARCHIVE_PROPOSED", "MOVE_AND_REFACTOR_PROPOSED",
        "SPLIT_AND_REFACTOR_PROPOSED", "SPLIT_REQUIRED",
        "CONVERT_TO_TYPED_BACKLOG", "FIX_THEN_MOVE_PROPOSED",
    } and "/" not in target:
        errors.append("rota de movimentação/refatoração exige destino fora da raiz")
    return errors


def validate(root: Path, policy: dict[str, Any], manifest: dict[str, Any]) -> dict[str, Any]:
    errors: list[str] = []
    warnings: list[str] = []
    if policy.get("schema") != POLICY_SCHEMA:
        errors.append("schema da política incompatível")
    if manifest.get("schema") != DECISION_SCHEMA:
        errors.append("schema do manifesto incompatível")

    decisions_raw = manifest.get("decisions", [])
    if not isinstance(decisions_raw, list):
        errors.append("decisions deve ser lista")
        decisions_raw = []

    by_path: dict[str, dict[str, Any]] = {}
    for index, decision in enumerate(decisions_raw):
        if not isinstance(decision, dict):
            errors.append(f"decisions[{index}] não é objeto")
            continue
        path = str(decision.get("path", ""))
        if path in by_path:
            errors.append(f"decisão duplicada: {path}")
        by_path[path] = decision
        for item in validate_decision(decision):
            errors.append(f"{path or f'decisions[{index}]'}: {item}")

    tracked = git_paths(root)
    tracked_set = set(tracked)
    loose = loose_root_files(tracked, policy)
    unmapped = sorted(set(loose) - set(by_path), key=str.casefold)
    obsolete = sorted(set(by_path) - set(loose), key=str.casefold)
    if unmapped:
        errors.append(f"arquivos de raiz sem decisão: {unmapped}")
    if obsolete:
        warnings.append(f"decisões sem arquivo solto correspondente: {obsolete}")

    stale_hashes: list[dict[str, str]] = []
    for path, decision in sorted(by_path.items()):
        if path not in tracked_set:
            continue
        actual = git_blob_sha(root, path)
        expected = str(decision.get("git_blob_sha", ""))
        if actual != expected:
            stale_hashes.append({"path": path, "expected": expected, "actual": actual})
    if stale_hashes:
        errors.append("hash de blob mudou; revisar conteúdo e atualizar decisão")

    critical = sorted(
        path for path, decision in by_path.items()
        if decision.get("risk") == "CRITICAL" or decision.get("route") == "QUARANTINE_REVIEW"
    )
    state = "FAIL" if errors or critical else ("REVIEW_REQUIRED" if loose else "PASS")
    return {
        "schema": "raf.root-file-decisions-validation.v1",
        "state": state,
        "claim_allowed": state == "PASS",
        "bundle_sources": manifest.get("bundle_sources", []),
        "summary": {
            "tracked_files": len(tracked),
            "loose_root_files": len(loose),
            "decisions": len(by_path),
            "unmapped": len(unmapped),
            "obsolete": len(obsolete),
            "stale_hashes": len(stale_hashes),
            "critical": len(critical),
        },
        "loose_root_files": loose,
        "unmapped": unmapped,
        "obsolete": obsolete,
        "stale_hashes": stale_hashes,
        "critical": critical,
        "errors": errors,
        "warnings": warnings,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Valida decisões para arquivos soltos da raiz")
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--policy", default="configs/document-governance.v1.json")
    parser.add_argument("--manifest", default="configs/root-file-decisions.v1.json")
    parser.add_argument("--supplement-dir", default=DEFAULT_SUPPLEMENT_DIR)
    parser.add_argument("--write", type=Path)
    args = parser.parse_args(argv)

    root = args.root.resolve()
    policy_path = Path(args.policy)
    manifest_path = Path(args.manifest)
    supplement_dir = Path(args.supplement_dir)
    if not policy_path.is_absolute():
        policy_path = root / policy_path
    if not manifest_path.is_absolute():
        manifest_path = root / manifest_path
    if not supplement_dir.is_absolute():
        supplement_dir = root / supplement_dir

    manifest = load_manifest_bundle(root, manifest_path, supplement_dir)
    report = validate(root, load_json(policy_path), manifest)
    payload = json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.write:
        target = args.write if args.write.is_absolute() else root / args.write
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)
    return 1 if report["state"] == "FAIL" else 0


if __name__ == "__main__":
    raise SystemExit(main())
