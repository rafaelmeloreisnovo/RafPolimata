#!/usr/bin/env python3
"""Deterministic, stdlib-only pre-audit for RAFAELIA library assimilation.

The tool never executes candidate code. It inventories source bytes and runtime
signals, then emits a JSON receipt. A clean scan is only CANDIDATE status; it is
not proof of freestanding equivalence.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

SCHEMA = "rafaelia.library.assimilation.audit.v1"
MAX_FILE_BYTES = 4 * 1024 * 1024
TEXT_SUFFIXES = {
    ".s", ".S", ".asm", ".c", ".h", ".cc", ".cpp", ".cxx", ".hpp",
    ".rs", ".kt", ".java", ".py", ".sh", ".pl", ".js", ".jsx",
    ".php", ".go", ".rb", ".swift", ".groovy", ".clj", ".comp",
    ".glsl", ".cl", ".hlsl", ".wgsl", ".dsp",
}
LICENSE_NAMES = {
    "license", "license.txt", "license.md", "copying", "copying.txt",
    "notice", "notice.txt",
}
DIRECT_CANDIDATE_LANGS = {"asm", "c", "cpp", "rs", "glsl", "cl", "hlsl", "wgsl", "dsp"}


@dataclass(frozen=True)
class Signal:
    category: str
    regex: re.Pattern[str]
    severity: str


SIGNALS = (
    Signal("heap", re.compile(r"\b(malloc|calloc|realloc|free|operator\s+new|operator\s+delete|Box::new|Vec::with_capacity)\b"), "BLOCK"),
    Signal("managed_gc", re.compile(r"\b(System\.gc|gc\.collect|runtime\.GC|GarbageCollector|java\.lang|kotlin\.)\b"), "BLOCK"),
    Signal("exception_unwind", re.compile(r"\b(throw|catch|try|panic!|raise|rescue|except)\b"), "REWRITE"),
    Signal("thread_or_scheduler", re.compile(r"\b(pthread_|std::thread|Thread\s*\(|async\s+fn|tokio::|goroutine|go\s+[A-Za-z_])"), "REVIEW"),
    Signal("dynamic_loading", re.compile(r"\b(dlopen|dlsym|LoadLibrary|GetProcAddress|importlib|Class\.forName|eval\s*\(|exec\s*\()"), "BLOCK"),
    Signal("external_process", re.compile(r"\b(system|popen|subprocess\.|ProcessBuilder|Runtime\.getRuntime|child_process)\b"), "BLOCK"),
    Signal("hosted_io", re.compile(r"\b(printf|fprintf|fopen|iostream|console\.log|print\s*\(|System\.out|os\.(open|read|write))\b"), "REVIEW"),
    Signal("reflection_or_rtti", re.compile(r"\b(dynamic_cast|typeid|reflect\.|getClass\s*\(|__getattr__)\b"), "BLOCK"),
    Signal("runtime_container", re.compile(r"\b(std::(vector|string|map|unordered_map)|ArrayList|HashMap|Promise|Object\b|dict\s*\(|list\s*\()"), "REWRITE"),
)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def candidate_files(root: Path) -> Iterable[Path]:
    if root.is_file():
        yield root
        return
    for path in sorted(root.rglob("*"), key=lambda p: p.as_posix()):
        if path.is_file() and (path.suffix in TEXT_SUFFIXES or path.name.lower() in LICENSE_NAMES):
            yield path


def scan_file(path: Path, root: Path) -> dict[str, object]:
    raw = path.read_bytes()
    relative = path.name if root.is_file() else path.relative_to(root).as_posix()
    record: dict[str, object] = {
        "path": relative,
        "bytes": len(raw),
        "sha256": sha256_bytes(raw),
        "signals": [],
    }
    if len(raw) > MAX_FILE_BYTES:
        record["signals"] = [{"category": "oversize", "severity": "REVIEW", "line": 0}]
        return record
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        record["signals"] = [{"category": "non_utf8", "severity": "REVIEW", "line": 0}]
        return record

    findings: list[dict[str, object]] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        for signal in SIGNALS:
            if signal.regex.search(line):
                findings.append(
                    {
                        "category": signal.category,
                        "severity": signal.severity,
                        "line": line_number,
                    }
                )
    record["signals"] = findings
    return record


def audit(source: Path, language: str) -> dict[str, object]:
    source = source.resolve()
    if not source.exists():
        raise FileNotFoundError(source)

    files = [scan_file(path, source) for path in candidate_files(source)]
    categories: dict[str, int] = {}
    severities: dict[str, int] = {}
    for file_record in files:
        for signal in file_record["signals"]:  # type: ignore[index]
            category = str(signal["category"])
            severity = str(signal["severity"])
            categories[category] = categories.get(category, 0) + 1
            severities[severity] = severities.get(severity, 0) + 1

    license_files = sorted(
        str(record["path"])
        for record in files
        if Path(str(record["path"])).name.lower() in LICENSE_NAMES
    )
    source_files = sorted(
        str(record["path"])
        for record in files
        if Path(str(record["path"])).name.lower() not in LICENSE_NAMES
    )
    tree_digest = sha256_bytes(
        "\n".join(f"{record['path']}\t{record['sha256']}" for record in files).encode("utf-8")
    )

    if not source_files:
        status = "TOKEN_VAZIO_NO_SCANNABLE_SOURCE"
    elif severities.get("BLOCK", 0) > 0:
        status = "REJECTED_RUNTIME_UNTIL_REWRITE"
    elif language not in DIRECT_CANDIDATE_LANGS:
        status = "LOWERING_REQUIRED"
    elif severities.get("REWRITE", 0) > 0:
        status = "CANDIDATE_REQUIRES_REWRITE"
    else:
        status = "CANDIDATE_FOR_KERNEL_EXTRACTION"

    return {
        "schema": SCHEMA,
        "source": {
            "path": source.as_posix(),
            "language": language,
            "tree_sha256": tree_digest,
            "file_count": len(files),
            "source_file_count": len(source_files),
            "license_files": license_files,
        },
        "inventory": {
            "categories": dict(sorted(categories.items())),
            "severities": dict(sorted(severities.items())),
        },
        "files": files,
        "decision": {
            "status": status,
            "execute_candidate_code": False,
            "requires_bit_exact_vectors": True,
            "requires_zero_undefined_symbols": True,
            "requires_final_binary_gate": True,
            "claim_allowed": False,
        },
    }


def selftest() -> int:
    with tempfile.TemporaryDirectory(prefix="rafaelia-m063-") as tmp:
        root = Path(tmp)

        empty = root / "empty"
        empty.mkdir()
        empty_report = audit(empty, "c")
        assert empty_report["decision"]["status"] == "TOKEN_VAZIO_NO_SCANNABLE_SOURCE"  # type: ignore[index]

        pure = root / "pure.c"
        pure.write_text(
            "unsigned long long patch(unsigned long long a, unsigned long long v, "
            "unsigned long long m){return a ^ ((a ^ v) & m);}\n",
            encoding="utf-8",
        )
        pure_report = audit(pure, "c")
        assert pure_report["decision"]["status"] == "CANDIDATE_FOR_KERNEL_EXTRACTION"  # type: ignore[index]

        hosted = root / "hosted.py"
        hosted.write_text("items = list()\nprint(items)\n", encoding="utf-8")
        hosted_report = audit(hosted, "py")
        assert hosted_report["decision"]["status"] in {  # type: ignore[index]
            "LOWERING_REQUIRED",
            "CANDIDATE_REQUIRES_REWRITE",
        }
        assert hosted_report["decision"]["claim_allowed"] is False  # type: ignore[index]

        heap = root / "heap.c"
        heap.write_text("void *f(unsigned n){return malloc(n);}\n", encoding="utf-8")
        heap_report = audit(heap, "c")
        assert heap_report["decision"]["status"] == "REJECTED_RUNTIME_UNTIL_REWRITE"  # type: ignore[index]
    print("library assimilation audit selftest: PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", type=Path)
    parser.add_argument("--language", default="TOKEN_VAZIO")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()

    if args.selftest:
        return selftest()
    if args.source is None:
        parser.error("source is required unless --selftest is used")

    report = audit(args.source, args.language.lower())
    encoded = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
