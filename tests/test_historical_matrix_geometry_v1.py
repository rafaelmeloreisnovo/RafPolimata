"""Scoped unit and adversarial gates for historical geometry; not device proof."""
import copy
import tempfile
import unittest
from pathlib import Path

from scripts.historical_matrix_geometry_v1 import (
    SCHEMA, TOPOLOGY, build,
)


def fixture(shape=(1, 2), op="xor"):
    cells = []
    for p in range(8):
        for layer in range(3):
            for o in range(2):
                value = (p * 6 + layer * 2 + o + 1) & 255
                if shape == (1, 2):
                    matrix = [[value, value ^ 0x5A]]
                else:
                    matrix = [[[value] * 10 for _ in range(10)] for _ in range(10)]
                cells.append({
                    "position": p, "layer": layer, "opposition": o,
                    "matrix": matrix,
                })
    return {
        "schema": SCHEMA, "topology": dict(TOPOLOGY),
        "operator": op, "cells": cells,
    }


class HistoricalMatrixGeometryTests(unittest.TestCase):
    def test_48_slots_and_three_projections(self):
        spec = fixture()
        got = build(spec, Path("."))
        m = got["geometry"]
        self.assertEqual(len(m["canonical_slots"]), 48)
        self.assertEqual(len(m["corners"]), 8)
        self.assertEqual(m["q16_scales"][:2], [65536, 56756])
        self.assertEqual([item["slot"] for item in m["canonical_slots"]], list(range(48)))
        for index, item in enumerate(m["canonical_slots"]):
            self.assertEqual((item["position"], item["layer"], item["opposition"]),
                             (index // 6, (index // 2) % 3, index % 2))
        self.assertEqual(m["central_matrix"], got["core"]["matrix"])
        self.assertFalse(m["projection_is_reversible"])
        self.assertFalse(got["receipt"]["claim_allowed"])
        self.assertEqual(got["receipt"]["total_input_cells"], 96)
        self.assertEqual(len(m["by_opposition"]), 2)
        self.assertEqual(len(m["by_layer"]), 3)
        for y in range(2):
            value = 0
            for cell in spec["cells"]:
                value ^= cell["matrix"][0][y]
            self.assertEqual(m["central_matrix"][0][y], value)
            self.assertEqual(m["by_opposition"][0]["matrix"][0][y] ^
                             m["by_opposition"][1]["matrix"][0][y], value)

    def test_mirror_axis_bijection_and_exact_ids(self):
        original = build(fixture(), Path("."))
        spec = fixture()
        spec["mirror_axis"] = 1
        mirrored = build(spec, Path("."))
        self.assertEqual(original["core"]["matrix"], mirrored["core"]["matrix"])
        for item in mirrored["geometry"]["canonical_slots"]:
            self.assertEqual(item["source_position"], item["position"] ^ 2)
        self.assertEqual(len({tuple(item["xyz"]) for item in
                              mirrored["geometry"]["corners"]}), 8)
        self.assertEqual(sum(item["parity"] for item in
                             mirrored["geometry"]["corners"]), 4)

    def test_lossy_xor_not_claimed_reversible(self):
        a = build(fixture(), Path("."))
        spec = fixture()
        spec["cells"][0]["matrix"][0][0] ^= 1
        b = build(spec, Path("."))
        self.assertEqual(a["core"]["matrix"][0][0] ^
                         b["core"]["matrix"][0][0], 1)
        self.assertFalse(b["geometry"]["projection_is_reversible"])

    def test_corrupt_missing_duplicate_and_type_rejected(self):
        spec = fixture()
        for variant in range(8):
            altered = copy.deepcopy(spec)
            if variant == 0:
                altered["cells"].pop()
            if variant == 1:
                altered["cells"][0] = copy.deepcopy(altered["cells"][1])
            if variant == 2:
                altered["cells"][2]["position"] = 8
            if variant == 3:
                altered["cells"][0]["matrix"][0][0] = True
            if variant == 4:
                altered["topology"]["layers"] = 4
            if variant == 5:
                altered["mirror_axis"] = 3
            if variant == 6:
                altered["cells"][0]["matrix"] = [[1], [2, 3]]
            if variant == 7:
                altered["cells"][0]["source"] = {"path": "../secret", "shape": [1, 2]}
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                build(altered, Path("."))

    def test_symbolic_1000_cube_rejected_materialization(self):
        spec = fixture()
        for cell in spec["cells"]:
            cell.pop("matrix")
            cell["source"] = {"path": "unused.bin", "shape": [1000, 1000, 1000]}
        with self.assertRaisesRegex(ValueError, "budget"):
            build(spec, Path("."))

    def test_bounded_10_cube_and_input_integrity(self):
        spec = fixture(shape=(10, 10, 10))
        with tempfile.TemporaryDirectory() as folder:
            result = build(spec, Path(folder))
            self.assertEqual(result["receipt"]["total_input_cells"], 48000)
            self.assertEqual(len(result["core"]["matrix"]), 10)
            self.assertEqual(len(result["core"]["matrix"][0]), 10)
            self.assertEqual(len(result["core"]["matrix"][0][0]), 10)
            self.assertEqual(result["receipt"]["slots"], 48)

    def test_weighted_mode_requires_exact_weights(self):
        spec = fixture(op="weighted_mod_256")
        with self.assertRaises(ValueError):
            build(spec, Path("."))
        spec["weights"] = [1] * 48
        result = build(spec, Path("."))
        self.assertEqual(result["receipt"]["operator"], "weighted_mod_256")

    def test_binary_source_path_escape_rejected(self):
        spec = fixture()
        spec["cells"][0].pop("matrix")
        spec["cells"][0]["source"] = {"path": "../external.bin", "shape": [1, 2]}
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ValueError):
                build(spec, Path(directory))


if __name__ == "__main__":
    unittest.main()
