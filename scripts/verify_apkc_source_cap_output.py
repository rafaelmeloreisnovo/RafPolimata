#!/usr/bin/env python3
from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
PATCH_PATH = ROOT / "scripts" / "patch_apkc_source_cap.py"


def load_patch_module():
    spec = importlib.util.spec_from_file_location("patch_apkc_source_cap", PATCH_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot import {PATCH_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("path", type=Path)
    args = parser.parse_args()

    module = load_patch_module()
    text = args.path.read_text(encoding="utf-8")
    errors: list[str] = []

    if module.OLD in text:
        errors.append("legacy source-read anchor remains")
    if text.count(module.NEW) != 1:
        errors.append(f"hardened source-read block count={text.count(module.NEW)} expected=1")
    if text.count("source exceeds SRC_CAP") != 1:
        errors.append(
            f"source overflow guard count={text.count('source exceeds SRC_CAP')} expected=1"
        )
    if "source read failed\\n" not in text:
        errors.append("source read-error guard missing")
    if "source overflow probe failed\\n" not in text:
        errors.append("source overflow-probe error guard missing")

    # This generic pattern may legitimately exist in unrelated bounded-read code.
    # It is observed for diagnostics only and must not be used as a global falsifier
    # for the source-input contract.
    generic_legacy_count = text.count("if (n<=0) break;")

    if errors:
        print(
            "source-cap exact verification: FAIL "
            + "; ".join(errors)
            + f" generic_n_le_zero_count={generic_legacy_count}",
            file=sys.stderr,
        )
        return 1

    print(
        "source-cap exact verification: PASS "
        f"old_anchor=0 new_anchor=1 overflow_guard=1 "
        f"generic_n_le_zero_count={generic_legacy_count} claim_allowed=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
