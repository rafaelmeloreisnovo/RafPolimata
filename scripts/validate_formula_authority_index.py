#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "configs/formula-authority-index.v1.json"


def main() -> int:
    data = json.loads(PATH.read_text(encoding="utf-8"))
    assert data["schema"] == "RAFPOLIMATA_FORMULA_AUTHORITY_INDEX_V1"
    assert data["claim_allowed"] is False
    entries = data["entries"]
    ids = [entry["id"] for entry in entries]
    assert len(ids) == len(set(ids))
    for entry in entries:
        for key in ("id","semantics","semantic_authority","local_role","producer_refs","gap"):
            assert key in entry, f"{entry.get('id')}: missing {key}"
        assert entry["semantic_authority"], f"{entry['id']}: use typed TOKEN_VAZIO"
        assert entry["producer_refs"], f"{entry['id']}: producer_refs empty"
    print(json.dumps({
        "state": "PASS",
        "schema": data["schema"],
        "entries": len(entries),
        "claim_allowed": False
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
