#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "configs" / "zipraf-ao42-governance.json"


def validate(config: dict) -> dict:
    errors = []
    if config.get("contract") != "ZIPRAF-AO42-GOVERNANCE-V1": errors.append("invalid contract")
    if config.get("claim_allowed") is not False: errors.append("claim gate must remain false")
    geometry = config.get("geometry") or {}
    views = geometry.get("content_views") or []
    phases = geometry.get("phases") or []
    if len(views) != 4: errors.append("exactly four content views required")
    expected_edges = len(views) * (len(views) - 1) // 2
    if geometry.get("edge_count") != expected_edges: errors.append("edge count mismatch")
    if len(phases) != 7 or geometry.get("hyperform_count") != expected_edges * len(phases): errors.append("42 hyperform contract mismatch")
    privacy = config.get("privacy") or {}
    if privacy.get("mode") != "POINTER_ONLY_OUTSIDE_AUTHORITY": errors.append("privacy mode mismatch")
    if privacy.get("public_mirror_may_copy_body") is not False: errors.append("public mirror body copy must be false")
    forbidden = set(privacy.get("forbidden_fields") or [])
    required_forbidden = {"payload", "body", "private_content", "secret", "token", "credential", "raw_private_bytes"}
    if not required_forbidden.issubset(forbidden): errors.append("forbidden private fields incomplete")
    cache = config.get("cache_promotion_gate") or {}
    expected_cache = {"operation_id", "input_hash", "parameters_hash", "result_hash", "code_hash", "abi", "version"}
    if set(cache.get("required") or []) != expected_cache: errors.append("cache key contract mismatch")
    if cache.get("invalidate_on_any_mismatch") is not True: errors.append("cache invalidation must be strict")
    if cache.get("crc32c_is_cryptographic") is not False: errors.append("CRC32C cannot be promoted to cryptographic hash")
    append = config.get("append_gate") or {}
    for field in ("prefix_preserved", "latest_zip_directory_readable", "manifest_chain_verified", "rollback_pointer_required"):
        if append.get(field) is not True: errors.append(f"append gate missing: {field}")
    return {"status": "PASS" if not errors else "FAIL", "errors": errors, "hyperform_count": expected_edges * len(phases), "claim_allowed": False}


def main():
    config = json.loads(CONFIG.read_text(encoding="utf-8"))
    report = validate(config)
    print(json.dumps(report, indent=2, sort_keys=True))
    if report["errors"]: raise SystemExit(1)


if __name__ == "__main__": main()
