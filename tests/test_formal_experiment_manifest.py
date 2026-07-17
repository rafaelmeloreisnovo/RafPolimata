from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "tools" / "validate_formal_experiment_manifest.py"
spec = importlib.util.spec_from_file_location("formal_manifest", VALIDATOR)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def fixture() -> dict:
    return json.loads(
        (ROOT / "examples" / "rll_formal_experiment_manifest.v1.json").read_text(encoding="utf-8")
    )


def test_fixture_valid() -> None:
    assert module.validate(fixture()) == []


def test_claim_promotion_rejected() -> None:
    data = fixture()
    data["claim_allowed"] = True
    assert any("claim_allowed" in error for error in module.validate(data))


def test_missing_proof_obligation_rejected() -> None:
    data = fixture()
    data["formalization"]["proof_obligations"] = []
    assert any("proof_obligations" in error for error in module.validate(data))


def test_sensitive_key_rejected() -> None:
    data = fixture()
    data["secret"] = "x"
    assert any("sensitive" in error for error in module.validate(data))


def test_digest_deterministic() -> None:
    assert module.digest(fixture()) == module.digest(copy.deepcopy(fixture()))
