#!/usr/bin/env python3
"""Compatibility repair for runtime-source transform C string escapes.

The canonical transformer intentionally keeps exact anchors. Three replacement
payloads historically used ordinary Python triple-quoted strings, so `\n` inside
C string literals became an actual newline in generated C. This adapter changes
only those replacement payload bytes before delegating to the canonical main().
It does not weaken replace_once or change the 13 transform IDs.
"""
from __future__ import annotations

import patch_apkc_runtime_source as base

_FIXES = {
    "APKC-RH-008": [
        ("validated DEX output\n\");", "validated DEX output\\n\");"),
    ],
    "APKC-RH-009": [
        ("AArch64 ET_DYN ELF\n\");", "AArch64 ET_DYN ELF\\n\");"),
    ],
    "APKC-RH-013": [
        ("source read failed\n\");", "source read failed\\n\");"),
        ("source exceeds 1MiB bounded input\n\");", "source exceeds 1MiB bounded input\\n\");"),
        ("source close failed\n\");", "source close failed\\n\");"),
    ],
}

patched = []
for change, old, new in base.TRANSFORMS:
    if change.change_id in _FIXES:
        for before, after in _FIXES[change.change_id]:
            if new.count(before) != 1:
                raise SystemExit(f"{change.change_id}: C escape repair anchor count != 1")
            new = new.replace(before, after, 1)
    patched.append((change, old, new))
base.TRANSFORMS = patched

if __name__ == "__main__":
    raise SystemExit(base.main())
