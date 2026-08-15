#!/usr/bin/env python3
"""
Ecosystem Matrix Inventory Tool

Builds authenticated, reproducible inventory of matrices/tensors in accessible repos.
Focuses on precision: records metadata with TOKEN_VAZIO for unknown fields.
"""

from __future__ import annotations

import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any, Optional


# Artifact type patterns
ARTIFACT_PATTERNS = {
    "matrix": r"\bmatrix|matriz|relmat|adjacency",
    "tensor": r"\btensor|tensore",
    "graph": r"\bgraph|grafo|dag",
    "embedding": r"\bembedding|embeddings",
    "transform": r"\btransform|transformation",
    "adjacency": r"\badjacency|adjacencia",
}

# Dimensional hints (very weak signal - only recorded as TOKEN_VAZIO or raw findings)
DIMENSIONAL_PATTERNS = {
    "T^7": r"T\^7|T7|7-dimensional|7d",
    "42_attractors": r"42\s+attractors?|attractors?\s+42",
    "10x10x10": r"10x10x10|10\*10\*10",
}

# Common file extensions for data artifacts
DATA_EXTENSIONS = {
    ".json", ".jsonl", ".yaml", ".yml", ".csv", ".tsv",
    ".npy", ".npz", ".h5", ".hdf5",
    ".c", ".h", ".cpp", ".hpp",
    ".py", ".rs", ".go", ".java",
}


class RepositoryScanner:
    """Scans a single repository for matrix artifacts."""

    def __init__(self, repo_path: Path | str, repo_name: str, repo_url: str = ""):
        self.repo_path = Path(repo_path)
        self.repo_name = repo_name
        self.repo_url = repo_url
        self.git_commit = self._get_git_commit()

    def _get_git_commit(self) -> str:
        """Get current git commit SHA."""
        try:
            result = subprocess.run(
                ["git", "-C", str(self.repo_path), "rev-parse", "HEAD"],
                capture_output=True,
                text=True,
                timeout=5,
            )
            if result.returncode == 0:
                return result.stdout.strip()
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass
        return "TOKEN_VAZIO"

    def sha256_file(self, path: Path) -> str:
        """Calculate SHA256 hash of file."""
        try:
            sha = hashlib.sha256()
            with open(path, "rb") as f:
                while True:
                    chunk = f.read(65536)
                    if not chunk:
                        break
                    sha.update(chunk)
            return sha.hexdigest()
        except (OSError, IOError):
            return "TOKEN_VAZIO"

    def detect_artifact_type(self, path: Path, content: str | None) -> str:
        """Detect artifact type from name and content patterns."""
        name_lower = path.name.lower()

        for artifact_type, pattern in ARTIFACT_PATTERNS.items():
            if re.search(pattern, name_lower, re.IGNORECASE):
                return artifact_type

        if content:
            content_lower = content.lower()
            for artifact_type, pattern in ARTIFACT_PATTERNS.items():
                if re.search(pattern, content_lower, re.IGNORECASE):
                    return artifact_type

        return "TOKEN_VAZIO"

    def detect_dimensional_hints(self, path: Path, content: str | None) -> dict[str, bool]:
        """Detect dimensional patterns (recorded as findings only, not claims)."""
        hints = {key: False for key in DIMENSIONAL_PATTERNS.keys()}

        name_lower = path.name.lower()
        for key, pattern in DIMENSIONAL_PATTERNS.items():
            if re.search(pattern, name_lower, re.IGNORECASE):
                hints[key] = True

        if content:
            content_lower = content.lower()
            for key, pattern in DIMENSIONAL_PATTERNS.items():
                if re.search(pattern, content_lower, re.IGNORECASE):
                    hints[key] = True

        return hints

    def scan(self, max_bytes: int = 1_000_000) -> list[dict[str, Any]]:
        """Scan repository for matrix artifacts."""
        artifacts = []

        if not self.repo_path.is_dir():
            return artifacts

        try:
            for path in sorted(self.repo_path.rglob("*")):
                if not path.is_file():
                    continue

                # Skip common excludes
                parts = path.relative_to(self.repo_path).parts
                if any(part.startswith(".") for part in parts):
                    continue

                # Check if file matches data extensions or has searchable content
                if path.suffix not in DATA_EXTENSIONS and path.suffix not in {".txt", ".md"}:
                    continue

                # Read file content (bounded)
                content = None
                try:
                    if path.stat().st_size <= max_bytes:
                        content = path.read_text(encoding="utf-8", errors="replace")
                except (OSError, ValueError):
                    pass

                # Look for matrix/tensor indicators
                artifact_type = self.detect_artifact_type(path, content)
                if artifact_type == "TOKEN_VAZIO" and path.suffix not in {".json", ".yaml", ".yml", ".csv"}:
                    continue

                # Record artifact
                rel_path = path.relative_to(self.repo_path).as_posix()
                file_hash = self.sha256_file(path)
                hints = self.detect_dimensional_hints(path, content)

                artifact = {
                    "repository": self.repo_name,
                    "path": rel_path,
                    "commit": self.git_commit,
                    "artifact_type": artifact_type,
                    "shape": "TOKEN_VAZIO",
                    "dtype": "TOKEN_VAZIO",
                    "units": "TOKEN_VAZIO",
                    "language_domain": "TOKEN_VAZIO",
                    "operators": [],
                    "provenance": "repository_scan",
                    "license": "TOKEN_VAZIO",
                    "hash": file_hash,
                    "status": "REFERENCE",
                    "dimensional_hints": {k: v for k, v in hints.items() if v},
                }

                artifacts.append(artifact)

        except (OSError, PermissionError):
            pass

        return artifacts


def load_repository_list(config_path: Path | str) -> list[dict[str, str]]:
    """Load list of repositories to scan from config."""
    try:
        config_path = Path(config_path)
        if config_path.exists():
            content = config_path.read_text(encoding="utf-8")
            data = json.loads(content)
            if isinstance(data, dict) and "repositories" in data:
                return data["repositories"]
    except (OSError, json.JSONDecodeError):
        pass
    return []


def build_inventory(
    repo_paths: list[tuple[str, Path | str]],
    output_path: Path | str,
) -> dict[str, Any]:
    """Build inventory from multiple repositories."""
    inventory = {
        "schema_version": "1.0.0",
        "generated_at": __import__("datetime").datetime.utcnow().isoformat() + "Z",
        "repositories": [],
        "artifacts": [],
        "summary": {
            "total_repositories": len(repo_paths),
            "total_artifacts": 0,
            "by_type": {},
            "by_status": {},
            "coverage_notes": "Repositories enumerated from filesystem; may not reflect all accessible repos",
        },
    }

    artifact_type_counts = {}
    status_counts = {}

    for repo_name, repo_path in repo_paths:
        scanner = RepositoryScanner(repo_path, repo_name)
        artifacts = scanner.scan()

        if artifacts:
            inventory["repositories"].append({
                "name": repo_name,
                "path": str(repo_path),
                "commit": scanner.git_commit,
                "artifact_count": len(artifacts),
            })

            for artifact in artifacts:
                inventory["artifacts"].append(artifact)
                artifact_type = artifact["artifact_type"]
                status = artifact["status"]

                artifact_type_counts[artifact_type] = artifact_type_counts.get(artifact_type, 0) + 1
                status_counts[status] = status_counts.get(status, 0) + 1

    inventory["summary"]["total_artifacts"] = len(inventory["artifacts"])
    inventory["summary"]["by_type"] = dict(sorted(artifact_type_counts.items()))
    inventory["summary"]["by_status"] = dict(sorted(status_counts.items()))

    # Write inventory
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Write as JSONL for easier parsing
    with open(output_path, "w", encoding="utf-8") as f:
        for artifact in inventory["artifacts"]:
            f.write(json.dumps(artifact, separators=(",", ":"), sort_keys=True) + "\n")

    # Also write summary
    summary_path = output_path.with_suffix(".summary.json")
    with open(summary_path, "w", encoding="utf-8") as f:
        json.dump(
            {
                "schema_version": inventory["schema_version"],
                "generated_at": inventory["generated_at"],
                "repositories": inventory["repositories"],
                "summary": inventory["summary"],
            },
            f,
            indent=2,
            ensure_ascii=False,
        )

    return inventory


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Inventory ecosystem matrices/tensors")
    parser.add_argument("--repo", action="append", type=str, help="REPO_NAME=PATH (can be repeated)")
    parser.add_argument("--config", type=str, help="Path to config file with repositories list")
    parser.add_argument("--output", required=True, help="Output JSONL file path")

    args = parser.parse_args()

    repo_paths = []

    # Parse command-line repos
    if args.repo:
        for repo_spec in args.repo:
            if "=" not in repo_spec:
                print(f"ERROR: --repo format is REPO_NAME=PATH", file=sys.stderr)
                sys.exit(1)
            name, path = repo_spec.split("=", 1)
            repo_paths.append((name.strip(), Path(path.strip()).expanduser()))

    # Load from config
    if args.config:
        config_repos = load_repository_list(args.config)
        for repo in config_repos:
            if "name" in repo and "path" in repo:
                repo_paths.append((repo["name"], Path(repo["path"]).expanduser()))

    if not repo_paths:
        print("ERROR: No repositories specified", file=sys.stderr)
        sys.exit(1)

    inventory = build_inventory(repo_paths, args.output)
    print(f"Inventory complete: {inventory['summary']['total_artifacts']} artifacts found")
    sys.exit(0)
