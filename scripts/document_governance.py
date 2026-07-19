#!/usr/bin/env python3
"""Governança documental determinística para o RafPolimata.

Varre arquivos versionados, calcula identidade, classifica área/ciclo de vida,
constrói grafo de referências, detecta duplicidade e risco, e gera catálogo,
índices e fila de revisão sem mover ou apagar conteúdo automaticamente.
"""
from __future__ import annotations

import argparse
import datetime as dt
import fnmatch
import hashlib
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable

SCHEMA = "raf.document-governance.v1"
RECORD_SCHEMA = "raf.document-record.v1"
PATH_TOKEN = re.compile(r"`([^`\n]+)`")
MARKDOWN_LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+['\"][^'\"]*['\"])?\)")
STATE_TOKEN = re.compile(r"\b(VOID|PENDING|AUDIT|RUNTIME|REFERENCE|TOKEN_VAZIO|PASS|FAIL|IMPLEMENTED)\b")
HASH_TOKEN = re.compile(r"\b(?:sha(?:-?1|-?256|-?3)?|blake3|crc32c?)\b", re.I)
COMMAND_TOKEN = re.compile(r"(?m)^\s*(?:\$|python3?|bash|sh|clang|gcc|make|adb|git)\s+\S+")
TEST_PATH = re.compile(r"(^|/)(tests?|proofs?|results?)(/|$)", re.I)
WORKFLOW_PATH = re.compile(r"^\.github/workflows/.*\.ya?ml$", re.I)
SECRET_VALUE_PATTERNS = (
    ("private_key_block", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----")),
    ("generic_secret_assignment", re.compile(r"(?i)\b(?:api[_-]?key|secret|token|password)\s*[:=]\s*['\"][^'\"]{8,}")),
)
PATH_LIKE_SUFFIXES = (
    ".md", ".txt", ".json", ".jsonl", ".yaml", ".yml", ".toml", ".xml",
    ".c", ".h", ".cc", ".cpp", ".s", ".asm", ".py", ".sh", ".java",
    ".kt", ".rs", ".go", ".rb", ".php", ".pl", ".groovy", ".clj",
    ".swift", ".js", ".jsx", ".apk", ".dex", ".elf", ".so", ".jar",
)


@dataclass
class Relation:
    source: str
    target: str
    relation: str
    evidence: str

    def as_dict(self) -> dict[str, str]:
        return {
            "source": self.source,
            "target": self.target,
            "relation": self.relation,
            "evidence": self.evidence,
        }


@dataclass
class Record:
    path: str
    sha256: str
    normalized_sha256: str | None
    size_bytes: int
    line_count: int | None
    media_class: str
    area: str
    owner_role: str
    classification: str
    lifecycle: str
    evidence_grade: str
    indexed: bool
    inbound_references: int
    outbound_references: int
    last_commit: str
    last_modified_at: str
    age_days: int | None
    review_interval_days: int
    review_due: bool | None
    sensitivity_flags: list[str] = field(default_factory=list)
    policy_flags: list[str] = field(default_factory=list)
    duplicate_group: str | None = None
    normalized_duplicate_group: str | None = None
    broken_references: int = 0
    risk_score: int = 0
    quality_score: int = 0
    route: str = "LINK_REQUIRED"
    route_reason: str = ""
    state_markers: list[str] = field(default_factory=list)

    def as_dict(self) -> dict[str, Any]:
        out = dict(self.__dict__)
        out["schema"] = RECORD_SCHEMA
        return out


def utc_now() -> dt.datetime:
    return dt.datetime.now(dt.timezone.utc)


def parse_iso(value: str) -> dt.datetime | None:
    if not value or value == "TOKEN_VAZIO":
        return None
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
        return parsed if parsed.tzinfo else parsed.replace(tzinfo=dt.timezone.utc)
    except ValueError:
        return None


def run_git(root: Path, args: list[str], *, binary: bool = False) -> bytes | str | None:
    try:
        return subprocess.check_output(
            ["git", *args],
            cwd=root,
            stderr=subprocess.DEVNULL,
            text=not binary,
        )
    except (OSError, subprocess.CalledProcessError):
        return None


def find_root(start: Path) -> Path:
    start = start.resolve()
    for candidate in (start, *start.parents):
        if (candidate / ".git").exists() or (candidate / "README.md").is_file():
            if (candidate / "docs").is_dir() and (candidate / "scripts").is_dir():
                return candidate
    raise SystemExit("document-governance: raiz do repositório não localizada")


def load_policy(path: Path) -> dict[str, Any]:
    try:
        policy = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"document-governance: política inválida: {exc}") from exc

    required = {"schema", "version", "scan", "areas", "canonical_indexes", "root_policy", "outputs"}
    missing = sorted(required - set(policy))
    if missing:
        raise SystemExit(f"document-governance: campos ausentes na política: {missing}")
    if policy["schema"] != "raf.document-governance-policy.v1":
        raise SystemExit("document-governance: schema de política incompatível")
    if not isinstance(policy["areas"], list) or not policy["areas"]:
        raise SystemExit("document-governance: áreas não configuradas")
    return policy


def tracked_paths(root: Path, policy: dict[str, Any]) -> list[Path]:
    raw = run_git(root, ["ls-files", "-z"], binary=True)
    if isinstance(raw, bytes):
        candidates = [root / os.fsdecode(item) for item in raw.split(b"\0") if item]
    else:
        candidates = [path for path in root.rglob("*") if path.is_file()]

    excluded = tuple(policy["scan"].get("exclude_globs", []))
    out: list[Path] = []
    for path in candidates:
        if not path.is_file():
            continue
        rel = path.relative_to(root).as_posix()
        if any(fnmatch.fnmatch(rel, pattern) for pattern in excluded):
            continue
        out.append(path)
    return sorted(out, key=lambda p: p.relative_to(root).as_posix().casefold())


def read_bytes(path: Path, limit: int | None = None) -> bytes:
    try:
        if limit is not None and path.stat().st_size > limit:
            return b""
        return path.read_bytes()
    except OSError:
        return b""


def read_text(path: Path, limit: int) -> str:
    raw = read_bytes(path, limit)
    if not raw:
        return ""
    if b"\0" in raw[:8192]:
        return ""
    return raw.decode("utf-8", errors="replace")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    try:
        with path.open("rb") as fh:
            for block in iter(lambda: fh.read(131072), b""):
                h.update(block)
    except OSError:
        return "TOKEN_VAZIO"
    return h.hexdigest()


def normalized_text_hash(text: str) -> str | None:
    if not text or len(text) < 200:
        return None
    normalized_lines = []
    for line in text.replace("\r\n", "\n").replace("\r", "\n").split("\n"):
        compact = " ".join(line.strip().split())
        if compact:
            normalized_lines.append(compact)
    if not normalized_lines:
        return None
    return sha256_bytes("\n".join(normalized_lines).encode("utf-8"))


def media_class(path: Path, text: str) -> str:
    suffix = path.suffix.casefold()
    if suffix in {".md", ".txt", ".rst", ".adoc"}:
        return "documentation"
    if suffix in {".json", ".jsonl", ".yaml", ".yml", ".toml", ".xml", ".csv"}:
        return "structured-data"
    if suffix in {".c", ".h", ".cc", ".cpp", ".s", ".asm", ".rs", ".go", ".java", ".kt"}:
        return "source"
    if suffix in {".py", ".sh", ".bash"}:
        return "automation"
    if suffix in {".apk", ".dex", ".elf", ".so", ".o", ".jar", ".zip", ".png", ".jpg", ".svg", ".pdf"}:
        return "artifact"
    return "text" if text else "binary-or-other"


def area_for(rel: str, policy: dict[str, Any]) -> dict[str, Any]:
    matches: list[tuple[int, dict[str, Any]]] = []
    for area in policy["areas"]:
        for prefix in area.get("prefixes", []):
            if prefix == "" or rel == prefix.rstrip("/") or rel.startswith(prefix):
                matches.append((len(prefix), area))
    if not matches:
        return {
            "id": "unclassified",
            "owner_role": "repository-maintainer",
            "classification": "INTERNAL",
            "review_interval_days": 90,
            "lifecycle": "ACTIVE",
        }
    return max(matches, key=lambda item: item[0])[1]


def current_commit(root: Path) -> str:
    value = run_git(root, ["rev-parse", "HEAD"])
    return value.strip() if isinstance(value, str) and value.strip() else "TOKEN_VAZIO"


def commit_time(root: Path) -> dt.datetime:
    value = run_git(root, ["show", "-s", "--format=%cI", "HEAD"])
    parsed = parse_iso(value.strip()) if isinstance(value, str) else None
    return parsed or utc_now()


def history_map(root: Path, tracked: set[str], max_commits: int) -> dict[str, tuple[str, str]]:
    """Resolve a última ocorrência por arquivo em uma única passagem de git log."""
    value = run_git(
        root,
        [
            "log",
            f"-n{max_commits}",
            "--date=iso-strict",
            "--format=@@%H%x1f%aI",
            "--name-only",
            "--no-renames",
        ],
    )
    if not isinstance(value, str):
        return {}

    result: dict[str, tuple[str, str]] = {}
    commit = "TOKEN_VAZIO"
    stamp = "TOKEN_VAZIO"
    for raw in value.splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.startswith("@@"):
            payload = line[2:].split("\x1f", 1)
            commit = payload[0]
            stamp = payload[1] if len(payload) > 1 else "TOKEN_VAZIO"
            continue
        rel = line.replace("\\", "/")
        if rel in tracked and rel not in result:
            result[rel] = (commit, stamp)
            if len(result) == len(tracked):
                break
    return result


def canonical_set(policy: dict[str, Any]) -> set[str]:
    return {str(path).replace("\\", "/") for path in policy["canonical_indexes"]}


def resolve_target(source_rel: str, token: str, tracked: set[str]) -> str | None:
    token = token.strip().strip("<>").split("#", 1)[0].split("?", 1)[0]
    if not token or "://" in token or token.startswith(("mailto:", "#", "/")):
        return None
    token = token.replace("\\", "/")
    if token.startswith("./"):
        token = token[2:]

    candidates = [token]
    source_parent = Path(source_rel).parent
    candidates.append((source_parent / token).as_posix())
    for candidate in candidates:
        normalized = str(Path(candidate)).replace("\\", "/")
        while normalized.startswith("../"):
            normalized = normalized[3:]
        if normalized in tracked:
            return normalized
    return None


def local_target_exists(root: Path, source_rel: str, token: str) -> bool:
    token = token.strip().strip("<>").split("#", 1)[0].split("?", 1)[0]
    if not token or "://" in token or token.startswith(("mailto:", "#")):
        return True
    candidates = [root / token, root / Path(source_rel).parent / token]
    for candidate in candidates:
        try:
            candidate.resolve().relative_to(root.resolve())
        except (OSError, ValueError):
            continue
        if candidate.exists():
            return True
    return False


def root_policy_flags(rel: str, policy: dict[str, Any]) -> list[str]:
    if "/" in rel:
        return []
    root_policy = policy.get("root_policy", {})
    allowed = set(root_policy.get("allowed_files", []))
    prefixes = tuple(root_policy.get("allowed_prefixes", []))
    if rel in allowed or rel.startswith(prefixes):
        return []
    return ["root_file_outside_policy"]


def extract_relations(root: Path, paths: list[Path], policy: dict[str, Any]) -> tuple[list[Relation], dict[str, list[str]]]:
    tracked = {path.relative_to(root).as_posix() for path in paths}
    relations: dict[tuple[str, str, str], Relation] = {}
    broken: dict[str, list[str]] = {}
    max_text = int(policy["scan"].get("max_text_bytes", 2_000_000))

    for path in paths:
        source = path.relative_to(root).as_posix()
        text = read_text(path, max_text)
        if not text:
            continue

        tokens: list[tuple[str, str]] = []
        tokens.extend(("markdown-link", token) for token in MARKDOWN_LINK.findall(text))
        for token in PATH_TOKEN.findall(text):
            if "/" in token or token.casefold().endswith(PATH_LIKE_SUFFIXES):
                tokens.append(("inline-path", token))

        for evidence, token in tokens:
            target = resolve_target(source, token, tracked)
            if target:
                key = (source, target, "references")
                relations[key] = Relation(source, target, "references", evidence)
            elif evidence == "markdown-link" and not local_target_exists(root, source, token):
                broken.setdefault(source, []).append(token)

    return sorted(relations.values(), key=lambda r: (r.source, r.target, r.relation)), broken


def duplicate_groups(records: Iterable[Record], attr: str) -> dict[str, list[str]]:
    groups: dict[str, list[str]] = {}
    for record in records:
        value = getattr(record, attr)
        if record.size_bytes == 0 or not value or value == "TOKEN_VAZIO":
            continue
        groups.setdefault(value, []).append(record.path)
    return {digest: sorted(paths) for digest, paths in groups.items() if len(paths) > 1}


def sensitivity_flags(rel: str, text: str, policy: dict[str, Any]) -> list[str]:
    flags: list[str] = []
    lowered = rel.casefold()
    for item in policy.get("sensitivity", {}).get("filename_globs", []):
        if fnmatch.fnmatch(lowered, str(item["glob"]).casefold()):
            flags.append(str(item["id"]))
    for detector_id, pattern in SECRET_VALUE_PATTERNS:
        if text and pattern.search(text):
            flags.append(detector_id)
    return sorted(set(flags))


def lifecycle_for(rel: str, area: dict[str, Any], canonical: set[str], policy: dict[str, Any]) -> str:
    if rel in canonical:
        return "CANONICAL"
    for prefix in policy["scan"].get("generated_prefixes", []):
        if rel.startswith(prefix):
            return "GENERATED"
    if TEST_PATH.search(rel):
        return "EVIDENCE"
    return str(area.get("lifecycle", "ACTIVE"))


def evidence_grade(
    rel: str,
    indexed: bool,
    text: str,
    related_paths: set[str],
    lifecycle: str,
) -> str:
    grade = 0
    if indexed:
        grade = 1
    if text and STATE_TOKEN.search(text):
        grade = max(grade, 2)
    if any(TEST_PATH.search(path) or WORKFLOW_PATH.search(path) for path in related_paths):
        grade = max(grade, 3)
    if lifecycle == "EVIDENCE" and text and HASH_TOKEN.search(text) and COMMAND_TOKEN.search(text):
        grade = max(grade, 4)
    return f"E{grade}"


def score_record(record: Record) -> tuple[int, int]:
    quality = 10
    quality += 20 if record.indexed else 0
    quality += 10 if record.inbound_references else 0
    quality += 10 if record.outbound_references else 0
    quality += 15 if record.owner_role != "repository-maintainer" or record.area != "unclassified" else 0
    quality += 10 if record.last_commit != "TOKEN_VAZIO" else 0
    quality += {"E0": 0, "E1": 5, "E2": 10, "E3": 20, "E4": 25}.get(record.evidence_grade, 0)
    if record.review_due is False:
        quality += 5
    quality -= min(20, record.broken_references * 5)
    quality -= 20 if record.sensitivity_flags else 0
    quality -= 10 if record.policy_flags else 0
    quality = max(0, min(100, quality))

    risk = 0
    risk += 25 if not record.indexed else 0
    risk += 15 if record.inbound_references == 0 else 0
    risk += 15 if record.review_due is True else 0
    risk += min(20, record.broken_references * 5)
    risk += 40 if record.sensitivity_flags else 0
    risk += 20 if record.policy_flags else 0
    risk += 15 if record.duplicate_group else 0
    risk += 8 if record.normalized_duplicate_group and not record.duplicate_group else 0
    risk += 15 if record.area == "unclassified" else 0
    risk = max(0, min(100, risk))
    return quality, risk


def route_record(record: Record, canonical: set[str]) -> tuple[str, str]:
    blocking_sensitive = {
        "private_key_block", "private_key_filename", "keystore_filename", "jks_filename"
    }
    if blocking_sensitive.intersection(record.sensitivity_flags):
        return "QUARANTINE_REVIEW", "material potencialmente secreto; conteúdo não é exposto no relatório"
    if record.sensitivity_flags:
        return "SENSITIVITY_REVIEW", "padrão sensível candidato; requer revisão humana"
    if record.path in canonical:
        return "CANONICAL", "entrada de governança ou índice canônico"
    if record.duplicate_group:
        return "DUPLICATE_REVIEW", "conteúdo byte-a-byte duplicado"
    if record.policy_flags:
        return "ROOT_REVIEW", "arquivo de raiz fora da política declarada"
    if record.area == "unclassified":
        return "OWNER_REQUIRED", "arquivo não pertence a área declarada"
    if record.broken_references:
        return "REFERENCE_REPAIR", "referência local quebrada"
    if not record.indexed or record.inbound_references == 0:
        return "LINK_REQUIRED", "sem relação de entrada em índice ou documento"
    if record.review_due is True:
        return "REVIEW_STALE", "intervalo de revisão ultrapassado"
    return "INDEXED", "referenciado e classificado"


def build_catalog(root: Path, policy: dict[str, Any], max_history_commits: int) -> dict[str, Any]:
    paths = tracked_paths(root, policy)
    rels = [path.relative_to(root).as_posix() for path in paths]
    tracked = set(rels)
    canonical = canonical_set(policy)
    relations, broken = extract_relations(root, paths, policy)

    inbound: dict[str, set[str]] = {rel: set() for rel in rels}
    outbound: dict[str, set[str]] = {rel: set() for rel in rels}
    for relation in relations:
        inbound.setdefault(relation.target, set()).add(relation.source)
        outbound.setdefault(relation.source, set()).add(relation.target)

    history = history_map(root, tracked, max_history_commits)
    as_of = commit_time(root)
    max_text = int(policy["scan"].get("max_text_bytes", 2_000_000))

    records: list[Record] = []
    for path, rel in zip(paths, rels):
        text = read_text(path, max_text)
        raw_sha = file_sha256(path)
        area = area_for(rel, policy)
        last_commit, last_at = history.get(rel, ("TOKEN_VAZIO", "TOKEN_VAZIO"))
        modified = parse_iso(last_at)
        age = max(0, (as_of - modified).days) if modified else None
        review_interval = int(area.get("review_interval_days", 180))
        review_due = age > review_interval if age is not None else None
        lifecycle = lifecycle_for(rel, area, canonical, policy)
        indexed = rel in canonical or bool(inbound.get(rel))
        related = set(inbound.get(rel, set())) | set(outbound.get(rel, set()))
        state_markers = sorted(set(STATE_TOKEN.findall(text))) if text else []
        record = Record(
            path=rel,
            sha256=raw_sha,
            normalized_sha256=normalized_text_hash(text),
            size_bytes=path.stat().st_size,
            line_count=(text.count("\n") + 1) if text else None,
            media_class=media_class(path, text),
            area=str(area["id"]),
            owner_role=str(area["owner_role"]),
            classification=str(area.get("classification", "INTERNAL")),
            lifecycle=lifecycle,
            evidence_grade=evidence_grade(rel, indexed, text, related, lifecycle),
            indexed=indexed,
            inbound_references=len(inbound.get(rel, set())),
            outbound_references=len(outbound.get(rel, set())),
            last_commit=last_commit,
            last_modified_at=last_at,
            age_days=age,
            review_interval_days=review_interval,
            review_due=review_due,
            sensitivity_flags=sensitivity_flags(rel, text, policy),
            policy_flags=root_policy_flags(rel, policy),
            broken_references=len(broken.get(rel, [])),
            state_markers=state_markers,
        )
        records.append(record)

    exact = duplicate_groups(records, "sha256")
    normalized = duplicate_groups(records, "normalized_sha256")
    exact_id = {path: digest for digest, group in exact.items() for path in group}
    normalized_id = {path: digest for digest, group in normalized.items() for path in group}

    for record in records:
        record.duplicate_group = exact_id.get(record.path)
        record.normalized_duplicate_group = normalized_id.get(record.path)
        record.quality_score, record.risk_score = score_record(record)
        record.route, record.route_reason = route_record(record, canonical)

    records.sort(key=lambda r: r.path.casefold())
    relations_dict = [relation.as_dict() for relation in relations]
    review_records = sorted(
        (record for record in records if record.route not in {"CANONICAL", "INDEXED"}),
        key=lambda r: (-r.risk_score, r.route, r.path.casefold()),
    )
    blockers = [
        record for record in records
        if record.route in set(policy.get("blocking_routes", ["QUARANTINE_REVIEW"]))
    ]
    missing_canonical = sorted(path for path in canonical if path not in tracked)

    route_counts: dict[str, int] = {}
    area_counts: dict[str, int] = {}
    classification_counts: dict[str, int] = {}
    for record in records:
        route_counts[record.route] = route_counts.get(record.route, 0) + 1
        area_counts[record.area] = area_counts.get(record.area, 0) + 1
        classification_counts[record.classification] = classification_counts.get(record.classification, 0) + 1

    state = "FAIL" if blockers or missing_canonical else ("REVIEW_REQUIRED" if review_records else "PASS")
    return {
        "schema": SCHEMA,
        "generated_at": as_of.isoformat().replace("+00:00", "Z"),
        "commit": current_commit(root),
        "state": state,
        "claim_allowed": state == "PASS",
        "policy_version": policy["version"],
        "summary": {
            "files": len(records),
            "relations": len(relations),
            "broken_reference_sources": len(broken),
            "exact_duplicate_groups": len(exact),
            "normalized_duplicate_groups": len(normalized),
            "review_queue": len(review_records),
            "blockers": len(blockers),
            "missing_canonical_indexes": missing_canonical,
            "routes": dict(sorted(route_counts.items())),
            "areas": dict(sorted(area_counts.items())),
            "classifications": dict(sorted(classification_counts.items())),
        },
        "records": [record.as_dict() for record in records],
        "relations": relations_dict,
        "broken_references": {key: sorted(set(values)) for key, values in sorted(broken.items())},
        "duplicates": {
            "exact": exact,
            "normalized_candidates": normalized,
        },
        "review_queue": [record.as_dict() for record in review_records],
        "blockers": [record.as_dict() for record in blockers],
    }


def json_dumps(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n"


def jsonl(records: Iterable[dict[str, Any]]) -> str:
    return "".join(json.dumps(record, ensure_ascii=False, sort_keys=True) + "\n" for record in records)


def markdown_index(report: dict[str, Any]) -> str:
    summary = report["summary"]
    lines = [
        "# Índice gerado de governança documental",
        "",
        "> Fonte: `scripts/document_governance.py`. Este arquivo descreve o catálogo",
        "> versionado; não promove implementação ou prova apenas pela existência.",
        "",
        f"- Commit: `{report['commit']}`",
        f"- Estado: `{report['state']}`",
        f"- Arquivos: **{summary['files']}**",
        f"- Relações: **{summary['relations']}**",
        f"- Fila de revisão: **{summary['review_queue']}**",
        f"- Bloqueadores: **{summary['blockers']}**",
        "",
        "## Distribuição por rota",
        "",
        "| Rota | Quantidade |",
        "|---|---:|",
    ]
    for route, count in summary["routes"].items():
        lines.append(f"| `{route}` | {count} |")

    lines.extend([
        "",
        "## Entradas canônicas",
        "",
        "| Arquivo | Área | Evidência | Qualidade | Risco |",
        "|---|---|---|---:|---:|",
    ])
    for record in report["records"]:
        if record["route"] == "CANONICAL":
            lines.append(
                f"| `{record['path']}` | {record['area']} | {record['evidence_grade']} | "
                f"{record['quality_score']} | {record['risk_score']} |"
            )

    lines.extend([
        "",
        "## Contrato operacional",
        "",
        "```text",
        "arquivo → identidade SHA-256 → área → dono lógico → relações → evidência",
        "       → temporalidade → risco → rota → revisão/promoção",
        "```",
        "",
        "O catálogo completo está em `results/document-governance/catalog.jsonl`.",
        "",
    ])
    return "\n".join(lines)


def markdown_queue(report: dict[str, Any]) -> str:
    lines = [
        "# Fila gerada de revisão documental",
        "",
        "> Ordenada por risco decrescente. Nenhum item é movido ou removido automaticamente.",
        "",
        "| Risco | Rota | Arquivo | Área | Motivo |",
        "|---:|---|---|---|---|",
    ]
    for record in report["review_queue"]:
        lines.append(
            f"| {record['risk_score']} | `{record['route']}` | `{record['path']}` | "
            f"{record['area']} | {record['route_reason']} |"
        )
    if not report["review_queue"]:
        lines.append("| 0 | `PASS` | — | — | sem itens pendentes |")
    lines.extend([
        "",
        "## Ordem de tratamento",
        "",
        "1. `QUARANTINE_REVIEW`: risco de segredo ou dado impróprio para versionamento.",
        "2. `SENSITIVITY_REVIEW`: possível dado sensível que exige confirmação.",
        "3. `REFERENCE_REPAIR`: referências locais quebradas.",
        "4. `ROOT_REVIEW`: arquivo de raiz fora da política declarada.",
        "5. `OWNER_REQUIRED`: arquivo sem área e responsável lógico.",
        "6. `DUPLICATE_REVIEW`: duplicidade exata; preservar antes de consolidar.",
        "7. `LINK_REQUIRED`: conteúdo sem relação de entrada.",
        "8. `REVIEW_STALE`: revisão temporal vencida.",
        "",
    ])
    return "\n".join(lines)


def output_payloads(report: dict[str, Any], policy: dict[str, Any]) -> dict[str, str]:
    outputs = policy["outputs"]
    return {
        outputs["summary"]: json_dumps({
            key: report[key]
            for key in ("schema", "generated_at", "commit", "state", "claim_allowed", "policy_version", "summary")
        }),
        outputs["catalog"]: jsonl(report["records"]),
        outputs["relations"]: jsonl(report["relations"]),
        outputs["duplicates"]: json_dumps(report["duplicates"]),
        outputs["review_json"]: json_dumps({
            "schema": SCHEMA,
            "commit": report["commit"],
            "state": report["state"],
            "review_queue": report["review_queue"],
            "blockers": report["blockers"],
            "broken_references": report["broken_references"],
        }),
        outputs["index_markdown"]: markdown_index(report),
        outputs["review_markdown"]: markdown_queue(report),
    }


def write_outputs(root: Path, payloads: dict[str, str]) -> None:
    for rel, content in payloads.items():
        target = root / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content, encoding="utf-8")


def check_outputs(root: Path, payloads: dict[str, str]) -> list[str]:
    drift: list[str] = []
    for rel, expected in payloads.items():
        path = root / rel
        try:
            current = path.read_text(encoding="utf-8")
        except OSError:
            drift.append(rel)
            continue
        if current != expected:
            drift.append(rel)
    return drift


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Catálogo e governança documental determinísticos")
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--policy", default="configs/document-governance.v1.json")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--strict", action="store_true", help="fila de revisão também falha o gate")
    parser.add_argument("--max-history-commits", type=int, default=2000)
    parser.add_argument("--print-summary", action="store_true")
    args = parser.parse_args(argv)

    root = find_root(args.root)
    policy_path = Path(args.policy)
    if not policy_path.is_absolute():
        policy_path = root / policy_path
    policy = load_policy(policy_path)
    report = build_catalog(root, policy, max(1, args.max_history_commits))
    payloads = output_payloads(report, policy)

    if args.write:
        write_outputs(root, payloads)
    drift = check_outputs(root, payloads) if args.check else []

    if args.print_summary or not args.write:
        sys.stdout.write(json_dumps({
            "schema": report["schema"],
            "commit": report["commit"],
            "state": report["state"],
            "summary": report["summary"],
            "drift": drift,
        }))

    if drift:
        print(f"document-governance: generated output drift: {', '.join(drift)}", file=sys.stderr)
        return 1
    if report["state"] == "FAIL":
        print("document-governance: critical blockers detected", file=sys.stderr)
        return 1
    if args.strict and report["state"] != "PASS":
        print("document-governance: review queue is non-empty", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
