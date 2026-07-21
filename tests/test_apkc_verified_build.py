#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


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


def make_elf64() -> bytes:
    size = 64 + 56
    buf = bytearray(size)
    buf[:16] = b"\x7fELF\x02\x01\x01" + b"\0" * 9
    struct.pack_into("<HHI", buf, 16, 3, 183, 1)
    struct.pack_into("<Q", buf, 24, 0)
    struct.pack_into("<Q", buf, 32, 64)
    struct.pack_into("<Q", buf, 40, 0)
    struct.pack_into("<I", buf, 48, 0)
    struct.pack_into("<HHHHHH", buf, 52, 64, 56, 1, 64, 0, 0)
    struct.pack_into("<IIQQQQQQ", buf, 64, 1, 5, 0, 0, 0, size, size, 4096)
    return bytes(buf)


def make_elf32() -> bytes:
    size = 52 + 32
    buf = bytearray(size)
    buf[:16] = b"\x7fELF\x01\x01\x01" + b"\0" * 9
    struct.pack_into("<HHI", buf, 16, 3, 40, 1)
    struct.pack_into("<I", buf, 24, 0)
    struct.pack_into("<I", buf, 28, 52)
    struct.pack_into("<I", buf, 32, 0)
    struct.pack_into("<I", buf, 36, 0)
    struct.pack_into("<HHHHHH", buf, 40, 52, 32, 1, 40, 0, 0)
    struct.pack_into("<IIIIIIII", buf, 52, 1, 0, 0, 0, size, size, 5, 4096)
    return bytes(buf)


def make_valid_apk(path: Path) -> None:
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_STORED) as zf:
        zf.writestr("classes.dex", make_dex())
        zf.writestr("lib/arm64-v8a/libmain.so", make_elf64())
        zf.writestr("lib/armeabi-v7a/libmain.so", make_elf32())


class ApkCVerifiedBuildTests(unittest.TestCase):
    def make_fake_apkc(self, root: Path, valid: bool) -> Path:
        script = root / "fake-apkc.py"
        script.write_text(
            """#!/usr/bin/env python3
import os
import pathlib
import shutil
import sys
args = sys.argv[1:]
out = pathlib.Path(args[args.index('-o') + 1])
if os.environ.get('FAKE_VALID') == '1':
    shutil.copyfile(os.environ['FIXTURE_APK'], out)
else:
    out.write_bytes(b'not-an-apk')
""",
            encoding="utf-8",
        )
        script.chmod(0o755)
        return script

    def run_wrapper(self, td: Path, valid: bool) -> tuple[subprocess.CompletedProcess[str], Path, Path]:
        fixture = td / "fixture.apk"
        make_valid_apk(fixture)
        fake = self.make_fake_apkc(td, valid)
        source = td / "input.c"
        source.write_text("int main(void){return 0;}\n", encoding="utf-8")
        output = td / "final.apk"
        report = td / "report.json"
        env = os.environ.copy()
        env["FIXTURE_APK"] = str(fixture)
        env["FAKE_VALID"] = "1" if valid else "0"
        proc = subprocess.run(
            [
                sys.executable,
                str(ROOT / "scripts/apkc_verified_build.py"),
                "--apkc", str(fake),
                "--input", str(source),
                "--output", str(output),
                "--report", str(report),
                "--require-both",
            ],
            cwd=ROOT,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
        return proc, output, report

    def test_valid_candidate_is_promoted(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            proc, output, report = self.run_wrapper(Path(raw), True)
            self.assertEqual(proc.returncode, 0, proc.stdout)
            self.assertTrue(output.is_file())
            payload = json.loads(report.read_text(encoding="utf-8"))
            self.assertEqual(payload["state"], "PASS")
            self.assertTrue(payload["claim_allowed"])
            self.assertEqual(payload["validation"]["state"], "PASS")

    def test_invalid_candidate_is_never_promoted(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            proc, output, report = self.run_wrapper(Path(raw), False)
            self.assertNotEqual(proc.returncode, 0)
            self.assertFalse(output.exists())
            payload = json.loads(report.read_text(encoding="utf-8"))
            self.assertEqual(payload["state"], "FAIL")
            self.assertFalse(payload["claim_allowed"])


if __name__ == "__main__":
    unittest.main()
