#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import glob
import hashlib
import json
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

SCHEMA = "rafpolimata.safe_extended.v2"
REPORT_SCHEMA = "rafpolimata.safe_extended.report.v2"
SUPPORTED_ACTIONS = {
    "actions/checkout@v4": "checkout_local",
    "actions/upload-artifact@v4": "upload_local",
}
DENY_PATTERNS = [
    ("network", re.compile(r"(^|[;&|]\s*)(curl|wget|ftp|scp|sftp|ssh|nc|ncat)\b")),
    ("git_network", re.compile(r"\bgit\s+(clone|fetch|pull|push|ls-remote)\b")),
    ("package_install", re.compile(r"\b(pkg|apt|apt-get|dnf|yum|pacman|apk|pip|pip3|npm|pnpm|yarn|gem|cargo)\s+(install|add|update|upgrade)\b")),
    ("privilege", re.compile(r"(^|[;&|]\s*)(sudo|su|doas)\b")),
    ("device_mutation", re.compile(r"(^|[;&|]\s*)(mount|umount|mkfs|reboot|shutdown|setprop|fastboot|adb)\b")),
    ("destructive_root", re.compile(r"\brm\s+-[^\n]*r[^\n]*f[^\n]*\s+/(\s|$)")),
    ("dynamic_eval", re.compile(r"(^|[;&|]\s*)(eval|source)\s+")),
]
WARN_PATTERNS = [
    ("masked_failure", re.compile(r"\|\|\s*true\b")),
    ("background_process", re.compile(r"(^|[^&])&($|[^&])")),
]


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def find_root(start: Path) -> Path:
    p = start.resolve()
    for candidate in [p, *p.parents]:
        if (candidate / ".github" / "workflows").is_dir() and (candidate / "raf_main.c").is_file():
            return candidate
    raise SystemExit("SAFE_EXTENDED: não encontrei a raiz do RafPolimata")


def strip_comment(value: str) -> str:
    quote = None
    out = []
    for i, ch in enumerate(value):
        if ch in "'\"":
            if quote == ch:
                quote = None
            elif quote is None:
                quote = ch
        if ch == "#" and quote is None and (i == 0 or value[i - 1].isspace()):
            break
        out.append(ch)
    return "".join(out).rstrip()


def scalar(value: str) -> Any:
    value = strip_comment(value.strip())
    if not value:
        return ""
    if value in {"true", "True"}:
        return True
    if value in {"false", "False"}:
        return False
    if value in {"null", "Null", "~"}:
        return None
    if value[0:1] == value[-1:] and value.startswith(("'", '"')):
        return value[1:-1]
    return value


def parse_workflow(path: Path) -> dict[str, Any]:
    raw = path.read_text(encoding="utf-8")
    if "\t" in raw:
        raise ValueError("tabs não são aceitos no YAML seguro")
    if re.search(r"(^|\s)[&*][A-Za-z0-9_-]+", raw):
        raise ValueError("anchors/aliases YAML não são aceitos")
    lines = raw.splitlines()
    jobs: list[dict[str, Any]] = []
    current_job: dict[str, Any] | None = None
    current_step: dict[str, Any] | None = None
    in_jobs = False
    in_steps = False
    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        indent = len(line) - len(line.lstrip(" "))
        if not stripped or stripped.startswith("#"):
            i += 1
            continue
        if indent == 0 and stripped == "jobs:":
            in_jobs = True
            i += 1
            continue
        if not in_jobs:
            i += 1
            continue
        if indent == 0 and stripped != "jobs:":
            break
        if indent == 2 and stripped.endswith(":") and not stripped.startswith("-"):
            if current_job:
                jobs.append(current_job)
            current_job = {"id": stripped[:-1], "name": stripped[:-1], "steps": [], "env": {}}
            current_step = None
            in_steps = False
            i += 1
            continue
        if current_job is None:
            i += 1
            continue
        if indent == 4 and stripped == "steps:":
            in_steps = True
            i += 1
            continue
        if indent == 4 and ":" in stripped and not in_steps:
            key, val = stripped.split(":", 1)
            current_job[key] = scalar(val)
            i += 1
            continue
        if in_steps and indent == 6 and stripped.startswith("-"):
            current_step = {"name": "unnamed", "env": {}, "with": {}}
            current_job["steps"].append(current_step)
            rest = stripped[1:].strip()
            if rest and ":" in rest:
                key, val = rest.split(":", 1)
                current_step[key] = scalar(val)
            i += 1
            continue
        if current_step is not None and indent >= 8 and ":" in stripped:
            key, val = stripped.split(":", 1)
            key = key.strip()
            val = val.strip()
            if key in {"env", "with"} and val == "":
                container = current_step[key]
                i += 1
                while i < len(lines):
                    sub = lines[i]
                    sub_indent = len(sub) - len(sub.lstrip(" "))
                    sub_s = sub.strip()
                    if sub_s and sub_indent <= indent:
                        break
                    if sub_s and not sub_s.startswith("#") and ":" in sub_s:
                        k, v = sub_s.split(":", 1)
                        k = k.strip()
                        v = v.strip()
                        if v in {"|", "|-", ">", ">-"}:
                            parent_indent = sub_indent
                            block: list[str] = []
                            i += 1
                            block_indent: int | None = None
                            while i < len(lines):
                                bline = lines[i]
                                bindent = len(bline) - len(bline.lstrip(" "))
                                if bline.strip() and bindent <= parent_indent:
                                    break
                                if bline.strip() and block_indent is None:
                                    block_indent = bindent
                                block.append(bline)
                                i += 1
                            cut = block_indent or (parent_indent + 2)
                            container[k] = "\n".join(x[cut:] if len(x) >= cut else "" for x in block).rstrip() + "\n"
                            continue
                        container[k] = scalar(v)
                    i += 1
                continue
            if key == "run" and val in {"|", "|-", ">", ">-"}:
                block: list[str] = []
                i += 1
                min_indent: int | None = None
                while i < len(lines):
                    sub = lines[i]
                    sub_indent = len(sub) - len(sub.lstrip(" "))
                    if sub.strip() and sub_indent <= indent:
                        break
                    if sub.strip() and min_indent is None:
                        min_indent = sub_indent
                    block.append(sub)
                    i += 1
                cut = min_indent or (indent + 2)
                body = "\n".join(x[cut:] if len(x) >= cut else "" for x in block)
                current_step["run"] = body.rstrip() + "\n"
                continue
            current_step[key] = scalar(val)
            i += 1
            continue
        i += 1
    if current_job:
        jobs.append(current_job)
    if not jobs:
        raise ValueError("nenhum job encontrado")
    return {
        "schema": SCHEMA,
        "workflow": str(path),
        "workflow_sha256": sha256_bytes(raw.encode()),
        "compiled_at": utc_now(),
        "jobs": jobs,
    }


def inspect_script(script: str) -> tuple[list[str], list[str]]:
    denied = [name for name, pattern in DENY_PATTERNS if pattern.search(script)]
    warnings = [name for name, pattern in WARN_PATTERNS if pattern.search(script)]
    if "${{" in script:
        denied.append("github_expression")
    return sorted(set(denied)), sorted(set(warnings))


def toolchain_info() -> dict[str, Any]:
    info: dict[str, Any] = {
        "machine": platform.machine(),
        "system": platform.system(),
        "python": platform.python_version(),
        "termux": bool(os.environ.get("TERMUX_VERSION") or Path("/data/data/com.termux/files/usr").is_dir()),
    }
    for command, key in [(["clang", "-dumpmachine"], "clang_target"), (["getconf", "LONG_BIT"], "long_bit")]:
        try:
            info[key] = subprocess.check_output(command, text=True, stderr=subprocess.DEVNULL).strip()
        except Exception:
            info[key] = "TOKEN_VAZIO"
    try:
        info["android_abi"] = subprocess.check_output(["getprop", "ro.product.cpu.abi"], text=True, stderr=subprocess.DEVNULL).strip() or "TOKEN_VAZIO"
    except Exception:
        info["android_abi"] = "TOKEN_VAZIO"
    return info


def make_shims(run_dir: Path) -> Path:
    shims = run_dir / "shims"
    shims.mkdir(parents=True, exist_ok=True)
    clang = shutil.which("clang") or shutil.which("cc")
    if clang:
        for name in ("gcc", "cc"):
            target = shims / name
            target.write_text(f"#!/bin/sh\nexec {shlex.quote(clang)} \"$@\"\n", encoding="utf-8")
            target.chmod(0o700)
    clangxx = shutil.which("clang++") or shutil.which("c++")
    if clangxx:
        target = shims / "g++"
        target.write_text(f"#!/bin/sh\nexec {shlex.quote(clangxx)} \"$@\"\n", encoding="utf-8")
        target.chmod(0o700)
    py = shutil.which("python3")
    if py:
        target = shims / "python"
        target.write_text(f"#!/bin/sh\nexec {shlex.quote(py)} \"$@\"\n", encoding="utf-8")
        target.chmod(0o700)
    return shims


def collect_artifact(root: Path, artifact_dir: Path, step: dict[str, Any]) -> list[str]:
    with_data = step.get("with") or {}
    name = str(with_data.get("name") or "artifact")
    paths = str(with_data.get("path") or "").splitlines()
    dest = artifact_dir / re.sub(r"[^A-Za-z0-9._-]+", "_", name)
    dest.mkdir(parents=True, exist_ok=True)
    copied: list[str] = []
    for pattern in paths:
        pattern = pattern.strip()
        if not pattern:
            continue
        for match in glob.glob(str(root / pattern), recursive=True):
            src = Path(match)
            try:
                rel = src.resolve().relative_to(root.resolve())
            except ValueError:
                continue
            out = dest / rel
            out.parent.mkdir(parents=True, exist_ok=True)
            if src.is_dir():
                shutil.copytree(src, out, dirs_exist_ok=True)
            elif src.is_file():
                shutil.copy2(src, out)
            copied.append(str(rel))
    return copied


def execute_plan(root: Path, plan: dict[str, Any], timeout_s: int, allow_masked: bool) -> dict[str, Any]:
    if os.geteuid() == 0 and os.environ.get("SAFE_EXTENDED_ALLOW_ROOT") != "1":
        raise RuntimeError("execução como root bloqueada")
    stamp = dt.datetime.now(dt.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    run_dir = root / "build" / "safe-extended" / f"{stamp}-{os.getpid()}"
    logs = run_dir / "logs"
    artifacts = run_dir / "artifacts"
    logs.mkdir(parents=True, exist_ok=True)
    artifacts.mkdir(parents=True, exist_ok=True)
    lock = root / "build" / ".safe-extended.lock"
    try:
        lock.mkdir(parents=False)
    except FileExistsError as exc:
        raise RuntimeError(f"outra CI local está ativa: {lock}") from exc
    start = time.time()
    results: list[dict[str, Any]] = []
    shims = make_shims(run_dir)
    base_env = os.environ.copy()
    base_env.update({
        "CI": "true",
        "GITHUB_ACTIONS": "false",
        "GITHUB_WORKSPACE": str(root),
        "RUNNER_TEMP": str(run_dir / "tmp"),
        "HOME": str(run_dir / "home"),
        "TMPDIR": str(run_dir / "tmp"),
        "PATH": f"{shims}{os.pathsep}{base_env.get('PATH', '')}",
        "LC_ALL": "C",
        "TZ": "UTC",
    })
    Path(base_env["HOME"]).mkdir(parents=True, exist_ok=True)
    Path(base_env["TMPDIR"]).mkdir(parents=True, exist_ok=True)
    overall = "PASS"
    try:
        for job in plan["jobs"]:
            job_id = str(job["id"])
            for index, step in enumerate(job.get("steps", []), start=1):
                step_name = str(step.get("name") or step.get("uses") or f"step-{index}")
                slug = re.sub(r"[^A-Za-z0-9._-]+", "_", f"{job_id}-{index:02d}-{step_name}")[:120]
                log_path = logs / f"{slug}.log"
                item: dict[str, Any] = {"job": job_id, "index": index, "name": step_name, "started": utc_now()}
                uses = step.get("uses")
                if uses:
                    action = SUPPORTED_ACTIONS.get(str(uses))
                    if action == "checkout_local":
                        item.update(status="PASS", action=action, note="checkout local já presente")
                    elif action == "upload_local":
                        copied = collect_artifact(root, artifacts, step)
                        item.update(status="PASS", action=action, copied=copied)
                    else:
                        item.update(status="UNSUPPORTED", action=str(uses))
                        overall = "FAIL"
                    item["ended"] = utc_now()
                    log_path.write_text(json.dumps(item, ensure_ascii=False, indent=2), encoding="utf-8")
                    item["log"] = str(log_path.relative_to(run_dir))
                    results.append(item)
                    if overall == "FAIL":
                        break
                    continue
                script = str(step.get("run") or "")
                denied, warnings = inspect_script(script)
                item["warnings"] = warnings
                if denied or (warnings and not allow_masked):
                    item.update(status="POLICY_DENY", denied=denied or warnings, rc=126, ended=utc_now())
                    log_path.write_text(script + "\n\nPOLICY_DENY=" + ",".join(item["denied"]) + "\n", encoding="utf-8")
                    item["log"] = str(log_path.relative_to(run_dir))
                    results.append(item)
                    overall = "FAIL"
                    break
                env = base_env.copy()
                env.update({str(k): str(v) for k, v in (job.get("env") or {}).items()})
                env.update({str(k): str(v) for k, v in (step.get("env") or {}).items()})
                cwd = root / str(step.get("working-directory") or ".")
                cwd = cwd.resolve()
                if root.resolve() not in [cwd, *cwd.parents]:
                    item.update(status="POLICY_DENY", denied=["working_directory_escape"], rc=126, ended=utc_now())
                    results.append(item)
                    overall = "FAIL"
                    break
                cmd = ["bash", "--noprofile", "--norc", "-euo", "pipefail", "-c", script]
                with log_path.open("w", encoding="utf-8") as log:
                    log.write(f"# job={job_id}\n# step={step_name}\n# cwd={cwd}\n# started={item['started']}\n\n")
                    log.flush()
                    try:
                        proc = subprocess.run(cmd, cwd=cwd, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=timeout_s)
                        rc = proc.returncode
                    except subprocess.TimeoutExpired:
                        rc = 124
                        log.write(f"\nSAFE_EXTENDED_TIMEOUT={timeout_s}\n")
                item.update(status="PASS" if rc == 0 else "FAIL", rc=rc, ended=utc_now(), log=str(log_path.relative_to(run_dir)))
                results.append(item)
                if rc != 0:
                    overall = "FAIL"
                    break
            if overall == "FAIL":
                break
    finally:
        shutil.rmtree(lock, ignore_errors=True)
    report = {
        "schema": REPORT_SCHEMA,
        "result": overall,
        "started": dt.datetime.fromtimestamp(start, dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "ended": utc_now(),
        "workflow": plan["workflow"],
        "workflow_sha256": plan["workflow_sha256"],
        "toolchain": toolchain_info(),
        "network": "DENY_BY_POLICY",
        "github_runner": "NOT_USED",
        "steps": results,
        "run_dir": str(run_dir),
    }
    report_path = run_dir / "report.json"
    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    latest = root / "build" / "safe-extended" / "latest-report.json"
    latest.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return report


def workflow_paths(root: Path, values: list[str]) -> list[Path]:
    if values:
        paths = [Path(v) if Path(v).is_absolute() else root / v for v in values]
    else:
        paths = [root / ".github" / "workflows" / "ci.yml"]
    for p in paths:
        if not p.is_file():
            raise SystemExit(f"workflow ausente: {p}")
    return paths


def main() -> int:
    parser = argparse.ArgumentParser(description="Compila e executa uma parte segura de GitHub Actions no Termux local")
    parser.add_argument("command", choices=["plan", "run", "all", "status", "clean"])
    parser.add_argument("workflow", nargs="*")
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--allow-masked-failures", action="store_true")
    args = parser.parse_args()
    root = find_root(Path(__file__).parent)
    base = root / "build" / "safe-extended"
    if args.command == "status":
        path = base / "latest-report.json"
        if not path.is_file():
            print("TOKEN_VAZIO: nenhuma execução local", file=sys.stderr)
            return 4
        print(path.read_text(encoding="utf-8"), end="")
        return 0
    if args.command == "clean":
        shutil.rmtree(base, ignore_errors=True)
        shutil.rmtree(root / "build" / ".safe-extended.lock", ignore_errors=True)
        print("SAFE_EXTENDED CLEAN PASS")
        return 0
    paths = workflow_paths(root, args.workflow if args.command != "all" else [str(p) for p in sorted((root / ".github" / "workflows").glob("*.y*ml"))])
    final = 0
    for path in paths:
        try:
            plan = parse_workflow(path)
        except Exception as exc:
            print(f"SAFE_EXTENDED COMPILE FAIL {path}: {exc}", file=sys.stderr)
            final = 2
            continue
        denied_summary = []
        for job in plan["jobs"]:
            for step in job.get("steps", []):
                if step.get("run"):
                    denied, warnings = inspect_script(str(step["run"]))
                    if denied or warnings:
                        denied_summary.append({"job": job["id"], "step": step.get("name"), "denied": denied, "warnings": warnings})
                elif step.get("uses") and str(step["uses"]) not in SUPPORTED_ACTIONS:
                    denied_summary.append({"job": job["id"], "step": step.get("name"), "unsupported_action": step["uses"]})
        plan["policy_findings"] = denied_summary
        plan_path = base / "plans" / f"{path.stem}.plan.json"
        plan_path.parent.mkdir(parents=True, exist_ok=True)
        plan_path.write_text(json.dumps(plan, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(f"SAFE_EXTENDED PLAN {path.relative_to(root)} jobs={len(plan['jobs'])} findings={len(denied_summary)} plan={plan_path}")
        if args.command == "plan":
            continue
        try:
            report = execute_plan(root, plan, args.timeout, args.allow_masked_failures)
        except Exception as exc:
            print(f"SAFE_EXTENDED RUN FAIL {path}: {exc}", file=sys.stderr)
            final = 1
            continue
        print(f"SAFE_EXTENDED RESULT {report['result']} workflow={path.relative_to(root)} report={report['run_dir']}/report.json")
        if report["result"] != "PASS":
            final = 1
    return final


if __name__ == "__main__":
    raise SystemExit(main())
