"""Strict RafPictures reader; rejects tampering and path traversal."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import tempfile
import zipfile

from .contract import (CHUNK, MAX_ASSET_BYTES, MAX_ASSETS, MAX_MANIFEST_BYTES,
                       MAX_TOTAL_BYTES, ROLES, RafError, canon, valid_asset_path)
from .sniff import AXES, MIME, SCHEMA, identify

def inspect(path, full=False):
    try:
        with zipfile.ZipFile(path, "r") as arc:
            infos = arc.infolist()
            names = [i.filename for i in infos]
            if (not 3 <= len(infos) <= MAX_ASSETS + 2 or
                    names[0] != "mimetype" or names[-1] != "manifest.json" or
                    len(names) != len(set(names))):
                raise RafError("invalid member order or duplicate ZIP entries")
            for info in infos:
                mode = (info.external_attr >> 16) & 0o170000
                if (info.is_dir() or mode == 0o120000 or
                        info.compress_type != zipfile.ZIP_STORED or
                        info.flag_bits & 1):
                    raise RafError("unsupported ZIP member")
            marker = arc.getinfo("mimetype")
            if marker.file_size != len(MIME) or arc.read("mimetype") != MIME:
                raise RafError("not a RafPictures file")
            entry = arc.getinfo("manifest.json")
            if entry.file_size > MAX_MANIFEST_BYTES:
                raise RafError("manifest exceeds limit")
            doc = json.loads(arc.read("manifest.json"))
            if not isinstance(doc, dict):
                raise RafError("manifest not an object")
            assets = doc.get("assets")
            if (doc.get("schema") != SCHEMA or doc.get("axes") != AXES or
                    not isinstance(assets, list) or
                    not 1 <= len(assets) <= MAX_ASSETS or
                    doc.get("asset_count") != len(assets) or
                    len(infos) != len(assets) + 2):
                raise RafError("manifest schema or member count mismatch")
            expected, total = [], 0
            for index, a in enumerate(assets):
                if not isinstance(a, dict):
                    raise RafError("asset descriptor must be object")
                name = a.get("path")
                digest = a.get("sha256")
                if (not valid_asset_path(name, index) or
                        a.get("role") not in ROLES or
                        not isinstance(digest, str) or
                        len(digest) != 64 or
                        any(ch not in "0123456789abcdef" for ch in digest) or
                        not isinstance(a.get("coordinates"), dict)):
                    raise RafError("invalid asset descriptor")
                info = arc.getinfo(name)
                total += info.file_size
                if (info.file_size > MAX_ASSET_BYTES or total > MAX_TOTAL_BYTES or
                        a.get("bytes") != info.file_size):
                    raise RafError("asset length exceeds limit or mismatches")
                expected.append(name)
                if full:
                    actual = hashlib.sha256()
                    header = b""
                    with arc.open(name) as inp:
                        while True:
                            block = inp.read(CHUNK)
                            if not block:
                                break
                            if not header:
                                header = block[:64]
                            actual.update(block)
                    if actual.hexdigest() != digest or identify(header) != a.get("mime"):
                        raise RafError("SHA256 or image signature mismatch: " + name)
            if names[1:-1] != expected:
                raise RafError("unexpected asset order or additional members")
            return doc
    except (zipfile.BadZipFile, KeyError, UnicodeError, json.JSONDecodeError) as e:
        raise RafError("malformed RafPictures archive: " + str(e)) from e

def extract(source, destination):
    doc = inspect(source, full=True)
    dst = Path(destination)
    if dst.exists() or not dst.parent.is_dir():
        raise RafError("destination exists or parent missing")
    stage = Path(tempfile.mkdtemp(prefix=".rafp-unpack-", dir=dst.parent))
    try:
        (stage / "assets").mkdir()
        with zipfile.ZipFile(source) as archive:
            for a in doc["assets"]:
                digest = hashlib.sha256()
                with archive.open(a["path"]) as inp, (stage / a["path"]).open("xb") as out:
                    while True:
                        block = inp.read(CHUNK)
                        if not block:
                            break
                        digest.update(block)
                        out.write(block)
                if digest.hexdigest() != a["sha256"]:
                    raise RafError("source changed during extraction")
        (stage / "manifest.json").write_bytes(canon(doc))
        if dst.exists():
            raise RafError("destination appeared during extraction")
        os.rename(stage, dst)
    finally:
        if stage.exists():
            shutil.rmtree(stage)
    return doc

def export_preview(source, destination):
    doc = inspect(source, full=True)
    asset = next((a for a in doc["assets"] if a["role"] == "preview"), None)
    if not asset:
        raise RafError("no embedded preview")
    # Never silently change or replace a previously exported image.
    with zipfile.ZipFile(source) as archive, open(destination, "xb") as out:
        with archive.open(asset["path"]) as inp:
            shutil.copyfileobj(inp, out, CHUNK)
    return asset
