#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path

from voynich_repro.dataset_audit import audit_dataset_zip


def main() -> int:
    parser = argparse.ArgumentParser(description="Audit a local Voynich dataset ZIP without publishing corpus bytes")
    parser.add_argument("zip_path", type=Path)
    parser.add_argument("--event-id", default="VOYNICH-DATASET-AUDIT-LOCAL")
    parser.add_argument("--timestamp-local", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    report = audit_dataset_zip(args.zip_path, args.event_id, args.timestamp_local)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
