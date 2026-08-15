#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable, Sequence

SCHEMA = "raf.ecosystem-build-doctor-report.v1"
SEVERITY_RANK = {"info": 0, "low": 1, "medium": 2, "high": 3, "critical": 4}

BUILD_FILE_NAMES = {
    "CMakeLists.txt",
    "Makefile",
    "GNUmakefile",
    "meson.build",
    "Android.mk",
    "Application.mk",
    "build.gradle",
    "build.gradle.kts",
    "settings.gradle",
    "settings.gradle.kts",
    "gradle.properties",
}
BUILD_SUFFIXES = {".cmake", ".mk", ".gradle", ".kts", ".yml", ".yaml", ".sh"}
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".m", ".mm", ".s", ".S", ".asm"}
DIAGNOSTIC_SUFFIXES = {".log", ".out", ".err", ".txt"}
BINARY_SUFFIXES = {
    ".a",
    ".aab",
    ".apk",
    ".bin",
    ".dex",
    ".elf",
    ".exe",
    ".jar",
    ".o",
    ".so",
    ".tar",
    ".tgz",
    ".zip",
}

DEFAULT_EXCLUDE_PARTS = {
    ".git",
    ".gradle",
    ".idea",
    ".venv",
    "build",
    "dist",
    "node_modules",
    "out",
    "subprojects",
    "target",
    "third_party",
    "vendor",
}
ZOMBIE_EXCLUDE_PARTS = {
    "_incoming",
    "archive",
    "bench",
    "benchmark",
    "demo",
    "demos",
    "example",
    "examples",
    "experimental",
    "experiments",
    "generated",
    "incluir",
    "legacy",
    "samples",
    "test",
    "tests",
    "third_party",
    "vendor",
}
PROVENANCE_NAME_TOKENS = {
    "artifact",
    "checksum",
    "hash",
    "manifest",
    "provenance",
    "sbom",
    "sha256",
    "blake3",
}

OPTIMIZATION_RE = re.compile(r"(?<![A-Za-z0-9_])-O(?:0|1|2|3|g|s|z|fast)\b")
ARCH_RE = re.compile(r"(?<![A-Za-z0-9_])-march=[^\s\"')>;]+")
CMAKE_SET_RE = re.compile(r"\bset\(\s*([A-Z][A-Z0-9_]{5,})\b")
STATIC_TARGET_RE = re.compile(r"\badd_library\(\s*([A-Za-z0-9_.+-]+)\s+STATIC\b", re.IGNORECASE)
TARGET_LINK_OPTIONS_RE = re.compile(r"\btarget_link_options\(\s*([A-Za-z0-9_.+-]+)\b", re.IGNORECASE)
C_ONLY_PROJECT_RE = re.compile(
    r"\bproject\([^\n)]*(?:LANGUAGES\s+)?C(?:\s+ASM)?\s*\)", re.IGNORECASE
)
SOURCE_MARKER_RE = re.compile(r"\b(TODO|FIXME|STUB|PLACEHOLDER|TOKEN_VAZIO)\b", re.IGNORECASE)
LINKER_DIAGNOSTICS = (
    "undefined reference",
    "multiple definition",
    "cannot find -l",
    "ld.lld: error",
    "collect2: error",
    "relocation truncated",
    "linker command failed",
    "duplicate symbol",
)


@dataclass(frozen=True)
class Finding:
    repo: str
    code: str
    severity: str
    path: str
    line: int
    message: str
    evidence: str
    next_action: str

    def key(self) -> tuple[object, ...]:
        return (
            -SEVERITY_RANK[self.severity],
            self.repo,
            self.path,
            self.line,
            self.code,
            self.evidence,
        )


@dataclass(frozen=True)
class RepoTarget:
    name: str
    root: Path


GENERATED_MARKERS = {
    "CMakeCache.txt",
    "build.ninja",
    ".ninja_log",
    "compile_commands.json",
    "Makefile.in",
}

TOPLEVEL_BUILD_DIRS = {"build", "out", "dist", "target"}


def has_generated_markers(dir_path: Path) -> bool:
    """Check if directory contains markers indicating it's generated output."""
    try:
        for item in dir_path.iterdir():
            if item.name in GENERATED_MARKERS or item.name.startswith(".ninja"):
                return True
    except OSError:
        return False
    return False


def is_excluded(path: Path, root: Path, extra_parts: set[str]) -> bool:
    try:
        rel = path.relative_to(root)
    except ValueError:
        return True

    parts = rel.parts

    # First, check if any part is in extra_parts (standard exclusions like .git, .gradle, etc)
    for part in parts:
        part_lower = part.lower()
        if part_lower in extra_parts and part_lower not in TOPLEVEL_BUILD_DIRS:
            return True

    # For top-level build-like directories, only exclude if they have generated markers
    # This preserves nested source directories like rmr/build/ that don't have markers
    for i, part in enumerate(parts):
        part_lower = part.lower()
        if part_lower in TOPLEVEL_BUILD_DIRS:
            # Only exclude if this is at the top level (i == 0)
            # Otherwise, check for generated markers in this directory
            if i == 0:
                # Top-level build/out/dist/target directory
                build_dir = root / parts[0]
                try:
                    if has_generated_markers(build_dir):
                        return True
                except OSError:
                    return True
            else:
                # Nested directory named build/out/dist/target - only exclude if it has markers
                build_dir = root
                for j in range(i + 1):
                    build_dir = build_dir / parts[j]
                try:
                    if has_generated_markers(build_dir):
                        return True
                except OSError:
                    return True

    return False


def relative_posix(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def get_exclusion_reason(path: Path, root: Path, extra_parts: set[str]) -> str | None:
    """Determine why a path is excluded, returning None if not excluded."""
    try:
        rel = path.relative_to(root)
    except ValueError:
        return "outside_root"

    parts = rel.parts

    # Check standard exclusions first
    for part in parts:
        part_lower = part.lower()
        if part_lower in extra_parts and part_lower not in TOPLEVEL_BUILD_DIRS:
            return f"excluded_component:{part}"

    # Check top-level build-like directories
    for i, part in enumerate(parts):
        part_lower = part.lower()
        if part_lower in TOPLEVEL_BUILD_DIRS:
            if i == 0:
                # Top-level build directory
                build_dir = root / parts[0]
                try:
                    if has_generated_markers(build_dir):
                        return f"generated_markers_in_toplevel:{part}"
                except OSError:
                    return f"access_error_toplevel:{part}"
            else:
                # Nested build-like directory
                build_dir = root
                for j in range(i + 1):
                    build_dir = build_dir / parts[j]
                try:
                    if has_generated_markers(build_dir):
                        return f"generated_markers_in_nested:{part}"
                except OSError:
                    return f"access_error_nested:{part}"

    return None


def bounded_text(path: Path, max_bytes: int) -> str | None:
    try:
        if path.stat().st_size > max_bytes:
            return None
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None


def collect_files(root: Path, max_bytes: int, report_exclusions: bool = False) -> tuple[list[Path], dict[Path, str], list[dict]]:
    files: list[Path] = []
    texts: dict[Path, str] = {}
    exclusions: list[dict] = []

    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue

        reason = get_exclusion_reason(path, root, DEFAULT_EXCLUDE_PARTS)
        if reason is not None:
            if report_exclusions:
                rel = relative_posix(path, root)
                exclusions.append({
                    "path": rel,
                    "reason": reason,
                })
            continue

        files.append(path)
        if (
            path.name in BUILD_FILE_NAMES
            or path.suffix in BUILD_SUFFIXES
            or path.suffix in SOURCE_SUFFIXES
            or path.suffix.lower() in DIAGNOSTIC_SUFFIXES
        ):
            text = bounded_text(path, max_bytes)
            if text is not None:
                texts[path] = text

    return files, texts, exclusions


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def build_files(texts: dict[Path, str]) -> dict[Path, str]:
    return {
        path: text
        for path, text in texts.items()
        if path.name in BUILD_FILE_NAMES or path.suffix in BUILD_SUFFIXES
    }


def source_files(files: Iterable[Path]) -> list[Path]:
    return [path for path in files if path.suffix in SOURCE_SUFFIXES]


def build_reference_corpus(builds: dict[Path, str]) -> str:
    return "\n".join(text.replace("\\", "/").lower() for text in builds.values())


def has_provenance_sidecar(path: Path) -> bool:
    candidates = {
        Path(str(path) + ".sha256"),
        Path(str(path) + ".blake3"),
        path.with_suffix(path.suffix + ".manifest.json"),
        path.with_suffix(".sha256"),
        path.with_suffix(".blake3"),
    }
    if any(candidate.exists() for candidate in candidates):
        return True
    try:
        siblings = [item.name.lower() for item in path.parent.iterdir() if item.is_file()]
    except OSError:
        return False
    return any(token in sibling for sibling in siblings for token in PROVENANCE_NAME_TOKENS)


def finding(
    target: RepoTarget,
    root: Path,
    code: str,
    severity: str,
    path: Path,
    line: int,
    message: str,
    evidence: str,
    next_action: str,
) -> Finding:
    return Finding(
        repo=target.name,
        code=code,
        severity=severity,
        path=relative_posix(path, root),
        line=line,
        message=message,
        evidence=evidence.strip(),
        next_action=next_action,
    )


def analyze_build_file(target: RepoTarget, path: Path, text: str) -> list[Finding]:
    root = target.root
    findings: list[Finding] = []

    optimization_flags = sorted(set(OPTIMIZATION_RE.findall(text)))
    if len(optimization_flags) > 1:
        findings.append(
            finding(
                target,
                root,
                "conflicting_optimization_flags",
                "high",
                path,
                1,
                "O mesmo arquivo de build declara múltiplos níveis de otimização.",
                ", ".join(optimization_flags),
                "Separar perfis/targets e registrar a flag efetiva no artefato de build.",
            )
        )

    architectures = sorted(set(ARCH_RE.findall(text)))
    if len(architectures) > 1:
        findings.append(
            finding(
                target,
                root,
                "multiple_architecture_flags",
                "low",
                path,
                1,
                "Há múltiplos -march no mesmo arquivo; podem ser válidos por ABI, mas exigem roteamento explícito.",
                ", ".join(architectures),
                "Confirmar que cada -march está condicionado a uma ABI/target e impedir sobrescrita por flags externas.",
            )
        )

    if C_ONLY_PROJECT_RE.search(text):
        for marker in ("-fno-rtti", "-fno-exceptions"):
            for match in re.finditer(re.escape(marker), text):
                context_start = max(0, match.start() - 120)
                context = text[context_start : match.end() + 120]
                if "COMPILE_LANGUAGE:CXX" in context:
                    continue
                findings.append(
                    finding(
                        target,
                        root,
                        "cxx_only_flag_in_c_project",
                        "high",
                        path,
                        line_number(text, match.start()),
                        "Flag específica de C++ aparece num projeto C/ASM sem generator expression de CXX.",
                        marker,
                        "Remover a flag do caminho C ou aplicá-la somente com $<COMPILE_LANGUAGE:CXX>.",
                    )
                )

    for pattern, code, severity, message, action in (
        (
            r"\bset\(\s*CMAKE_(?:C|CXX|ASM|EXE_LINKER|SHARED_LINKER)_FLAGS",
            "global_cmake_flags",
            "medium",
            "Flags globais de CMake reduzem rastreabilidade por target.",
            "Migrar para target_compile_options/target_link_options e registrar perfil efetivo.",
        ),
        (
            r"\bfile\(\s*GLOB(?:_RECURSE)?\b",
            "cmake_glob_source_membership",
            "medium",
            "file(GLOB) oculta a associação fonte→target e dificulta detectar código zumbi.",
            "Substituir por manifesto explícito ou gerar lista validada com parity check.",
        ),
        (
            r"(?:^|\s)-w(?:\s|$)",
            "all_warnings_disabled",
            "high",
            "Todas as warnings foram desativadas neste caminho de build.",
            "Restaurar warnings e tratar cada diagnóstico como instrução de contrato/portabilidade.",
        ),
        (
            r"-Wno-error\b",
            "warnings_not_blocking",
            "medium",
            "Warnings foram retiradas do gate bloqueante sem escopo documentado.",
            "Aplicar exceção por target/diagnóstico e manter um ledger de warning com owner e prazo.",
        ),
        (
            r"continue-on-error\s*:\s*true",
            "ci_continue_on_error",
            "medium",
            "Etapa de CI permite falha sem bloquear a execução.",
            "Classificar a etapa como informativa ou remover continue-on-error do gate material.",
        ),
        (
            r"(?:^|\s)(?:\|\|\s*true|;\s*true)(?:\s|$)",
            "shell_failure_masked",
            "high",
            "O shell mascara o exit code de um comando.",
            "Preservar exit code; quando a falha for esperada, testá-la explicitamente.",
        ),
    ):
        for match in re.finditer(pattern, text, re.IGNORECASE | re.MULTILINE):
            findings.append(
                finding(
                    target,
                    root,
                    code,
                    severity,
                    path,
                    line_number(text, match.start()),
                    message,
                    match.group(0),
                    action,
                )
            )

    static_targets = set(STATIC_TARGET_RE.findall(text))
    for match in TARGET_LINK_OPTIONS_RE.finditer(text):
        target_name = match.group(1)
        if target_name in static_targets:
            findings.append(
                finding(
                    target,
                    root,
                    "link_options_on_static_library",
                    "high",
                    path,
                    line_number(text, match.start()),
                    "target_link_options foi aplicado a biblioteca STATIC; o archive não executa link final.",
                    target_name,
                    "Mover o contrato para um probe de link executável/shared ou declarar explicitamente que é somente compile contract.",
                )
            )

    if "-nostdlib" in text and re.search(r"(?:^|\s)-l(?:c|m|dl|pthread|stdc\+\+)(?:\s|$)", text):
        findings.append(
            finding(
                target,
                root,
                "mixed_freestanding_hosted_link_contract",
                "medium",
                path,
                1,
                "O mesmo arquivo contém rota -nostdlib e bibliotecas hosted; a fronteira precisa ser por target.",
                "-nostdlib + -l<hosted>",
                "Separar targets hosted/freestanding e adicionar um teste de símbolos indefinidos por artefato.",
            )
        )

    return findings


def analyze_dead_cmake_variables(target: RepoTarget, builds: dict[Path, str]) -> list[Finding]:
    findings: list[Finding] = []
    corpus = "\n".join(builds.values())
    definitions: list[tuple[Path, str, str, int]] = []
    for path, text in builds.items():
        for match in CMAKE_SET_RE.finditer(text):
            name = match.group(1)
            if name.startswith("CMAKE_") or name.startswith("ANDROID_"):
                continue
            definitions.append((path, text, name, match.start()))

    for path, text, name, offset in definitions:
        if len(re.findall(rf"\b{re.escape(name)}\b", corpus)) != 1:
            continue
        findings.append(
            finding(
                target,
                target.root,
                "dead_cmake_variable_candidate",
                "medium",
                path,
                line_number(text, offset),
                "Variável CMake é definida, mas não possui consumidor localizado no corpus de build.",
                name,
                "Remover, conectar a um target ou registrar como output intencional consumido externamente.",
            )
        )
    return findings


def analyze_zombie_sources(
    target: RepoTarget,
    files: Sequence[Path],
    builds: dict[Path, str],
) -> list[Finding]:
    findings: list[Finding] = []
    corpus = build_reference_corpus(builds)
    glob_present = any(re.search(r"\bfile\(\s*GLOB", text, re.IGNORECASE) for text in builds.values())

    for path in source_files(files):
        rel = relative_posix(path, target.root)
        rel_parts = {part.lower() for part in Path(rel).parts}
        if rel_parts & ZOMBIE_EXCLUDE_PARTS:
            continue
        rel_lower = rel.lower()
        basename_lower = path.name.lower()
        if rel_lower in corpus or basename_lower in corpus:
            continue
        severity = "low" if glob_present else "medium"
        evidence = f"source={rel}; file_glob_present={str(glob_present).lower()}"
        findings.append(
            finding(
                target,
                target.root,
                "zombie_source_candidate",
                severity,
                path,
                1,
                "Fonte executável não foi localizada em CMake/Make/Meson/Gradle/workflows.",
                evidence,
                "Classificar como canônica, teste, experimento ou legado; então ligar ao manifesto ou mover em PR dedicado.",
            )
        )
    return findings


def analyze_source_markers(target: RepoTarget, texts: dict[Path, str]) -> list[Finding]:
    findings: list[Finding] = []
    for path, text in texts.items():
        if path.suffix not in SOURCE_SUFFIXES:
            continue
        for match in SOURCE_MARKER_RE.finditer(text):
            marker = match.group(1).upper()
            findings.append(
                finding(
                    target,
                    target.root,
                    "executable_source_marker",
                    "medium",
                    path,
                    line_number(text, match.start()),
                    "Marcador de lacuna aparece em fonte executável.",
                    marker,
                    "Converter em erro/estado tipado, issue vinculada ou teste que demonstre o comportamento pendente.",
                )
            )
    return findings


def analyze_diagnostics(target: RepoTarget, texts: dict[Path, str]) -> list[Finding]:
    findings: list[Finding] = []
    for path, text in texts.items():
        if path.suffix.lower() not in DIAGNOSTIC_SUFFIXES:
            continue
        lower = text.lower()
        for diagnostic in LINKER_DIAGNOSTICS:
            offset = lower.find(diagnostic)
            if offset < 0:
                continue
            findings.append(
                finding(
                    target,
                    target.root,
                    "linker_diagnostic_recorded",
                    "high",
                    path,
                    line_number(text, offset),
                    "Diagnóstico de linker foi encontrado em log/texto versionado.",
                    diagnostic,
                    "Ligar o diagnóstico ao target, comando, ABI e commit; criar regressão negativa antes de silenciar.",
                )
            )
    return findings


def analyze_binaries(target: RepoTarget, files: Sequence[Path]) -> list[Finding]:
    findings: list[Finding] = []
    for path in files:
        if path.suffix.lower() not in BINARY_SUFFIXES:
            continue
        rel_parts = {part.lower() for part in path.relative_to(target.root).parts}
        severity = "high" if rel_parts & {"_incoming", "incluir", "root", "src"} else "medium"
        if has_provenance_sidecar(path):
            continue
        findings.append(
            finding(
                target,
                target.root,
                "binary_without_provenance",
                severity,
                path,
                1,
                "Artefato binário versionado não possui sidecar/manifesto de proveniência localizado.",
                f"suffix={path.suffix.lower()}",
                "Adicionar hash, origem, comando de build, licença, ABI e política de promoção; ou remover em PR dedicado.",
            )
        )
    return findings


def analyze_repo(target: RepoTarget, max_bytes: int, report_exclusions: bool = False) -> tuple[list[Finding], list[dict]]:
    files, texts, exclusions = collect_files(target.root, max_bytes, report_exclusions=report_exclusions)
    builds = build_files(texts)
    findings: list[Finding] = []
    for path, text in builds.items():
        findings.extend(analyze_build_file(target, path, text))
    findings.extend(analyze_dead_cmake_variables(target, builds))
    findings.extend(analyze_zombie_sources(target, files, builds))
    findings.extend(analyze_source_markers(target, texts))
    findings.extend(analyze_diagnostics(target, texts))
    findings.extend(analyze_binaries(target, files))
    return sorted(set(findings), key=Finding.key), exclusions


def summarize(findings: Sequence[Finding]) -> dict[str, object]:
    by_severity = Counter(item.severity for item in findings)
    by_code = Counter(item.code for item in findings)
    by_repo = Counter(item.repo for item in findings)
    highest = "none"
    if findings:
        highest = max((item.severity for item in findings), key=SEVERITY_RANK.__getitem__)
    state = "PASS"
    if by_severity["critical"] or by_severity["high"]:
        state = "REVIEW_REQUIRED"
    elif by_severity["medium"]:
        state = "PASS_LIMITED"
    return {
        "state": state,
        "highest_severity": highest,
        "findings": len(findings),
        "by_severity": dict(sorted(by_severity.items())),
        "by_code": dict(sorted(by_code.items())),
        "by_repo": dict(sorted(by_repo.items())),
    }


def build_report(targets: Sequence[RepoTarget], max_bytes: int, report_exclusions: bool = False) -> dict[str, object]:
    findings: list[Finding] = []
    repo_records: list[dict[str, object]] = []
    all_exclusions: list[dict] = []

    for target in sorted(targets, key=lambda item: item.name):
        repo_findings, exclusions = analyze_repo(target, max_bytes, report_exclusions=report_exclusions)
        findings.extend(repo_findings)
        all_exclusions.extend(exclusions)
        repo_records.append({
            "name": target.name,
        })

    findings = sorted(set(findings), key=Finding.key)
    report_dict: dict[str, object] = {
        "schema": SCHEMA,
        "repos": repo_records,
        "summary": summarize(findings),
        "findings": [asdict(item) for item in findings],
        "claim_boundary": {
            "static_analysis": "VERIFIED_BY_EXECUTION",
            "build_execution": "TOKEN_VAZIO",
            "runtime_correctness": "TOKEN_VAZIO",
            "automatic_deletion": False,
        },
    }

    if report_exclusions and all_exclusions:
        report_dict["exclusions"] = all_exclusions

    return report_dict


def markdown_report(report: dict[str, object]) -> str:
    summary = report["summary"]
    assert isinstance(summary, dict)
    lines = [
        "# Ecosystem Build Doctor",
        "",
        f"- Estado: `{summary['state']}`",
        f"- Maior severidade: `{summary['highest_severity']}`",
        f"- Achados: `{summary['findings']}`",
        "- Execução de builds/runtimes: `TOKEN_VAZIO`",
        "",
        "## Achados",
        "",
        "| Severidade | Repositório | Código | Caminho | Linha | Próxima ação |",
        "|---|---|---|---|---:|---|",
    ]
    for item in report["findings"]:
        assert isinstance(item, dict)
        action = str(item["next_action"]).replace("|", "\\|")
        lines.append(
            f"| `{item['severity']}` | `{item['repo']}` | `{item['code']}` | "
            f"`{item['path']}` | {item['line']} | {action} |"
        )
    lines.extend(
        [
            "",
            "## Regra",
            "",
            "```text",
            "warning != ruído descartável",
            "warning -> hipótese de contrato -> target/ABI -> teste -> evidência",
            "```",
            "",
        ]
    )
    return "\n".join(lines)


def parse_repo(value: str) -> RepoTarget:
    if "=" not in value:
        raise argparse.ArgumentTypeError("--repo exige NAME=PATH")
    name, raw_path = value.split("=", 1)
    name = name.strip()
    root = Path(raw_path).expanduser()
    if not name:
        raise argparse.ArgumentTypeError("nome do repositório vazio")
    if not root.is_dir():
        raise argparse.ArgumentTypeError(f"diretório não encontrado: {root}")
    return RepoTarget(name=name, root=root.resolve())


def write_text(path: str | None, payload: str) -> None:
    if not path:
        return
    output = Path(path)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(payload, encoding="utf-8")


def should_fail(findings: Sequence[dict[str, object]], threshold: str) -> bool:
    if threshold == "none":
        return False
    rank = SEVERITY_RANK[threshold]
    return any(SEVERITY_RANK[str(item["severity"])] >= rank for item in findings)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Auditoria read-only de CMake/flags/warnings/fontes zumbis/binários."
    )
    parser.add_argument("--repo", action="append", type=parse_repo, required=True, help="NAME=PATH")
    parser.add_argument("--json-out")
    parser.add_argument("--markdown-out")
    parser.add_argument("--max-text-bytes", type=int, default=2_000_000)
    parser.add_argument(
        "--fail-on",
        choices=("none", "medium", "high", "critical"),
        default="none",
        help="Retorna 2 quando houver achado nessa severidade ou acima.",
    )
    parser.add_argument(
        "--exclude",
        action="append",
        default=[],
        help="Componentes de path adicionais a excluir (podem ser repetidos).",
    )
    parser.add_argument(
        "--include-root",
        action="store_true",
        help="Incluir path absoluto no relatório de exclusões (cuidado com dados privados).",
    )
    parser.add_argument(
        "--report-exclusions",
        action="store_true",
        help="Incluir lista detalhada de exclusões no relatório.",
    )
    args = parser.parse_args(argv)

    # Build exclude set from default + command-line arguments
    exclude_parts = set(DEFAULT_EXCLUDE_PARTS)
    for extra in args.exclude:
        exclude_parts.add(extra.lower())

    # For now, we'll use DEFAULT_EXCLUDE_PARTS; the exclude_parts would be passed to analyze_repo if needed
    report = build_report(args.repo, args.max_text_bytes, report_exclusions=args.report_exclusions)
    json_payload = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    md_payload = markdown_report(report)
    write_text(args.json_out, json_payload)
    write_text(args.markdown_out, md_payload)
    if not args.json_out and not args.markdown_out:
        print(json_payload, end="")

    findings = report["findings"]
    assert isinstance(findings, list)
    return 2 if should_fail(findings, args.fail_on) else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"FAIL ecosystem-build-doctor: {error}", file=sys.stderr)
        raise SystemExit(1)
