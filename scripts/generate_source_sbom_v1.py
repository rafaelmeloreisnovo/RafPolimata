#!/usr/bin/env python3
"""Generate deterministic source inventory + CycloneDX 1.7 SBOM for a Git checkout.

Governance anchor: CLOSURE_L11.

This generator inventories *tracked source material*. It does not claim that the
result is a release SBOM, dependency-resolution SBOM, legal license opinion, or
proof that a binary contains exactly these components. Those require separate
release/build evidence and human review.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

CDX_SPEC_VERSION = "1.7"
SBOM_SCHEMA = "raf.source-sbom-receipt.v1"
LICENSE_INVENTORY_SCHEMA = "raf.license-evidence-inventory.v1"
TOKEN = "TOKEN_VAZIO"
SPDX_RE = re.compile(r"SPDX-License-Identifier:\s*([^\r\n*]+)")
LICENSE_NAMES = {"license", "licence", "copying", "notice", "copyright"}
MANIFEST_NAMES = {
    "package.json", "package-lock.json", "pnpm-lock.yaml", "yarn.lock",
    "pyproject.toml", "requirements.txt", "poetry.lock", "pipfile", "pipfile.lock",
    "cargo.toml", "cargo.lock", "go.mod", "go.sum", "pom.xml", "build.gradle",
    "build.gradle.kts", "gradle.properties", "settings.gradle", "settings.gradle.kts",
    "gemfile", "gemfile.lock", "composer.json", "composer.lock", "makefile", "cmakelists.txt",
}


class SbomError(RuntimeError):
    pass


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def run_git(root: Path, *args: str, text: bool = True) -> str | bytes:
    result = subprocess.run(
        ["git", "-C", str(root), *args],
        capture_output=True,
        text=text,
        check=False,
    )
    if result.returncode != 0:
        detail = result.stderr.strip() if text else result.stderr.decode("utf-8", errors="replace").strip()
        raise SbomError(f"git {' '.join(args)} failed: {detail}")
    return result.stdout


def tracked_paths(root: Path) -> list[str]:
    raw = run_git(root, "ls-files", "-z", text=False)
    assert isinstance(raw, bytes)
    return sorted(
        item.decode("utf-8", errors="surrogateescape")
        for item in raw.split(b"\0")
        if item
    )


def head_commit(root: Path) -> str:
    value = run_git(root, "rev-parse", "HEAD")
    assert isinstance(value, str)
    value = value.strip()
    if not re.fullmatch(r"[a-f0-9]{40}", value):
        raise SbomError("HEAD is not a 40-hex commit")
    return value


def git_blob_sha(root: Path, path: str) -> str:
    value = run_git(root, "hash-object", "--", path)
    assert isinstance(value, str)
    value = value.strip()
    if not re.fullmatch(r"[a-f0-9]{40}", value):
        raise SbomError(f"invalid git blob SHA for {path}")
    return value


def file_bytes(root: Path, path: str) -> bytes:
    target = root / path
    try:
        return target.read_bytes()
    except OSError as exc:
        raise SbomError(f"cannot read tracked file {path}: {exc}") from exc


def component_ref(path: str) -> str:
    return "urn:rafpolimata:file:" + sha256_bytes(path.encode("utf-8", errors="surrogateescape"))


def is_license_evidence_path(path: str) -> bool:
    name = Path(path).name.casefold()
    stem = name.split(".", 1)[0]
    return stem in LICENSE_NAMES


def detect_spdx_expression(data: bytes) -> str | None:
    sample = data[:16384].decode("utf-8", errors="ignore")
    match = SPDX_RE.search(sample)
    if not match:
        return None
    value = match.group(1).strip().rstrip("*/ ")
    return value or None


def build(root: Path) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    root = root.resolve()
    commit = head_commit(root)
    paths = tracked_paths(root)
    if not paths:
        raise SbomError("repository has no tracked files")

    components: list[dict[str, Any]] = []
    evidence_records: list[dict[str, Any]] = []
    manifest_records: list[dict[str, Any]] = []
    aggregate_hasher = hashlib.sha256()
    total_bytes = 0

    for path in paths:
        data = file_bytes(root, path)
        digest = sha256_bytes(data)
        blob = git_blob_sha(root, path)
        total_bytes += len(data)
        aggregate_hasher.update(path.encode("utf-8", errors="surrogateescape"))
        aggregate_hasher.update(b"\0")
        aggregate_hasher.update(bytes.fromhex(digest))

        component = {
            "type": "file",
            "bom-ref": component_ref(path),
            "name": path,
            "hashes": [{"alg": "SHA-256", "content": digest}],
            "properties": [
                {"name": "rafpolimata:git:blob-sha1", "value": blob},
                {"name": "rafpolimata:file:size-bytes", "value": str(len(data))},
            ],
        }
        expression = detect_spdx_expression(data)
        if expression:
            component["properties"].append(
                {"name": "rafpolimata:license:spdx-expression-observed", "value": expression}
            )
            evidence_records.append({
                "path": path,
                "evidence_type": "SPDX_HEADER",
                "expression_observed": expression,
                "sha256": digest,
                "legal_conclusion": False,
            })
        if is_license_evidence_path(path):
            evidence_records.append({
                "path": path,
                "evidence_type": "LICENSE_LIKE_FILE",
                "sha256": digest,
                "legal_conclusion": False,
            })
        if Path(path).name.casefold() in MANIFEST_NAMES:
            manifest_records.append({
                "path": path,
                "sha256": digest,
                "parser_state": "REFERENCE_ONLY_NOT_DEPENDENCY_RESOLVED",
            })
        components.append(component)

    components.sort(key=lambda item: item["name"])
    evidence_records.sort(key=lambda item: (item["path"], item["evidence_type"]))
    manifest_records.sort(key=lambda item: item["path"])

    root_license = [item for item in evidence_records if item["evidence_type"] == "LICENSE_LIKE_FILE" and "/" not in item["path"]]
    root_license_state = "OBSERVED" if root_license else TOKEN

    bom = {
        "bomFormat": "CycloneDX",
        "specVersion": CDX_SPEC_VERSION,
        "version": 1,
        "metadata": {
            "component": {
                "type": "application",
                "bom-ref": "urn:rafpolimata:repository:" + commit,
                "name": "RafPolimata",
                "version": commit,
                "properties": [
                    {"name": "rafpolimata:inventory:scope", "value": "GIT_TRACKED_SOURCE"},
                    {"name": "rafpolimata:inventory:commit", "value": commit},
                    {"name": "rafpolimata:inventory:claim-boundary", "value": "SOURCE_ONLY_NOT_RELEASE_OR_BINARY_CONTENT_PROOF"},
                ],
            }
        },
        "components": components,
    }

    license_inventory = {
        "schema": LICENSE_INVENTORY_SCHEMA,
        "repository": "RafPolimata",
        "commit_sha": commit,
        "root_license_state": root_license_state,
        "owner_license_decision_state": "TOKEN_VAZIO_OWNER_DECISION" if not root_license else "REVIEW_REQUIRED",
        "claim_allowed": False,
        "evidence": evidence_records,
        "dependency_manifests": manifest_records,
        "limitations": [
            "License-like filenames and SPDX headers are evidence signals, not a legal compatibility opinion.",
            "Dependency manifests are inventoried but dependencies are not resolved by this generator.",
            "A source inventory does not prove inclusion in any release binary or APK.",
        ],
    }

    receipt = {
        "schema": SBOM_SCHEMA,
        "generator_version": "1.0.0",
        "cyclonedx_spec_version": CDX_SPEC_VERSION,
        "repository_commit": commit,
        "tracked_file_count": len(paths),
        "tracked_bytes": total_bytes,
        "source_set_hash": "sha256:" + aggregate_hasher.hexdigest(),
        "sbom_hash": "sha256:" + sha256_bytes(canonical_bytes(bom)),
        "license_inventory_hash": "sha256:" + sha256_bytes(canonical_bytes(license_inventory)),
        "claim_allowed": False,
        "scope": "SOURCE_INVENTORY_ONLY",
    }
    return bom, license_inventory, receipt


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate deterministic CycloneDX source SBOM and license evidence inventory")
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--sbom", type=Path, default=Path("ci/reports/source-sbom/bom.cdx.json"))
    parser.add_argument("--licenses", type=Path, default=Path("ci/reports/source-sbom/license-evidence.json"))
    parser.add_argument("--receipt", type=Path, default=Path("ci/reports/source-sbom/receipt.json"))
    args = parser.parse_args(argv)
    try:
        bom, licenses, receipt = build(args.root)
        write_json(args.sbom, bom)
        write_json(args.licenses, licenses)
        write_json(args.receipt, receipt)
    except SbomError as exc:
        print(f"source-sbom-v1: FAIL: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(receipt, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
