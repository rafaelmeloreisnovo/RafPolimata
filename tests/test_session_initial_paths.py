from __future__ import annotations

import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "validate_session_initial_paths.py"
SPEC = importlib.util.spec_from_file_location("validate_session_initial_paths", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def test_canonical_manifest_passes() -> None:
    report = MODULE.validate(ROOT, ROOT / "manifests/session-initial-paths.v1.json")
    assert report["status"] == "PASS"
    assert report["checks_failed"] == 0
    assert report["claim_allowed"] is False


def test_runtime_cannot_be_promoted_without_evidence(tmp_path: Path) -> None:
    source = json.loads((ROOT / "manifests/session-initial-paths.v1.json").read_text(encoding="utf-8"))
    source["paths"][1]["evidence"]["termux_runtime"] = "VERIFIED"
    mutated = tmp_path / "mutated.json"
    mutated.write_text(json.dumps(source), encoding="utf-8")
    report = MODULE.validate(ROOT, mutated)
    assert report["status"] == "FAIL"
    failed_names = {item["name"] for item in report["checks"] if not item["pass"]}
    assert "termux_runtime_not_promoted" in failed_names


def test_path_order_is_canonical(tmp_path: Path) -> None:
    source = json.loads((ROOT / "manifests/session-initial-paths.v1.json").read_text(encoding="utf-8"))
    source["paths"][0], source["paths"][1] = source["paths"][1], source["paths"][0]
    mutated = tmp_path / "reordered.json"
    mutated.write_text(json.dumps(source), encoding="utf-8")
    report = MODULE.validate(ROOT, mutated)
    assert report["status"] == "FAIL"
