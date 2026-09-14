import tempfile
import unittest
from pathlib import Path

from scripts.matrix_compose_v1 import SCHEMA_IN, compose, decode_cell10, encode_cell10

class MatrixComposeV1Tests(unittest.TestCase):
    def test_all_bytes_roundtrip_and_single_bit_detection(self):
        for value in range(256):
            cell = encode_cell10(value)
            self.assertEqual(decode_cell10(cell), value)
            for bit in range(10):
                with self.assertRaises(ValueError):
                    decode_cell10(cell ^ (1 << bit))

    def test_xor_fixture_is_deterministic(self):
        spec = {
            "schema": SCHEMA_IN,
            "operator": "xor",
            "layers": [
                {"id": "A", "matrix": [[1, 2], [3, 4]]},
                {"id": "B", "matrix": [[4, 3], [2, 1]]}
            ]
        }
        result = compose(spec, Path("."))
        self.assertEqual(result["matrix"], [[5, 1], [1, 5]])
        self.assertFalse(result["receipt"]["claim_allowed"])
        self.assertEqual(
            result["receipt"]["packed_cell10_sha256"],
            result["ifdex"]["output"]["packed_cell10_sha256"],
        )

    def test_binary_source_is_bounded(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "bytes.bin").write_bytes(bytes(range(32)))
            spec = {
                "schema": SCHEMA_IN,
                "operator": "sum_mod_256",
                "layers": [
                    {"id": "FILE", "source": {"path": "bytes.bin", "shape": [2, 4], "offset": 4}},
                    {"id": "INLINE", "matrix": [[1, 1, 1, 1], [1, 1, 1, 1]]}
                ]
            }
            result = compose(spec, root)
            self.assertEqual(result["matrix"], [[5, 6, 7, 8], [9, 10, 11, 12]])
            self.assertEqual(result["ifdex"]["layers"][0]["offset"], 4)
            self.assertEqual(result["ifdex"]["layers"][0]["length"], 8)

    def test_source_path_escape_rejected(self):
        spec = {
            "schema": SCHEMA_IN,
            "operator": "xor",
            "layers": [{"id": "X", "source": {"path": "../escape.bin", "shape": [1], "offset": 0}}]
        }
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ValueError):
                compose(spec, Path(directory))

if __name__ == "__main__":
    unittest.main()
