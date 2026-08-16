#!/usr/bin/env python3
"""Auditoria estrutural L0 do RafPolimata.

Valida a árvore física, política da raiz, diretórios esperados, links Markdown e
coerência do índice RAF. A governança semântica L1–L5 permanece em
scripts/document_governance.py.
"""
from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

DEFAULT_DEPTH = 5
DEFAULT_POLICY = "configs/document-governance.v1.json"
IGNORED_DIRS = {".git", "__pycache__", "build_host_check", "node_modules", ".gradle", ".idea"}
MARKDOWN_LINK = re.compile(r"\[[^\]]+\]\(([^)\s]+)(?:\s+['\"][^'\"]*['\"])?\)")
FENCED_CODE = re.compile(r"(?ms)^[ \t]*(```+|~~~+)[^\n]*\n.*?^[ \t]*\1[ \t]*$")


@dataclass(frozen=True)
class Entry:
    path: Path
    depth: int
    kind: str


def load_policy(root: Path, policy_path: str) -> dict[str, Any]:
    path = Path(policy_path)
    if not path.is_absolute():
        path = root / path
    try:
        policy = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"repository-structure: política inválida: {exc}") from exc
    if policy.get("schema") != "raf.document-governance-policy.v1":
        raise SystemExit("repository-structure: schema de política incompatível")
    return policy


def rel(root: Path, path: Path) -> str:
    return path.relative_to(root).as_posix()


def visible_children(path: Path) -> list[Path]:
    try:
        children = [child for child in path.iterdir() if child.name not in IGNORED_DIRS]
    except OSError:
        return []
    return sorted(children, key=lambda item: item.as_posix().casefold())


def walk_depth(root: Path, max_depth: int) -> list[Entry]:
    entries: list[Entry] = []
    stack: list[tuple[Path, int]] = [(root, 0)]
    while stack:
        path, depth = stack.pop()
        if depth > max_depth:
            continue
        if path != root:
            entries.append(Entry(path, depth, "dir" if path.is_dir() else "file"))
        if path.is_dir() and depth < max_depth:
            for child in reversed(visible_children(path)):
                stack.append((child, depth + 1))
    return entries


def empty_dirs(root: Path, max_depth: int) -> list[str]:
    return [
        rel(root, entry.path)
        for entry in walk_depth(root, max_depth)
        if entry.kind == "dir" and not visible_children(entry.path)
    ]


def root_loose_files(root: Path, policy: dict[str, Any]) -> list[str]:
    root_policy = policy.get("root_policy", {})
    allowed = set(root_policy.get("allowed_files", []))
    prefixes = tuple(root_policy.get("allowed_prefixes", []))
    loose: list[str] = []
    for child in visible_children(root):
        if not child.is_file():
            continue
        if child.name in allowed or child.name.startswith(prefixes):
            continue
        loose.append(child.name)
    return loose


def expected_top_dirs(policy: dict[str, Any]) -> set[str]:
    expected = {".github", "docs", "scripts", "configs", "tests"}
    for area in policy.get("areas", []):
        for prefix in area.get("prefixes", []):
            if not prefix or "/" not in prefix:
                continue
            top = prefix.split("/", 1)[0]
            if top and not top.startswith("*"):
                expected.add(top)
    return expected


def missing_expected_dirs(root: Path, policy: dict[str, Any]) -> list[str]:
    return sorted(name for name in expected_top_dirs(policy) if not (root / name).is_dir())


def raf_c_files(root: Path) -> set[str]:
    return {path.name for path in root.glob("RAF_[0-9][0-9][0-9]_*.c")}


def index_method_refs(root: Path) -> set[str]:
    index = root / "RAF_INDEX.md"
    if not index.exists():
        return set()
    try:
        text = index.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return set()
    return set(re.findall(r"`([^`]+\.c)`", text))


def local_target_status(root: Path, source: Path, target: str) -> str:
    target = target.strip().strip("<>").split("#", 1)[0].split("?", 1)[0]
    if not target or "://" in target or target.startswith(("mailto:", "#")):
        return "external-or-anchor"
    candidates = [source.parent / target, root / target]
    for candidate in candidates:
        try:
            resolved = candidate.resolve()
            resolved.relative_to(root.resolve())
        except (OSError, ValueError):
            continue
        if resolved.exists():
            return "exists"
    return "missing"


def markdown_files(root: Path) -> list[Path]:
    files = [root / "README.md"] if (root / "README.md").is_file() else []
    docs = root / "docs"
    if docs.is_dir():
        files.extend(path for path in docs.rglob("*.md") if path.is_file())
    return sorted(set(files), key=lambda path: rel(root, path).casefold())


def markdown_link_text(text: str) -> str:
    """Return Markdown prose eligible for link validation.

    Fenced code is executable/example content, not Markdown navigation. Keeping
    it in the link regex misclassifies expressions such as `identity[Int](42)`
    as a local link to a file named `42`.
    """
    return FENCED_CODE.sub("", text)


def broken_markdown_links(root: Path) -> list[str]:
    broken: list[str] = []
    for path in markdown_files(root):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for target in MARKDOWN_LINK.findall(markdown_link_text(text)):
            if local_target_status(root, path, target) == "missing":
                broken.append(f"{rel(root, path)} -> {target}")
    return sorted(set(broken))


def raf_index_diff(root: Path) -> dict[str, list[str]]:
    existing = raf_c_files(root)
    refs = index_method_refs(root)
    referenced_names = {Path(item).name for item in refs}
    return {
        "existing_not_indexed": sorted(existing - referenced_names),
        "indexed_not_existing": sorted(referenced_names - existing),
    }


def build_report(root: Path, policy: dict[str, Any], max_depth: int) -> tuple[dict[str, Any], int]:
    entries = walk_depth(root, max_depth)
    directories = [entry for entry in entries if entry.kind == "dir"]
    files = [entry for entry in entries if entry.kind == "file"]
    empties = empty_dirs(root, max_depth)
    loose = root_loose_files(root, policy)
    missing = missing_expected_dirs(root, policy)
    broken = broken_markdown_links(root)
    raf_diff = raf_index_diff(root)

    blockers = {
        "missing_expected_top_dirs": missing,
        "broken_markdown_links": broken,
    }
    reviews = {
        "empty_directories": empties,
        "root_loose_files": loose,
        "raf_existing_not_indexed": raf_diff["existing_not_indexed"],
        "raf_indexed_not_existing": raf_diff["indexed_not_existing"],
    }
    state = "FAIL" if any(blockers.values()) else (
        "REVIEW_REQUIRED" if any(reviews.values()) else "PASS"
    )
    report = {
        "schema": "raf.repository-structure-audit.v2",
        "depth": max_depth,
        "state": state,
        "claim_allowed": state == "PASS",
        "summary": {
            "directories": len(directories),
            "files": len(files),
            "root_raf_method_files": len(raf_c_files(root)),
            "empty_directories": len(empties),
            "root_loose_files": len(loose),
            "missing_expected_top_dirs": len(missing),
            "broken_markdown_links": len(broken),
            "raf_existing_not_indexed": len(raf_diff["existing_not_indexed"]),
            "raf_indexed_not_existing": len(raf_diff["indexed_not_existing"]),
        },
        "blockers": blockers,
        "reviews": reviews,
    }
    return report, 1 if state == "FAIL" else 0


def text_report(report: dict[str, Any]) -> str:
    lines = [
        "repository_structure_audit:",
        f"  schema: {report['schema']}",
        f"  depth: {report['depth']}",
        f"  state: {report['state']}",
        f"  claim_allowed: {str(report['claim_allowed']).lower()}",
        "  summary:",
    ]
    for key, value in report["summary"].items():
        lines.append(f"    {key}: {value}")
    lines.append("  blockers:")
    for key, values in report["blockers"].items():
        lines.append(f"    {key}:")
        lines.extend(f"      - {item}" for item in values)
    lines.append("  reviews:")
    for key, values in report["reviews"].items():
        lines.append(f"    {key}:")
        lines.extend(f"      - {item}" for item in values)
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Auditoria estrutural L0")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--depth", type=int, default=DEFAULT_DEPTH)
    parser.add_argument("--policy", default=DEFAULT_POLICY)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--strict-review", action="store_true")
    args = parser.parse_args(argv)

    root = args.root.resolve()
    if args.depth < 1:
        raise SystemExit("depth must be >= 1")
    policy = load_policy(root, args.policy)
    report, code = build_report(root, policy, args.depth)
    if args.json:
        print(json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2))
    else:
        print(text_report(report))
    if args.strict_review and report["state"] != "PASS":
        return 1
    return code


if __name__ == "__main__":
    raise SystemExit(main())
