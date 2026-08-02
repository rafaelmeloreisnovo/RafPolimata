#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GEN = ROOT / "scripts" / "bitraf_matrix_manifest.py"
C_TEST = ROOT / "tests" / "bitraf_matrix_test.c"
TIMELINE = ROOT / "data" / "encoding_timeline.v1.json"


def load_generator():
    spec = importlib.util.spec_from_file_location("bitraf_matrix_manifest", GEN)
    assert spec and spec.loader
    mod = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = mod
    spec.loader.exec_module(mod)
    return mod


class BitrafMatrixTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.mod = load_generator()

    def test_exhaustive_roundtrip_and_partition(self):
        core = 0
        shell = 0
        seen = set()
        for i in range(8000):
            s = self.mod.decode(i)
            self.assertEqual(self.mod.encode(s), i)
            code = self.mod.base20_3(i)
            self.assertEqual(len(code), 3)
            self.assertNotIn(code, seen)
            seen.add(code)
            o = self.mod.opposite(s)
            self.assertEqual(self.mod.opposite(o), s)
            if self.mod.is_core(s):
                core += 1
                ci = self.mod.core_index(s)
                self.assertIsNotNone(ci)
                self.assertTrue(0 <= ci < 4096)
            else:
                shell += 1
                self.assertIsNone(self.mod.core_index(s))
        self.assertEqual((core, shell, len(seen)), (4096, 3904, 8000))

    def test_manifest_is_deterministic(self):
        with tempfile.TemporaryDirectory() as td:
            out = Path(td) / "out"
            cmd = [sys.executable, str(GEN), "--out", str(out)]
            first = subprocess.run(cmd, check=True, text=True, capture_output=True)
            content1 = (out / "manifest.json").read_bytes()
            second = subprocess.run(cmd, check=True, text=True, capture_output=True)
            content2 = (out / "manifest.json").read_bytes()
            self.assertEqual(first.stdout, second.stdout)
            self.assertEqual(content1, content2)
            payload = json.loads(content1)
            self.assertEqual(payload["counts"], {"core": 4096, "shell": 3904, "total": 8000})
            self.assertFalse(payload["claim_allowed"])

    def test_encoding_timeline_contract(self):
        data = json.loads(TIMELINE.read_text(encoding="utf-8"))
        self.assertEqual(data["schema_version"], 1)
        ids = [row["id"] for row in data["encodings"]]
        self.assertEqual(len(ids), len(set(ids)))
        for required in ("ITA2", "ASCII", "ECMA-6", "EBCDIC-037", "UNICODE", "UTF-8", "UTF-16", "UTF-32"):
            self.assertIn(required, ids)
        for row in data["encodings"]:
            self.assertIn(
                row["epistemic_state"],
                {"VERIFIED_PRIMARY_SOURCE", "FAMILY_NO_SINGLE_LATEST_VERSION", "TOKEN_VAZIO_FIRST_EDITION"},
            )
            self.assertTrue(row["sources"])

    def test_c_core_with_available_compilers(self):
        compilers = [c for c in ("clang", "gcc", "cc") if shutil.which(c)]
        if not compilers:
            self.skipTest("no C compiler available")
        outputs = []
        with tempfile.TemporaryDirectory() as td:
            for compiler in compilers[:2]:
                binary = Path(td) / f"bitraf-{compiler}"
                subprocess.run(
                    [compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", str(C_TEST), "-o", str(binary)],
                    check=True,
                    cwd=ROOT,
                )
                run = subprocess.run([str(binary)], check=True, text=True, capture_output=True)
                outputs.append(run.stdout)
                self.assertIn("BITRAF_MATRIX_V1=PASS", run.stdout)
                self.assertIn("CORE=4096 SHELL=3904", run.stdout)
        self.assertTrue(all(out == outputs[0] for out in outputs))


if __name__ == "__main__":
    unittest.main()
