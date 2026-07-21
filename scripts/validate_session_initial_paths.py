#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

SCHEMA = "raf.session-initial-paths.v1"
REQUIRED_PATH_IDS = [
    "PATH-00-TRUTH-SNAPSHOT",
    "PATH-01-LOCAL-CI",
    "PATH-02-TAIL-TOKEN",
    "PATH-03-SESSION-SEGMENTATION",
]


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)
    if not isinstance(data, dict):
        raise ValueError(f"{path}: root must be an object")
    return data


def validate(root: Path, manifest_path: Path) -> dict[str, Any]:
    manifest = load_json(manifest_path)
    checks: list[dict[str, Any]] = []

    def check(name: str, condition: bool, detail: str) -> None:
        checks.append({"name": name, "pass": bool(condition), "detail": detail})

    check("schema", manifest.get("schema") == SCHEMA, f"expected {SCHEMA}")

    allowed_states = set(manifest.get("allowed_states", []))
    allowed_evidence = set(manifest.get("allowed_evidence_states", []))
    check("allowed_states_nonempty", bool(allowed_states), "allowed_states must not be empty")
    check("allowed_evidence_nonempty", bool(allowed_evidence), "allowed_evidence_states must not be empty")

    paths = manifest.get("paths")
    check("paths_is_list", isinstance(paths, list), "paths must be a list")
    if not isinstance(paths, list):
        paths = []

    ids = [item.get("id") for item in paths if isinstance(item, dict)]
    orders = [item.get("order") for item in paths if isinstance(item, dict)]
    check("required_path_order", ids == REQUIRED_PATH_IDS, f"expected {REQUIRED_PATH_IDS}")
    check("orders_contiguous", orders == list(range(len(paths))), "orders must be 0..n-1")
    check("ids_unique", len(ids) == len(set(ids)), "path IDs must be unique")

    for index, item in enumerate(paths):
        prefix = f"path_{index}"
        if not isinstance(item, dict):
            check(f"{prefix}_object", False, "path must be an object")
            continue
        state = item.get("state")
        check(f"{prefix}_state", state in allowed_states, f"state={state!r}")
        check(f"{prefix}_title", bool(item.get("title")), "title required")
        check(f"{prefix}_executor", bool(item.get("executor")), "executor required")
        check(f"{prefix}_gates", bool(item.get("gates")), "at least one gate required")
        check(f"{prefix}_next", bool(item.get("next")), "next transition required")

        evidence = item.get("evidence", {})
        check(f"{prefix}_evidence_object", isinstance(evidence, dict), "evidence must be an object")
        if isinstance(evidence, dict):
            invalid = {key: value for key, value in evidence.items() if value not in allowed_evidence}
            check(f"{prefix}_evidence_states", not invalid, f"invalid evidence={invalid}")

        for rel_path in item.get("implementation_paths", []):
            exists = (root / rel_path).exists()
            check(f"{prefix}_implementation_{rel_path}", exists, f"required path: {rel_path}")

    invariant_set = set(manifest.get("invariants", []))
    check("invariant_no_fake_pass", "WORKFLOW_NOT_EXECUTED_NE_PASS" in invariant_set, "workflow non-execution must not be PASS")
    check("invariant_token_not_zero", "TOKEN_VAZIO_NE_ZERO" in invariant_set, "unknown weight/value must not become zero")
    check("invariant_license_separation", "THIRD_PARTY_LICENSE_NE_RAFAEL_AUTHORSHIP" in invariant_set, "third-party licensing must remain separate")

    scope = manifest.get("session_scope", {})
    claim_allowed = manifest.get("claim_allowed")
    check("claim_allowed_false", claim_allowed is False, "initial tranche must stay claim_allowed=false")
    check("scope_claim_allowed_false", isinstance(scope, dict) and scope.get("claim_allowed") is False, "session scope must also be false")

    termux_runtime = None
    for item in paths:
        if isinstance(item, dict) and item.get("id") == "PATH-01-LOCAL-CI":
            termux_runtime = item.get("evidence", {}).get("termux_runtime")
    check("termux_runtime_not_promoted", termux_runtime == "TOKEN_VAZIO", "device execution remains TOKEN_VAZIO until report exists")

    evidence_manifest = root / "manifests/evidence-compiler-projection.v1.json"
    check("evidence_compiler_manifest_exists", evidence_manifest.is_file(), str(evidence_manifest.relative_to(root)))

    passed = sum(1 for item in checks if item["pass"])
    failed = len(checks) - passed
    try:
        manifest_label = str(manifest_path.relative_to(root))
    except ValueError:
        manifest_label = str(manifest_path)

    return {
        "schema": "raf.session-initial-paths.validation.v1",
        "manifest": manifest_label,
        "checks_total": len(checks),
        "checks_passed": passed,
        "checks_failed": failed,
        "status": "PASS" if failed == 0 else "FAIL",
        "claim_allowed": False,
        "checks": checks,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate the initial executable paths of the complete session")
    parser.add_argument("--root", default=None, help="repository root; defaults to script parent/..")
    parser.add_argument("--manifest", default="manifests/session-initial-paths.v1.json")
    parser.add_argument("--write", default=None, help="optional report path relative to root")
    args = parser.parse_args()

    root = Path(args.root).resolve() if args.root else Path(__file__).resolve().parents[1]
    manifest_path = (root / args.manifest).resolve()
    report = validate(root, manifest_path)

    rendered = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True)
    print(rendered)
    if args.write:
        output = (root / args.write).resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(rendered + "\n", encoding="utf-8")
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
