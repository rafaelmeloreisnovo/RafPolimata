#!/usr/bin/env python3
"""Build deterministic minimal ARM32/ARM64 ELF probes for physical RafCI evidence.

CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE

These bytes are construction artifacts only. Hosted generation or inspection never
promotes gate.physical-execution. A physical receipt must still execute the exact
SHA-256-pinned bytes through capture_physical_execution_v1.sh.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
from typing import Any

HEX40 = re.compile(r"^[0-9a-f]{40}$")

ARM32_SHA256 = "95030251a76a57341c8be9d0b9565692ba240b72b2253e57d7be166cf25b9b1d"
ARM64_SHA256 = "48d8d278f2565b4e5bb01f535b8e3c7b2e2b7963244c884dc99facf7b0fd8370"


class ProbeError(RuntimeError):
    pass


def require(value: bool, message: str) -> None:
    if not value:
        raise ProbeError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def build_arm32() -> bytes:
    """ELF32 EM_ARM EABI5, one RX PT_LOAD, _start => exit(0)."""
    base = 0x00010000
    code_offset = 0x100
    # ARM state: mov r7,#1 ; mov r0,#0 ; svc #0
    code = bytes.fromhex("0170a0e30000a0e3000000ef")
    total = code_offset + len(code)
    ident = b"\x7fELF" + bytes([1, 1, 1, 0]) + bytes(8)
    ehdr = struct.pack(
        "<16sHHIIIIIHHHHHH",
        ident,
        2,          # ET_EXEC
        40,         # EM_ARM
        1,
        base + code_offset,
        52,
        0,
        0x05000000, # EABI5
        52,
        32,
        1,
        0,
        0,
        0,
    )
    phdr = struct.pack(
        "<IIIIIIII",
        1,          # PT_LOAD
        0,
        base,
        base,
        total,
        total,
        5,          # PF_R | PF_X
        0x1000,
    )
    return ehdr + phdr + bytes(code_offset - len(ehdr) - len(phdr)) + code


def build_arm64() -> bytes:
    """ELF64 EM_AARCH64, one RX PT_LOAD, _start => exit(0)."""
    base = 0x00400000
    code_offset = 0x100
    # AArch64: mov x8,#93 ; mov x0,#0 ; svc #0
    code = bytes.fromhex("a80b80d2000080d2010000d4")
    total = code_offset + len(code)
    ident = b"\x7fELF" + bytes([2, 1, 1, 0]) + bytes(8)
    ehdr = struct.pack(
        "<16sHHIQQQIHHHHHH",
        ident,
        2,          # ET_EXEC
        183,        # EM_AARCH64
        1,
        base + code_offset,
        64,
        0,
        0,
        64,
        56,
        1,
        0,
        0,
        0,
    )
    phdr = struct.pack(
        "<IIQQQQQQ",
        1,          # PT_LOAD
        5,          # PF_R | PF_X
        0,
        base,
        base,
        total,
        total,
        0x1000,
    )
    return ehdr + phdr + bytes(code_offset - len(ehdr) - len(phdr)) + code


def inspect(data: bytes, scope: str) -> dict[str, Any]:
    require(data[:4] == b"\x7fELF", "ELF magic missing")
    require(data[5] == 1, "ELF must be little-endian")
    require(data[6] == 1, "ELF version drift")
    require(len(data) == 268, "probe size drift")

    if scope == "arm32":
        require(data[4] == 1, "arm32 must be ELFCLASS32")
        fields = struct.unpack("<16sHHIIIIIHHHHHH", data[:52])
        _, e_type, e_machine, e_version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx = fields
        require(e_type == 2 and e_machine == 40 and e_version == 1, "arm32 ELF identity drift")
        require(flags == 0x05000000, "arm32 EABI flags drift")
        require((ehsize, phoff, phentsize, phnum) == (52, 52, 32, 1), "arm32 header layout drift")
        require((shoff, shentsize, shnum, shstrndx) == (0, 0, 0, 0), "arm32 section table must be absent")
        p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align = struct.unpack("<IIIIIIII", data[52:84])
        require((p_type, p_offset, p_filesz, p_memsz, p_flags, p_align) == (1, 0, len(data), len(data), 5, 0x1000), "arm32 PT_LOAD drift")
        require(p_vaddr == p_paddr == 0x10000 and entry == 0x10100, "arm32 address drift")
        require(data[0x100:] == bytes.fromhex("0170a0e30000a0e3000000ef"), "arm32 instruction bytes drift")
        expected_hash = ARM32_SHA256
        elf_class = "ELF32"
        machine = "EM_ARM"
    elif scope == "arm64":
        require(data[4] == 2, "arm64 must be ELFCLASS64")
        fields = struct.unpack("<16sHHIQQQIHHHHHH", data[:64])
        _, e_type, e_machine, e_version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx = fields
        require(e_type == 2 and e_machine == 183 and e_version == 1, "arm64 ELF identity drift")
        require(flags == 0, "arm64 ELF flags drift")
        require((ehsize, phoff, phentsize, phnum) == (64, 64, 56, 1), "arm64 header layout drift")
        require((shoff, shentsize, shnum, shstrndx) == (0, 0, 0, 0), "arm64 section table must be absent")
        p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align = struct.unpack("<IIQQQQQQ", data[64:120])
        require((p_type, p_flags, p_offset, p_filesz, p_memsz, p_align) == (1, 5, 0, len(data), len(data), 0x1000), "arm64 PT_LOAD drift")
        require(p_vaddr == p_paddr == 0x400000 and entry == 0x400100, "arm64 address drift")
        require(data[0x100:] == bytes.fromhex("a80b80d2000080d2010000d4"), "arm64 instruction bytes drift")
        expected_hash = ARM64_SHA256
        elf_class = "ELF64"
        machine = "EM_AARCH64"
    else:
        raise ProbeError(f"unsupported scope: {scope}")

    actual_hash = sha256(data)
    require(actual_hash == expected_hash, f"{scope} deterministic hash drift")
    return {
        "scope": scope,
        "elf_class": elf_class,
        "machine": machine,
        "size": len(data),
        "sha256": actual_hash,
        "program_headers": ["PT_LOAD:R-X"],
        "pt_interp": False,
        "pt_dynamic": False,
        "section_table": False,
        "behavior": "zero-argument process exits 0 using one raw Linux/Android kernel exit syscall",
        "physical_execution": "TOKEN_VAZIO",
    }


def build_set(source_sha: str, out_dir: Path) -> dict[str, Any]:
    require(bool(HEX40.fullmatch(source_sha)), "--source-sha must be exact lowercase 40-hex")
    out_dir.mkdir(parents=True, exist_ok=True)
    outputs = []
    for scope, filename, data in (
        ("arm32", "rafci-physical-probe-arm32.elf", build_arm32()),
        ("arm64", "rafci-physical-probe-arm64.elf", build_arm64()),
    ):
        meta = inspect(data, scope)
        path = out_dir / filename
        path.write_bytes(data)
        path.chmod(0o755)
        meta["filename"] = filename
        outputs.append(meta)

    manifest = {
        "schema": "rafaelia.rafci.physical-probe-set/v1",
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "source_commit": source_sha,
        "state": "PASS_ARTIFACT_CONSTRUCTION_ONLY",
        "artifacts": outputs,
        "construction": {
            "stdlib_only": True,
            "external_assembler": False,
            "external_linker": False,
            "libc": False,
            "pt_interp": False,
            "pt_dynamic": False,
        },
        "physical_execution_observed": "TOKEN_VAZIO",
        "claim_allowed": False,
        "closure": "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
        "F_ok": ["deterministic exact ARM32/ARM64 executable bytes materialized and structurally verified"],
        "F_gap": ["physical execution receipts remain TOKEN_VAZIO"],
        "F_next": ["execute the exact SHA-256-pinned artifact on matching physical Termux and verify the receipt against external pins"],
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return manifest


def selftest() -> None:
    arm32 = bytearray(build_arm32())
    arm64 = bytearray(build_arm64())
    inspect(bytes(arm32), "arm32")
    inspect(bytes(arm64), "arm64")

    arm32[-1] ^= 1
    try:
        inspect(bytes(arm32), "arm32")
    except ProbeError:
        pass
    else:
        raise ProbeError("arm32 mutation falsifier accepted")

    arm64[18] = 40 & 0xFF
    arm64[19] = 0
    try:
        inspect(bytes(arm64), "arm64")
    except ProbeError:
        pass
    else:
        raise ProbeError("arm64 machine falsifier accepted")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-sha", required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()

    source_sha = args.source_sha.lower()
    if args.selftest:
        selftest()
    manifest = build_set(source_sha, args.out_dir)
    print(json.dumps(manifest, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ProbeError, struct.error) as exc:
        print(f"RAFCI_PHYSICAL_PROBE_FAIL: {exc}")
        raise SystemExit(1)
