#!/usr/bin/env python3
"""Observe GitHub provider rules without promoting untested enforcement.

CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY

Stdlib-only. A successful readback proves only what GitHub reports at the time
of observation. It does not prove that a zero-approval merge attempt was
rejected, so the full provider-enforcement gate remains TOKEN_VAZIO until a
separate negative rejection receipt exists.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[2]
POLICY_PATH = ROOT / "rafci" / "provider_policy.v1.json"


class AuditError(RuntimeError):
    pass


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise AuditError(message)


def _load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fh:
        data = json.load(fh)
    _require(isinstance(data, dict), f"expected object: {path}")
    return data


def _sha256_json(value: Any) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _http_json(url: str, token: str) -> Any:
    headers = {
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
        "User-Agent": "rafci-provider-audit-v1",
    }
    if token:
        headers["Authorization"] = f"Bearer {token}"
    request = Request(url, headers=headers)
    try:
        with urlopen(request, timeout=20) as response:
            return json.loads(response.read().decode("utf-8"))
    except HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")[:500]
        raise AuditError(f"provider_http_error status={exc.code} body={body}") from exc
    except (URLError, TimeoutError, json.JSONDecodeError) as exc:
        raise AuditError(f"provider_readback_error: {exc}") from exc


def _ruleset_applies_to_ref(ruleset: dict[str, Any], target_ref: str) -> bool:
    conditions = ruleset.get("conditions") or {}
    ref_name = conditions.get("ref_name") or {}
    includes = ref_name.get("include") or []
    excludes = ref_name.get("exclude") or []

    aliases = {target_ref, target_ref.removeprefix("refs/heads/")}
    if target_ref == "refs/heads/main":
        aliases.add("~DEFAULT_BRANCH")

    if any(item in aliases or item == "~ALL" for item in excludes):
        return False
    if not includes:
        return True
    return any(item in aliases or item == "~ALL" for item in includes)


def _normalize(rulesets: list[dict[str, Any]], target_ref: str) -> dict[str, Any]:
    active = [r for r in rulesets if r.get("enforcement") == "active"]
    applicable = [r for r in active if _ruleset_applies_to_ref(r, target_ref)]

    rule_types: set[str] = set()
    approval_counts: list[int] = []
    status_check_counts: list[int] = []
    bypass_total = 0
    normalized_rulesets: list[dict[str, Any]] = []

    for ruleset in applicable:
        rules = ruleset.get("rules") or []
        types: list[str] = []
        for rule in rules:
            rule_type = str(rule.get("type", ""))
            if rule_type:
                rule_types.add(rule_type)
                types.append(rule_type)
            params = rule.get("parameters") or {}
            if rule_type == "pull_request":
                approval_counts.append(int(params.get("required_approving_review_count", 0) or 0))
            if rule_type == "required_status_checks":
                checks = params.get("required_status_checks") or []
                status_check_counts.append(len(checks))
        bypass = ruleset.get("bypass_actors") or []
        bypass_total += len(bypass)
        normalized_rulesets.append({
            "id": ruleset.get("id"),
            "name": ruleset.get("name"),
            "target": ruleset.get("target"),
            "enforcement": ruleset.get("enforcement"),
            "rule_types": sorted(set(types)),
            "bypass_actor_count": len(bypass),
        })

    return {
        "active_ruleset_count": len(active),
        "applicable_ruleset_count": len(applicable),
        "rule_types": sorted(rule_types),
        "max_required_approvals": max(approval_counts, default=0),
        "max_required_status_checks": max(status_check_counts, default=0),
        "bypass_actor_count": bypass_total,
        "rulesets": sorted(normalized_rulesets, key=lambda x: (str(x.get("id")), str(x.get("name")))),
    }


def _evaluate(policy: dict[str, Any], observed: dict[str, Any]) -> dict[str, Any]:
    req = policy["required_configuration_for_readiness"]
    rule_types = set(observed["rule_types"])
    coverage = {
        "active_ruleset": observed["active_ruleset_count"] > 0,
        "applies_to_target_ref": observed["applicable_ruleset_count"] > 0,
        "zero_bypass_actors": observed["bypass_actor_count"] == 0,
        "deletion_protection": "deletion" in rule_types,
        "non_fast_forward_protection": "non_fast_forward" in rule_types,
        "pull_request_rule": "pull_request" in rule_types,
        "minimum_required_approvals": observed["max_required_approvals"] >= int(req["minimum_required_approvals"]),
        "required_status_checks_rule": "required_status_checks" in rule_types,
        "minimum_required_status_checks": observed["max_required_status_checks"] >= int(req["minimum_required_status_checks"]),
    }
    configuration_ready = all(coverage.values())
    missing = sorted(name for name, ok in coverage.items() if not ok)
    return {
        "coverage": coverage,
        "configuration_ready": configuration_ready,
        "missing_configuration": missing,
    }


def _selftest(policy: dict[str, Any]) -> None:
    weak = [{
        "id": 1,
        "name": "weak",
        "target": "branch",
        "enforcement": "active",
        "conditions": {"ref_name": {"include": [], "exclude": []}},
        "rules": [{"type": "deletion"}, {"type": "non_fast_forward"}],
        "bypass_actors": [],
    }]
    strong = [{
        "id": 2,
        "name": "strong",
        "target": "branch",
        "enforcement": "active",
        "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
        "rules": [
            {"type": "deletion"},
            {"type": "non_fast_forward"},
            {"type": "pull_request", "parameters": {"required_approving_review_count": 1}},
            {"type": "required_status_checks", "parameters": {"required_status_checks": [{"context": "CI"}]}},
        ],
        "bypass_actors": [],
    }]
    weak_eval = _evaluate(policy, _normalize(weak, policy["target_ref"]))
    strong_eval = _evaluate(policy, _normalize(strong, policy["target_ref"]))
    _require(not weak_eval["configuration_ready"], "selftest weak configuration was accepted")
    _require(strong_eval["configuration_ready"], "selftest strong configuration was rejected")


def _fetch_rulesets(api_base: str, repository: str, token: str) -> list[dict[str, Any]]:
    list_url = f"{api_base.rstrip('/')}/repos/{repository}/rulesets"
    summaries = _http_json(list_url, token)
    _require(isinstance(summaries, list), "ruleset list response is not a list")
    details: list[dict[str, Any]] = []
    for summary in summaries:
        ruleset_id = summary.get("id")
        _require(ruleset_id is not None, "ruleset summary missing id")
        detail = _http_json(f"{list_url}/{ruleset_id}", token)
        _require(isinstance(detail, dict), f"ruleset detail {ruleset_id} is not an object")
        details.append(detail)
    return details


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Audit RafCI provider ruleset readback")
    parser.add_argument("--repository", default="rafaelmeloreisnovo/RafPolimata")
    parser.add_argument("--api-base", default="https://api.github.com")
    parser.add_argument("--token-env", default="GITHUB_TOKEN")
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--snapshot", type=Path)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)

    policy = _load_json(POLICY_PATH)
    _require(policy.get("schema") == "rafaelia.rafci.provider-policy/v1", "unexpected provider policy schema")
    _require(policy.get("closure") == "CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY", "provider policy closure drift")
    _require(policy.get("claim_allowed") is False, "provider policy claim_allowed must remain false")
    _require(args.repository == policy.get("repository"), "repository does not match provider policy")

    if args.selftest:
        _selftest(policy)

    receipt: dict[str, Any] = {
        "schema": "rafaelia.rafci.provider-observation-receipt/v1",
        "repository": args.repository,
        "target_ref": policy["target_ref"],
        "closure": policy["closure"],
        "provider_readback_state": "TOKEN_VAZIO",
        "configuration_ready": False,
        "zero_approval_rejection_receipt": "TOKEN_VAZIO",
        "provider_enforcement_gate": "TOKEN_VAZIO",
        "claim_allowed": False,
    }

    try:
        rulesets = _fetch_rulesets(args.api_base, args.repository, os.environ.get(args.token_env, ""))
        observed = _normalize(rulesets, policy["target_ref"])
        evaluation = _evaluate(policy, observed)
        receipt.update({
            "provider_readback_state": "PASS",
            "observed": observed,
            "observed_sha256": _sha256_json(observed),
            "coverage": evaluation["coverage"],
            "configuration_ready": evaluation["configuration_ready"],
            "missing_configuration": evaluation["missing_configuration"],
            "zero_approval_rejection_receipt": "TOKEN_VAZIO_NOT_EXECUTED",
            "provider_enforcement_gate": "TOKEN_VAZIO",
            "promotion_reason": (
                "readback observed but full gate requires separate zero-approval rejection receipt"
                if evaluation["configuration_ready"]
                else "provider configuration does not yet satisfy readiness policy and no negative rejection receipt exists"
            ),
        })
        if args.snapshot:
            args.snapshot.parent.mkdir(parents=True, exist_ok=True)
            args.snapshot.write_text(json.dumps(observed, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    except AuditError as exc:
        receipt.update({
            "provider_readback_state": "TOKEN_VAZIO_READBACK_UNAVAILABLE",
            "readback_error": str(exc),
            "promotion_reason": "provider readback unavailable; fail closed",
        })

    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({
        "provider_readback_state": receipt["provider_readback_state"],
        "configuration_ready": receipt["configuration_ready"],
        "provider_enforcement_gate": receipt["provider_enforcement_gate"],
        "missing_configuration": receipt.get("missing_configuration", []),
        "claim_allowed": receipt["claim_allowed"],
    }, sort_keys=True))

    # The observation job succeeds when it produces a truthful fail-closed receipt.
    # Promotion to PASS is intentionally impossible in this readback-only tool.
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AuditError as exc:
        print(f"RAFCI_PROVIDER_AUDIT_FAIL: {exc}", file=sys.stderr)
        raise SystemExit(2)
