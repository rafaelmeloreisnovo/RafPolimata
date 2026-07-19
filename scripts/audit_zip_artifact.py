#!/usr/bin/env python3
"""Auditoria segura de artefato ZIP sem extração no filesystem.

Inspeciona estrutura central, nomes, tamanhos, compressão, flags, CRC e hashes por
entrada. Entradas perigosas não são abertas para hashing. Não executa conteúdo.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import stat
import sys
import zipfile
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any

SCHEMA = "raf.zip-artifact-audit.v1"
EXECUTABLE_OR_SENSITIVE_SUFFIXES = {
    ".apk", ".dex", ".elf", ".so", ".o", ".a", ".jar", ".class",
    ".exe", ".dll", ".bat", ".cmd", ".ps1", ".sh", ".bash",
    ".pem", ".key", ".keystore", ".jks", ".env",
}


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for block in iter(lambda: fh.read(131072), b""):
            h.update(block)
    return h.hexdigest()


def unsafe_name(name: str) -> list[str]:
    flags: list[str] = []
    normalized = name.replace("\\", "/")
    pure = PurePosixPath(normalized)
    if normalized.startswith("/"):
        flags.append("absolute_path")
    if any(part == ".." for part in pure.parts):
        flags.append("path_traversal")
    if any(part in {"", "."} for part in pure.parts[:-1]):
        flags.append("ambiguous_path_component")
    if "\x00" in name:
        flags.append("nul_in_name")
    if len(name.encode("utf-8", errors="replace")) > 4096:
        flags.append("name_too_long")
    return flags


def is_symlink(info: zipfile.ZipInfo) -> bool:
    mode = (info.external_attr >> 16) & 0xFFFF
    return stat.S_IFMT(mode) == stat.S_IFLNK


def compression_ratio(info: zipfile.ZipInfo) -> float:
    if info.file_size == 0:
        return 1.0
    if info.compress_size == 0:
        return float("inf")
    return info.file_size / info.compress_size


def hash_entry(
    archive: zipfile.ZipFile,
    info: zipfile.ZipInfo,
    max_entry_bytes: int,
) -> tuple[str | None, str | None]:
    if info.is_dir():
        return None, None
    if info.file_size > max_entry_bytes:
        return None, "entry_exceeds_hash_limit"
    h = hashlib.sha256()
    total = 0
    try:
        with archive.open(info, "r") as fh:
            while True:
                block = fh.read(min(131072, max_entry_bytes - total + 1))
                if not block:
                    break
                total += len(block)
                if total > max_entry_bytes:
                    return None, "decompressed_stream_exceeds_limit"
                h.update(block)
    except (RuntimeError, OSError, zipfile.BadZipFile) as exc:
        return None, f"entry_read_error:{type(exc).__name__}"
    if total != info.file_size:
        return None, "decompressed_size_mismatch"
    return h.hexdigest(), None


def audit_zip(
    path: Path,
    *,
    max_entry_bytes: int = 256 * 1024 * 1024,
    max_total_bytes: int = 1024 * 1024 * 1024,
    max_ratio: float = 1000.0,
    max_entries: int = 100000,
) -> dict[str, Any]:
    report: dict[str, Any] = {
        "schema": SCHEMA,
        "path": str(path),
        "state": "PASS",
        "claim_allowed": False,
        "errors": [],
        "warnings": [],
        "summary": {},
        "entries": [],
    }
    if not path.is_file():
        report["state"] = "TOKEN_VAZIO"
        report["errors"].append("arquivo ZIP ausente")
        return report

    report["summary"]["archive_size_bytes"] = path.stat().st_size
    report["summary"]["archive_sha256"] = sha256_file(path)

    try:
        with zipfile.ZipFile(path, "r") as archive:
            infos = archive.infolist()
            names = [info.filename for info in infos]
            duplicate_names = sorted(name for name, count in Counter(names).items() if count > 1)
            total_uncompressed = sum(info.file_size for info in infos)
            total_compressed = sum(info.compress_size for info in infos)
            report["summary"].update({
                "entry_count": len(infos),
                "file_count": sum(not info.is_dir() for info in infos),
                "directory_count": sum(info.is_dir() for info in infos),
                "total_uncompressed_bytes": total_uncompressed,
                "total_compressed_bytes": total_compressed,
                "duplicate_names": duplicate_names,
            })

            if len(infos) > max_entries:
                report["errors"].append(f"entry_count excede limite: {len(infos)} > {max_entries}")
            if total_uncompressed > max_total_bytes:
                report["errors"].append(
                    f"total descompactado excede limite: {total_uncompressed} > {max_total_bytes}"
                )
            if duplicate_names:
                report["errors"].append("nomes de entrada duplicados")

            bad_crc = archive.testzip()
            report["summary"]["first_bad_crc_entry"] = bad_crc
            if bad_crc:
                report["errors"].append(f"CRC inválido em {bad_crc}")

            for info in infos:
                flags = unsafe_name(info.filename)
                ratio = compression_ratio(info)
                encrypted = bool(info.flag_bits & 0x1)
                symlink = is_symlink(info)
                suffix = PurePosixPath(info.filename).suffix.casefold()
                review_flags: list[str] = []
                if encrypted:
                    review_flags.append("encrypted_entry")
                if symlink:
                    flags.append("symlink_entry")
                if info.file_size > max_entry_bytes:
                    flags.append("entry_too_large")
                if ratio > max_ratio:
                    flags.append("compression_ratio_exceeded")
                if suffix in EXECUTABLE_OR_SENSITIVE_SUFFIXES:
                    review_flags.append("executable_or_sensitive_suffix")

                can_hash = not flags and not encrypted and not symlink
                digest: str | None = None
                hash_error: str | None = None
                if can_hash:
                    digest, hash_error = hash_entry(archive, info, max_entry_bytes)
                    if hash_error:
                        flags.append(hash_error)

                entry = {
                    "name": info.filename,
                    "is_directory": info.is_dir(),
                    "compressed_size": info.compress_size,
                    "uncompressed_size": info.file_size,
                    "compression_ratio": None if ratio == float("inf") else round(ratio, 6),
                    "crc32": f"{info.CRC:08x}",
                    "compression_method": info.compress_type,
                    "encrypted": encrypted,
                    "symlink": symlink,
                    "sha256": digest,
                    "blocking_flags": sorted(set(flags)),
                    "review_flags": sorted(set(review_flags)),
                }
                report["entries"].append(entry)

            blocking_entries = [
                entry["name"] for entry in report["entries"] if entry["blocking_flags"]
            ]
            review_entries = [
                entry["name"] for entry in report["entries"] if entry["review_flags"]
            ]
            report["summary"]["blocking_entry_count"] = len(blocking_entries)
            report["summary"]["review_entry_count"] = len(review_entries)
            report["summary"]["blocking_entries"] = blocking_entries
            report["summary"]["review_entries"] = review_entries
            if blocking_entries:
                report["errors"].append("uma ou mais entradas possuem flags bloqueantes")
            if review_entries:
                report["warnings"].append("entradas executáveis ou sensíveis exigem revisão")
    except (OSError, zipfile.BadZipFile, RuntimeError, NotImplementedError) as exc:
        report["errors"].append(f"ZIP inválido ou não suportado: {type(exc).__name__}: {exc}")

    if report["errors"]:
        report["state"] = "FAIL"
    elif report["warnings"]:
        report["state"] = "REVIEW_REQUIRED"
    else:
        report["state"] = "PASS"
    report["claim_allowed"] = report["state"] == "PASS"
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Audita ZIP sem extrair no filesystem")
    parser.add_argument("zip_path", type=Path)
    parser.add_argument("--write", type=Path)
    parser.add_argument("--max-entry-mib", type=int, default=256)
    parser.add_argument("--max-total-mib", type=int, default=1024)
    parser.add_argument("--max-ratio", type=float, default=1000.0)
    parser.add_argument("--max-entries", type=int, default=100000)
    parser.add_argument("--strict-review", action="store_true")
    args = parser.parse_args(argv)

    if args.max_entry_mib < 1 or args.max_total_mib < 1 or args.max_ratio < 1 or args.max_entries < 1:
        parser.error("limites devem ser positivos")

    report = audit_zip(
        args.zip_path,
        max_entry_bytes=args.max_entry_mib * 1024 * 1024,
        max_total_bytes=args.max_total_mib * 1024 * 1024,
        max_ratio=args.max_ratio,
        max_entries=args.max_entries,
    )
    payload = json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.write:
        args.write.parent.mkdir(parents=True, exist_ok=True)
        args.write.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)

    if report["state"] == "FAIL":
        return 1
    if args.strict_review and report["state"] != "PASS":
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
