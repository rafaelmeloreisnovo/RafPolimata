#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import importlib.util
import struct
import sys
import tempfile
import unittest
import zlib
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "validate_apkc_formats", ROOT / "scripts/validate_apkc_formats.py"
)
assert SPEC and SPEC.loader
FMT = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = FMT
SPEC.loader.exec_module(FMT)


def make_dex() -> bytes:
    buf = bytearray(140)
    buf[0:8] = b"dex\n035\0"
    struct.pack_into("<I", buf, 32, len(buf))
    struct.pack_into("<I", buf, 36, 0x70)
    struct.pack_into("<I", buf, 40, 0x12345678)
    struct.pack_into("<I", buf, 52, 112)
    struct.pack_into("<I", buf, 104, 28)
    struct.pack_into("<I", buf, 108, 112)
    struct.pack_into("<I", buf, 112, 2)
    struct.pack_into("<HHII", buf, 116, 0x0000, 0, 1, 0)
    struct.pack_into("<HHII", buf, 128, 0x1000, 0, 1, 112)
    buf[12:32] = hashlib.sha1(buf[32:]).digest()
    struct.pack_into("<I", buf, 8, zlib.adler32(buf[12:]) & 0xFFFFFFFF)
    return bytes(buf)


def make_elf64(machine: int = 183) -> bytes:
    size = 64 + 56
    buf = bytearray(size)
    buf[:16] = b"\x7fELF\x02\x01\x01" + b"\0" * 9
    struct.pack_into("<HHI", buf, 16, 3, machine, 1)
    struct.pack_into("<Q", buf, 24, 0)
    struct.pack_into("<Q", buf, 32, 64)
    struct.pack_into("<Q", buf, 40, 0)
    struct.pack_into("<I", buf, 48, 0)
    struct.pack_into("<HHHHHH", buf, 52, 64, 56, 1, 64, 0, 0)
    struct.pack_into("<IIQQQQQQ", buf, 64, 1, 5, 0, 0, 0, size, size, 4096)
    return bytes(buf)


def make_elf32(machine: int = 40) -> bytes:
    size = 52 + 32
    buf = bytearray(size)
    buf[:16] = b"\x7fELF\x01\x01\x01" + b"\0" * 9
    struct.pack_into("<HHI", buf, 16, 3, machine, 1)
    struct.pack_into("<I", buf, 24, 0)
    struct.pack_into("<I", buf, 28, 52)
    struct.pack_into("<I", buf, 32, 0)
    struct.pack_into("<I", buf, 36, 0)
    struct.pack_into("<HHHHHH", buf, 40, 52, 32, 1, 40, 0, 0)
    struct.pack_into("<IIIIIIII", buf, 52, 1, 0, 0, 0, size, size, 5, 4096)
    return bytes(buf)


class ApkCFormatValidationTests(unittest.TestCase):
    def test_valid_minimal_dex(self) -> None:
        result = FMT.validate_dex_bytes(make_dex())
        self.assertEqual(result.state, "PASS", result.errors)
        self.assertEqual(result.metadata["version"], "035")
        self.assertEqual(result.metadata["map_count"], 2)

    def test_dex_checksum_corruption_fails(self) -> None:
        corrupted = bytearray(make_dex())
        corrupted[-1] ^= 0x01
        result = FMT.validate_dex_bytes(bytes(corrupted))
        self.assertEqual(result.state, "FAIL")
        self.assertTrue(any("SHA-1" in item or "Adler" in item for item in result.errors))

    def test_valid_arm_and_aarch64_shared_objects(self) -> None:
        a64 = FMT.validate_elf_bytes(make_elf64(), expected_machine=183)
        arm = FMT.validate_elf_bytes(make_elf32(), expected_machine=40)
        self.assertEqual(a64.state, "PASS", a64.errors)
        self.assertEqual(arm.state, "PASS", arm.errors)
        self.assertEqual(a64.metadata["pt_load_count"], 1)
        self.assertEqual(arm.metadata["pt_load_count"], 1)

    def test_wrong_abi_machine_fails(self) -> None:
        result = FMT.validate_elf_bytes(make_elf64(machine=40), expected_machine=183)
        self.assertEqual(result.state, "FAIL")
        self.assertTrue(any("e_machine" in item for item in result.errors))

    def test_apk_with_both_abis_and_dex_passes(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            apk = Path(td) / "fixture.apk"
            with zipfile.ZipFile(apk, "w", compression=zipfile.ZIP_STORED) as zf:
                zf.writestr("classes.dex", make_dex())
                zf.writestr("lib/arm64-v8a/libmain.so", make_elf64())
                zf.writestr("lib/armeabi-v7a/libmain.so", make_elf32())
            report = FMT.validate_apk(apk, require_both=True)
            self.assertEqual(report["state"], "PASS", report["errors"])
            self.assertEqual(report["abis"], ["arm64-v8a", "armeabi-v7a"])

    def test_apk_missing_one_abi_fails_require_both(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            apk = Path(td) / "fixture.apk"
            with zipfile.ZipFile(apk, "w", compression=zipfile.ZIP_STORED) as zf:
                zf.writestr("classes.dex", make_dex())
                zf.writestr("lib/arm64-v8a/libmain.so", make_elf64())
            report = FMT.validate_apk(apk, require_both=True)
            self.assertEqual(report["state"], "FAIL")
            self.assertTrue(any("ABIs ausentes" in item for item in report["errors"]))


if __name__ == "__main__":
    unittest.main()
