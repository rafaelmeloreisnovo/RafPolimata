from __future__ import annotations

import zipfile
from pathlib import Path

import pytest

from voynich_repro.dataset_audit import _onset_stats, _safe_members, sha256_hex


def test_sha256_known_vector() -> None:
    assert sha256_hex(b"abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"


def test_onset_stats_excludes_bare_two_character_token() -> None:
    stats = _onset_stats(["ch", "cha", "cho", "cha"])
    assert stats["ch"]["support"] == 3
    assert stats["ch"]["distinct_words"] == 2


def test_safe_members_accepts_normal_archive(tmp_path: Path) -> None:
    archive = tmp_path / "ok.zip"
    with zipfile.ZipFile(archive, "w") as zf:
        zf.writestr("data/file.txt", "ok")
    with zipfile.ZipFile(archive) as zf:
        assert len(_safe_members(zf)) == 1


def test_safe_members_rejects_path_traversal(tmp_path: Path) -> None:
    archive = tmp_path / "bad.zip"
    with zipfile.ZipFile(archive, "w") as zf:
        zf.writestr("../escape.txt", "bad")
    with zipfile.ZipFile(archive) as zf:
        with pytest.raises(ValueError):
            _safe_members(zf)
