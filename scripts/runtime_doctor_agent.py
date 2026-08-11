#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import shlex
import subprocess
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

REPORT_SCHEMA = "raf.runtime-doctor-agent-report.v1"
SKILL_SCHEMA = "raf.runtime-doctor-skills.v1"
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


def symptom_routes(skills: list[Skill], symptoms: list[str], repos: dict[str, Path], history: dict[str, Counter[str]]) -> list[dict[str, Any]]:
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
        f"- runtime routes: `{report['summary']['runtime_routes']}`",
        "",
        "## Skill map",
        "",
    ]
    for item in report["skill_routes"]:
        lines.append(
            f"- **{item['level']} / {item['skill_id']}** — {item['repo_state']} — {item['role']}"
        )
    lines += ["", "## Runtime prescriptions", ""]
    if not report["runtime_routes"]:
        lines.append("- `TOKEN_VAZIO_RUNTIME_NOT_EXECUTED`: run with `--execute-probes` on the target host/device.")
    for route in report["runtime_routes"]:
        lines.append(f"- **{route['code']}** `{route['state']}` — {route['reason']} Next: {route['next_action']}")
    lines += ["", "## Claim boundary", "", "```json", json.dumps(report["claim_boundary"], indent=2, ensure_ascii=False), "```", ""]
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
        for skill in selected:
            repo_root = repos.get(skill.repo)
            if repo_root is None:
                probes.append(
                    {
                        "skill_id": skill.id,
                        "state": "TOKEN_VAZIO_REPOSITORY_NOT_AVAILABLE",
                        "repo": skill.repo,
                    }
                )
                continue
            probes.append(run_probe(skill, repo_root, args.timeout, trace))
    else:
        trace.add("probe", "runtime probes not executed; explicit --execute-probes required")

    runtime_routes = route_from_runtime(probes, trace)
    append_outcomes(Path(args.append_outcome).expanduser().resolve(), args.outcome, trace) if args.append_outcome else None

    failed_probes = sum(1 for probe in probes if str(probe.get("state", "")).startswith("FAIL"))
    unresolved_repos = sum(1 for route in skill_routes if str(route["repo_state"]).startswith("TOKEN_VAZIO"))
    state = "REVIEW_REQUIRED" if failed_probes else "PASS_LIMITED"
    if args.execute_probes and probes and not failed_probes and runtime_routes:
        state = "RUNTIME_OBSERVED_LIMITED"

    report = {
        "schema": REPORT_SCHEMA,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "mode": "READ_ONLY_DIAGNOSTIC",
        "registry_schema": registry.get("schema"),
        "summary": {
            "state": state,
            "selected_skills": len(skill_routes),
            "probes_executed": sum(1 for p in probes if "exit_code" in p),
            "failed_probes": failed_probes,
            "runtime_routes": len(runtime_routes),
            "unresolved_repositories": unresolved_repos,
        },
        "repos": {name: str(path) for name, path in sorted(repos.items())},
        "symptoms": list(args.symptom),
        "skill_routes": skill_routes,
        "probes": probes,
        "runtime_routes": runtime_routes,
        "trace": trace.events,
        "claim_boundary": {
            "static_mapping": "VERIFIED_BY_CONFIGURATION",
            "runtime_execution": "VERIFIED_BY_EXECUTION" if any("exit_code" in p for p in probes) else "TOKEN_VAZIO_NOT_EXECUTED",
            "gpu_library_presence": "CAPABILITY_CANDIDATE_ONLY",
            "llm_performance": "TOKEN_VAZIO_BENCHMARK_REQUIRED",
            "vm_guest_boot": "TOKEN_VAZIO_DEDICATED_RECEIPT_REQUIRED",
            "automatic_repair": false,
            "automatic_install": false,
            "automatic_delete": false,
            "claim_allowed": false,
        },
        "F_ok": "skills and routes are explicit; read-only probes can emit evidence",
        "F_gap": "device execution, benchmarks, VM guest boot and unresolved repositories remain evidence-gated",
        "F_next": "run on the real workspace/device with --execute-probes and preserve the generated receipt",
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
