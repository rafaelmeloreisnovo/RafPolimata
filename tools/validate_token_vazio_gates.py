#!/usr/bin/env python3
"""Validate TOKEN_VAZIO closure linkage without re-failing legacy debt.

Governance anchor: CLOSURE_L1.

Default mode preserves the historical full-repository inventory.  CI may use
``--changed-since <git-ref>`` to enforce the rule only on added/modified lines,
so pre-existing unresolved references remain visible without making every
unrelated pull request permanently red.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from hashlib import sha256
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Set, Tuple


@dataclass
class TokenVazioFinding:
    file_path: str
    line_number: int
    context: str
    has_closure: bool
    closure_file: str = ""
    severity: str = "ERROR"

    def to_dict(self) -> Dict[str, object]:
        return asdict(self)


class TokenVazioValidator:
    """Validate explicit gap markers against repository closure records."""

    TOKEN_PATTERN = re.compile(r"TOKEN_VAZIO", re.IGNORECASE)
    CLOSURE_PATTERN = re.compile(r"CLOSURE_([LG]\d+)", re.IGNORECASE)
    HUNK_PATTERN = re.compile(r"@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")
    TEXT_SUFFIXES = {".json", ".py", ".md", ".txt", ".h", ".c", ".sh"}
    SKIP_MARKERS = ("docs/generated/", "results/", ".git/", "__pycache__", ".pyc")
    ALLOWED_MISSING = {"L9"}

    # These paths form structured closure domains.  A closure binds explicit
    # unknowns to governance; it never promotes the underlying material gap.
    # Date-stamped receipts remain append-only: bind them here instead of
    # rewriting historical evidence merely to satisfy the validator.
    STRUCTURED_CLOSURE_PATHS = {
        "configs/operational-gap-topology.v1.json": "L11",
        "schemas/operational-gap-topology.v1.schema.json": "L11",
        "scripts/validate_operational_gap_topology.py": "L11",
        "tests/test_operational_gap_topology.py": "L11",
        "docs/OPERATIONAL_GAP_TOPOLOGY_V1.md": "L11",
        "docs/LICENSE_DECISION_RECORD.md": "L11",
        ".github/SECURITY.md": "L11",
        ".github/workflows/operational-gap-topology.yml": "L11",
        "data/receipts/daily-bibliography-ci-closure-20260828.v1.json": "L11",
        "tests/test_daily_bibliography_evolution.py": "L11",
        "research/formula_registry_01_20/registry.v1.json": "G1",
        "research/formula_registry_01_20/README.md": "G1",
        "research/formula_registry_01_20/reference.py": "G1",
        "scripts/validate_formula_registry_01_20.py": "G1",
        "tests/test_formula_registry_01_20.py": "G1",
    }

    def __init__(self, repo_root: Path = Path(".")):
        self.repo_root = Path(repo_root).resolve()
        self.findings: List[TokenVazioFinding] = []
        self.closure_dir = self.repo_root / "docs" / "closures"
        self.closure_map: Dict[str, bool] = self._build_closure_map()
        self.scope = "full_repository"
        self.changed_since: Optional[str] = None

    def _build_closure_map(self) -> Dict[str, bool]:
        closure_map: Dict[str, bool] = {}
        if self.closure_dir.exists():
            for closure_file in self.closure_dir.glob("CLOSURE_*.md"):
                match = re.search(r"CLOSURE_([LG]\d+)", closure_file.name, re.IGNORECASE)
                if match:
                    closure_map[match.group(1).upper()] = True
        return closure_map

    def _is_valid_closure(self, closure_id: str) -> bool:
        closure_id = closure_id.upper()
        return closure_id in self.closure_map or closure_id in self.ALLOWED_MISSING

    def _valid_closures(self, text: str) -> List[str]:
        ids = {match.group(1).upper() for match in self.CLOSURE_PATTERN.finditer(text)}
        return sorted(closure_id for closure_id in ids if self._is_valid_closure(closure_id))

    def _structured_closure(self, relative_path: str) -> str:
        closure_id = self.STRUCTURED_CLOSURE_PATHS.get(relative_path, "")
        return closure_id if closure_id and self._is_valid_closure(closure_id) else ""

    def _is_scannable(self, file_path: Path) -> bool:
        try:
            relative = file_path.resolve().relative_to(self.repo_root).as_posix()
        except (ValueError, OSError):
            return False
        if any(marker in relative for marker in self.SKIP_MARKERS):
            return False
        return file_path.suffix.lower() in self.TEXT_SUFFIXES and file_path.is_file()

    def scan_file(
        self,
        file_path: Path,
        line_numbers: Optional[Set[int]] = None,
    ) -> List[TokenVazioFinding]:
        """Scan one file.

        In diff mode, a valid closure reference anywhere in the changed file may
        govern its changed TOKEN_VAZIO lines.  Canonical structured-governance
        paths may additionally bind to a dedicated closure by exact path.  Full
        inventory mode retains line-local behavior for ordinary files while
        honoring those explicit structured bindings.
        """
        file_path = Path(file_path)
        local_findings: List[TokenVazioFinding] = []
        if not self._is_scannable(file_path):
            return local_findings

        try:
            content = file_path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            return local_findings

        relative = file_path.resolve().relative_to(self.repo_root).as_posix()
        file_closures = self._valid_closures(content) if line_numbers is not None else []
        structured = self._structured_closure(relative)
        if structured:
            file_closures = sorted(set(file_closures + [structured]))

        for line_num, line in enumerate(content.splitlines(), 1):
            if line_numbers is not None and line_num not in line_numbers:
                continue
            if not self.TOKEN_PATTERN.search(line):
                continue

            inline_valid = self._valid_closures(line)
            valid_closures = inline_valid or file_closures
            closure_id = valid_closures[0] if valid_closures else ""
            finding = TokenVazioFinding(
                file_path=relative,
                line_number=line_num,
                context=line.strip()[:160],
                has_closure=bool(closure_id),
                closure_file=f"CLOSURE_{closure_id}" if closure_id else "",
                severity="WARNING" if closure_id else "ERROR",
            )
            local_findings.append(finding)
            self.findings.append(finding)
        return local_findings

    def scan_repository(
        self,
        line_scope: Optional[Dict[str, Set[int]]] = None,
    ) -> Tuple[int, int]:
        """Scan the full repository or only selected changed lines."""
        error_count = 0
        warning_count = 0

        if line_scope is None:
            candidates: Iterable[Tuple[Path, Optional[Set[int]]]] = (
                (path, None) for path in self.repo_root.rglob("*") if path.is_file()
            )
        else:
            candidates = (
                (self.repo_root / relative, line_numbers)
                for relative, line_numbers in sorted(line_scope.items())
            )

        for file_path, line_numbers in candidates:
            for finding in self.scan_file(file_path, line_numbers=line_numbers):
                if finding.severity == "ERROR":
                    error_count += 1
                else:
                    warning_count += 1
        return error_count, warning_count

    @staticmethod
    def _decode_diff_path(raw_path: str) -> Optional[str]:
        raw_path = raw_path.strip()
        if raw_path == "/dev/null":
            return None
        if raw_path.startswith("b/"):
            raw_path = raw_path[2:]
        if raw_path.startswith('"') and raw_path.endswith('"'):
            try:
                raw_path = bytes(raw_path[1:-1], "utf-8").decode("unicode_escape")
            except UnicodeDecodeError:
                return None
        return Path(raw_path).as_posix()

    def changed_lines_since(self, base_ref: str, head_ref: str = "HEAD") -> Dict[str, Set[int]]:
        """Return new-line numbers introduced/modified since ``base_ref``."""
        result = subprocess.run(
            [
                "git",
                "-C",
                str(self.repo_root),
                "diff",
                "--unified=0",
                "--no-color",
                "--no-ext-diff",
                base_ref,
                head_ref,
                "--",
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            detail = (result.stderr or result.stdout).strip()
            raise RuntimeError(f"git_diff_failed({result.returncode}): {detail}")

        scope: Dict[str, Set[int]] = {}
        current_path: Optional[str] = None
        for line in result.stdout.splitlines():
            if line.startswith("+++ "):
                current_path = self._decode_diff_path(line[4:])
                if current_path is not None:
                    scope.setdefault(current_path, set())
                continue
            if current_path is None or not line.startswith("@@ "):
                continue
            match = self.HUNK_PATTERN.search(line)
            if not match:
                continue
            start = int(match.group(1))
            count = int(match.group(2)) if match.group(2) is not None else 1
            if count > 0:
                scope[current_path].update(range(start, start + count))
        return {path: lines for path, lines in scope.items() if lines}

    def report(self, output_file: Optional[Path] = None) -> Dict[str, object]:
        summary = {
            "total_findings": len(self.findings),
            "errors": sum(1 for finding in self.findings if finding.severity == "ERROR"),
            "warnings": sum(1 for finding in self.findings if finding.severity == "WARNING"),
            "closures_present": len(self.closure_map),
            "allowed_missing": sorted(self.ALLOWED_MISSING),
            "scope": self.scope,
            "changed_since": self.changed_since,
        }
        hash_content = {
            "findings": [finding.to_dict() for finding in self.findings],
            "summary": summary,
        }
        report_hash = sha256(
            json.dumps(hash_content, sort_keys=True, separators=(",", ":")).encode("utf-8")
        ).hexdigest()
        report: Dict[str, object] = {
            "schema": "rafaelia.token_vazio_validator.v1",
            "timestamp": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
            "repository": str(self.repo_root),
            "findings": [finding.to_dict() for finding in self.findings],
            "summary": summary,
            "status": "PASS" if summary["errors"] == 0 else "FAIL",
            "report_hash": report_hash,
        }
        if output_file:
            output_file.parent.mkdir(parents=True, exist_ok=True)
            output_file.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
            print(f"Report written to {output_file}")
        return report

    def should_halt_ci(self) -> bool:
        return any(finding.severity == "ERROR" for finding in self.findings)


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Validate TOKEN_VAZIO closure linkage")
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument("--output", type=Path)
    parser.add_argument("--summary", action="store_true")
    parser.add_argument("--strict", action="store_true")
    parser.add_argument(
        "--changed-since",
        help="Enforce only TOKEN_VAZIO references on added/modified lines since this git ref",
    )
    args = parser.parse_args(argv)

    validator = TokenVazioValidator(args.repo)
    if args.changed_since:
        try:
            scope = validator.changed_lines_since(args.changed_since)
        except RuntimeError as exc:
            print(f"ERROR: {exc}", file=sys.stderr)
            return 2
        validator.scope = "changed_lines"
        validator.changed_since = args.changed_since
        validator.scan_repository(scope)
    else:
        validator.scan_repository()

    report = validator.report(args.output)
    if args.summary or not sys.stdout.isatty():
        summary = report["summary"]
        print("\nTOKEN_VAZIO Validation Summary")
        print("=" * 50)
        print(f"Scope:    {summary['scope']}")
        print(f"Errors:   {summary['errors']}")
        print(f"Warnings: {summary['warnings']}")
        print(f"Status:   {report['status']}")
        print(f"Hash:     {report['report_hash']}")
    else:
        print(json.dumps(report, indent=2, ensure_ascii=False))

    if args.strict and validator.should_halt_ci():
        print("\nERROR: TOKEN_VAZIO validation failed. CI halting.")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
