#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "apkc_runtime_preflight", ROOT / "scripts/apkc_runtime_preflight.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)


class ApkCRuntimePreflightTests(unittest.TestCase):
    def test_detects_known_external_pipeline_blockers(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "Apkc").mkdir()
            (root / "Apkc/apkc.c").write_text(
                r'''
static unsigned char _dex_buf[200];
static int fork_exec_wait(void) {
  const char *out = "/tmp/apkc_out.so";
  while (total < outbuf_cap) os_read(fd, outbuf + total, outbuf_cap - total);
  return total;
}
static int arm32(void) { pr_err("fork_exec_wait: not supported on ARM32\n"); }
void pack(void) {
  m_cpy(_so64_buf, _fork_out, outsz < sizeof(_so64_buf) ? outsz : sizeof(_so64_buf));
  m_cpy(_dex_buf, _fork_out, outsz < sizeof(_dex_buf) ? outsz : sizeof(_dex_buf));
}
''',
                encoding="utf-8",
            )
            (root / "Apkc/lang_profile.h").write_text(
                'Unknown input is rejected\n"aarch64-linux-android"\n"kotlinc"\n"d8"\n',
                encoding="utf-8",
            )
            result = MOD.inspect_source(root)
            ids = {item["id"] for item in result["blockers"]}
            self.assertTrue({
                "APKC-RUN-001", "APKC-RUN-002", "APKC-RUN-003",
                "APKC-RUN-004", "APKC-RUN-005", "APKC-RUN-006",
                "APKC-RUN-007", "APKC-RUN-008", "APKC-RUN-009",
                "APKC-RUN-010",
            }.issubset(ids))

    def test_clean_source_contract_has_no_blockers(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "Apkc").mkdir()
            (root / "Apkc/apkc.c").write_text(
                r'''
os_unlink(outfile);
if (os_waitpid(pid, &status, 0) < 0) return 0;
if (output exceeds outbuf_cap) return 0;
validate_apkc_formats();
''',
                encoding="utf-8",
            )
            (root / "Apkc/lang_profile.h").write_text(
                'Unknown input is rejected\n'
                '"aarch64-linux-android21"\n'
                'GOOS GOARCH CGO_ENABLED\n'
                '"kotlinc" "d8"\n',
                encoding="utf-8",
            )
            result = MOD.inspect_source(root)
            self.assertEqual(result["blockers"], [])

    def test_writable_directory_probe(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            ok, reason = MOD.writable_directory(Path(td))
            self.assertTrue(ok)
            self.assertEqual(reason, "writable")


if __name__ == "__main__":
    unittest.main()
