"""Lossless RafPictures encoder. Input images are never rewritten."""
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
import zipfile

from .contract import (CHUNK, MAX_ASSET_BYTES, MAX_ASSETS,
                       MAX_MANIFEST_BYTES, MAX_TOTAL_BYTES,
                       RafError, canon, coords_for, item)
from .sniff import AXES, MIME, SCHEMA, identify

def pack(output, images, depth=(), mask=(), preview=None, coords_path=None):
    target = Path(output)
    if target.exists() or not target.parent.is_dir():
        raise RafError("output exists or parent directory missing")
    sources = [("color", Path(p)) for p in images]
    sources += [("depth", Path(p)) for p in depth]
    sources += [("mask", Path(p)) for p in mask]
    if preview:
        sources.append(("preview", Path(preview)))
    if not (1 <= len(sources) <= MAX_ASSETS):
        raise RafError("asset count must be 1..128")
    coords = json.loads(Path(coords_path).read_text("utf-8")) if coords_path else {}
    if not isinstance(coords, dict):
        raise RafError("coordinates JSON must be an object")
    records = []
    total = 0
    fd, tmp = tempfile.mkstemp(prefix=".rafp-", dir=target.parent)
    os.close(fd)
    try:
        with zipfile.ZipFile(tmp, "w", compression=zipfile.ZIP_STORED,
                             allowZip64=True) as arc:
            arc.writestr(item("mimetype"), MIME)
            for idx, (role, source) in enumerate(sources):
                if not source.is_file() or source.is_symlink():
                    raise RafError("invalid input file or symlink")
                before = source.stat()
                if before.st_size > MAX_ASSET_BYTES:
                    raise RafError("asset too large")
                with source.open("rb") as inp:
                    mime = identify(inp.read(64))
                if mime is None:
                    raise RafError("unrecognized image signature: " + source.name)
                suffix = source.suffix.lower()
                if not re.fullmatch(r"\.[a-z0-9]{1,8}", suffix):
                    suffix = ".bin"
                member = "assets/%04d%s" % (idx, suffix)
                digest, count = hashlib.sha256(), 0
                with source.open("rb") as inp, arc.open(
                        item(member), "w", force_zip64=True) as out:
                    while True:
                        block = inp.read(CHUNK)
                        if not block:
                            break
                        count += len(block)
                        if count > MAX_ASSET_BYTES or total + count > MAX_TOTAL_BYTES:
                            raise RafError("asset or total size limit exceeded")
                        digest.update(block)
                        out.write(block)
                after = source.stat()
                if (before.st_size, before.st_mtime_ns) != (
                        after.st_size, after.st_mtime_ns) or count != before.st_size:
                    raise RafError("input changed during packaging")
                total += count
                records.append({"path": member, "source_name": source.name,
                                "mime": mime, "role": role, "bytes": count,
                                "sha256": digest.hexdigest(),
                                "coordinates": coords_for(coords, idx)})
            manifest = {"schema": SCHEMA, "axes": AXES, "assets": records,
                        "asset_count": len(records),
                        "preserves_original_bytes": True}
            payload = canon(manifest)
            if len(payload) > MAX_MANIFEST_BYTES:
                raise RafError("manifest too large")
            arc.writestr(item("manifest.json"), payload)
        from .verify import inspect
        inspect(tmp, full=True)
        if target.exists():
            raise RafError("output appeared while packaging")
        os.rename(tmp, target)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)
    return manifest
