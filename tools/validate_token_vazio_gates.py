#!/usr/bin/env python3
"""
Hotfix H1: TOKEN_VAZIO Consistency Across CI Gates

Validates that all TOKEN_VAZIO states in commits are linked to explicit closure files.
Rejects PRs/commits that contain TOKEN_VAZIO without corresponding closure.

Protocol: RAFAELIA-PSC-1 (parabolic_semantic_codec.routing.v1)
Scope: Local repository scan, no external API calls
"""

import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple
from dataclasses import dataclass, asdict
from hashlib import sha256
from datetime import datetime


@dataclass
class TokenVazioFinding:
    """Represents a TOKEN_VAZIO detection."""
    file_path: str
    line_number: int
    context: str
    has_closure: bool
    closure_file: str = ""
    severity: str = "ERROR"  # ERROR or WARNING

    def to_dict(self) -> Dict:
        return asdict(self)


class TokenVazioValidator:
    """Validates TOKEN_VAZIO linkage to closure files."""

    PATTERNS = {
        "token_vazio_explicit": re.compile(r"TOKEN_VAZIO|token_vazio"),
        "closure_reference": re.compile(r"CLOSURE_[LG]\d+", flags=re.IGNORECASE),
        "closure_file": re.compile(r"docs/closures/CLOSURE_[LG]\d+"),
    }

    ALLOWED_MISSING = [
        "L9",  # 42 attractors falsified, explicitly TOKEN_VAZIO
    ]

    def __init__(self, repo_root: Path = Path(".")):
        self.repo_root = Path(repo_root).resolve()
        self.findings: List[TokenVazioFinding] = []
        self.closure_dir = self.repo_root / "docs" / "closures"
        self.closure_map: Dict[str, bool] = self._build_closure_map()

    def _build_closure_map(self) -> Dict[str, bool]:
        """Map which closures exist."""
        closure_map = {}
        if self.closure_dir.exists():
            for closure_file in self.closure_dir.glob("CLOSURE_*.md"):
                match = re.search(r"CLOSURE_([LG]\d+)", closure_file.name)
                if match:
                    closure_map[match.group(1)] = True
        return closure_map

    def scan_file(self, file_path: Path) -> List[TokenVazioFinding]:
        """Scan single file for TOKEN_VAZIO references."""
        local_findings = []

        # Skip generated and config files
        if any(x in str(file_path) for x in [
            "docs/generated/",
            "results/",
            ".git/",
            "__pycache__",
            ".pyc",
        ]):
            return local_findings

        # Only scan text files
        if file_path.suffix in [".json", ".py", ".md", ".txt", ".h", ".c", ".sh"]:
            try:
                if not file_path.exists():
                    return local_findings

                content = file_path.read_text(encoding="utf-8", errors="ignore")
                lines = content.split("\n")

                for line_num, line in enumerate(lines, 1):
                    if self.PATTERNS["token_vazio_explicit"].search(line):
                        # Check if closure is referenced
                        has_closure_ref = bool(
                            self.PATTERNS["closure_reference"].search(line)
                        )

                        # Extract closure ID if present (e.g., "L0", "L1", etc.)
                        closure_match = self.PATTERNS["closure_reference"].search(line)
                        closure_full = closure_match.group(0) if closure_match else ""  # e.g., "CLOSURE_L0"
                        # Extract just the ID part (e.g., "L0")
                        closure_id_match = re.search(r"([LG]\d+)", closure_full)
                        closure_id = closure_id_match.group(1) if closure_id_match else ""

                        # Check if closure file exists or is allowed to be missing
                        closure_exists = (
                            closure_id in self.closure_map or
                            closure_id in self.ALLOWED_MISSING
                        )

                        severity = "WARNING" if (has_closure_ref and closure_exists) else "ERROR"

                        finding = TokenVazioFinding(
                            file_path=str(file_path.relative_to(self.repo_root)),
                            line_number=line_num,
                            context=line.strip()[:100],
                            has_closure=has_closure_ref and closure_exists,
                            closure_file=f"CLOSURE_{closure_id}" if closure_id else "",
                            severity=severity,
                        )
                        local_findings.append(finding)
                        self.findings.append(finding)
            except Exception as e:
                # Silently skip unreadable files
                pass

        return local_findings

    def scan_repository(self) -> Tuple[int, int]:
        """Scan entire repository. Returns (error_count, warning_count)."""
        error_count = 0
        warning_count = 0

        for file_path in self.repo_root.rglob("*"):
            if file_path.is_file():
                findings = self.scan_file(file_path)
                self.findings.extend(findings)

                for finding in findings:
                    if finding.severity == "ERROR":
                        error_count += 1
                    else:
                        warning_count += 1

        return error_count, warning_count

    def report(self, output_file: Path = None) -> Dict:
        """Generate report."""
        # Compute hash over findings only (timestamp-independent)
        hash_content = {
            "findings": [f.to_dict() for f in self.findings],
            "summary": {
                "total_findings": len(self.findings),
                "errors": sum(1 for f in self.findings if f.severity == "ERROR"),
                "warnings": sum(1 for f in self.findings if f.severity == "WARNING"),
                "closures_present": len(self.closure_map),
                "allowed_missing": self.ALLOWED_MISSING,
            },
        }
        hash_json = json.dumps(hash_content, sort_keys=True)
        report_hash = sha256(hash_json.encode()).hexdigest()

        report = {
            "schema": "rafaelia.token_vazio_validator.v1",
            "timestamp": datetime.utcnow().isoformat() + "Z",
            "repository": str(self.repo_root.absolute()),
            "findings": [f.to_dict() for f in self.findings],
            "summary": {
                "total_findings": len(self.findings),
                "errors": sum(1 for f in self.findings if f.severity == "ERROR"),
                "warnings": sum(1 for f in self.findings if f.severity == "WARNING"),
                "closures_present": len(self.closure_map),
                "allowed_missing": self.ALLOWED_MISSING,
            },
            "status": "PASS" if all(f.severity != "ERROR" for f in self.findings) else "FAIL",
            "report_hash": report_hash,
        }

        if output_file:
            output_file.write_text(json.dumps(report, indent=2))
            print(f"Report written to {output_file}")

        return report

    def should_halt_ci(self) -> bool:
        """Return True if CI should halt (errors present)."""
        return any(f.severity == "ERROR" for f in self.findings)


def main():
    """CLI entry point."""
    import argparse

    parser = argparse.ArgumentParser(
        description="Validate TOKEN_VAZIO gates across repository"
    )
    parser.add_argument(
        "--repo",
        type=Path,
        default=Path("."),
        help="Repository root (default: current directory)"
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Write report to JSON file"
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        help="Print summary only"
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit with error code 1 if any errors found"
    )

    args = parser.parse_args()

    validator = TokenVazioValidator(args.repo)
    error_count, warning_count = validator.scan_repository()

    report = validator.report(args.output)

    if args.summary or not sys.stdout.isatty():
        print(f"\nTOKEN_VAZIO Validation Summary")
        print(f"{'=' * 50}")
        print(f"Errors:   {report['summary']['errors']}")
        print(f"Warnings: {report['summary']['warnings']}")
        print(f"Status:   {report['status']}")
        print(f"Hash:     {report['report_hash']}")
    else:
        print(json.dumps(report, indent=2))

    if args.strict and validator.should_halt_ci():
        print("\nERROR: TOKEN_VAZIO validation failed. CI halting.")
        sys.exit(1)

    return 0


if __name__ == "__main__":
    sys.exit(main())
