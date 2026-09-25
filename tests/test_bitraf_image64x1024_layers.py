#!/usr/bin/env python3
from __future__ import annotations
import importlib.util
import hashlib
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "bitraf_image64x1024_layers.py"
spec = importlib.util.spec_from_file_location("bitraf_image", SCRIPT)
mod = importlib.util.module_from_spec(spec)
assert spec.loader
spec.loader.exec_module(mod)

class BitrafImageLayersTest(unittest.TestCase):
    def make_ppm(self, path: Path, w=32, h=32):
        header = f"P6\n{w} {h}\n255\n".encode()
        data = bytearray()
        for y in range(h):
            for x in range(w):
                if x == y or x + y == w - 1:
                    rgb = (255,255,255)
                else:
                    rgb = ((x*255)//(w-1),(y*255)//(h-1),((x+y)*255)//(2*(w-1)))
                data.extend(rgb)
        path.write_bytes(header + data)

    def test_base20_and_carrier(self):
        self.assertEqual(mod.to_base20(0), "0")
        self.assertEqual(64*1024, 65536)

    def test_canonical_sequences(self):
        self.assertEqual(mod.rafael_values(300)[:10], [2,4,7,12,20,33,54,88,143,232])
        self.assertEqual(mod.tribonacci_values(100)[:9], [0,1,2,4,7,13,24,44,81])
        self.assertIn(13, mod.fib_values(100))

    def test_ppm_report_and_conservation(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "x.ppm"
            self.make_ppm(p)
            report, matrix = mod.analyze(p, max_dim=32)
            self.assertEqual(report["carrier"]["lanes"], 64)
            self.assertEqual(report["carrier"]["states_per_lane"], 1024)
            self.assertEqual(len(matrix),64)
            self.assertTrue(all(len(row)==1024 for row in matrix))
            self.assertEqual(sum(sum(row) for row in matrix),32*32)
            self.assertEqual(report["source"]["sha256"],hashlib.sha256(p.read_bytes()).hexdigest())
            self.assertFalse(report["claim_allowed"])
            self.assertEqual(len(report["orientation_energy"]),8)
            self.assertEqual(len(report["lane_entropy"]),64)

    def test_binary_size(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "x.ppm"
            b = Path(td) / "m.bin"
            self.make_ppm(p,16,16)
            _report,matrix = mod.analyze(p,max_dim=16)
            mod.emit_matrix_bin(b,matrix)
            self.assertEqual(b.stat().st_size,64*1024*4)

if __name__ == "__main__":
    unittest.main()
