#!/usr/bin/env python3
"""Compatibility entrypoint for the APKC–RMR coupled build verifier.

SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
Contract-Role: VERIFIER_COMPATIBILITY_ENTRYPOINT
License-Role: RESEARCH_NONCOMMERCIAL_ONLY
Normative comment: this entrypoint must preserve the strict coupled gate.
"""
from __future__ import annotations

import json
import sys

from coupled_build import verify


def main() -> int:
    receipt = verify()
    print(json.dumps(receipt, indent=2, sort_keys=True))
    return 0 if receipt["state"] == "PASS_COUPLED" else 1


if __name__ == "__main__":
    sys.exit(main())
