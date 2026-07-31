#!/usr/bin/env python3
"""Fail-closed computational evidence gate for a Foundation local receipt.

This gate deliberately stops at COMPUTATIONAL_REVIEW_RESULT.  A positive
result means that a local execution is bound to observed source bytes,
environment and explicit test accounting; it never promotes scientific,
legal, security, ethical or production claims.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any


GATE_SCHEMA = "rafaelia.gate.computational/v1"
RECEIPT_SCHEMA = "rafaelia.foundation.receipt/v1"
TEST_SUMMARY_SCHEMA = "rafaelia.test-summary/v1"
SHA256_HEX_LENGTH = 64


class GateFailure(RuntimeError):
    """An integrity or contract violation that invalidates the review."""


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def safe_repo_path(repo_root: Path, value: str, field: str) -> Path:
    if not isinstance(value, str) or not value:
        raise GateFailure(f"{field} must be a non-empty repository-relative path")
    candidate = PurePosixPath(value.replace("\\", "/"))
    if candidate.is_absolute() or ".." in candidate.parts:
        raise GateFailure(f"{field} escapes repository root: {value!r}")
    target = (repo_root / candidate).resolve()
    root = repo_root.resolve()
    if target != root and root not in target.parents:
        raise GateFailure(f"{field} escapes repository root after resolution")
    return target


def read_json(path: Path, label: str) -> dict[str, Any]:
    if not path.is_file():
        raise GateFailure(f"{label} is missing: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GateFailure(f"{label} is not valid JSON: {exc}") from exc
    if not isinstance(value, dict):
        raise GateFailure(f"{label} must be a JSON object")
    return value


def capture_git(argv: list[str], repo_root: Path) -> tuple[int, str] | None:
    try:
        result = subprocess.run(
            argv,
            cwd=repo_root,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
            text=True,
            timeout=5,
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    return result.returncode, result.stdout.strip()


def current_git_identity(repo_root: Path) -> dict[str, Any] | None:
    if shutil.which("git") is None:
        return None
    head = capture_git(["git", "rev-parse", "HEAD"], repo_root)
    status = capture_git(["git", "status", "--porcelain=v1", "--untracked-files=all"], repo_root)
    if head is None or head[0] != 0 or status is None or status[0] != 0:
        return None
    return {
        "head_sha": head[1],
        "worktree_clean": not bool(status[1]),
    }


def valid_sha256(value: Any) -> bool:
    return isinstance(value, str) and len(value) == SHA256_HEX_LENGTH and all(
        character in "0123456789abcdef" for character in value.lower()
    )


def record(checks: list[dict[str, Any]], name: str, outcome: str, detail: str) -> None:
    checks.append({"name": name, "outcome": outcome, "detail": detail})


def validate_artifacts(
    repo_root: Path,
    receipt: dict[str, Any],
    checks: list[dict[str, Any]],
    failures: list[str],
) -> dict[str, dict[str, Any]]:
    artifacts = receipt.get("artifacts")
    if not isinstance(artifacts, list) or not artifacts:
        failures.append("RECEIPT_ARTIFACTS_MISSING")
        record(checks, "receipt-artifacts", "FAIL", "Receipt has no hashed artifacts.")
        return {}
    indexed: dict[str, dict[str, Any]] = {}
    for artifact in artifacts:
        if not isinstance(artifact, dict):
            failures.append("RECEIPT_ARTIFACT_INVALID")
            continue
        path_value = artifact.get("path")
        digest = artifact.get("sha256")
        if not isinstance(path_value, str) or not valid_sha256(digest):
            failures.append("RECEIPT_ARTIFACT_INVALID")
            continue
        if path_value in indexed:
            failures.append("RECEIPT_ARTIFACT_DUPLICATE")
            continue
        try:
            path = safe_repo_path(repo_root, path_value, "artifact.path")
        except GateFailure as exc:
            failures.append("RECEIPT_ARTIFACT_PATH_ESCAPE")
            record(checks, "artifact-path", "FAIL", str(exc))
            continue
        if not path.is_file() or path.is_symlink():
            failures.append("RECEIPT_ARTIFACT_MISSING")
            record(checks, "artifact-present", "FAIL", f"Missing regular file: {path_value}")
            continue
        actual = sha256_file(path)
        if actual != digest:
            failures.append("RECEIPT_ARTIFACT_HASH_MISMATCH")
            record(checks, "artifact-sha256", "FAIL", f"Hash mismatch: {path_value}")
            continue
        indexed[path_value] = artifact
    if not failures:
        record(checks, "receipt-artifacts", "PASS", f"Verified {len(indexed)} hashed artifacts.")
    return indexed


def validate_input_manifest(
    repo_root: Path,
    input_path: Path,
    checks: list[dict[str, Any]],
    failures: list[str],
) -> None:
    try:
        manifest = read_json(input_path, "input manifest")
    except GateFailure as exc:
        failures.append("INPUT_MANIFEST_INVALID")
        record(checks, "input-manifest", "FAIL", str(exc))
        return
    files = manifest.get("files")
    if not isinstance(files, list) or not files:
        failures.append("INPUT_MANIFEST_EMPTY")
        record(checks, "input-manifest", "FAIL", "Input manifest has no files.")
        return
    for item in files:
        if not isinstance(item, dict) or not isinstance(item.get("path"), str) or not valid_sha256(item.get("sha256")):
            failures.append("INPUT_MANIFEST_ENTRY_INVALID")
            continue
        try:
            source = safe_repo_path(repo_root, item["path"], "input_manifest.files.path")
        except GateFailure:
            failures.append("INPUT_MANIFEST_PATH_ESCAPE")
            continue
        if not source.is_file() or sha256_file(source) != item["sha256"]:
            failures.append("INPUT_MANIFEST_SOURCE_MISMATCH")
    if not failures:
        record(checks, "source-identity", "PASS", f"Verified {len(files)} input SHA-256 records.")


def validate_commands(
    command_log: Path,
    receipt: dict[str, Any],
    checks: list[dict[str, Any]],
    failures: list[str],
) -> None:
    command_records = receipt.get("commands")
    executed = receipt.get("commands_executed")
    if not isinstance(command_records, list) or not isinstance(executed, int) or executed <= 0:
        failures.append("COMMAND_ACCOUNTING_MISSING")
        record(checks, "command-accounting", "FAIL", "Receipt has no executed-command accounting.")
        return
    if (
        len(command_records) != executed
        or any(not isinstance(item, dict) or item.get("exit_code") != 0 for item in command_records)
    ):
        failures.append("COMMAND_ACCOUNTING_INVALID")
        record(checks, "command-accounting", "FAIL", "Receipt command count or exit codes are invalid.")
        return
    try:
        lines = [json.loads(line) for line in command_log.read_text(encoding="utf-8").splitlines() if line]
    except (OSError, json.JSONDecodeError) as exc:
        failures.append("COMMAND_LOG_INVALID")
        record(checks, "command-log", "FAIL", f"commands.jsonl is invalid: {exc}")
        return
    starts = [line for line in lines if isinstance(line, dict) and line.get("event") == "COMMAND_STARTED"]
    finished = [line for line in lines if isinstance(line, dict) and line.get("event") == "COMMAND_FINISHED"]
    if len(starts) != executed or len(finished) != executed or any(line.get("exit_code") != 0 for line in finished):
        failures.append("COMMAND_LOG_INCOMPLETE")
        record(checks, "command-log", "FAIL", "Command start/finish events do not prove a clean complete run.")
        return
    record(checks, "command-accounting", "PASS", f"Verified {executed} command start/finish pairs with exit code 0.")


def validate_test_summary(
    summary_path: Path,
    artifact_index: dict[str, dict[str, Any]],
    repo_root: Path,
    checks: list[dict[str, Any]],
    gaps: list[str],
    failures: list[str],
) -> None:
    summary_relative = summary_path.relative_to(repo_root).as_posix()
    if not summary_path.is_file():
        gaps.append("TOKEN_VAZIO_TEST_SUMMARY_MISSING")
        record(checks, "test-summary", "TOKEN_VAZIO", "The execution did not produce a test summary.")
        return
    if summary_relative not in artifact_index:
        gaps.append("TOKEN_VAZIO_TEST_SUMMARY_NOT_BOUND_TO_RECEIPT")
        record(checks, "test-summary-bound", "TOKEN_VAZIO", "Test summary was not hashed into the execution receipt.")
        return
    try:
        summary = read_json(summary_path, "test summary")
    except GateFailure as exc:
        failures.append("TEST_SUMMARY_INVALID")
        record(checks, "test-summary", "FAIL", str(exc))
        return
    if summary.get("schema") != TEST_SUMMARY_SCHEMA:
        failures.append("TEST_SUMMARY_SCHEMA_INVALID")
        record(checks, "test-summary", "FAIL", f"Expected {TEST_SUMMARY_SCHEMA}.")
        return
    counts = summary.get("counts")
    tests = summary.get("tests")
    falsifiers = summary.get("falsifiers")
    if not isinstance(counts, dict) or not isinstance(tests, list) or not isinstance(falsifiers, list) or not falsifiers:
        gaps.append("TOKEN_VAZIO_TEST_ACCOUNTING_INCOMPLETE")
        record(checks, "test-accounting", "TOKEN_VAZIO", "Tests, counts, or falsifiers are absent.")
        return
    required_counts = ("discovered", "executed", "passed", "failed", "skipped")
    if any(not isinstance(counts.get(name), int) or counts[name] < 0 for name in required_counts):
        failures.append("TEST_COUNTS_INVALID")
        record(checks, "test-accounting", "FAIL", "Test counts must be non-negative integers.")
        return
    test_ids: set[str] = set()
    all_passed = True
    for test in tests:
        if not isinstance(test, dict) or not isinstance(test.get("id"), str) or not test["id"] or test["id"] in test_ids:
            failures.append("TEST_INVENTORY_INVALID")
            continue
        test_ids.add(test["id"])
        if test.get("result") != "PASS":
            all_passed = False
    if (
        counts["discovered"] <= 0
        or counts["discovered"] != len(tests)
        or counts["executed"] != counts["discovered"]
        or counts["passed"] != counts["discovered"]
        or counts["failed"] != 0
        or counts["skipped"] != 0
        or not all_passed
    ):
        failures.append("TEST_ACCOUNTING_NOT_COMPLETE")
        record(checks, "test-accounting", "FAIL", "Discovered tests were not all executed and passed without failures or skips.")
        return
    falsifier_ids: set[str] = set()
    for falsifier in falsifiers:
        if (
            not isinstance(falsifier, dict)
            or not isinstance(falsifier.get("id"), str)
            or not falsifier["id"]
            or falsifier["id"] in falsifier_ids
            or not isinstance(falsifier.get("condition"), str)
            or not falsifier["condition"]
            or falsifier.get("status") != "EXERCISED"
        ):
            failures.append("FALSIFIER_ACCOUNTING_INVALID")
            record(checks, "falsifiers", "FAIL", "Each falsifier needs a unique id, condition, and EXERCISED status.")
            return
        falsifier_ids.add(falsifier["id"])
    record(checks, "test-accounting", "PASS", f"{counts['discovered']} discovered tests = executed = passed; 0 failed; 0 skipped.")
    record(checks, "falsifiers", "PASS", f"{len(falsifiers)} falsifiers were explicitly exercised.")


def review(
    repo_root: Path,
    receipt_path: Path,
    test_summary_path: Path,
    expected_profile: str | None,
) -> dict[str, Any]:
    checks: list[dict[str, Any]] = []
    gaps: list[str] = []
    failures: list[str] = []
    receipt = read_json(receipt_path, "receipt")
    receipt_relative = receipt_path.relative_to(repo_root).as_posix()
    receipt_digest = sha256_file(receipt_path)

    receipt_hash_path = receipt_path.with_name("receipt.sha256")
    if not receipt_hash_path.is_file() or receipt_digest not in receipt_hash_path.read_text(encoding="utf-8", errors="replace"):
        failures.append("RECEIPT_SHA256_INVALID")
        record(checks, "receipt-sha256", "FAIL", "receipt.sha256 does not bind the receipt bytes.")
    else:
        record(checks, "receipt-sha256", "PASS", "receipt.json matches receipt.sha256.")
    if receipt.get("schema") != RECEIPT_SCHEMA:
        failures.append("RECEIPT_SCHEMA_INVALID")
        record(checks, "receipt-schema", "FAIL", f"Expected {RECEIPT_SCHEMA}.")
    if receipt.get("status") != "PASS_LOCAL_EXECUTION" or receipt.get("exit_code") != 0:
        failures.append("RECEIPT_EXECUTION_NOT_PASS")
        record(checks, "local-execution", "FAIL", "Receipt is not a successful local execution.")
    else:
        record(checks, "local-execution", "PASS", "Receipt reports PASS_LOCAL_EXECUTION with exit code 0.")
    if receipt.get("claim_allowed") is not False or receipt.get("decision") != "NOT_PROMOTED":
        failures.append("CLAIM_BOUNDARY_INVALID")
        record(checks, "claim-boundary", "FAIL", "Receipt attempted a claim or an unsupported decision.")
    else:
        record(checks, "claim-boundary", "PASS", "Claim boundary remains false / NOT_PROMOTED.")
    if expected_profile is not None and receipt.get("profile") != expected_profile:
        failures.append("PROFILE_MISMATCH")
        record(checks, "profile", "FAIL", f"Expected profile {expected_profile!r}.")
    elif isinstance(receipt.get("profile"), str):
        record(checks, "profile", "PASS", f"Profile: {receipt['profile']}.")
    else:
        failures.append("PROFILE_MISSING")
        record(checks, "profile", "FAIL", "Receipt profile is absent.")

    identity = receipt.get("repository_identity")
    if not isinstance(identity, dict) or identity.get("state") != "BOUND":
        gaps.append("TOKEN_VAZIO_GIT_IDENTITY_NOT_BOUND")
        record(checks, "git-identity", "TOKEN_VAZIO", "Receipt is not bound to an observed Git HEAD.")
    elif not isinstance(identity.get("head_sha"), str) or len(identity["head_sha"]) < 12 or identity.get("worktree_clean") is not True:
        gaps.append("TOKEN_VAZIO_GIT_WORKTREE_NOT_CLEAN")
        record(checks, "git-identity", "TOKEN_VAZIO", "Receipt has no clean exact Git source identity.")
    else:
        observed = current_git_identity(repo_root)
        if observed is None:
            gaps.append("TOKEN_VAZIO_GIT_IDENTITY_NOT_CURRENTLY_OBSERVABLE")
            record(checks, "git-identity", "TOKEN_VAZIO", "Current Git identity cannot be observed for comparison.")
        elif observed["head_sha"] != identity["head_sha"] or observed["worktree_clean"] is not True:
            failures.append("GIT_IDENTITY_MISMATCH")
            record(checks, "git-identity", "FAIL", "Current checkout does not match the clean receipt HEAD.")
        else:
            record(checks, "git-identity", "PASS", f"Clean HEAD is bound: {identity['head_sha']}.")

    artifact_index = validate_artifacts(repo_root, receipt, checks, failures)
    run_directory = receipt_path.parent.relative_to(repo_root).as_posix()
    required = {
        f"{run_directory}/environment.json",
        f"{run_directory}/input_manifest.json",
        f"{run_directory}/commands.jsonl",
        f"{run_directory}/stdout.log",
        f"{run_directory}/stderr.log",
    }
    missing = sorted(required.difference(artifact_index))
    if missing:
        failures.append("REQUIRED_EXECUTION_ARTIFACT_MISSING")
        record(checks, "execution-artifacts", "FAIL", "Missing: " + ", ".join(missing))
    else:
        record(checks, "execution-artifacts", "PASS", "Environment, inputs, command log and streams are hashed.")
        environment = read_json(safe_repo_path(repo_root, f"{run_directory}/environment.json", "environment"), "environment")
        if (
            environment.get("runtime_class") != "ANDROID_TERMUX_LOCAL"
            or not isinstance(environment.get("captured_at"), str)
            or not isinstance(environment.get("observed_tools"), list)
        ):
            failures.append("ENVIRONMENT_OBSERVATION_INVALID")
            record(checks, "environment", "FAIL", "Observed runtime environment is incomplete.")
        else:
            record(checks, "environment", "PASS", "Runtime, timestamp and executable observations are present.")
        validate_input_manifest(
            repo_root,
            safe_repo_path(repo_root, f"{run_directory}/input_manifest.json", "input manifest"),
            checks,
            failures,
        )
        validate_commands(
            safe_repo_path(repo_root, f"{run_directory}/commands.jsonl", "command log"),
            receipt,
            checks,
            failures,
        )
    validate_test_summary(test_summary_path, artifact_index, repo_root, checks, gaps, failures)

    if failures:
        result = "FAIL"
        next_step = "Correct the listed integrity or accounting failures and rerun the exact local profile."
    elif gaps:
        result = "TOKEN_VAZIO"
        next_step = "Supply the named missing evidence, rerun the exact profile, then gate the new receipt."
    else:
        result = "READY_FOR_DOMAIN_SPECIFIC_REVIEW"
        next_step = "Submit this computational receipt to the repository-specific reviewer; claim_allowed remains false."
    return {
        "schema": GATE_SCHEMA,
        "created_at": utc_now(),
        "receipt": {
            "path": receipt_relative,
            "sha256": receipt_digest,
        },
        "claim_allowed": False,
        "decision": "NOT_PROMOTED",
        "COMPUTATIONAL_REVIEW_RESULT": result,
        "checks": checks,
        "gaps": sorted(set(gaps)),
        "failures": sorted(set(failures)),
        "next_verifiable_step": next_step,
    }


def output_path(receipt_path: Path, requested: str | None, repo_root: Path) -> Path:
    if requested is not None:
        target = safe_repo_path(repo_root, requested, "--out")
        if target.exists():
            raise GateFailure("--out must not overwrite an existing gate artifact")
        return target
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    candidate = receipt_path.parent / f"gate.computational.v1-{stamp}.json"
    suffix = 1
    while candidate.exists():
        suffix += 1
        candidate = receipt_path.parent / f"gate.computational.v1-{stamp}-{suffix}.json"
    return candidate


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path("."))
    parser.add_argument("--receipt", required=True, help="Repository-relative receipt.json path.")
    parser.add_argument("--test-summary", help="Repository-relative test summary; defaults beside receipt.")
    parser.add_argument("--expected-profile")
    parser.add_argument("--out", help="New repository-relative gate artifact path.")
    args = parser.parse_args(argv)
    repo_root = args.repo_root.resolve()
    try:
        receipt_path = safe_repo_path(repo_root, args.receipt, "--receipt")
        test_summary_path = (
            safe_repo_path(repo_root, args.test_summary, "--test-summary")
            if args.test_summary
            else receipt_path.with_name("test-summary.json")
        )
        report = review(repo_root, receipt_path, test_summary_path, args.expected_profile)
        destination = output_path(receipt_path, args.out, repo_root)
        destination.parent.mkdir(parents=True, exist_ok=False) if not destination.parent.exists() else None
        destination.write_text(json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        digest = sha256_file(destination)
        destination.with_suffix(destination.suffix + ".sha256").write_text(
            f"{digest}  {destination.name}\n", encoding="utf-8"
        )
    except GateFailure as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print(f"COMPUTATIONAL_REVIEW_RESULT={report['COMPUTATIONAL_REVIEW_RESULT']}")
    print(f"GATE={destination.relative_to(repo_root).as_posix()}")
    print(f"GATE_SHA256={digest}")
    if report["COMPUTATIONAL_REVIEW_RESULT"] == "READY_FOR_DOMAIN_SPECIFIC_REVIEW":
        return 0
    if report["COMPUTATIONAL_REVIEW_RESULT"] == "TOKEN_VAZIO":
        return 2
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
