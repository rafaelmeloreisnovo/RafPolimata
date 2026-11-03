"""RafPictures V1 resource, schema and truth limits."""
import json
import re
import zipfile

from .sniff import AXES, MIME, SCHEMA

CHUNK = 1024 * 1024
MAX_ASSETS = 128
MAX_ASSET_BYTES = 512 * 1024 * 1024
MAX_TOTAL_BYTES = 4 * 1024 * 1024 * 1024
MAX_MANIFEST_BYTES = 1024 * 1024
ROLES = frozenset(("color", "depth", "mask", "preview"))

class RafError(ValueError):
    pass

def canon(obj):
    return json.dumps(obj, ensure_ascii=False, sort_keys=True,
                      separators=(",", ":"), allow_nan=False).encode("utf-8")

def item(name):
    return zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))

def coords_for(mapping, index):
    raw = mapping.get(str(index), {})
    if not isinstance(raw, dict):
        raise RafError("coordinate entry must be an object")
    if set(raw) - {"x", "y", "z", "t", "c", "angle", "projection"}:
        raise RafError("unknown coordinate")
    result = {key: "TOKEN_VAZIO" for key in ("x", "y", "z", "t", "c")}
    result["layer"] = index
    for key, value in raw.items():
        if isinstance(value, bool) or not isinstance(value, (str, int, float)):
            raise RafError("invalid coordinate value")
        if isinstance(value, str) and (len(value) > 80 or
                                      any(ord(ch) < 32 for ch in value)):
            raise RafError("invalid coordinate string")
        result[key] = value
    return result

def valid_asset_path(name, index):
    return (isinstance(name, str) and
            bool(re.fullmatch(r"assets/[0-9]{4}\.[a-z0-9]{1,8}", name)) and
            name.startswith("assets/%04d." % index))
