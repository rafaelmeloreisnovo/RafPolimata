#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

REPORT_SCHEMA = "raf.runtime-doctor-agent-report.v1"
SKILL_SCHEMA = "raf.runtime-doctor-skills.v1"
BUILD_DOCTOR_SCHEMA = "raf.ecosystem-build-doctor-report.v1"
ALLOWED_PROBE_EXECUTABLES = {"sh", "bash", "python3", "python"}
PASS_STATES = {"PASS", "OK", "VERIFIED_BY_EXECUTION"}
FAIL_STATES = {"FAIL", "ERROR", "BLOCKED"}


@dataclass(frozen=True)
class Skill:
    id: str
    level: str
    repo: str
    role: str
    probe: tuple[str, ...]
    keywords: tuple[str, ...]
    evidence: str


class Trace:
    def __init__(self, verbose: bool) -> None:
        self.verbose = verbose
        self.events: list[dict[str, Any]] = []

    def add(self, stage: str, message: str, **data: Any) -> None:
        event = {"stage": stage, "message": message, **data}
        self.events.append(event)
        if self.verbose:
            suffix = "" if not data else " " + json.dumps(data, ensure_ascii=False, sort_keys=True)
            print(f"[runtime-doctor:{stage}] {message}{suffix}", file=sys.stderr)


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_skills(path: Path) -> tuple[dict[str, Any], list[Skill]]:
    raw = load_json(path)
    if raw.get("schema") != SKILL_SCHEMA:
        raise ValueError(f"skill registry schema mismatch: {raw.get('schema')!r}")
    skills = []
    for item in raw.get("skills", []):
        skills.append(
            Skill(
                id=str(item["id"]),
                level=str(item["level"]),
                repo=str(item["repo"]),
                role=str(item["role"]),
                probe=tuple(str(x) for x in item.get("probe", [])),
                keywords=tuple(str(x).lower() for x in item.get("keywords", [])),
                evidence=str(item["evidence"]),
            )
        )
    return raw, skills


def parse_repo_args(items: Iterable[str]) -> dict[str, Path]:
    out: dict[str, Path] = {}
    for item in items:
        if "=" not in item:
            raise ValueError(f"--repo expects name=path, got {item!r}")
        name, value = item.split("=", 1)
        name = name.strip()
        if not name:
            raise ValueError("repository name cannot be empty")
        out[name] = Path(value).expanduser().resolve()
    return out


def auto_detect_repos(workspace: Path, skills: Iterable[Skill], trace: Trace) -> dict[str, Path]:
    repos: dict[str, Path] = {}
    for skill in skills:
        if skill.repo.startswith("TOKEN_VAZIO"):
            continue
        candidate = (workspace / skill.repo).resolve()
        if candidate.is_dir():
            repos[skill.repo] = candidate
            trace.add("inventory", "repository auto-detected", repo=skill.repo, path=str(candidate))
    return repos


def parse_last_json_line(text: str) -> dict[str, Any] | None:
    for line in reversed(text.splitlines()):
        line = line.strip()
        if not line.startswith("{"):
            continue
        try:
            value = json.loads(line)
        except json.JSONDecodeError:
            continue
        if isinstance(value, dict):
            return value
    return None


def safe_probe_command(command: tuple[str, ...]) -> bool:
    if not command:
        return False
    exe = Path(command[0]).name
    if exe not in ALLOWED_PROBE_EXECUTABLES:
        return False
    joined = " ".join(command)
    forbidden = (" rm ", " sudo ", " su ", " apt install", " pkg install", " git push", " git reset", " git clean")
    padded = f" {joined.lower()} "
    return not any(token in padded for token in forbidden)


def run_probe(skill: Skill, repo_root: Path, timeout_s: int, trace: Trace) -> dict[str, Any]:
    if not skill.probe:
        return {
            "skill_id": skill.id,
            "state": "TOKEN_VAZIO_NO_RUNTIME_PROBE",
            "repo": skill.repo,
            "evidence": skill.evidence,
        }
    if not safe_probe_command(skill.probe):
        return {
            "skill_id": skill.id,
            "state": "BLOCKED_UNSAFE_PROBE",
            "repo": skill.repo,
            "command": list(skill.probe),
        }

    trace.add("probe", "executing read-only probe", skill=skill.id, repo=skill.repo, command=list(skill.probe))
    try:
        proc = subprocess.run(
            list(skill.probe),
            cwd=repo_root,
            text=True,
            capture_output=True,
            timeout=timeout_s,
            check=False,
            env={**os.environ, "LC_ALL": "C"},
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        return {
            "skill_id": skill.id,
            "state": "FAIL_PROBE_EXECUTION",
            "repo": skill.repo,
            "error": str(exc),
        }

    payload = parse_last_json_line(proc.stdout)
    state = "PASS" if proc.returncode == 0 else "FAIL"
    if proc.returncode == 0 and payload is None:
        state = "INCOMPLETE_JSON_NOT_FOUND"
    return {
        "skill_id": skill.id,
        "state": state,
        "repo": skill.repo,
        "exit_code": proc.returncode,
        "command": list(skill.probe),
        "payload": payload,
        "stdout_tail": proc.stdout[-4000:],
        "stderr_tail": proc.stderr[-4000:],
    }


def execute_selected_probes(
    selected: list[Skill],
    repos: dict[str, Path],
    timeout_s: int,
    trace: Trace,
) -> list[dict[str, Any]]:
    probes: list[dict[str, Any]] = []
    cache: dict[tuple[str, tuple[str, ...]], dict[str, Any]] = {}

    for skill in selected:
        repo_root = repos.get(skill.repo)
        if repo_root is None:
            probes.append(
                {
                    "skill_id": skill.id,
                    "state": "TOKEN_VAZIO_REPOSITORY_NOT_AVAILABLE",
                    "repo": skill.repo,
                    "probe_cache": {"reused": False, "source_skill_id": None},
                }
            )
            continue

        if not skill.probe:
            record = run_probe(skill, repo_root, timeout_s, trace)
            record["probe_cache"] = {"reused": False, "source_skill_id": skill.id}
            probes.append(record)
            continue

        key = (str(repo_root), skill.probe)
        if key in cache:
            source = cache[key]
            record = dict(source)
            record["skill_id"] = skill.id
            record["repo"] = skill.repo
            record["probe_cache"] = {
                "reused": True,
                "source_skill_id": str(source.get("skill_id", "")),
            }
            trace.add(
                "probe-cache",
                "reused identical read-only probe result",
                skill=skill.id,
                source_skill_id=record["probe_cache"]["source_skill_id"],
                repo=skill.repo,
                command=list(skill.probe),
            )
            probes.append(record)
            continue

        record = run_probe(skill, repo_root, timeout_s, trace)
        record["probe_cache"] = {"reused": False, "source_skill_id": skill.id}
        cache[key] = dict(record)
        probes.append(record)

    return probes


def load_outcome_history(path: Path | None, trace: Trace) -> dict[str, Counter[str]]:
    history: dict[str, Counter[str]] = defaultdict(Counter)
    if path is None or not path.exists():
        return history

    files = [path] if path.is_file() else sorted(path.glob("*.json*"))
    for file_path in files:
        try:
            if file_path.suffix == ".jsonl":
                records = []
                for line in file_path.read_text(encoding="utf-8").splitlines():
                    if line.strip():
                        records.append(json.loads(line))
            else:
                value = load_json(file_path)
                records = value if isinstance(value, list) else [value]
        except (OSError, json.JSONDecodeError):
            trace.add("learning", "history file unreadable", path=str(file_path))
            continue

        for record in records:
            if not isinstance(record, dict):
                continue
            if "skill_outcomes" in record and isinstance(record["skill_outcomes"], list):
                candidates = record["skill_outcomes"]
            else:
                candidates = [record]
            for outcome in candidates:
                if not isinstance(outcome, dict):
                    continue
                skill_id = str(outcome.get("skill_id", ""))
                result = str(outcome.get("result", outcome.get("state", ""))).upper()
                if skill_id and result:
                    history[skill_id][result] += 1
    return history


def confidence_for(counter: Counter[str]) -> dict[str, Any]:
    passes = sum(v for k, v in counter.items() if k in PASS_STATES)
    failures = sum(v for k, v in counter.items() if k in FAIL_STATES)
    observations = passes + failures
    if observations == 0:
        return {
            "state": "TOKEN_VAZIO_NO_OUTCOME_HISTORY",
            "observations": 0,
            "success_rate_laplace": None,
        }
    return {
        "state": "EVIDENCE_WEIGHT_AVAILABLE",
        "observations": observations,
        "passes": passes,
        "failures": failures,
        "success_rate_laplace": round((passes + 1) / (observations + 2), 6),
    }


def selected_skills(skills: list[Skill], symptoms: list[str]) -> list[Skill]:
    if not symptoms:
        return skills
    terms = {token.lower() for symptom in symptoms for token in symptom.replace("/", " ").split()}
    selected = []
    for skill in skills:
        if any(keyword in terms for keyword in skill.keywords):
            selected.append(skill)
    return selected or skills


def diagnostic_payloads(probes: list[dict[str, Any]]) -> list[tuple[str, dict[str, Any]]]:
    out = []
    for probe in probes:
        payload = probe.get("payload")
        if isinstance(payload, dict):
            out.append((str(probe.get("skill_id", "unknown")), payload))
    return out


def route_from_runtime(probes: list[dict[str, Any]], trace: Trace) -> list[dict[str, Any]]:
    routes: list[dict[str, Any]] = []
    seen_codes: set[str] = set()

    def add(code: str, state: str, source: str, reason: str, next_action: str) -> None:
        if code in seen_codes:
            return
        seen_codes.add(code)
        routes.append(
            {
                "code": code,
                "state": state,
                "source": source,
                "reason": reason,
                "next_action": next_action,
            }
        )
        trace.add("route", reason, code=code, state=state, source=source)

    for skill_id, payload in diagnostic_payloads(probes):
        arch = str(payload.get("arch", "unknown"))
        neon = payload.get("neon") is True
        opencl = payload.get("opencl") is True
        vulkan = payload.get("vulkan") is True
        ram = payload.get("ram_avail")
        oom = payload.get("oom")
        page_sz = payload.get("page_sz")

        if arch.startswith("armv7") or arch in {"armv8l", "arm"}:
            add(
                "ARM32_ROUTE",
                "CANDIDATE",
                skill_id,
                "ARM32/AArch32 observed; ABI-specific build flags and NEON availability must be preserved.",
                "Run the ARMv7 build/receipt path; do not reuse ARM64 artifacts.",
            )
        elif arch == "aarch64":
            add(
                "ARM64_ROUTE",
                "CANDIDATE",
                skill_id,
                "AArch64 observed.",
                "Use the arm64-v8a/AArch64 build path and preserve a device runtime receipt.",
            )

        if vulkan:
            add(
                "LLAMA_VULKAN_CANDIDATE",
                "CANDIDATE_NOT_BENCHMARKED",
                skill_id,
                "Vulkan loader was observed by the host diagnostic.",
                "Build llamaRafaelia with GGML_VULKAN=ON, then compare llama-bench CPU vs Vulkan and preserve the JSON receipt.",
            )
        else:
            add(
                "LLAMA_CPU_FALLBACK",
                "SAFE_FALLBACK_CANDIDATE",
                skill_id,
                "No Vulkan loader was observed in this diagnostic payload.",
                "Use CPU/NEON as fallback and benchmark before making performance claims.",
            )

        if opencl:
            add(
                "OPENCL_GENERAL_GPU_CANDIDATE",
                "CANDIDATE_NOT_VALIDATED",
                skill_id,
                "An OpenCL library/platform candidate was reported.",
                "Run a minimal OpenCL kernel receipt; do not infer llama.cpp OpenCL acceleration from library presence alone.",
            )

        if neon:
            add(
                "NEON_ACCELERATION_CANDIDATE",
                "OBSERVED_FEATURE",
                skill_id,
                "NEON was reported by the host diagnostic.",
                "Use the NEON-specific benchmark/self-test path and retain compiler flags in the receipt.",
            )

        if isinstance(ram, int):
            if ram < 512000:
                add(
                    "MEMORY_PRESSURE_CRITICAL",
                    "HIGH_RISK",
                    skill_id,
                    f"Available RAM is {ram} kB, below the diagnostic 512 MB floor.",
                    "Reduce model/context/batch, avoid parallel VM+LLM load, and collect an OOM-safe runtime receipt.",
                )
            elif ram < 1048576:
                add(
                    "MEMORY_PRESSURE",
                    "REVIEW_REQUIRED",
                    skill_id,
                    f"Available RAM is {ram} kB, below 1 GiB.",
                    "Prefer smaller quantization/context and benchmark memory before enabling VM+LLM concurrency.",
                )

        if isinstance(oom, int) and oom > 500:
            add(
                "OOM_KILL_RISK",
                "HIGH_RISK",
                skill_id,
                f"oom_score_adj={oom} is high.",
                "Collect Android/Termux process pressure evidence before long-running inference or guest boot.",
            )

        if page_sz == 16384:
            add(
                "ANDROID_16K_PAGE_CANDIDATE",
                "OBSERVED_RUNTIME_PROPERTY",
                skill_id,
                "16 KiB page size was observed.",
                "Run native-library compatibility gates with 16 KiB page assumptions explicitly recorded.",
            )

    return routes


def symptom_routes(
    skills: list[Skill],
    symptoms: list[str],
    repos: dict[str, Path],
    history: dict[str, Counter[str]],
) -> list[dict[str, Any]]:
    routes = []
    for skill in selected_skills(skills, symptoms):
        repo_state = "TOKEN_VAZIO_REPOSITORY_NOT_FOUND"
        if skill.repo in repos:
            repo_state = "AVAILABLE"
        elif skill.repo.startswith("TOKEN_VAZIO"):
            repo_state = skill.repo
        routes.append(
            {
                "skill_id": skill.id,
                "level": skill.level,
                "repo": skill.repo,
                "repo_state": repo_state,
                "role": skill.role,
                "evidence_contract": skill.evidence,
                "learning": confidence_for(history[skill.id]),
            }
        )
    return routes


def reachable_route_graph(registry: dict[str, Any], seed_ids: set[str]) -> list[list[str]]:
    raw_edges = registry.get("route_graph", [])
    edges: list[tuple[str, str]] = []
    for item in raw_edges:
        if isinstance(item, list) and len(item) == 2:
            edges.append((str(item[0]), str(item[1])))

    reachable = set(seed_ids)
    used: list[list[str]] = []
    changed = True
    while changed:
        changed = False
        for src, dst in edges:
            if src in reachable and [src, dst] not in used:
                used.append([src, dst])
                if dst != "L7" and dst not in reachable:
                    reachable.add(dst)
                    changed = True
    return used


def load_build_doctor_reports(paths: Iterable[str], trace: Trace) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for raw_path in paths:
        path = Path(raw_path).expanduser().resolve()
        base = {
            "source_name": path.name,
            "source_sha256": None,
            "state": "TOKEN_VAZIO_NOT_READ",
        }
        try:
            base["source_sha256"] = sha256_file(path)
            payload = load_json(path)
        except (OSError, json.JSONDecodeError) as exc:
            base["state"] = "BLOCKED_UNREADABLE_REPORT"
            base["error"] = str(exc)
            records.append(base)
            trace.add("evidence", "Build Doctor report unreadable", source_name=path.name)
            continue

        if not isinstance(payload, dict) or payload.get("schema") != BUILD_DOCTOR_SCHEMA:
            base["state"] = "BLOCKED_SCHEMA_MISMATCH"
            base["observed_schema"] = payload.get("schema") if isinstance(payload, dict) else None
            records.append(base)
            trace.add(
                "evidence",
                "Build Doctor report schema mismatch",
                source_name=path.name,
                observed_schema=base.get("observed_schema"),
            )
            continue

        summary = payload.get("summary") if isinstance(payload.get("summary"), dict) else {}
        claim_boundary = payload.get("claim_boundary") if isinstance(payload.get("claim_boundary"), dict) else {}
        base.update(
            {
                "state": "INGESTED_HASH_BOUND_REPORT",
                "summary": {
                    "state": summary.get("state", "TOKEN_VAZIO_SUMMARY_STATE"),
                    "highest_severity": summary.get("highest_severity", "TOKEN_VAZIO"),
                    "findings": summary.get("findings", 0),
                    "by_code": summary.get("by_code", {}),
                    "by_repo": summary.get("by_repo", {}),
                },
                "claim_boundary": claim_boundary,
            }
        )
        records.append(base)
        trace.add(
            "evidence",
            "Build Doctor report ingested",
            source_name=path.name,
            source_sha256=base["source_sha256"],
            state=base["summary"]["state"],
        )
    return records


def build_gap_ledger(
    args: argparse.Namespace,
    skill_routes: list[dict[str, Any]],
    probes: list[dict[str, Any]],
    build_doctor_evidence: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    gaps: list[dict[str, Any]] = []

    def add(gap_id: str, urgency: str, state: str, provenance: str, f_next: str) -> None:
        gaps.append(
            {
                "id": gap_id,
                "urgency": urgency,
                "state": state,
                "provenance": provenance,
                "F_next": f_next,
            }
        )

    selected_ids = {str(item.get("skill_id", "")) for item in skill_routes}

    if not args.execute_probes:
        add(
            "GAP-RD-RUNTIME-EXECUTION",
            "P0",
            "TOKEN_VAZIO_NOT_EXECUTED",
            "runtime-doctor invocation lacks --execute-probes",
            "Run allowlisted read-only probes on the target runtime and preserve the JSON receipt.",
        )

    for item in skill_routes:
        repo_state = str(item.get("repo_state", ""))
        if repo_state.startswith("TOKEN_VAZIO"):
            add(
                f"GAP-RD-REPO-{item.get('skill_id', 'UNKNOWN')}",
                "P1",
                repo_state,
                f"skill registry route for {item.get('skill_id')}",
                "Resolve the exact repository/provider identity before promoting this route.",
            )

    for probe in probes:
        probe_state = str(probe.get("state", ""))
        if probe_state.startswith(("FAIL", "BLOCKED", "INCOMPLETE", "TOKEN_VAZIO")):
            urgency = "P0" if probe_state.startswith(("FAIL", "BLOCKED")) else "P1"
            add(
                f"GAP-RD-PROBE-{probe.get('skill_id', 'UNKNOWN')}",
                urgency,
                probe_state,
                f"probe:{probe.get('skill_id', 'unknown')}",
                "Preserve stdout/stderr and resolve the probe-specific blocker without converting absence into PASS.",
            )

    if not args.build_doctor_report:
        add(
            "GAP-RD-BUILD-DOCTOR-EVIDENCE",
            "P1",
            "TOKEN_VAZIO_BUILD_DOCTOR_REPORT_NOT_INGESTED",
            "no --build-doctor-report input",
            "Run Ecosystem Build Doctor separately, preserve its JSON report, then ingest it by exact file hash.",
        )
    else:
        for record in build_doctor_evidence:
            state = str(record.get("state", ""))
            if state.startswith("BLOCKED"):
                add(
                    "GAP-RD-BUILD-DOCTOR-INPUT",
                    "P0",
                    state,
                    str(record.get("source_name", "build-doctor-report")),
                    "Repair the evidence input/schema and re-ingest; do not infer static health from an unreadable report.",
                )
            summary = record.get("summary")
            if isinstance(summary, dict) and summary.get("state") == "REVIEW_REQUIRED":
                add(
                    "GAP-RD-BUILD-DOCTOR-FINDINGS",
                    "P1",
                    "OPEN_STATIC_FINDINGS",
                    f"{record.get('source_name')}@{record.get('source_sha256')}",
                    "Route high/critical static findings to source-specific fixes and independent build/runtime gates.",
                )

    if "llama_backend_doctor" in selected_ids:
        add(
            "GAP-RD-LLAMA-BENCHMARK",
            "P1",
            "TOKEN_VAZIO_BENCHMARK_REQUIRED",
            "llama_backend_doctor claim boundary",
            "Run matched CPU/Vulkan benchmarks with model/config/artifact hashes.",
        )

    if "qemu_runtime" in selected_ids or "vectras_vm_runtime" in selected_ids:
        add(
            "GAP-RD-VM-GUEST-BOOT",
            "P1",
            "TOKEN_VAZIO_DEDICATED_RECEIPT_REQUIRED",
            "VM/QEMU runtime claim boundary",
            "Produce a dedicated guest-boot receipt with command, image hash, exit state and boot marker.",
        )

    if "frida_runtime_observer" in selected_ids:
        add(
            "GAP-RD-FRIDA-PHYSICAL",
            "P0",
            "TOKEN_VAZIO_PHYSICAL_DEVICE_RECEIPT_REQUIRED",
            "L2.5 dynamic observability boundary",
            "Execute the read-only readiness probe on the user-controlled physical device before any dynamic capability promotion.",
        )

    return gaps


def append_outcomes(path: Path, outcomes: list[str], trace: Trace) -> None:
    if not outcomes:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    now = datetime.now(timezone.utc).isoformat()
    with path.open("a", encoding="utf-8") as fh:
        for item in outcomes:
            if "=" not in item:
                raise ValueError(f"--outcome expects skill_id=PASS|FAIL, got {item!r}")
            skill_id, result = item.split("=", 1)
            result = result.upper().strip()
            if result not in {"PASS", "FAIL"}:
                raise ValueError("outcome must be PASS or FAIL")
            record = {"timestamp": now, "skill_id": skill_id.strip(), "result": result}
            fh.write(json.dumps(record, ensure_ascii=False, sort_keys=True) + "\n")
            trace.add("learning", "outcome appended", **record)


def markdown_report(report: dict[str, Any]) -> str:
    lines = [
        "# Runtime Doctor Agent Report",
        "",
        f"- state: `{report['summary']['state']}`",
        f"- selected skills: `{report['summary']['selected_skills']}`",
        f"- probes executed: `{report['summary']['probes_executed']}`",
        f"- probe cache hits: `{report['summary']['probe_cache_hits']}`",
        f"- runtime routes: `{report['summary']['runtime_routes']}`",
        f"- open gaps: `{report['summary']['open_gaps']}`",
        "",
        "## Skill map",
        "",
    ]
    for item in report["skill_routes"]:
        lines.append(
            f"- **{item['level']} / {item['skill_id']}** — {item['repo_state']} — {item['role']}"
        )

    lines += ["", "## Route graph used", ""]
    if not report["route_graph_used"]:
        lines.append("- `TOKEN_VAZIO_NO_ROUTE_EDGE_SELECTED`")
    for src, dst in report["route_graph_used"]:
        lines.append(f"- `{src}` → `{dst}`")

    lines += ["", "## Runtime prescriptions", ""]
    if not report["runtime_routes"]:
        lines.append("- `TOKEN_VAZIO_RUNTIME_NOT_EXECUTED`: run with `--execute-probes` on the target host/device.")
    for route in report["runtime_routes"]:
        lines.append(f"- **{route['code']}** `{route['state']}` — {route['reason']} Next: {route['next_action']}")

    lines += ["", "## Gap ledger", ""]
    for gap in report["gap_ledger"]:
        lines.append(
            f"- **{gap['id']}** `{gap['urgency']}` / `{gap['state']}` — provenance: {gap['provenance']}. "
            f"Next: {gap['F_next']}"
        )

    lines += [
        "",
        "## Claim boundary",
        "",
        "```json",
        json.dumps(report["claim_boundary"], indent=2, ensure_ascii=False),
        "```",
        "",
    ]
    return "\n".join(lines)


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    trace = Trace(args.verbose)
    registry_path = Path(args.skills).resolve()
    registry, skills = load_skills(registry_path)
    workspace = Path(args.workspace).expanduser().resolve()

    repos = auto_detect_repos(workspace, skills, trace)
    repos.update(parse_repo_args(args.repo))
    history = load_outcome_history(Path(args.history).expanduser().resolve() if args.history else None, trace)

    skill_routes = symptom_routes(skills, args.symptom, repos, history)
    selected_ids = {item["skill_id"] for item in skill_routes}
    selected = [skill for skill in skills if skill.id in selected_ids]

    probes: list[dict[str, Any]] = []
    if args.execute_probes:
        probes = execute_selected_probes(selected, repos, args.timeout, trace)
    else:
        trace.add("probe", "runtime probes not executed; explicit --execute-probes required")

    runtime_routes = route_from_runtime(probes, trace)
    route_graph_used = reachable_route_graph(registry, selected_ids)
    build_doctor_evidence = load_build_doctor_reports(args.build_doctor_report, trace)
    gap_ledger = build_gap_ledger(args, skill_routes, probes, build_doctor_evidence)

    append_outcomes(Path(args.append_outcome).expanduser().resolve(), args.outcome, trace) if args.append_outcome else None

    failed_probes = sum(
        1
        for probe in probes
        if str(probe.get("state", "")).startswith(("FAIL", "BLOCKED"))
    )
    incomplete_probes = sum(
        1
        for probe in probes
        if str(probe.get("state", "")).startswith(("INCOMPLETE", "TOKEN_VAZIO"))
    )
    unresolved_repos = sum(
        1 for route in skill_routes if str(route["repo_state"]).startswith("TOKEN_VAZIO")
    )
    evidence_blocks = sum(
        1 for record in build_doctor_evidence if str(record.get("state", "")).startswith("BLOCKED")
    )
    static_review_required = any(
        isinstance(record.get("summary"), dict)
        and record["summary"].get("state") == "REVIEW_REQUIRED"
        for record in build_doctor_evidence
    )

    state = "PASS_LIMITED"
    if failed_probes or evidence_blocks or static_review_required:
        state = "REVIEW_REQUIRED"
    elif args.execute_probes and probes and not incomplete_probes and runtime_routes:
        state = "RUNTIME_OBSERVED_LIMITED"

    report = {
        "schema": REPORT_SCHEMA,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "mode": "READ_ONLY_DIAGNOSTIC",
        "registry_schema": registry.get("schema"),
        "summary": {
            "state": state,
            "selected_skills": len(skill_routes),
            "probe_results": len(probes),
            "probes_executed": sum(
                1
                for p in probes
                if "exit_code" in p
                and not (isinstance(p.get("probe_cache"), dict) and p["probe_cache"].get("reused") is True)
            ),
            "probe_cache_hits": sum(
                1
                for p in probes
                if isinstance(p.get("probe_cache"), dict) and p["probe_cache"].get("reused") is True
            ),
            "failed_probes": failed_probes,
            "incomplete_probes": incomplete_probes,
            "runtime_routes": len(runtime_routes),
            "unresolved_repositories": unresolved_repos,
            "build_doctor_reports": len(build_doctor_evidence),
            "open_gaps": len(gap_ledger),
        },
        "repos": {name: str(path) for name, path in sorted(repos.items())},
        "symptoms": list(args.symptom),
        "skill_routes": skill_routes,
        "route_graph_used": route_graph_used,
        "probes": probes,
        "runtime_routes": runtime_routes,
        "evidence_inputs": {
            "build_doctor": build_doctor_evidence,
        },
        "gap_ledger": gap_ledger,
        "trace": trace.events,
        "claim_boundary": {
            "static_mapping": "VERIFIED_BY_CONFIGURATION",
            "runtime_execution": "VERIFIED_BY_EXECUTION"
            if any("exit_code" in p for p in probes)
            else "TOKEN_VAZIO_NOT_EXECUTED",
            "build_doctor_evidence": "HASH_BOUND_INPUT_REPORT_ONLY"
            if build_doctor_evidence
            else "TOKEN_VAZIO_NOT_INGESTED",
            "gpu_library_presence": "CAPABILITY_CANDIDATE_ONLY",
            "llm_performance": "TOKEN_VAZIO_BENCHMARK_REQUIRED",
            "vm_guest_boot": "TOKEN_VAZIO_DEDICATED_RECEIPT_REQUIRED",
            "automatic_repair": False,
            "automatic_install": False,
            "automatic_delete": False,
            "claim_allowed": False,
        },
        "F_ok": (
            "skills, reachable route graph, read-only probes, probe deduplication and hash-bound static evidence "
            "ingestion are explicit"
        ),
        "F_gap": (
            "gap_ledger is authoritative for unresolved runtime, repository, benchmark, VM and physical-device evidence"
        ),
        "F_next": (
            "resolve P0 gaps first; then execute physical read-only receipts and ingest exact Build Doctor evidence "
            "without promoting configuration into runtime proof"
        ),
    }
    return report


def parser() -> argparse.ArgumentParser:
    here = Path(__file__).resolve().parents[1]
    p = argparse.ArgumentParser(description="RAFAELIA Runtime Doctor Agent — evidence-first multilevel runtime router")
    p.add_argument("--skills", default=str(here / "configs" / "runtime-doctor-skills.v1.json"))
    p.add_argument("--workspace", default=str(here.parent), help="directory containing sibling repository clones")
    p.add_argument("--repo", action="append", default=[], help="explicit repository mapping name=path")
    p.add_argument("--symptom", action="append", default=[], help="symptom/intent term used to select skills")
    p.add_argument("--history", help="JSON/JSONL file or directory of prior skill outcomes")
    p.add_argument(
        "--build-doctor-report",
        action="append",
        default=[],
        help="exact Ecosystem Build Doctor JSON report to ingest by SHA-256; may be repeated",
    )
    p.add_argument("--execute-probes", action="store_true", help="execute only allowlisted read-only probes")
    p.add_argument("--timeout", type=int, default=30)
    p.add_argument("--verbose", action="store_true")
    p.add_argument("--json-out")
    p.add_argument("--markdown-out")
    p.add_argument("--append-outcome", help="append-only JSONL ledger path")
    p.add_argument("--outcome", action="append", default=[], help="skill_id=PASS|FAIL; requires --append-outcome")
    return p


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    if args.outcome and not args.append_outcome:
        print("--outcome requires --append-outcome", file=sys.stderr)
        return 2
    try:
        report = build_report(args)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"runtime-doctor error: {exc}", file=sys.stderr)
        return 2

    text = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True)
    if args.json_out:
        out = Path(args.json_out)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text + "\n", encoding="utf-8")
    if args.markdown_out:
        out = Path(args.markdown_out)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(markdown_report(report) + "\n", encoding="utf-8")
    print(text)
    return 0 if report["summary"]["failed_probes"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
