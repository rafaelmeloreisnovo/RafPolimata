#!/usr/bin/env python3
"""Validate the fail-closed Canari/Maltego integration boundary.

This validator proves only that the RafPolimata integration contract is
structurally coherent. It does not prove that Canari, Maltego, remote
transforms, or a physical Termux runtime executed successfully.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import re
from pathlib import Path
from typing import Any

SCHEMA = "raf.canari-integration.v1"
DEFAULT_MANIFEST = "configs/canari-integration.v1.json"
IMPORT_CANARI = re.compile(r"(?m)^\s*(?:from\s+canari(?:\.|\s)|import\s+canari(?:\.|\s|$))")
SKIP_DIRS = {".git", ".venv", "venv", "__pycache__", "tests", "generated", "results"}
VENDOR_CANDIDATES = (
    "canari",
    "vendor/canari",
    "third_party/canari",
    "src/canari",
)


def _check(checks: list[dict[str, Any]], check_id: str, ok: bool, detail: str) -> None:
    checks.append({"id": check_id, "status": "PASS" if ok else "FAIL", "detail": detail})


def _load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def _source_date() -> str:
    raw = os.environ.get("SOURCE_DATE_EPOCH")
    if raw:
        try:
            value = dt.datetime.fromtimestamp(int(raw), tz=dt.timezone.utc)
            return value.isoformat().replace("+00:00", "Z")
        except (ValueError, OverflowError, OSError):
            pass
    return dt.datetime.now(dt.timezone.utc).isoformat().replace("+00:00", "Z")


def _active_canari_imports(root: Path) -> list[str]:
    leaks: list[str] = []
    for path in root.rglob("*.py"):
        try:
            rel = path.relative_to(root)
        except ValueError:
            continue
        if any(part in SKIP_DIRS for part in rel.parts):
            continue
        if rel.as_posix() == "scripts/validate_canari_integration.py":
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        if IMPORT_CANARI.search(text):
            leaks.append(rel.as_posix())
    return sorted(leaks)


def validate(root: Path, manifest_path: Path) -> dict[str, Any]:
    checks: list[dict[str, Any]] = []

    if not manifest_path.is_absolute():
        manifest_path = root / manifest_path

    if not manifest_path.exists():
        _check(checks, "manifest.exists", False, str(manifest_path))
        return _receipt(checks, None)

    try:
        manifest = _load_json(manifest_path)
    except (OSError, json.JSONDecodeError) as exc:
        _check(checks, "manifest.parse", False, f"{type(exc).__name__}: {exc}")
        return _receipt(checks, None)

    _check(checks, "manifest.parse", True, manifest_path.relative_to(root).as_posix())
    _check(checks, "manifest.schema", manifest.get("schema") == SCHEMA, str(manifest.get("schema")))
    _check(checks, "manifest.mode", manifest.get("mode") == "external-isolated", str(manifest.get("mode")))

    upstream = manifest.get("upstream") or {}
    _check(
        checks,
        "upstream.repository",
        upstream.get("repository") == "malleum-inc/canari3",
        str(upstream.get("repository")),
    )
    _check(checks, "upstream.license", upstream.get("license") == "GPL-3.0", str(upstream.get("license")))

    compat = manifest.get("compatibility") or {}
    _check(
        checks,
        "compatibility.isolation",
        compat.get("isolated_environment_required") is True,
        str(compat.get("isolated_environment_required")),
    )
    _check(
        checks,
        "compatibility.network_default",
        compat.get("network_execution_default") == "disabled",
        str(compat.get("network_execution_default")),
    )

    legal = manifest.get("legal") or {}
    _check(checks, "legal.vendoring", legal.get("vendoring_allowed") is False, str(legal.get("vendoring_allowed")))

    license_record = root / "docs" / "LICENSE_DECISION_RECORD.md"
    license_text = ""
    if license_record.exists():
        try:
            license_text = license_record.read_text(encoding="utf-8")
        except OSError:
            license_text = ""
    _check(
        checks,
        "legal.rafpolimata_license_gate",
        license_record.exists() and "TOKEN_VAZIO_OWNER_DECISION" in license_text,
        "docs/LICENSE_DECISION_RECORD.md",
    )

    vendored = [candidate for candidate in VENDOR_CANDIDATES if (root / candidate).exists()]
    _check(
        checks,
        "boundary.no_vendored_canari",
        not vendored,
        "none" if not vendored else ", ".join(vendored),
    )

    leaks = _active_canari_imports(root)
    _check(
        checks,
        "boundary.no_core_import",
        not leaks,
        "none" if not leaks else ", ".join(leaks),
    )

    claims = manifest.get("claims") or {}
    evidence = manifest.get("evidence") or {}
    claim_allowed = claims.get("claim_allowed") is True
    source_pin = upstream.get("source_pin")
    legal_review = legal.get("compatibility_review")

    if claim_allowed:
        promotion_ok = (
            source_pin not in (None, "", "TOKEN_VAZIO")
            and legal_review not in (None, "", "TOKEN_VAZIO")
            and evidence.get("runtime_verified") is True
            and evidence.get("maltego_profile_verified") is True
        )
        promotion_detail = "claim requested; closure evidence required"
    else:
        promotion_ok = True
        promotion_detail = "claim_allowed=false; runtime remains independently gated"
    _check(checks, "claims.promotion_gate", promotion_ok, promotion_detail)

    runtime_state = {
        "runtime_verified": evidence.get("runtime_verified", "TOKEN_VAZIO"),
        "maltego_profile_verified": evidence.get("maltego_profile_verified", "TOKEN_VAZIO"),
        "termux_verified": evidence.get("termux_verified", "TOKEN_VAZIO"),
    }
    return _receipt(checks, runtime_state, claim_allowed=claim_allowed)


def _receipt(
    checks: list[dict[str, Any]],
    runtime_state: dict[str, Any] | None,
    *,
    claim_allowed: bool = False,
) -> dict[str, Any]:
    failed = [item["id"] for item in checks if item["status"] != "PASS"]
    return {
        "schema": "raf.canari-integration-receipt.v1",
        "generated_at": _source_date(),
        "contract_status": "PASS" if not failed else "FAIL",
        "runtime_state": runtime_state
        or {
            "runtime_verified": "TOKEN_VAZIO",
            "maltego_profile_verified": "TOKEN_VAZIO",
            "termux_verified": "TOKEN_VAZIO",
        },
        "claim_allowed": claim_allowed if not failed else False,
        "checks": checks,
        "failed_checks": failed,
        "interpretation": "Structural integration contract only; PASS is not Canari/Maltego runtime proof.",
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".")
    parser.add_argument("--manifest", default=DEFAULT_MANIFEST)
    parser.add_argument("--output")
    args = parser.parse_args(argv)

    root = Path(args.root).resolve()
    receipt = validate(root, Path(args.manifest))
    payload = json.dumps(receipt, ensure_ascii=False, indent=2, sort_keys=True)
    print(payload)

    if args.output:
        output = Path(args.output)
        if not output.is_absolute():
            output = root / output
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(payload + "\n", encoding="utf-8")

    return 0 if receipt["contract_status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
