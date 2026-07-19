#!/usr/bin/env python3
"""First executable tranche for the RafPolimata/ApkC gap closure.

The gate does not infer PASS from file presence. It reconciles canonical proof
artifacts with documentation, checks the code-level fixes introduced in this
branch, locates any browser/TLS implementation, and writes a deterministic map
of loose or weakly-indexed repository files.
"""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

SCHEMA = "raf.apkc-first-part-gate.v1"
TEXT_SUFFIXES = {
    ".c", ".h", ".cc", ".cpp", ".s", ".asm", ".S", ".py", ".sh",
    ".md", ".txt", ".json", ".jsonl", ".yaml", ".yml", ".toml",
    ".xml", ".gradle", ".properties", ".java", ".kt", ".rs", ".go",
    ".rb", ".php", ".pl", ".groovy", ".clj", ".swift", ".js", ".jsx",
}
SKIP_PARTS = {".git", "build", "dist", "node_modules", "__pycache__", ".gradle", ".idea"}
CANONICAL_ROOT_FILES = {
    "README.md", "LICENSE", "NOTICE", "CHANGELOG.md", "CONTRIBUTING.md",
    "SECURITY.md", "CODE_OF_CONDUCT.md", "GOVERNANCE.md", "CITATION.cff",
    "Makefile", "CLAUDE.md", "ECOSYSTEM_RUNTIME_STATE.json",
}


@dataclass(frozen=True)
class Check:
    check_id: str
    state: str
    detail: str
    paths: tuple[str, ...] = ()

    def as_dict(self) -> dict[str, object]:
        return {
            "id": self.check_id,
            "state": self.state,
            "detail": self.detail,
            "paths": list(self.paths),
        }


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def find_root(start: Path) -> Path:
    start = start.resolve()
    for candidate in (start, *start.parents):
        if (candidate / "Apkc" / "apkc.c").is_file() and (candidate / ".github").is_dir():
            return candidate
    raise SystemExit("apkc-first-part: repository root not found")


def read_text(path: Path, limit: int = 2_000_000) -> str:
    try:
        if not path.is_file() or path.stat().st_size > limit:
            return ""
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(131072), b""):
            h.update(chunk)
    return h.hexdigest()


def git_commit(root: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=root, text=True, stderr=subprocess.DEVNULL
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "TOKEN_VAZIO"


def tracked_files(root: Path) -> list[Path]:
    try:
        raw = subprocess.check_output(
            ["git", "ls-files", "-z"], cwd=root, stderr=subprocess.DEVNULL
        )
        paths = [root / os.fsdecode(item) for item in raw.split(b"\0") if item]
    except (OSError, subprocess.CalledProcessError):
        paths = [p for p in root.rglob("*") if p.is_file()]
    return sorted(
        (p for p in paths if not any(part in SKIP_PARTS for part in p.relative_to(root).parts)),
        key=lambda p: p.relative_to(root).as_posix().lower(),
    )


def artifact_state(path: Path) -> tuple[str, str]:
    if not path.is_file():
        return "TOKEN_VAZIO", "arquivo ausente"
    text = read_text(path)
    upper = text.upper()
    if not text.strip():
        return "TOKEN_VAZIO", "arquivo vazio"
    if text.lstrip().startswith("TOKEN_VAZIO"):
        return "TOKEN_VAZIO", text.strip().splitlines()[0][:240]
    if "ERROR:" in upper or "UNRECOGNIZED COMMAND-LINE OPTION" in upper or "INVALID REGISTER NAME" in upper:
        return "CONTRADICTION", "transcript contém erro de compilação"
    if "STATUS: PASS" in upper or re.search(r"\bPASS\b", upper):
        return "PASS", "artefato contém marcador PASS; validação semântica específica ainda se aplica"
    return "REFERENCE", "arquivo existe sem marcador canônico suficiente"


def check_code_markers(root: Path) -> list[Check]:
    proof = read_text(root / "tools/raf_source_to_binary_proof.sh")
    profiles = read_text(root / "Apkc/lang_profile.h")
    sys_h = read_text(root / "Apkc/sys.h")
    dex_h = read_text(root / "Apkc/fmt_dex.h")
    elf_h = read_text(root / "Apkc/fmt_elf.h")
    java_stage = read_text(root / "scripts/apkc_java_to_jar.sh")
    groovy_stage = read_text(root / "scripts/apkc_groovy_to_jar.sh")

    checks = [
        Check(
            "PROOF-FAIL-CLOSED",
            "PASS" if all(x in proof for x in ("FINAL_STATE", "canonical PASS was not promoted", "exit 1", "source-to-binary-proof.v2")) else "FAIL",
            "source→binary só promove prova completa; parcial falha fechado",
            ("tools/raf_source_to_binary_proof.sh",),
        ),
        Check(
            "LANG-UNKNOWN-REJECTED",
            "PASS" if "Missing or unknown extensions are hard errors" in profiles or "return (const LangProfile *)0" in profiles else "FAIL",
            "extensão ausente/desconhecida não é convertida silenciosamente em ASM",
            ("Apkc/lang_profile.h",),
        ),
        Check(
            "LANG-TABLE-VALIDATED",
            "PASS" if "lang_profile_table_validate" in profiles and "families != 1" in profiles else "FAIL",
            "cada perfil possui uma única família de execução e tabela sem duplicatas",
            ("Apkc/lang_profile.h",),
        ),
        Check(
            "TERMUX-EXEC-RESOLUTION",
            "PASS" if all(x in sys_h for x in ("execve does not search PATH", "/data/data/com.termux/files/usr/bin/", "fallback_env")) else "FAIL",
            "execve resolve ferramentas por prefixos determinísticos e ambiente mínimo",
            ("Apkc/sys.h",),
        ),
        Check(
            "JAVA-JAR-STAGE",
            "PASS" if all(x in java_stage for x in ("javac", "jar --create", "set -eu")) and "apkc_java_to_jar.sh" in profiles else "FAIL",
            "javac gera classes e JAR antes do D8",
            ("scripts/apkc_java_to_jar.sh", "Apkc/lang_profile.h"),
        ),
        Check(
            "GROOVY-JAR-STAGE",
            "PASS" if all(x in groovy_stage for x in ("groovyc", "jar --create", "set -eu")) and "apkc_groovy_to_jar.sh" in profiles else "FAIL",
            "groovyc gera classes e JAR antes do D8",
            ("scripts/apkc_groovy_to_jar.sh", "Apkc/lang_profile.h"),
        ),
        Check(
            "DEX-STRUCTURE",
            "IMPLEMENTED" if all(x in dex_h for x in ("dex_build", "sha1_final", "adler32", "cap < (sz)TOTAL")) else "FAIL",
            "gerador mínimo DEX035 tem limite, SHA-1 e Adler-32; runtime Java continua separado",
            ("Apkc/fmt_dex.h",),
        ),
        Check(
            "ELF-STRUCTURE",
            "IMPLEMENTED" if all(x in elf_h for x in ("elf64_build_so", "elf32_build_so", "PT_LOAD", "PT_DYNAMIC")) else "FAIL",
            "geradores ELF32/ELF64 existem; carga Android real depende do artefato e runtime",
            ("Apkc/fmt_elf.h",),
        ),
    ]
    return checks


def parse_gap_claims(text: str) -> dict[str, str]:
    claims: dict[str, str] = {}
    for line in text.splitlines():
        if not line.startswith("|") or line.startswith("|---"):
            continue
        cells = [cell.strip().strip("*") for cell in line.strip().strip("|").split("|")]
        if len(cells) >= 2 and cells[0].lower() != "gap":
            claims[cells[0].lower()] = cells[1].upper()
    return claims


def reconcile_proofs(root: Path) -> tuple[list[dict[str, str]], list[dict[str, str]]]:
    mapping = {
        "Build reproduzível de `apkc.c`": root / "Apkc/proofs/out/apkc-compile.txt",
        "Geração de `hello.apk`": root / "Apkc/proofs/out/apkc-generate.txt",
        "ELF ARM32 dentro do APK": root / "Apkc/proofs/out/readelf-arm32.txt",
        "ELF ARM64 dentro do APK": root / "Apkc/proofs/out/readelf-arm64.txt",
        "DEX SHA-1 do APK atual": root / "Apkc/proofs/out/dex-sha1.txt",
    }
    gap_text = read_text(root / "Apkc/proofs/GAPS.md")
    claims = parse_gap_claims(gap_text)
    artifacts: list[dict[str, str]] = []
    contradictions: list[dict[str, str]] = []

    for label, path in mapping.items():
        state, detail = artifact_state(path)
        rel = path.relative_to(root).as_posix()
        claim = claims.get(label.lower(), "TOKEN_VAZIO")
        artifacts.append({
            "label": label,
            "path": rel,
            "artifact_state": state,
            "document_state": claim,
            "detail": detail,
        })
        if "PASS" in claim and state != "PASS":
            contradictions.append({
                "label": label,
                "document_state": claim,
                "artifact_state": state,
                "path": rel,
                "reason": "documento promove estado acima do artefato",
            })
    return artifacts, contradictions


def scan_browser_tls(root: Path, files: Iterable[Path]) -> dict[str, object]:
    asm_markers = ("__asm__", "__asm(", "svc #0", "swi #0", ".global _start")
    http_markers = ("http/1.1", "https://", "get /", "host:", "user-agent:")
    net_markers = ("socket(", "connect(", "sys_socket", "_nr_socket", "sockaddr")
    tls_markers = ("tls 1.2", "tls1_2", "tls 1.3", "tls1_3", "x509", "ssl_ctx", "mbedtls", "openssl")

    asm_files: set[str] = set()
    http_files: set[str] = set()
    net_files: set[str] = set()
    tls_files: set[str] = set()

    for path in files:
        if path.suffix not in TEXT_SUFFIXES:
            continue
        text = read_text(path, 600_000).lower()
        if not text:
            continue
        rel = path.relative_to(root).as_posix()
        if path.suffix.lower() in {".s", ".asm"} or any(m.lower() in text for m in asm_markers):
            asm_files.add(rel)
        if any(m in text for m in http_markers):
            http_files.add(rel)
        if any(m in text for m in net_markers):
            net_files.add(rel)
        if any(m in text for m in tls_markers):
            tls_files.add(rel)

    web_candidates = sorted((http_files & net_files) | (http_files & tls_files))
    asm_web_candidates = sorted(set(web_candidates) & asm_files)
    tls_web_candidates = sorted(set(web_candidates) & tls_files)
    tui = "raf_shell/raf_shell.c" if (root / "raf_shell/raf_shell.c").is_file() else None

    return {
        "tui_file_browser": {
            "state": "IMPLEMENTED" if tui else "TOKEN_VAZIO",
            "path": tui,
            "scope": "local file browser / orchestration TUI; not a web browser",
        },
        "web_browser": {
            "state": "IMPLEMENTED" if web_candidates else "TOKEN_VAZIO",
            "candidates": web_candidates,
        },
        "asm_web_browser": {
            "state": "IMPLEMENTED" if asm_web_candidates else "TOKEN_VAZIO",
            "candidates": asm_web_candidates,
        },
        "tls_1_2_1_3_x509": {
            "state": "IMPLEMENTED" if tls_web_candidates else "TOKEN_VAZIO",
            "candidates": tls_web_candidates,
            "tls_marker_files": sorted(tls_files)[:100],
        },
    }


def category_for(path: Path) -> str:
    suffix = path.suffix.lower()
    if suffix in {".md", ".txt"}:
        return "documentation"
    if suffix in {".c", ".h", ".cc", ".cpp", ".s", ".asm"}:
        return "native-source"
    if suffix in {".py", ".sh"}:
        return "automation"
    if suffix in {".json", ".jsonl", ".yaml", ".yml", ".toml"}:
        return "structured-data"
    if suffix in {".apk", ".so", ".o", ".elf", ".dex", ".jar", ".zip"}:
        return "binary-artifact"
    return "other"


def build_loose_map(root: Path, files: list[Path]) -> tuple[dict[str, object], str]:
    index_paths = [
        root / "README.md",
        root / "CLAUDE.md",
        root / "docs/AGENTES.md",
        root / "docs/MAPA_ESTRUTURAL_REPOSITORIO.md",
        root / "docs/INDEX.md",
    ]
    index_text = "\n".join(read_text(p, 1_500_000) for p in index_paths if p.is_file())

    entries: list[dict[str, object]] = []
    for path in files:
        rel = path.relative_to(root).as_posix()
        parts = path.relative_to(root).parts
        is_root = len(parts) == 1
        weak_zone = is_root or parts[0] in {"docs", "scripts", "tools", "Apkc"}
        if not weak_zone:
            continue
        if is_root and path.name in CANONICAL_ROOT_FILES:
            continue
        try:
            size = path.stat().st_size
            digest = sha256_file(path)
        except OSError:
            continue
        referenced = rel in index_text or path.name in index_text
        route = "INDEXED" if referenced else (
            "MOVE_OR_INDEX" if is_root else "ADD_TO_CANONICAL_INDEX"
        )
        entries.append({
            "path": rel,
            "category": category_for(path),
            "size_bytes": size,
            "sha256": digest,
            "referenced_by_canonical_index": referenced,
            "route": route,
        })

    entries.sort(key=lambda item: (str(item["route"]), str(item["category"]), str(item["path"]).lower()))
    summary: dict[str, int] = {}
    for entry in entries:
        key = str(entry["route"])
        summary[key] = summary.get(key, 0) + 1

    lines = [
        "# Mapa gerado — arquivos soltos e fracamente indexados",
        "",
        "> Gerado por `scripts/apkc_first_part_gate.py`. Um arquivo não referenciado",
        "> não é apagado: recebe rota explícita para indexação, movimentação ou revisão.",
        "",
        "| Caminho | Categoria | Bytes | SHA-256 | Indexado | Rota |",
        "|---|---|---:|---|---|---|",
    ]
    for entry in entries:
        lines.append(
            f"| `{entry['path']}` | {entry['category']} | {entry['size_bytes']} | "
            f"`{str(entry['sha256'])[:16]}…` | "
            f"{'sim' if entry['referenced_by_canonical_index'] else 'não'} | {entry['route']} |"
        )
    lines.extend([
        "",
        "## Regra de incorporação documental",
        "",
        "```text",
        "arquivo detectado → hash → categoria → relação com índice → rota",
        "MOVE_OR_INDEX não significa excluir; significa decidir destino e preservar proveniência.",
        "```",
        "",
    ])
    return {"summary": summary, "entries": entries}, "\n".join(lines)


def run(root: Path) -> tuple[dict[str, object], str, int]:
    files = tracked_files(root)
    checks = check_code_markers(root)
    artifacts, contradictions = reconcile_proofs(root)
    browser_tls = scan_browser_tls(root, files)
    loose_map, loose_markdown = build_loose_map(root, files)

    hard_failures = [c for c in checks if c.state == "FAIL"]
    state = "FAIL" if hard_failures or contradictions else "PASS"
    report: dict[str, object] = {
        "schema": SCHEMA,
        "generated_at": utc_now(),
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "commit": git_commit(root),
        "scope": "first executable tranche: truth, toolchains, ELF/DEX, browser/TLS discovery, loose-file map",
        "checks": [c.as_dict() for c in checks],
        "canonical_artifacts": artifacts,
        "contradictions": contradictions,
        "browser_tls": browser_tls,
        "loose_files": loose_map,
        "state": state,
        "claim_allowed": False,
        "next": [
            "run fail-closed source-to-binary proof on Termux",
            "generate one APK run containing ELF32, ELF64 and DEX evidence",
            "validate Java/Kotlin/Groovy D8 output with dexdump and Android runtime",
            "identify or implement the claimed ASM web browser with TLS 1.2/1.3 and X.509",
            "apply generated loose-file routes without deleting provenance",
        ],
    }
    return report, loose_markdown, 0 if state == "PASS" else 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--write", type=Path)
    parser.add_argument("--write-map", type=Path)
    parser.add_argument("--allow-open-gaps", action="store_true")
    args = parser.parse_args(argv)

    root = find_root(args.root)
    report, markdown, rc = run(root)

    payload = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.write:
        target = args.write if args.write.is_absolute() else root / args.write
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)

    if args.write_map:
        target = args.write_map if args.write_map.is_absolute() else root / args.write_map
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(markdown, encoding="utf-8")

    checks = report["checks"]
    passed = sum(1 for item in checks if item["state"] in {"PASS", "IMPLEMENTED"})
    print(
        f"apkc-first-part: {passed}/{len(checks)} code gates; "
        f"contradictions={len(report['contradictions'])}; state={report['state']}",
        file=sys.stderr,
    )
    return 0 if args.allow_open_gaps else rc


if __name__ == "__main__":
    raise SystemExit(main())
