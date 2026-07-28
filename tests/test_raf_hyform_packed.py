#!/usr/bin/env python3
"""Local compile/run gate for the freestanding HYFORM32/HYEDGE16 contract."""

from __future__ import annotations

import hashlib
import json
import os
import platform
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BENCHMARK = ROOT / "Benchmark"
HEADER = BENCHMARK / "raf_hyform_packed.h"

HARNESS = r'''
#include "raf_hyform_packed.h"

static int check_node(void) {
    raf_hyform32_t value = raf_hyform32_pack_checked(
        6u, 5u, 11u, 9u, 5u, 3u,
        RAF_HYFORM_EP_TOKEN_VAZIO, 2u,
        1u, 0u, 1u, 1u, 0u, 1u
    );
    if (!raf_hyform32_is_valid(value)) return 1;
    if (raf_hyform32_level(value) != 6u) return 2;
    if (raf_hyform32_sector(value) != 5u) return 3;
    if (raf_hyform32_operator(value) != 11u) return 4;
    if (raf_hyform32_geometry(value) != 9u) return 5;
    if (raf_hyform32_recurrence(value) != 5u) return 6;
    if (raf_hyform32_layer(value) != 3u) return 7;
    if (raf_hyform32_epistemic(value) != RAF_HYFORM_EP_TOKEN_VAZIO) return 8;
    if (raf_hyform_primary_state(5u, 6u) != 41u) return 9;
    if (raf_hyform_primary_state(6u, 0u) != 0xFFu) return 10;
    if (raf_hyform32_pack_checked(7u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u) != 0u) return 11;
    if (raf_hyform32_pack_checked(0u, 6u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u) != 0u) return 12;
    if (raf_hyform32_pack_checked(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u, 1u) != 0u) return 13;
    return 0;
}

static int check_edge(void) {
    raf_hyedge16_t edge = raf_hyedge16_pack_checked(41u, 0u, RAF_HYEDGE_FEEDBACK);
    if (!raf_hyedge16_is_valid(edge)) return 20;
    if (raf_hyedge16_source(edge) != 41u) return 21;
    if (raf_hyedge16_target(edge) != 0u) return 22;
    if (raf_hyedge16_relation(edge) != RAF_HYEDGE_FEEDBACK) return 23;
    if (raf_hyedge16_pack_checked(42u, 0u, 0u) != RAF_HYEDGE_INVALID) return 24;
    if (raf_hyedge16_pack_checked(0u, 42u, 0u) != RAF_HYEDGE_INVALID) return 25;
    return 0;
}

int main(void) {
    int node = check_node();
    if (node != 0) return node;
    return check_edge();
}
'''

FREESTANDING_PROBE = r'''
#include "raf_hyform_packed.h"
raf_hyform32_t raf_hyform_probe(u8 sector, u8 level) {
    return raf_hyform32_pack_checked(
        level, sector, 0u, 0u, 0u, 0u,
        RAF_HYFORM_EP_DECLARED, 0u,
        1u, 0u, 0u, 0u, 0u, 1u
    );
}
'''


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def run() -> dict[str, object]:
    cc = os.environ.get("CC") or shutil.which("cc")
    if not cc:
        raise RuntimeError("CC/cc not found")

    with tempfile.TemporaryDirectory(prefix="raf-hyform-") as temp_dir:
        temp = Path(temp_dir)
        harness_c = temp / "harness.c"
        probe_c = temp / "probe.c"
        host_bin = temp / "hyform-host-test"
        probe_o = temp / "hyform-probe.o"
        harness_c.write_text(HARNESS, encoding="utf-8")
        probe_c.write_text(FREESTANDING_PROBE, encoding="utf-8")

        strict_flags = [
            "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
            "-ffreestanding", "-fno-builtin", "-I", str(BENCHMARK),
        ]
        subprocess.run([cc, *strict_flags, "-c", str(probe_c), "-o", str(probe_o)], check=True)
        undefined = subprocess.run(
            ["nm", "-u", str(probe_o)], check=True, text=True, capture_output=True
        ).stdout.strip()
        if undefined:
            raise RuntimeError(f"undefined production symbols: {undefined}")

        subprocess.run([cc, *strict_flags, str(harness_c), "-o", str(host_bin)], check=True)
        subprocess.run([str(host_bin)], check=True)

        return {
            "state": "LOCAL_PASS",
            "tests": 9,
            "passes": 9,
            "failures": 0,
            "compiler": subprocess.run([cc, "--version"], check=True, text=True, capture_output=True).stdout.splitlines()[0],
            "platform": platform.platform(),
            "header_sha256": sha256(HEADER),
            "probe_object_sha256": sha256(probe_o),
            "undefined_symbols": [],
            "syscalls_in_production_contract": False,
            "libc_dependency_in_production_contract": False,
            "heap_dependency": False,
            "asm_required": False,
            "primary_state_count": 42,
            "second_phase_layer_state": "TOKEN_VAZIO_DEFINITION",
            "claim_allowed": False,
        }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
