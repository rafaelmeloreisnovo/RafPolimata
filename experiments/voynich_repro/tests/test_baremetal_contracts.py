from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
CROOT = ROOT / "c_baremetal"


def run(args: list[str], cwd: Path, check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=check)


def require_tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} unavailable")
    return path


def build_x86(tmp_path: Path) -> tuple[Path, Path]:
    gcc = require_tool("gcc")
    common = [gcc, "-O2", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-Wconversion", "-Wshadow", "-nostdlib", "-ffreestanding", "-fno-builtin", "-static"]
    angular = tmp_path / "voy_angular"
    core = tmp_path / "voy_core"
    run(common + ["-o", str(angular), str(CROOT / "start_angular_x86_64.s"), str(CROOT / "sys_x86_64_fixed.s"), str(CROOT / "voynich_angular_v5.c")], ROOT)
    run(common + ["-o", str(core), str(CROOT / "start_core_x86_64.s"), str(CROOT / "sys_x86_64_fixed.s"), str(CROOT / "voy_core_v2.c")], ROOT)
    return angular, core


def test_x86_openat_uses_r10() -> None:
    text = (CROOT / "sys_x86_64_fixed.s").read_text()
    assert "mov %rcx, %r10" in text


def test_angular_claim_gate_is_conservative() -> None:
    text = (CROOT / "voynich_angular_v5.c").read_text()
    assert "CHI2_005_DF7_X100 1407u" in text
    assert "MIN_CHI2_N 40u" in text
    assert "DIRECTIONALITY_DETECTED_NOT_HYPERLINK_PROOF" in text
    assert "claim_allowed=false" in text


def test_core_uses_exact_token_bytes_and_invalid_sentinel() -> None:
    text = (CROOT / "voy_core_v2.c").read_text()
    assert "token_bytes_equal" in text
    assert "INVALID_INDEX" in text
    assert "TOKENS_FIRST_256" in text


def test_x86_builds_without_warnings(tmp_path: Path) -> None:
    angular, core = build_x86(tmp_path)
    assert angular.exists() and core.exists()


def test_angular_kernel_vectors_and_small_sample_gate(tmp_path: Path) -> None:
    angular, _ = build_x86(tmp_path)
    out = run([str(angular), "--test"], ROOT).stdout
    assert [int(v) for v in re.findall(r"angle=(\d+)deg", out)] == [21, 54, 90, 130]
    assert "GATE=TOKEN_VAZIO_SAMPLE_TOO_SMALL" in out
    assert "claim_allowed=false" in out


def test_barecore_sample_is_deterministic_and_mode_0644(tmp_path: Path) -> None:
    _, core = build_x86(tmp_path)
    sample = tmp_path / "sample.txt"
    sample.write_text("qokeedy qokeedy qokedy chol chor shol shor\nqokain qokar qokain qokal daiin daiin ol\n")
    p1 = tmp_path / "out1"
    p2 = tmp_path / "out2"
    run([str(core), str(sample), str(p1), "0x3F"], ROOT)
    run([str(core), str(sample), str(p2), "0x3F"], ROOT)
    assert p1.with_suffix(".txt").read_bytes() == p2.with_suffix(".txt").read_bytes()
    assert p1.with_suffix(".pgm").read_bytes() == p2.with_suffix(".pgm").read_bytes()
    assert os.stat(p1.with_suffix(".txt")).st_mode & 0o777 == 0o644
    assert os.stat(p1.with_suffix(".pgm")).st_mode & 0o777 == 0o644


def test_exact_1m_is_not_marked_truncated_but_over_1m_is(tmp_path: Path) -> None:
    _, core = build_x86(tmp_path)
    exact = tmp_path / "exact.bin"
    over = tmp_path / "over.bin"
    exact.write_bytes(b"a" * (1024 * 1024))
    over.write_bytes(b"a" * (1024 * 1024 + 1))
    run([str(core), str(exact), str(tmp_path / "exact_out"), "0x30"], ROOT)
    run([str(core), str(over), str(tmp_path / "over_out"), "0x30"], ROOT)
    exact_report = (tmp_path / "exact_out.txt").read_text()
    over_report = (tmp_path / "over_out.txt").read_text()
    assert "lacunas=0" in exact_report
    assert "lacunas=1" in over_report
    assert "0x00001A11" in over_report


def test_aarch64_cross_builds_when_clang_and_lld_exist(tmp_path: Path) -> None:
    clang = require_tool("clang")
    require_tool("ld.lld")
    common = [clang, "--target=aarch64-linux-gnu", "-O2", "-Wall", "-Wextra", "-Werror", "-Wpedantic", "-Wconversion", "-Wshadow", "-nostdlib", "-ffreestanding", "-fno-builtin", "-static", "-fuse-ld=lld", "-Wl,-e,_start"]
    angular = tmp_path / "voy_angular_aarch64"
    core = tmp_path / "voy_core_aarch64"
    run(common + ["-o", str(angular), str(CROOT / "start_angular_aarch64.s"), str(CROOT / "sys_aarch64_fixed.s"), str(CROOT / "voynich_angular_v5.c")], ROOT)
    run(common + ["-o", str(core), str(CROOT / "start_core_aarch64.s"), str(CROOT / "sys_aarch64_fixed.s"), str(CROOT / "voy_core_v2.c")], ROOT)
    assert angular.exists() and core.exists()
