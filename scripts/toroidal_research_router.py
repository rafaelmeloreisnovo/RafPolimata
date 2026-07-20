#!/usr/bin/env python3
"""Validate and execute deterministic cross-repository research routing."""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Any

CONTRACT_SCHEMA = "rafpolimata.toroidal-research-router.v1"
MANIFEST_SCHEMA = "rafpolimata.toroidal-research-routing-manifest.v1"
TOKEN_VAZIO = "TOKEN_VAZIO"


class RouterError(ValueError):
    pass


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise RouterError(f"file not found: {path}") from exc
    except json.JSONDecodeError as exc:
        raise RouterError(f"invalid JSON in {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise RouterError(f"{path}: root must be an object")
    return value


def _nonempty(value: Any, field: str, errors: list[str]) -> None:
    if not isinstance(value, str) or not value.strip():
        errors.append(f"{field} must be a non-empty string")


def _unique(items: Any, field: str, errors: list[str]) -> dict[str, dict[str, Any]]:
    if not isinstance(items, list):
        errors.append(f"{field} must be an array")
        return {}
    output: dict[str, dict[str, Any]] = {}
    for index, item in enumerate(items):
        if not isinstance(item, dict):
            errors.append(f"{field}[{index}] must be an object")
            continue
        identifier = item.get("id")
        if not isinstance(identifier, str) or not identifier:
            errors.append(f"{field}[{index}].id must be a non-empty string")
            continue
        if identifier in output:
            errors.append(f"duplicate {field} id: {identifier}")
            continue
        output[identifier] = item
    return output


def validate_contract(contract: dict[str, Any]) -> dict[str, Any]:
    errors: list[str] = []
    if contract.get("schema") != CONTRACT_SCHEMA:
        errors.append(f"schema must be {CONTRACT_SCHEMA}")
    if contract.get("authority") != "rafaelmeloreisnovo/RafPolimata":
        errors.append("authority must be rafaelmeloreisnovo/RafPolimata")
    if contract.get("claim_allowed") is not False:
        errors.append("contract claim_allowed must be false")
    governance = contract.get("canonical_governance", {})
    if governance.get("repository") != "rafaelmeloreisnovo/RafGitTools":
        errors.append("canonical governance authority mismatch")
    authorities = _unique(contract.get("authorities"), "authorities", errors)
    repositories: set[str] = set()
    for role, item in authorities.items():
        _nonempty(item.get("repository"), f"authorities.{role}.repository", errors)
        repo = item.get("repository")
        if repo in repositories:
            errors.append(f"duplicate authority repository: {repo}")
        repositories.add(repo)
    expected_roles = {"GOVERNANCE", "MAP", "SCIENCE", "ORCHESTRATION", "RUNTIME", "MEMORY"}
    if set(authorities) != expected_roles:
        errors.append("authorities must contain exactly the six canonical roles")
    object_kinds = contract.get("object_kinds")
    actions = contract.get("actions")
    if not isinstance(object_kinds, list) or not object_kinds:
        errors.append("object_kinds must be non-empty")
        object_kinds = []
    if not isinstance(actions, list) or not actions:
        errors.append("actions must be non-empty")
        actions = []
    rules = contract.get("route_rules")
    if not isinstance(rules, list):
        errors.append("route_rules must be an array")
        rules = []
    rule_map: dict[str, dict[str, Any]] = {}
    for index, rule in enumerate(rules):
        if not isinstance(rule, dict):
            errors.append(f"route_rules[{index}] must be an object")
            continue
        kind = rule.get("kind")
        if kind not in object_kinds:
            errors.append(f"route_rules[{index}].kind invalid")
            continue
        if kind in rule_map:
            errors.append(f"duplicate route rule: {kind}")
            continue
        rule_map[kind] = rule
        for field in ("source_roles", "allowed_targets", "allowed_actions"):
            values = rule.get(field)
            if not isinstance(values, list) or not values:
                errors.append(f"route_rules.{kind}.{field} must be non-empty")
                continue
            allowed = expected_roles if field != "allowed_actions" else set(actions)
            unknown = set(values) - allowed
            if unknown:
                errors.append(f"route_rules.{kind}.{field} unknown values: {sorted(unknown)}")
    if set(rule_map) != set(object_kinds):
        errors.append("every object kind requires exactly one route rule")
    policy = contract.get("promotion_policy")
    if not isinstance(policy, dict):
        errors.append("promotion_policy must be an object")
    else:
        if policy.get("router_may_promote") is not False:
            errors.append("router_may_promote must be false")
        if policy.get("promotion_action_allowed") is not False:
            errors.append("promotion_action_allowed must be false")
        if policy.get("scientific_claim_authority") != "SCIENCE":
            errors.append("scientific claim authority must remain SCIENCE")
        if policy.get("runtime_receipt_required_for_device_claim") is not True:
            errors.append("device claims must require runtime receipt")
    if errors:
        raise RouterError("\n".join(f"- {error}" for error in errors))
    return {
        "authorities": authorities,
        "repository_to_role": {item["repository"]: role for role, item in authorities.items()},
        "object_kinds": set(object_kinds),
        "actions": set(actions),
        "rules": rule_map,
        "policy": policy,
    }


def route_object(compiled: dict[str, Any], item: dict[str, Any]) -> dict[str, Any]:
    kind = item.get("kind")
    source_repo = item.get("source_repository")
    target_repo = item.get("target_repository")
    action = item.get("requested_action")
    source_role = compiled["repository_to_role"].get(source_repo)
    target_role = compiled["repository_to_role"].get(target_repo)
    if kind not in compiled["object_kinds"]:
        raise RouterError(f"unknown object kind: {kind}")
    if source_role is None:
        raise RouterError(f"unknown source repository: {source_repo}")
    if target_role is None:
        raise RouterError(f"unknown target repository: {target_repo}")
    if action not in compiled["actions"]:
        raise RouterError(f"unknown action: {action}")
    if action == "PROMOTE":
        raise RouterError("router cannot promote claims")
    rule = compiled["rules"][kind]
    if source_role not in rule["source_roles"]:
        raise RouterError(f"{kind} cannot originate from role {source_role}")
    if target_role not in rule["allowed_targets"]:
        raise RouterError(f"{kind} cannot target role {target_role}")
    if action not in rule["allowed_actions"]:
        raise RouterError(f"{kind} cannot use action {action}")
    if kind == "CLAIM" and source_role != "SCIENCE":
        raise RouterError("scientific claims must originate from SCIENCE")
    if item.get("device_claim") is True:
        receipt = item.get("runtime_receipt")
        if not isinstance(receipt, str) or not receipt.strip() or receipt == TOKEN_VAZIO:
            return {
                "object_id": item.get("id"),
                "decision": "BLOCKED_TOKEN_VAZIO",
                "reason": "device claim requires runtime receipt",
                "source_role": source_role,
                "target_role": target_role,
                "claim_allowed": False,
            }
    if item.get("state") == TOKEN_VAZIO or kind == TOKEN_VAZIO:
        return {
            "object_id": item.get("id"),
            "decision": "BLOCKED_TOKEN_VAZIO",
            "reason": "missing evidence preserved",
            "source_role": source_role,
            "target_role": target_role,
            "claim_allowed": False,
        }
    return {
        "object_id": item.get("id"),
        "decision": "ROUTED_LIMITED",
        "reason": "authority and route rule satisfied",
        "source_role": source_role,
        "target_role": target_role,
        "claim_allowed": False,
    }


def validate_manifest(contract: dict[str, Any], manifest: dict[str, Any]) -> dict[str, Any]:
    compiled = validate_contract(contract)
    errors: list[str] = []
    if manifest.get("schema") != MANIFEST_SCHEMA:
        errors.append(f"schema must be {MANIFEST_SCHEMA}")
    for field in ("manifest_id", "observed_at", "declared_scope"):
        _nonempty(manifest.get(field), field, errors)
    if manifest.get("claim_allowed") is not False:
        errors.append("manifest claim_allowed must be false")
    objects = _unique(manifest.get("objects"), "objects", errors)
    decisions: list[dict[str, Any]] = []
    if not errors:
        for object_id in sorted(objects):
            item = objects[object_id]
            for field in ("kind", "source_repository", "target_repository", "requested_action", "state"):
                _nonempty(item.get(field), f"objects.{object_id}.{field}", errors)
            if errors:
                continue
            try:
                decisions.append(route_object(compiled, item))
            except RouterError as exc:
                errors.append(f"objects.{object_id}: {exc}")
    if errors:
        raise RouterError("\n".join(f"- {error}" for error in errors))
    return {
        "status": "PASS",
        "manifest_id": manifest["manifest_id"],
        "object_count": len(objects),
        "routed_limited_count": sum(d["decision"] == "ROUTED_LIMITED" for d in decisions),
        "blocked_token_vazio_count": sum(d["decision"] == "BLOCKED_TOKEN_VAZIO" for d in decisions),
        "claim_allowed": False,
        "decisions": decisions,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    p1 = sub.add_parser("validate-contract")
    p1.add_argument("contract", type=Path)
    p2 = sub.add_parser("validate-manifest")
    p2.add_argument("contract", type=Path)
    p2.add_argument("manifest", type=Path)
    p3 = sub.add_parser("summarize")
    p3.add_argument("contract", type=Path)
    p3.add_argument("manifest", type=Path)
    args = parser.parse_args(argv)
    try:
        contract = load_json(args.contract)
        if args.command == "validate-contract":
            validate_contract(contract)
            result = {"status": "PASS", "schema": CONTRACT_SCHEMA, "claim_allowed": False}
        else:
            result = validate_manifest(contract, load_json(args.manifest))
    except RouterError as exc:
        print(str(exc), file=sys.stderr)
        return 1
    print(json.dumps(result, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
