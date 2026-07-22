#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
import urllib.error
import urllib.parse
import urllib.request
import zipfile
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

CONFIG_SCHEMA = "raf.repository-tracker-config.v1"
STATE_SCHEMA = "raf.repository-tracker-state.v1"
MANIFEST_SCHEMA = "raf.repository-tracker-manifest.v1"
SHARD_SCHEMA = "raf.repository-tracker-shard.v1"
SAFE_ALPHABET_RE = re.compile(r"^[0-9A-Za-z._~-]+$")
FULL_NAME_RE = re.compile(r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")
SHA40_RE = re.compile(r"^[0-9a-f]{40}$")
TOKEN_VAZIO_AUTH = "TOKEN_VAZIO_AUTH_SCOPE_OR_NOT_FOUND"
TOKEN_VAZIO_API = "TOKEN_VAZIO_GITHUB_API"
ZERO_CHAIN = "0" * 64

SEMANTIC_RULES: tuple[tuple[str, tuple[str, ...]], ...] = (
    ("build", ("build", "cmake", "gradle", "compile", "compiler", "linker", "toolchain")),
    ("ci", ("ci", "workflow", "actions", "runner", "pipeline")),
    ("test", ("test", "tests", "benchmark", "coverage", "fixture", "fuzz")),
    ("docs", ("docs", "documentation", "readme", "paper", "schema")),
    ("security", ("security", "root", "magisk", "permission", "auth", "token", "secret")),
    ("performance", ("performance", "optimize", "simd", "avx", "neon", "cache", "parallel")),
    ("android", ("android", "termux", "apk", "dex", "gradle")),
    ("virtualization", ("qemu", "vm", "virtual", "emulator", "userland")),
    ("kernel", ("kernel", "linux", "freebsd", "ubuntu", "driver")),
    ("crypto", ("blake3", "hash", "crypto", "cipher", "sha")),
    ("governance", ("audit", "evidence", "provenance", "custody", "token_vazio")),
    ("fix", ("fix", "bug", "warning", "error", "revert")),
    ("feature", ("feat", "feature", "implement", "add", "create")),
)


class TrackerError(RuntimeError):
    pass


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def canonical_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")


def atomic_write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(data)
    os.replace(temporary, path)


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise TrackerError(f"JSON object required: {path}")
    return value


def validate_shard_alphabet(alphabet: str) -> None:
    if len(alphabet) < 2:
        raise TrackerError("shard alphabet must contain at least two symbols")
    if len(set(alphabet)) != len(alphabet):
        raise TrackerError("shard alphabet contains duplicate symbols")
    if not SAFE_ALPHABET_RE.fullmatch(alphabet):
        raise TrackerError("shard alphabet must use URL/path-safe visible ASCII only")
    if "/" in alphabet or "\\" in alphabet:
        raise TrackerError("path separators are forbidden in shard alphabet")


def increment_counter(counter: str, alphabet: str) -> str:
    validate_shard_alphabet(alphabet)
    if not counter or any(char not in alphabet for char in counter):
        raise TrackerError("counter contains a symbol outside the configured alphabet")
    indexes = {char: position for position, char in enumerate(alphabet)}
    digits = [indexes[char] for char in counter]
    for position in range(len(digits) - 1, -1, -1):
        if digits[position] + 1 < len(alphabet):
            digits[position] += 1
            break
        digits[position] = 0
    else:
        raise TrackerError("fixed-width shard counter overflow")
    return "".join(alphabet[index] for index in digits)


def normalize_commit_message(message: str) -> str:
    first_line = (message or "").splitlines()[0].strip()
    return re.sub(r"\s+", " ", first_line)[:240]


def semantic_tags(message: str) -> list[str]:
    lowered = normalize_commit_message(message).casefold()
    tags = [tag for tag, terms in SEMANTIC_RULES if any(term in lowered for term in terms)]
    return tags or ["unclassified"]


def semantic_fingerprint(message: str, tags: Iterable[str]) -> str:
    normalized = normalize_commit_message(message).casefold()
    tokens = sorted(set(re.findall(r"[a-z0-9_+-]{2,}", normalized)))
    payload = {"tokens": tokens, "tags": sorted(set(tags))}
    return hashlib.sha256(canonical_json_bytes(payload)).hexdigest()


def chain_hash(previous: str, shard_id: str, payload_sha256: str) -> str:
    if not re.fullmatch(r"[0-9a-f]{64}", previous):
        raise TrackerError("invalid previous chain hash")
    if not re.fullmatch(r"[0-9a-f]{64}", payload_sha256):
        raise TrackerError("invalid payload sha256")
    material = f"{previous}\n{shard_id}\n{payload_sha256}\n".encode("ascii")
    return hashlib.sha256(material).hexdigest()


def validate_config(config: dict[str, Any]) -> None:
    if config.get("schema") != CONFIG_SCHEMA:
        raise TrackerError("wrong tracker config schema")
    owner = config.get("owner")
    if not isinstance(owner, str) or not owner:
        raise TrackerError("owner is required")
    interval = config.get("poll_interval_minutes")
    if not isinstance(interval, int) or interval < 15:
        raise TrackerError("poll interval must be at least 15 minutes")
    api = config.get("api")
    if not isinstance(api, dict):
        raise TrackerError("api config is required")
    for field, lower, upper in (
        ("max_requests", 1, 500),
        ("timeout_seconds", 1, 60),
        ("max_repositories", 1, 100),
        ("max_commits_per_repository", 1, 100),
        ("max_forks_per_repository", 0, 20),
    ):
        value = api.get(field)
        if not isinstance(value, int) or not lower <= value <= upper:
            raise TrackerError(f"invalid api.{field}")
    shard = config.get("shard")
    if not isinstance(shard, dict):
        raise TrackerError("shard config is required")
    validate_shard_alphabet(shard.get("alphabet", ""))
    width = shard.get("width")
    if not isinstance(width, int) or not 2 <= width <= 32:
        raise TrackerError("shard width must be between 2 and 32")
    if not isinstance(shard.get("snapshot_after_stable_runs"), int) or shard["snapshot_after_stable_runs"] < 1:
        raise TrackerError("snapshot_after_stable_runs must be positive")
    if not isinstance(shard.get("max_history_events"), int) or shard["max_history_events"] < 1:
        raise TrackerError("max_history_events must be positive")
    repositories = config.get("repositories")
    if not isinstance(repositories, list) or not repositories:
        raise TrackerError("repositories must be a non-empty list")
    seen: set[str] = set()
    for record in repositories:
        if not isinstance(record, dict):
            raise TrackerError("repository records must be objects")
        full_name = record.get("full_name")
        if not isinstance(full_name, str) or not FULL_NAME_RE.fullmatch(full_name):
            raise TrackerError(f"invalid repository full_name: {full_name}")
        if full_name in seen:
            raise TrackerError(f"duplicate repository: {full_name}")
        seen.add(full_name)
        if not isinstance(record.get("priority"), int):
            raise TrackerError(f"priority required: {full_name}")
        if not isinstance(record.get("follow_forks"), bool):
            raise TrackerError(f"follow_forks must be boolean: {full_name}")
    safety = config.get("safety")
    required_safety = {
        "read_only": True,
        "auto_fork": False,
        "auto_merge": False,
        "auto_push": False,
        "execute_external_code": False,
        "clone_external_repositories": False,
    }
    if safety != required_safety:
        raise TrackerError("safety policy drift: tracker must remain read-only and non-executing")


@dataclass
class GitHubClient:
    token: str
    max_requests: int
    timeout_seconds: int
    api_url: str = "https://api.github.com"
    requests_used: int = 0

    def get(self, path: str, params: dict[str, Any] | None = None) -> Any:
        if self.requests_used >= self.max_requests:
            raise TrackerError("GitHub API request budget exhausted")
        if not path.startswith("/"):
            raise TrackerError("GitHub API path must be absolute")
        query = urllib.parse.urlencode(params or {})
        url = f"{self.api_url}{path}" + (f"?{query}" if query else "")
        headers = {
            "Accept": "application/vnd.github+json",
            "User-Agent": "RafPolimata-Repository-Tracker/1.0",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"
        request = urllib.request.Request(url, headers=headers, method="GET")
        self.requests_used += 1
        try:
            with urllib.request.urlopen(request, timeout=self.timeout_seconds) as response:
                return json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as error:
            body = error.read().decode("utf-8", errors="replace")[:500]
            if error.code in {401, 403, 404}:
                raise PermissionError(f"GitHub API {error.code}: {path}") from error
            raise TrackerError(f"GitHub API {error.code}: {path}: {body}") from error
        except urllib.error.URLError as error:
            raise TrackerError(f"GitHub API unavailable: {path}: {error.reason}") from error


def initial_state(config: dict[str, Any]) -> dict[str, Any]:
    alphabet = config["shard"]["alphabet"]
    width = config["shard"]["width"]
    return {
        "schema": STATE_SCHEMA,
        "version": 1,
        "next_counter": alphabet[0] * width,
        "chain_head": ZERO_CHAIN,
        "global_stable_runs": 0,
        "repositories": {},
        "history": [],
        "last_run_utc": None,
    }


def load_state(path: Path, config: dict[str, Any]) -> dict[str, Any]:
    if not path.exists():
        return initial_state(config)
    state = load_json(path)
    if state.get("schema") != STATE_SCHEMA:
        raise TrackerError("wrong tracker state schema")
    expected_width = config["shard"]["width"]
    if len(state.get("next_counter", "")) != expected_width:
        raise TrackerError("state counter width drift")
    validate_shard_alphabet(config["shard"]["alphabet"])
    return state


def repo_api_path(full_name: str) -> str:
    if not FULL_NAME_RE.fullmatch(full_name):
        raise TrackerError(f"invalid repository name: {full_name}")
    owner, name = full_name.split("/", 1)
    return f"/repos/{urllib.parse.quote(owner)}/{urllib.parse.quote(name)}"


def compact_repo_metadata(data: dict[str, Any]) -> dict[str, Any]:
    parent = data.get("parent") or {}
    source = data.get("source") or {}
    return {
        "full_name": data.get("full_name"),
        "default_branch": data.get("default_branch"),
        "private": bool(data.get("private")),
        "fork": bool(data.get("fork")),
        "archived": bool(data.get("archived")),
        "disabled": bool(data.get("disabled")),
        "parent_full_name": parent.get("full_name"),
        "source_full_name": source.get("full_name"),
        "updated_at": data.get("updated_at"),
        "pushed_at": data.get("pushed_at"),
    }


def compact_commit(data: dict[str, Any], repository: str, relation: str = "repository") -> dict[str, Any]:
    sha = str(data.get("sha", ""))
    if not SHA40_RE.fullmatch(sha):
        raise TrackerError(f"invalid commit SHA from GitHub: {repository}: {sha}")
    commit = data.get("commit") or {}
    author = data.get("author") or {}
    message = normalize_commit_message(commit.get("message", ""))
    tags = semantic_tags(message)
    return {
        "event_type": "commit_head" if relation == "fork" else "commit",
        "repository": repository,
        "relation": relation,
        "sha": sha,
        "message": message,
        "author_login": author.get("login"),
        "authored_at": ((commit.get("author") or {}).get("date")),
        "committed_at": ((commit.get("committer") or {}).get("date")),
        "html_url": data.get("html_url"),
        "semantic_tags": tags,
        "semantic_fingerprint": semantic_fingerprint(message, tags),
    }


def poll_repository(
    client: GitHubClient,
    record: dict[str, Any],
    previous: dict[str, Any],
    api_config: dict[str, Any],
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    full_name = record["full_name"]
    base_path = repo_api_path(full_name)
    result: dict[str, Any] = {
        "full_name": full_name,
        "family": record.get("family", "TOKEN_VAZIO_FAMILY"),
        "priority": record["priority"],
        "aliases": record.get("aliases", []),
        "status": "TOKEN_VAZIO",
        "token_vazio": None,
        "metadata": None,
        "head_sha": None,
        "new_commit_count": 0,
        "forks": [],
    }
    events: list[dict[str, Any]] = []
    try:
        metadata = client.get(base_path)
        result["metadata"] = compact_repo_metadata(metadata)
        branch = metadata.get("default_branch")
        commits = client.get(
            f"{base_path}/commits",
            {"sha": branch, "per_page": api_config["max_commits_per_repository"]},
        )
        if not isinstance(commits, list):
            raise TrackerError(f"commit list expected: {full_name}")
        compacted = [compact_commit(item, full_name) for item in commits]
        previous_seen = set(previous.get("seen_commit_shas", []))
        new_events = [event for event in compacted if event["sha"] not in previous_seen]
        if previous.get("initialized"):
            events.extend(new_events)
        result["head_sha"] = compacted[0]["sha"] if compacted else None
        result["new_commit_count"] = len(new_events) if previous.get("initialized") else 0
        result["status"] = "OK"
        result["token_vazio"] = None

        if record.get("follow_forks") and api_config["max_forks_per_repository"] > 0:
            forks = client.get(
                f"{base_path}/forks",
                {"sort": "newest", "per_page": api_config["max_forks_per_repository"]},
            )
            if not isinstance(forks, list):
                raise TrackerError(f"fork list expected: {full_name}")
            previous_forks = previous.get("fork_heads", {})
            for fork in forks:
                fork_name = fork.get("full_name")
                branch = fork.get("default_branch")
                if not isinstance(fork_name, str) or not FULL_NAME_RE.fullmatch(fork_name):
                    continue
                fork_record = {
                    "full_name": fork_name,
                    "default_branch": branch,
                    "head_sha": None,
                    "status": "TOKEN_VAZIO",
                }
                try:
                    fork_commits = client.get(
                        f"{repo_api_path(fork_name)}/commits",
                        {"sha": branch, "per_page": 1},
                    )
                    if isinstance(fork_commits, list) and fork_commits:
                        event = compact_commit(fork_commits[0], fork_name, relation="fork")
                        fork_record["head_sha"] = event["sha"]
                        fork_record["status"] = "OK"
                        if previous.get("initialized") and previous_forks.get(fork_name) != event["sha"]:
                            event["source_repository"] = full_name
                            events.append(event)
                except (PermissionError, TrackerError):
                    fork_record["status"] = TOKEN_VAZIO_API
                result["forks"].append(fork_record)

        seen = [event["sha"] for event in compacted]
        fork_heads = {
            item["full_name"]: item["head_sha"]
            for item in result["forks"]
            if item.get("head_sha")
        }
        result["next_state"] = {
            "initialized": True,
            "head_sha": result["head_sha"],
            "seen_commit_shas": seen,
            "fork_heads": fork_heads,
            "last_poll_utc": utc_now(),
        }
    except PermissionError:
        result["status"] = "TOKEN_VAZIO"
        result["token_vazio"] = TOKEN_VAZIO_AUTH
        result["next_state"] = dict(previous)
    except TrackerError as error:
        result["status"] = "TOKEN_VAZIO"
        result["token_vazio"] = f"{TOKEN_VAZIO_API}:{type(error).__name__}"
        result["next_state"] = dict(previous)
    return result, events


def allocate_shards(
    events: list[dict[str, Any]],
    state: dict[str, Any],
    config: dict[str, Any],
    output_dir: Path,
) -> list[dict[str, Any]]:
    alphabet = config["shard"]["alphabet"]
    counter = state["next_counter"]
    previous_chain = state["chain_head"]
    allocated: list[dict[str, Any]] = []
    for event in sorted(events, key=lambda item: (item["repository"], item.get("committed_at") or "", item["sha"])):
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
        atomic_write(output_dir / "shards" / f"{counter}.json", canonical_json_bytes(shard))
        allocated.append(shard)
        previous_chain = current_chain
        counter = increment_counter(counter, alphabet)
    state["next_counter"] = counter
    state["chain_head"] = previous_chain
    return allocated


def update_stability(state: dict[str, Any], event_count: int, threshold: int) -> bool:
    if event_count == 0:
        state["global_stable_runs"] = int(state.get("global_stable_runs", 0)) + 1
    else:
        state["global_stable_runs"] = 0
    return state["global_stable_runs"] >= threshold


def append_history(state: dict[str, Any], events: list[dict[str, Any]], limit: int) -> None:
    history = state.setdefault("history", [])
    for event in events:
        history.append({
            "repository": event["repository"],
            "sha": event["sha"],
            "event_type": event["event_type"],
            "semantic_tags": event["semantic_tags"],
            "semantic_fingerprint": event["semantic_fingerprint"],
            "committed_at": event.get("committed_at"),
        })
    if len(history) > limit:
        del history[:-limit]


def deterministic_zip(source_dir: Path, target: Path) -> None:
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = target.with_suffix(target.suffix + ".tmp")
    with zipfile.ZipFile(temporary, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(item for item in source_dir.rglob("*") if item.is_file() and item not in {target, temporary}):
            relative = path.relative_to(source_dir).as_posix()
            info = zipfile.ZipInfo(relative, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            archive.writestr(info, path.read_bytes(), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    os.replace(temporary, target)


def run_tracker(config_path: Path, state_dir: Path, output_dir: Path, token: str) -> dict[str, Any]:
    config = load_json(config_path)
    validate_config(config)
    output_dir.mkdir(parents=True, exist_ok=True)
    state_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / "state.json"
    state = load_state(state_path, config)
    client = GitHubClient(
        token=token,
        max_requests=config["api"]["max_requests"],
        timeout_seconds=config["api"]["timeout_seconds"],
    )
    selected = sorted(
        (record for record in config["repositories"] if record.get("enabled", True)),
        key=lambda item: (item["priority"], item["full_name"]),
    )[: config["api"]["max_repositories"]]

    repository_results: list[dict[str, Any]] = []
    all_events: list[dict[str, Any]] = []
    next_repository_state = dict(state.get("repositories", {}))
    for record in selected:
        previous = next_repository_state.get(record["full_name"], {})
        result, events = poll_repository(client, record, previous, config["api"])
        next_repository_state[record["full_name"]] = result.pop("next_state")
        repository_results.append(result)
        all_events.extend(events)

    shards = allocate_shards(all_events, state, config, output_dir)
    state["repositories"] = next_repository_state
    append_history(state, all_events, config["shard"]["max_history_events"])
    snapshot_ready = update_stability(
        state,
        len(all_events),
        config["shard"]["snapshot_after_stable_runs"],
    )
    generated_at = utc_now()
    state["last_run_utc"] = generated_at
    ok_count = sum(1 for item in repository_results if item["status"] == "OK")
    token_vazio_count = len(repository_results) - ok_count
    manifest = {
        "schema": MANIFEST_SCHEMA,
        "version": 1,
        "generated_at": generated_at,
        "owner": config["owner"],
        "mode": "READ_ONLY_METADATA_AND_COMMIT_LINEAGE",
        "claim_allowed": False,
        "requests_used": client.requests_used,
        "request_budget": client.max_requests,
        "poll_interval_minutes": config["poll_interval_minutes"],
        "summary": {
            "repositories_selected": len(selected),
            "repositories_ok": ok_count,
            "repositories_token_vazio": token_vazio_count,
            "new_events": len(all_events),
            "shards_created": len(shards),
            "global_stable_runs": state["global_stable_runs"],
            "snapshot_ready": snapshot_ready,
            "next_shard_id": state["next_counter"],
            "chain_head": state["chain_head"],
        },
        "repositories": repository_results,
        "safety": config["safety"],
        "limitations": [
            "Commit metadata is evidence of repository state, not proof of causality or code execution.",
            "Fork traversal is bounded and does not clone or execute external code.",
            "Private repository coverage depends on RAFAELIA_GITHUB_READ_TOKEN scope.",
            "Scheduled workflows run from the default branch and may be delayed by GitHub."
        ]
    }
    atomic_write(output_dir / "manifest.json", canonical_json_bytes(manifest))
    atomic_write(output_dir / "state-sanitized.json", canonical_json_bytes({
        "schema": STATE_SCHEMA,
        "next_counter": state["next_counter"],
        "chain_head": state["chain_head"],
        "global_stable_runs": state["global_stable_runs"],
        "history_size": len(state["history"]),
        "last_run_utc": state["last_run_utc"]
    }))
    atomic_write(state_path, canonical_json_bytes(state))
    deterministic_zip(output_dir, output_dir / "repository-tracker-snapshot.zip")
    return manifest


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Bounded read-only repository/fork/commit tracker")
    parser.add_argument("--config", type=Path, default=Path("configs/repository-tracker.v1.json"))
    parser.add_argument("--state-dir", type=Path, default=Path(".tracker-state"))
    parser.add_argument("--output-dir", type=Path, default=Path("build/repository-tracker"))
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args(argv)
    try:
        config = load_json(args.config)
        validate_config(config)
        if args.validate_only:
            print(json.dumps({"state": "PASS", "schema": CONFIG_SCHEMA}, sort_keys=True))
            return 0
        token = os.environ.get("GITHUB_API_TOKEN", "")
        manifest = run_tracker(args.config, args.state_dir, args.output_dir, token)
        print(json.dumps({
            "state": "PASS",
            "schema": MANIFEST_SCHEMA,
            "new_events": manifest["summary"]["new_events"],
            "snapshot_ready": manifest["summary"]["snapshot_ready"],
            "requests_used": manifest["requests_used"],
            "claim_allowed": False
        }, sort_keys=True))
        return 0
    except (TrackerError, OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        print(f"FAIL repository-tracker: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
