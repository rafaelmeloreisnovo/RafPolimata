#!/usr/bin/env python3
"""Audita a estrutura do repositório RafPolimata até uma profundidade definida.

A saída é determinística e sem dependências externas para servir como gate de
mapeamento: classifica arquivos/diretórios, detecta diretórios vazios, arquivos
soltos de raiz, referências documentais quebradas e divergências entre o índice
RAF_INDEX.md e os arquivos RAF_*.c realmente existentes.
"""
from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DEPTH = 5
IGNORED_DIRS = {".git", "__pycache__", "build_host_check"}
ROOT_ALLOWED_FILES = {
    ".gitignore",
    "README.md",
    "README_RAFAELIA_ROOT_OPTIMIZER.md",
    "RAFAELIA_MASTER_DOC.txt",
    "RAFAELIA_COMPLETE_v4.zip",
    "Arduíno.txt",
    "Arm64 Mixer leve criptografia.md",
    "L1.md",
    "RASBERY.MD",
}
ROOT_PREFIXES = (
    "RAF_",
    "raf_",
    "raiz_",
    "ci_out",
)
EXPECTED_TOP_DIRS = {
    ".github",
    "Apkc",
    "Benchmark",
    "configs",
    "data",
    "docs",
    "results",
    "scripts",
    "tools",
}


@dataclass(frozen=True)
class Entry:
    path: Path
    depth: int
    kind: str


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def visible_children(path: Path) -> list[Path]:
    return sorted(
        (child for child in path.iterdir() if child.name not in IGNORED_DIRS),
        key=lambda item: item.as_posix(),
    )


def walk_depth(max_depth: int) -> list[Entry]:
    entries: list[Entry] = []
    stack: list[tuple[Path, int]] = [(ROOT, 0)]
    while stack:
        path, depth = stack.pop()
        if depth > max_depth:
            continue
        if path != ROOT:
            entries.append(Entry(path, depth, "dir" if path.is_dir() else "file"))
        if path.is_dir() and depth < max_depth:
            for child in reversed(visible_children(path)):
                stack.append((child, depth + 1))
    return entries


def empty_dirs(max_depth: int) -> list[str]:
    return [rel(entry.path) for entry in walk_depth(max_depth) if entry.kind == "dir" and not visible_children(entry.path)]


def root_loose_files() -> list[str]:
    loose: list[str] = []
    for child in visible_children(ROOT):
        if not child.is_file():
            continue
        name = child.name
        if name in ROOT_ALLOWED_FILES or name.startswith(ROOT_PREFIXES):
            continue
        loose.append(name)
    return loose


def missing_expected_dirs() -> list[str]:
    return sorted(name for name in EXPECTED_TOP_DIRS if not (ROOT / name).is_dir())


def raf_c_files() -> list[str]:
    return sorted(path.name for path in ROOT.glob("RAF_[0-9][0-9][0-9]_*.c"))


def index_method_refs() -> list[str]:
    index = ROOT / "RAF_INDEX.md"
    if not index.exists():
        return []
    return re.findall(r"`([^`]+\.c)`", index.read_text(encoding="utf-8"))


def broken_markdown_links() -> list[str]:
    broken: list[str] = []
    pattern = re.compile(r"\[[^\]]+\]\(([^)#:]+)(?:#[^)]+)?\)")
    for path in sorted(list((ROOT / "docs").glob("*.md")) + [ROOT / "README.md"]):
        text = path.read_text(encoding="utf-8")
        for target in pattern.findall(text):
            if "://" in target or target.startswith("mailto:"):
                continue
            candidate = (path.parent / target).resolve()
            try:
                candidate.relative_to(ROOT)
            except ValueError:
                broken.append(f"{rel(path)} -> {target} (fora_da_raiz)")
                continue
            if not candidate.exists():
                broken.append(f"{rel(path)} -> {target}")
    return broken


def build_report(max_depth: int) -> tuple[str, int]:
    entries = walk_depth(max_depth)
    dirs = [entry for entry in entries if entry.kind == "dir"]
    files = [entry for entry in entries if entry.kind == "file"]
    empties = empty_dirs(max_depth)
    loose = root_loose_files()
    missing_dirs = missing_expected_dirs()
    broken_links = broken_markdown_links()
    refs = index_method_refs()
    existing_raf = raf_c_files()
    index_mismatch = bool(refs) and not any((ROOT / ref).exists() for ref in refs) and bool(existing_raf)

    lines = [
        "repository_structure_audit:",
        f"  depth: {max_depth}",
        f"  directories: {len(dirs)}",
        f"  files: {len(files)}",
        f"  root_raf_method_files: {len(existing_raf)}",
        f"  empty_directories: {len(empties)}",
        f"  root_loose_files: {len(loose)}",
        f"  missing_expected_top_dirs: {len(missing_dirs)}",
        f"  broken_markdown_links: {len(broken_links)}",
        f"  raf_index_points_to_missing_methods_dir: {str(index_mismatch).lower()}",
        "details:",
        "  empty_directories:",
        *(f"    - {item}" for item in empties),
        "  root_loose_files:",
        *(f"    - {item}" for item in loose),
        "  missing_expected_top_dirs:",
        *(f"    - {item}" for item in missing_dirs),
        "  broken_markdown_links:",
        *(f"    - {item}" for item in broken_links),
    ]
    # O mismatch do índice é informativo enquanto o repositório mantiver os 56
    # métodos no padrão RAF_###_* na raiz; falha apenas em links quebrados,
    # diretórios esperados ausentes ou arquivos raiz inesperados.
    exit_code = 1 if missing_dirs or broken_links or loose else 0
    return "\n".join(lines), exit_code


def main() -> int:
    parser = argparse.ArgumentParser(description="Audit repository layout up to a bounded depth")
    parser.add_argument("--depth", type=int, default=DEFAULT_DEPTH)
    args = parser.parse_args()
    if args.depth < 1:
        raise SystemExit("depth must be >= 1")
    report, exit_code = build_report(args.depth)
    print(report)
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
