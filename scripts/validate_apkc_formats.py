#!/usr/bin/env python3
"""Strict structural validator for ApkC APK, DEX and ELF artifacts.

This tool validates bytes, not filenames or documentation. It is deliberately
independent from the C builders so the implementation cannot validate itself by
reusing the same parsing assumptions.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
import zlib
import zipfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

SCHEMA = "raf.apkc-format-validation.v1"
EM_ARM = 40
EM_AARCH64 = 183
ET_DYN = 3
PT_LOAD = 1
DEX_ENDIAN_CONSTANT = 0x12345678


@dataclass
class Validation:
    kind: str
    path: str
    state: str = "PASS"
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    metadata: dict[str, Any] = field(default_factory=dict)

    def fail(self, message: str) -> None:
        self.state = "FAIL"
        self.errors.append(message)

    def as_dict(self) -> dict[str, Any]:
        return {
            "kind": self.kind,
            "path": self.path,
            "state": self.state,
            "errors": self.errors,
            "warnings": self.warnings,
            "metadata": self.metadata,
        }


def u16(data: bytes, off: int) -> int:
    return struct.unpack_from("<H", data, off)[0]


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def u64(data: bytes, off: int) -> int:
    return struct.unpack_from("<Q", data, off)[0]


def checked_span(result: Validation, label: str, offset: int, size: int, total: int) -> bool:
    if offset < 0 or size < 0 or offset > total or size > total - offset:
        result.fail(f"{label} fora dos limites: offset={offset} size={size} total={total}")
        return False
    return True


def validate_dex_bytes(data: bytes, path: str = "classes.dex") -> Validation:
    result = Validation("DEX", path)
    result.metadata["size_bytes"] = len(data)
    result.metadata["sha256"] = hashlib.sha256(data).hexdigest()

    if len(data) < 0x70:
        result.fail("DEX menor que o header de 0x70 bytes")
        return result

    magic = data[:8]
    if not (magic.startswith(b"dex\n") and magic[7] == 0 and magic[4:7].isdigit()):
        result.fail(f"magic DEX inválido: {magic!r}")
    else:
        result.metadata["version"] = magic[4:7].decode("ascii")

    stored_adler = u32(data, 8)
    computed_adler = zlib.adler32(data[12:]) & 0xFFFFFFFF
    stored_sha1 = data[12:32]
    computed_sha1 = hashlib.sha1(data[32:]).digest()
    result.metadata.update(
        stored_adler32=f"{stored_adler:08x}",
        computed_adler32=f"{computed_adler:08x}",
        stored_sha1=stored_sha1.hex(),
        computed_sha1=computed_sha1.hex(),
    )
    if stored_adler != computed_adler:
        result.fail("Adler-32 do DEX não confere")
    if stored_sha1 != computed_sha1:
        result.fail("SHA-1 interno do DEX não confere")

    file_size = u32(data, 32)
    header_size = u32(data, 36)
    endian_tag = u32(data, 40)
    map_off = u32(data, 52)
    data_size = u32(data, 104)
    data_off = u32(data, 108)
    result.metadata.update(
        file_size=file_size,
        header_size=header_size,
        endian_tag=f"0x{endian_tag:08x}",
        map_off=map_off,
        data_size=data_size,
        data_off=data_off,
    )

    if file_size != len(data):
        result.fail(f"file_size={file_size} difere do tamanho real={len(data)}")
    if header_size != 0x70:
        result.fail(f"header_size inválido: {header_size}")
    if endian_tag != DEX_ENDIAN_CONSTANT:
        result.fail(f"endian_tag inválido: 0x{endian_tag:08x}")
    checked_span(result, "data section", data_off, data_size, len(data))

    if map_off == 0:
        result.fail("map_off é zero")
    elif checked_span(result, "map_list header", map_off, 4, len(data)):
        map_count = u32(data, map_off)
        result.metadata["map_count"] = map_count
        if map_count > 65535:
            result.fail(f"map_count implausível: {map_count}")
        elif checked_span(result, "map_list items", map_off + 4, map_count * 12, len(data)):
            seen: set[int] = set()
            items: list[dict[str, int]] = []
            for index in range(map_count):
                off = map_off + 4 + index * 12
                item_type = u16(data, off)
                unused = u16(data, off + 2)
                item_size = u32(data, off + 4)
                item_off = u32(data, off + 8)
                if unused != 0:
                    result.fail(f"map_item[{index}] unused != 0")
                if item_type in seen:
                    result.fail(f"map_item duplicado: type=0x{item_type:04x}")
                seen.add(item_type)
                if item_size and item_off >= len(data):
                    result.fail(f"map_item[{index}] offset fora do arquivo")
                items.append({"type": item_type, "size": item_size, "offset": item_off})
            result.metadata["map_items"] = items
            if 0x0000 not in seen:
                result.fail("map_list não referencia header_item")
            if 0x1000 not in seen:
                result.fail("map_list não referencia map_list")

    return result


def validate_elf_bytes(data: bytes, path: str = "lib.so", expected_machine: int | None = None) -> Validation:
    result = Validation("ELF", path)
    result.metadata["size_bytes"] = len(data)
    result.metadata["sha256"] = hashlib.sha256(data).hexdigest()

    if len(data) < 16 or data[:4] != b"\x7fELF":
        result.fail("magic ELF ausente")
        return result

    elf_class = data[4]
    elf_data = data[5]
    if elf_data != 1:
        result.fail("somente ELF little-endian é aceito")
        return result

    if elf_class == 1:
        header_size_min = 52
        phoff_field = 28
        shoff_field = 32
        ehsize_field = 40
        phentsize_field = 42
        phnum_field = 44
        shentsize_field = 46
        shnum_field = 48
        offset_reader = u32
        ph_expected_size = 32
    elif elf_class == 2:
        header_size_min = 64
        phoff_field = 32
        shoff_field = 40
        ehsize_field = 52
        phentsize_field = 54
        phnum_field = 56
        shentsize_field = 58
        shnum_field = 60
        offset_reader = u64
        ph_expected_size = 56
    else:
        result.fail(f"EI_CLASS inválida: {elf_class}")
        return result

    if len(data) < header_size_min:
        result.fail("header ELF truncado")
        return result

    e_type = u16(data, 16)
    machine = u16(data, 18)
    version = u32(data, 20)
    phoff = offset_reader(data, phoff_field)
    shoff = offset_reader(data, shoff_field)
    ehsize = u16(data, ehsize_field)
    phentsize = u16(data, phentsize_field)
    phnum = u16(data, phnum_field)
    shentsize = u16(data, shentsize_field)
    shnum = u16(data, shnum_field)

    result.metadata.update(
        elf_class=32 if elf_class == 1 else 64,
        e_type=e_type,
        machine=machine,
        version=version,
        phoff=phoff,
        phentsize=phentsize,
        phnum=phnum,
        shoff=shoff,
        shentsize=shentsize,
        shnum=shnum,
    )

    if e_type != ET_DYN:
        result.fail(f"ELF não é ET_DYN: e_type={e_type}")
    if version != 1:
        result.fail(f"versão ELF inválida: {version}")
    if expected_machine is not None and machine != expected_machine:
        result.fail(f"e_machine={machine}, esperado={expected_machine}")
    if ehsize < header_size_min:
        result.fail(f"e_ehsize menor que o mínimo: {ehsize}")
    if phnum == 0:
        result.fail("ELF sem program headers")
    if phnum and phentsize < ph_expected_size:
        result.fail(f"e_phentsize insuficiente: {phentsize}")

    if phnum and checked_span(result, "program header table", phoff, phentsize * phnum, len(data)):
        load_count = 0
        segments: list[dict[str, int]] = []
        for index in range(phnum):
            off = phoff + index * phentsize
            p_type = u32(data, off)
            if elf_class == 1:
                p_offset = u32(data, off + 4)
                p_filesz = u32(data, off + 16)
                p_memsz = u32(data, off + 20)
            else:
                p_offset = u64(data, off + 8)
                p_filesz = u64(data, off + 32)
                p_memsz = u64(data, off + 40)
            if p_filesz > p_memsz:
                result.fail(f"segment[{index}] p_filesz > p_memsz")
            checked_span(result, f"segment[{index}] file range", p_offset, p_filesz, len(data))
            if p_type == PT_LOAD:
                load_count += 1
            segments.append({
                "type": p_type,
                "offset": p_offset,
                "filesz": p_filesz,
                "memsz": p_memsz,
            })
        result.metadata["segments"] = segments
        result.metadata["pt_load_count"] = load_count
        if load_count == 0:
            result.fail("ELF sem segmento PT_LOAD")

    if shnum:
        if shentsize == 0:
            result.fail("e_shnum > 0 com e_shentsize == 0")
        else:
            checked_span(result, "section header table", shoff, shentsize * shnum, len(data))

    return result


def validate_apk(path: Path, require_both: bool = False) -> dict[str, Any]:
    report: dict[str, Any] = {
        "kind": "APK",
        "path": str(path),
        "state": "PASS",
        "errors": [],
        "entries": [],
        "dex": [],
        "elf": [],
    }
    if not path.is_file():
        report["state"] = "TOKEN_VAZIO"
        report["errors"].append("APK ausente")
        return report

    try:
        with zipfile.ZipFile(path) as apk:
            bad = apk.testzip()
            if bad:
                report["state"] = "FAIL"
                report["errors"].append(f"ZIP CRC inválido em {bad}")
            names = sorted(apk.namelist())
            report["entries"] = names
            dex_names = [name for name in names if name == "classes.dex" or re_match_secondary_dex(name)]
            so_names = [name for name in names if name.startswith("lib/") and name.endswith(".so")]

            if not dex_names:
                report["state"] = "FAIL"
                report["errors"].append("APK sem classes.dex")
            for name in dex_names:
                result = validate_dex_bytes(apk.read(name), name)
                report["dex"].append(result.as_dict())
                if result.state != "PASS":
                    report["state"] = "FAIL"

            abi_seen: set[str] = set()
            for name in so_names:
                parts = name.split("/")
                abi = parts[1] if len(parts) >= 3 else "unknown"
                abi_seen.add(abi)
                expected = EM_AARCH64 if abi == "arm64-v8a" else EM_ARM if abi == "armeabi-v7a" else None
                result = validate_elf_bytes(apk.read(name), name, expected)
                result.metadata["abi"] = abi
                report["elf"].append(result.as_dict())
                if result.state != "PASS":
                    report["state"] = "FAIL"

            report["abis"] = sorted(abi_seen)
            if require_both:
                missing = {"arm64-v8a", "armeabi-v7a"} - abi_seen
                if missing:
                    report["state"] = "FAIL"
                    report["errors"].append(f"ABIs ausentes: {sorted(missing)}")
            if not so_names:
                report["state"] = "FAIL"
                report["errors"].append("APK sem bibliotecas nativas")
    except (OSError, zipfile.BadZipFile, RuntimeError) as exc:
        report["state"] = "FAIL"
        report["errors"].append(f"APK/ZIP inválido: {exc}")

    return report


def re_match_secondary_dex(name: str) -> bool:
    if not (name.startswith("classes") and name.endswith(".dex")):
        return False
    middle = name[len("classes") : -len(".dex")]
    return bool(middle) and middle.isdigit() and int(middle) >= 2


def overall_state(items: list[dict[str, Any]]) -> str:
    if any(item.get("state") == "FAIL" for item in items):
        return "FAIL"
    if items and all(item.get("state") == "TOKEN_VAZIO" for item in items):
        return "TOKEN_VAZIO"
    if any(item.get("state") == "TOKEN_VAZIO" for item in items):
        return "PARTIAL"
    return "PASS"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--apk", type=Path)
    parser.add_argument("--dex", type=Path)
    parser.add_argument("--elf", type=Path)
    parser.add_argument("--abi", choices=["arm64-v8a", "armeabi-v7a"])
    parser.add_argument("--require-both", action="store_true")
    parser.add_argument("--allow-missing", action="store_true")
    parser.add_argument("--write", type=Path)
    args = parser.parse_args(argv)

    items: list[dict[str, Any]] = []
    if args.apk:
        items.append(validate_apk(args.apk, args.require_both))
    if args.dex:
        if args.dex.is_file():
            items.append(validate_dex_bytes(args.dex.read_bytes(), str(args.dex)).as_dict())
        else:
            items.append({"kind": "DEX", "path": str(args.dex), "state": "TOKEN_VAZIO", "errors": ["arquivo ausente"]})
    if args.elf:
        expected = EM_AARCH64 if args.abi == "arm64-v8a" else EM_ARM if args.abi == "armeabi-v7a" else None
        if args.elf.is_file():
            items.append(validate_elf_bytes(args.elf.read_bytes(), str(args.elf), expected).as_dict())
        else:
            items.append({"kind": "ELF", "path": str(args.elf), "state": "TOKEN_VAZIO", "errors": ["arquivo ausente"]})
    if not items:
        parser.error("informe --apk, --dex ou --elf")

    state = overall_state(items)
    report = {
        "schema": SCHEMA,
        "state": state,
        "claim_allowed": state == "PASS",
        "items": items,
    }
    payload = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.write:
        args.write.parent.mkdir(parents=True, exist_ok=True)
        args.write.write_text(payload, encoding="utf-8")
    else:
        sys.stdout.write(payload)

    if state == "PASS":
        return 0
    if args.allow_missing and state in {"TOKEN_VAZIO", "PARTIAL"}:
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
