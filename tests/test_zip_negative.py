#!/usr/bin/env python3
"""test_zip_negative.py — negative-case contract for APKc's ZIP writer (L17).

Self-contained, pure Python, x86-runnable. No ARM, no apkc.

Contract documented: APKc's ZIP/APK writer (Apkc/fmt_zip.h) MUST emit a valid
End-Of-Central-Directory record and a valid central directory. A consumer (the
Android packager, or here Python's zipfile) MUST be able to reject output that
violates that contract.

This test builds a *valid* APK-shaped ZIP in /tmp, confirms it is accepted,
then mangles the central-directory signature to corrupt it, and asserts the
corrupt file is REJECTED (raises / fails testzip). That demonstrates the
corruption is detectable — the property apkc's well-formed output relies on.

Run:
    python3 tests/test_zip_negative.py
"""
import io
import sys
import tempfile
import zipfile
from pathlib import Path

# Central Directory File Header signature: "PK\x01\x02" (0x02014b50).
CEN_SIG = b"PK\x01\x02"


def build_valid_apk(path: Path):
    """Write a minimal APK-shaped ZIP with the members apkc would emit."""
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("AndroidManifest.xml", b"\x03\x00\x08\x00AXMLSTUB")
        z.writestr("classes.dex", b"dex\n035\x00MINIMALDEXBODY")
        z.writestr("lib/arm64-v8a/libmain.so", b"\x7fELFstubsharedobject")
    return path


def assert_valid(path: Path):
    """A well-formed ZIP must open and pass testzip() (no bad CRC)."""
    with zipfile.ZipFile(path, "r") as z:
        bad = z.testzip()
        assert bad is None, f"unexpected bad member in valid ZIP: {bad}"
        names = z.namelist()
    assert "AndroidManifest.xml" in names, names
    assert "classes.dex" in names, names
    assert any(n.startswith("lib/") and n.endswith(".so") for n in names), names
    return names


def corrupt_central_directory(src: Path, dst: Path):
    """Copy src to dst but break the central-directory file header signature.

    We flip the first central-directory signature 'PK\\x01\\x02' to garbage.
    A conformant reader can no longer locate/parse the central directory, so
    opening must fail with BadZipFile.
    """
    data = bytearray(src.read_bytes())
    idx = data.find(CEN_SIG)
    assert idx != -1, "test bug: no central-directory signature found to corrupt"
    # Overwrite the 4-byte signature with a non-signature value.
    data[idx:idx + 4] = b"XXXX"
    dst.write_bytes(bytes(data))
    return idx


def assert_rejected(path: Path):
    """Opening / validating the corrupt archive MUST raise."""
    raised = False
    try:
        with zipfile.ZipFile(path, "r") as z:
            # If it somehow opened, testzip must still surface the damage.
            z.testzip()
    except (zipfile.BadZipFile, OSError, EOFError) as e:
        raised = True
        print(f"    corrupt APK correctly rejected: {type(e).__name__}: {e}")
    assert raised, "corruption NOT detected: corrupt APK opened cleanly (FAIL)"


def main():
    print("=== test_zip_negative (L17 negative case) ===")
    tmp = Path(tempfile.mkdtemp(prefix="apkc_zipneg_"))
    valid = tmp / "valid.apk"
    corrupt = tmp / "corrupt.apk"

    build_valid_apk(valid)
    names = assert_valid(valid)
    print(f"    built valid APK ({valid.stat().st_size} bytes): members={names}")

    idx = corrupt_central_directory(valid, corrupt)
    print(f"    corrupted central-directory signature at offset {idx}")

    assert_rejected(corrupt)

    # Sanity guard: ensure the corruption actually changed the bytes, i.e. we
    # are not trivially passing because the valid file itself was unreadable.
    assert valid.read_bytes() != corrupt.read_bytes(), "corruption was a no-op"

    print("zip-negative: PASS")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print(f"zip-negative: FAIL: {e}", file=sys.stderr)
        sys.exit(1)
