"""Byte-signature format registry; extension is only an application hint."""
MIME = b"application/vnd.rafaelia.rafpictures+zip"
SCHEMA = "rafaelia.rafpictures.v1"
AXES = ["x", "y", "z", "t", "c", "layer"]

def identify(b):
    if b.startswith(b"\x89PNG\r\n\x1a\n"):
        return "image/png"
    if b.startswith(b"\xff\xd8\xff"):
        return "image/jpeg"
    if b.startswith((b"GIF87a", b"GIF89a")):
        return "image/gif"
    if b.startswith(b"RIFF") and len(b) >= 12 and b[8:12] == b"WEBP":
        return "image/webp"
    if b.startswith((b"II*\x00", b"MM\x00*")):
        return "image/tiff"
    if b.startswith(b"BM"):
        return "image/bmp"
    if b.startswith(b"\x76\x2f\x31\x01"):
        return "image/x-exr"
    if b.startswith(b"qoif"):
        return "image/qoi"
    if b.startswith(b"\xff\x0a") or b.startswith(b"\x00\x00\x00\x0cJXL \r\n\x87\n"):
        return "image/jxl"
    if len(b) >= 12 and b[4:8] == b"ftyp":
        brand = b[8:12]
        if brand in (b"avif", b"avis"):
            return "image/avif"
        if brand in (b"heic", b"heix", b"hevc", b"hevx"):
            return "image/heic"
        if brand in (b"mif1", b"msf1"):
            return "image/heif"
    return None
