"""Deterministic contract tests; samples are magic-byte fixtures, not decoded photos."""
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

from rafpictures.contract import MIME, RafError
from rafpictures.pack import pack
from rafpictures.sniff import identify
from rafpictures.verify import inspect, extract, export_preview

PNG = b"\x89PNG\r\n\x1a\n" + b"opaque-png-payload"
JPG = b"\xff\xd8\xff" + b"opaque-jpeg-payload"
WEBP = b"RIFF" + b"\x10\x00\x00\x00WEBP" + b"opaque-webp"
HEIC = b"\x00\x00\x00\x18ftypheic" + b"opaque-heic"

class RafPicturesTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.png = self.root / "color.unknown"
        self.jpg = self.root / "cover.jpg"
        self.png.write_bytes(PNG)
        self.jpg.write_bytes(JPG)
        self.archive = self.root / "photo.rafp"

    def tearDown(self):
        self.tmp.cleanup()

    def build(self):
        return pack(self.archive, [self.png], preview=self.jpg)

    def test_signature_registry(self):
        self.assertEqual(identify(PNG), "image/png")
        self.assertEqual(identify(JPG), "image/jpeg")
        self.assertEqual(identify(WEBP), "image/webp")
        self.assertEqual(identify(HEIC), "image/heic")
        self.assertIsNone(identify(b"not a picture"))

    def test_roundtrip_original_bytes(self):
        doc = self.build()
        self.assertEqual(doc["assets"][0]["mime"], "image/png")
        self.assertEqual(doc["assets"][0]["path"], "assets/0000.unknown")
        self.assertEqual(doc["assets"][0]["sha256"], hashlib.sha256(PNG).hexdigest())
        self.assertEqual(inspect(self.archive, full=True)["asset_count"], 2)
        restored = self.root / "restored"
        extract(self.archive, restored)
        self.assertEqual((restored / "assets/0000.unknown").read_bytes(), PNG)
        self.assertEqual((restored / "assets/0001.jpg").read_bytes(), JPG)
        self.assertEqual((restored / "manifest.json").read_bytes(),
                         json.dumps(doc, sort_keys=True, ensure_ascii=False,
                                    separators=(",", ":")).encode())
        self.assertEqual(zipfile.ZipFile(self.archive).namelist()[0], "mimetype")

    def test_embedded_preview_exact(self):
        self.build()
        output = self.root / "cover-export.jpg"
        export_preview(self.archive, output)
        self.assertEqual(output.read_bytes(), JPG)
        with self.assertRaises(FileExistsError):
            export_preview(self.archive, output)

    def test_axis_metadata_and_token_vazio(self):
        metadata = self.root / "coordinates.json"
        metadata.write_text('{"0":{"x":13,"z":2,"t":1,"projection":"front"}}')
        doc = pack(self.archive, [self.png], coords_path=metadata)
        axes = doc["assets"][0]["coordinates"]
        self.assertEqual((axes["x"], axes["z"], axes["t"]), (13, 2, 1))
        self.assertEqual(axes["y"], "TOKEN_VAZIO")
        self.assertEqual(axes["layer"], 0)

    def test_no_overwrite(self):
        self.build()
        with self.assertRaises(RafError):
            self.build()
        with self.assertRaises(RafError):
            extract(self.archive, self.root)

    def test_unknown_image_rejected_without_output(self):
        self.png.write_bytes(b"this is not an image")
        with self.assertRaises(RafError):
            pack(self.archive, [self.png])
        self.assertFalse(self.archive.exists())

    def test_digest_tamper_rejected(self):
        self.build()
        altered = self.root / "altered.rafp"
        with zipfile.ZipFile(self.archive) as source, zipfile.ZipFile(altered, "w") as dst:
            for entry in source.namelist():
                payload = source.read(entry)
                if entry.startswith("assets/0000"):
                    payload = PNG + b"tamper"
                dst.writestr(entry, payload, compress_type=zipfile.ZIP_STORED)
        with self.assertRaises(RafError):
            inspect(altered, full=True)

    def test_extra_member_traversal_rejected(self):
        self.build()
        with zipfile.ZipFile(self.archive, "a") as archive:
            archive.writestr("../escape.png", PNG)
        with self.assertRaises(RafError):
            inspect(self.archive, full=False)

    def test_mime_marker_required(self):
        bad = self.root / "fake.rafp"
        with zipfile.ZipFile(bad, "w") as archive:
            archive.writestr("mimetype", b"application/zip")
            archive.writestr("assets/0000.png", PNG)
            archive.writestr("manifest.json", b"{}")
        with self.assertRaises(RafError):
            inspect(bad)

    def test_duplicate_entries_rejected(self):
        self.build()
        with zipfile.ZipFile(self.archive, "a") as archive:
            archive.writestr("mimetype", MIME)
        with self.assertRaises(RafError):
            inspect(self.archive)

if __name__ == "__main__":
    unittest.main()
