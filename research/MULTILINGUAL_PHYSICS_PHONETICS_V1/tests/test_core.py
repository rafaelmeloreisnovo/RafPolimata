from __future__ import annotations
import importlib.util, json, math, unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def loadmod(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    module=importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    spec.loader.exec_module(module)
    return module

A=loadmod("analyze",ROOT/"core/analyze.py")
G=loadmod("geom",ROOT/"core/acoustic_geometry.py")

class TestMPP(unittest.TestCase):
    def test_delta(self):
        d=A.delta_42_58()
        self.assertEqual(d["percentage_points"],16.0)
        self.assertEqual(d["midpoint"],0.5)
        self.assertEqual(d["half_delta"],0.08)

    def test_semantic_coverage(self):
        fixture=json.loads((ROOT/"fixtures/e_mc2.v1.json").read_text())
        report=A.analyze(fixture)
        self.assertTrue(all(x["semantic_coverage"]==1.0 for x in report["rows"]))

    def test_triangle(self):
        t=G.triangle()
        self.assertAlmostEqual(t["pythagoras_check"],1.0,places=12)

    def test_pg_not_harmonic(self):
        pg=G.pg_frequencies(440,3)
        harmonics=G.harmonic_frequencies(440,3)
        self.assertNotEqual(pg,harmonics)
        self.assertAlmostEqual(pg[1]/pg[0],math.sqrt(3)/2,places=12)

    def test_venturi(self):
        v=G.venturi(0.01,0.005,2.0)
        self.assertAlmostEqual(v["v2"],4.0)
        self.assertGreater(v["ideal_static_pressure_drop_P1_minus_P2"],0)

if __name__=="__main__":
    unittest.main()
