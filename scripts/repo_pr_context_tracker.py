#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any

from repo_commit_tracker import (
    GitHubClient,
    TOKEN_VAZIO_API,
    TOKEN_VAZIO_AUTH,
    TrackerError,
    atomic_write,
    canonical_json_bytes,
    chain_hash,
    increment_counter,
    load_json,
    normalize_commit_message,
    repo_api_path,
    semantic_fingerprint,
    semantic_tags,
    utc_now,
    validate_config,
)

STATE_SCHEMA = "raf.repository-pr-context-state.v1"
REPORT_SCHEMA = "raf.repository-pr-context-report.v1"
SHARD_SCHEMA = "raf.repository-pr-context-shard.v1"
SHA40_RE = re.compile(r"^[0-9a-f]{40}$")
ZERO_CHAIN = "0" * 64


def initial_state(config: dict[str, Any]) -> dict[str, Any]:
    alphabet = config["shard"]["alphabet"]
    width = config["shard"]["width"]
    return {
        "schema": STATE_SCHEMA,
        "version": 1,
        "next_counter": alphabet[0] * width,
        "chain_head": ZERO_CHAIN,
        "repositories": {},
        "history": [],
        "last_run_utc": None,
    }


def load_state(path: Path, config: dict[str, Any]) -> dict[str, Any]:
    if not path.exists():
        return initial_state(config)
    state = load_json(path)
    if state.get("schema") != STATE_SCHEMA:
        raise TrackerError("wrong PR context state schema")
    if len(state.get("next_counter", "")) != config["shard"]["width"]:
        raise TrackerError("PR context counter width drift")
    return state


def compact_pull_request(data: dict[str, Any], repository: str, family: str) -> dict[str, Any]:
    number = data.get("number")
    if not isinstance(number, int) or number < 1:
        raise TrackerError(f"invalid PR number: {repository}: {number}")
    head = data.get("head") or {}
    base = data.get("base") or {}
    user = data.get("user") or {}
    head_sha = str(head.get("sha", ""))
    if not SHA40_RE.fullmatch(head_sha):
        raise TrackerError(f"invalid PR head SHA: {repository}#{number}")
    title = normalize_commit_message(data.get("title", ""))
    tags = semantic_tags(title)
    basis = {
        "number": number,
        "title": title,
        "state": data.get("state"),
        "draft": bool(data.get("draft")),
        "merged_at": data.get("merged_at"),
        "updated_at": data.get("updated_at"),
        "head_sha": head_sha,
        "head_ref": head.get("ref"),
        "base_ref": base.get("ref"),
    }
    return {
        "event_type": "pull_request",
        "repository": repository,
        "source_family": family,
        "pr_number": number,
        "title": title,
        "state": data.get("state"),
        "draft": bool(data.get("draft")),
        "merged_at": data.get("merged_at"),
        "created_at": data.get("created_at"),
        "updated_at": data.get("updated_at"),
        "head_sha": head_sha,
        "head_ref": head.get("ref"),
        "base_ref": base.get("ref"),
        "author_login": user.get("login"),
        "html_url": data.get("html_url"),
        "semantic_tags": tags,
        "semantic_fingerprint": semantic_fingerprint(title, tags),
        "change_fingerprint": hashlib.sha256(canonical_json_bytes(basis)).hexdigest(),
    }


def poll_pull_requests(
    client: GitHubClient,
    repository: dict[str, Any],
    previous: dict[str, str],
    max_per_repository: int,
) -> tuple[dict[str, Any], list[dict[str, Any]], dict[str, str]]:
    full_name = repository["full_name"]
    family = repository.get("family", "TOKEN_VAZIO_FAMILY")
    result = {
        "full_name": full_name,
        "family": family,
        "priority": repository["priority"],
        "status": "TOKEN_VAZIO",
        "token_vazio": None,
        "pull_requests": [],
        "changed_count": 0,
    }
    try:
        values = client.get(
            f"{repo_api_path(full_name)}/pulls",
            {
                "state": "all",
                "sort": "updated",
                "direction": "desc",
                "per_page": max_per_repository,
            },
        )
        if not isinstance(values, list):
            raise TrackerError(f"PR list expected: {full_name}")
        pulls = [compact_pull_request(value, full_name, family) for value in values]
        current = {str(item["pr_number"]): item["change_fingerprint"] for item in pulls}
        initialized = bool(previous)
        changed = [
            item for item in pulls
            if initialized and previous.get(str(item["pr_number"])) != item["change_fingerprint"]
        ]
        result["status"] = "OK"
        result["pull_requests"] = pulls
        result["changed_count"] = len(changed)
        return result, changed, current
    except PermissionError:
        result["token_vazio"] = TOKEN_VAZIO_AUTH
        return result, [], dict(previous)
    except TrackerError as error:
        result["token_vazio"] = f"{TOKEN_VAZIO_API}:{type(error).__name__}"
        return result, [], dict(previous)


def allocate_shards(
    events: list[dict[str, Any]],
    state: dict[str, Any],
    config: dict[str, Any],
    output_dir: Path,
) -> list[dict[str, Any]]:
    alphabet = config["shard"]["alphabet"]
    counter = state["next_counter"]
    previous_chain = state["chain_head"]
    shards: list[dict[str, Any]] = []
    for event in sorted(events, key=lambda item: (item["repository"], item["pr_number"])):
        payload_sha = hashlib.sha256(canonical_json_bytes(event)).hexdigest()
        current_chain = chain_hash(previous_chain, counter, payload_sha)
        shard = {
            "schema": SHARD_SCHEMA,
            "shard_id": counter,
            "previous_chain_hash": previous_chain,
            "payload_sha256": payload_sha,
            "chain_hash": current_chain,
            "event": event,
        }
        atomic_write(output_dir / "pr-shards" / f"{counter}.json", canonical_json_bytes(shard))
        shards.append(shard)
        counter = increment_counter(counter, alphabet)
        previous_chain = current_chain
    state["next_counter"] = counter
    state["chain_head"] = previous_chain
    return shards


def append_history(state: dict[str, Any], events: list[dict[str, Any]], limit: int = 2048) -> None:
    history = state.setdefault("history", [])
    for event in events:
        history.append({
            "repository": event["repository"],
            "source_family": event["source_family"],
            "pr_number": event["pr_number"],
            "title": event["title"],
            "state": event["state"],
            "draft": event["draft"],
            "updated_at": event["updated_at"],
            "head_sha": event["head_sha"],
            "semantic_tags": event["semantic_tags"],
            "semantic_fingerprint": event["semantic_fingerprint"],
            "change_fingerprint": event["change_fingerprint"],
        })
    if len(history) > limit:
        del history[:-limit]


def build_report(
    generated_at: str,
    repository_results: list[dict[str, Any]],
    events: list[dict[str, Any]],
    state: dict[str, Any],
    client: GitHubClient,
) -> tuple[dict[str, Any], str]:
    tag_counts: Counter[str] = Counter()
    family_counts: Counter[str] = Counter()
    state_counts: Counter[str] = Counter()
    for record in state.get("history", []):
        tag_counts.update(record.get("semantic_tags", []))
        family_counts[record.get("source_family", "TOKEN_VAZIO_FAMILY")] += 1
        state_counts[record.get("state", "TOKEN_VAZIO_STATE")] += 1
    report = {
        "schema": REPORT_SCHEMA,
        "generated_at": generated_at,
        "mode": "READ_ONLY_PULL_REQUEST_CONTEXT",
        "claim_allowed": False,
        "requests_used": client.requests_used,
        "request_budget": client.max_requests,
        "summary": {
            "repositories": len(repository_results),
            "repositories_ok": sum(1 for item in repository_results if item["status"] == "OK"),
            "changed_pull_requests": len(events),
            "history_events": len(state.get("history", [])),
            "next_shard_id": state["next_counter"],
            "chain_head": state["chain_head"],
        },
        "tag_counts": dict(tag_counts.most_common()),
        "family_counts": dict(family_counts.most_common()),
        "pull_request_state_counts": dict(state_counts.most_common()),
        "repositories": repository_results,
        "recent_changes": events[-50:],
        "weights": "TOKEN_VAZIO_CALIBRATION",
        "limitations": [
            "PR title semantics are deterministic indexing, not human comprehension.",
            "Changed PR metadata does not prove causality, quality, or authorship transfer.",
            "Private coverage depends on the read-only token scope."
        ]
    }
    lines = [
        "# Pull request context report",
        "",
        f"- generated: `{generated_at}`",
        f"- repositories: `{report['summary']['repositories']}`",
        f"- accessible: `{report['summary']['repositories_ok']}`",
        f"- changed PRs: `{len(events)}`",
        f"- bounded history: `{report['summary']['history_events']}`",
        f"- next PR shard: `{state['next_counter']}`",
        "- weights: `TOKEN_VAZIO_CALIBRATION`",
        "- claim_allowed: `false`",
        "",
        "## Semantic tags",
        ""
    ]
    if tag_counts:
        lines.extend(f"- `{tag}`: {count}" for tag, count in tag_counts.most_common())
    else:
        lines.append("- `TOKEN_VAZIO_BASELINE`: no PR delta has been observed yet")
    lines.extend([
        "",
        "## Interpretation limit",
        "",
        "The report records PR state transitions and transparent title tags. It does not infer intent, merit, copying, or causal influence.",
        ""
    ])
    return report, "\n".join(lines)


def run(
    config_path: Path,
    state_dir: Path,
    output_dir: Path,
    token: str,
    max_per_repository: int,
) -> dict[str, Any]:
    config = load_json(config_path)
    validate_config(config)
    if not 1 <= max_per_repository <= 20:
        raise TrackerError("max_per_repository must be between 1 and 20")
    state_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / "pr-context-state.json"
    state = load_state(state_path, config)
    selected = sorted(
        (item for item in config["repositories"] if item.get("enabled", True)),
        key=lambda item: (item["priority"], item["full_name"]),
    )[: config["api"]["max_repositories"]]
    client = GitHubClient(
        token=token,
        max_requests=min(40, config["api"]["max_requests"]),
        timeout_seconds=config["api"]["timeout_seconds"],
    )
    repository_results: list[dict[str, Any]] = []
    events: list[dict[str, Any]] = []
    next_states = dict(state.get("repositories", {}))
    for repository in selected:
        previous = next_states.get(repository["full_name"], {})
        result, changed, current = poll_pull_requests(
            client, repository, previous, max_per_repository
        )
        next_states[repository["full_name"]] = current
        repository_results.append(result)
        events.extend(changed)
    shards = allocate_shards(events, state, config, output_dir)
    state["repositories"] = next_states
    append_history(state, events)
    generated_at = utc_now()
    state["last_run_utc"] = generated_at
    report, markdown = build_report(generated_at, repository_results, events, state, client)
    report["summary"]["shards_created"] = len(shards)
    atomic_write(output_dir / "learning" / "pr-context.json", canonical_json_bytes(report))
    atomic_write(output_dir / "learning" / "pr-context.md", markdown.encode("utf-8"))
    atomic_write(state_path, canonical_json_bytes(state))
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Bounded read-only pull request context tracker")
    parser.add_argument("--config", type=Path, default=Path("configs/repository-tracker.v1.json"))
    parser.add_argument("--state-dir", type=Path, default=Path(".tracker-state"))
    parser.add_argument("--output-dir", type=Path, default=Path("build/repository-tracker"))
    parser.add_argument("--max-per-repository", type=int, default=6)
    args = parser.parse_args(argv)
    try:
        report = run(
            args.config,
            args.state_dir,
            args.output_dir,
            os.environ.get("GITHUB_API_TOKEN", ""),
            args.max_per_repository,
        )
        print(json.dumps({
            "state": "PASS",
            "schema": REPORT_SCHEMA,
            "changed_pull_requests": report["summary"]["changed_pull_requests"],
            "requests_used": report["requests_used"],
            "claim_allowed": False
        }, sort_keys=True))
        return 0
    except (TrackerError, OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        print(f"FAIL repository-pr-context: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
