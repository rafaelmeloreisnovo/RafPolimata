#!/usr/bin/env python3
"""
Tests for Extended ELF Validation (Hotfix H3)

Tests ARM64 ELF validation chain completeness:
- Endianness validation
- Symbol table integrity
- Relocation entries validation
"""

import json
import tempfile
import unittest
from pathlib import Path
from sys import path as sys_path

# Add tools to path
sys_path.insert(0, str(Path(__file__).parent.parent / "tools"))

from gap_atlas_builder import GapAtlasBuilder


class TestELFValidationExtended(unittest.TestCase):
    """Test extended ELF validation for H3 hotfix."""

    def setUp(self):
        """Create temporary directory."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_path = Path(self.temp_dir.name)

    def tearDown(self):
        """Clean up."""
        self.temp_dir.cleanup()

    def _create_valid_elf64_header(self) -> bytearray:
        """Create a valid 64-bit little-endian ARM64 ELF header."""
        header = bytearray(64)

        # ELF magic bytes
        header[0:4] = b"\x7fELF"

        # EI_CLASS: 64-bit (2)
        header[4] = 2

        # EI_DATA: little-endian (1)
        header[5] = 1

        # EI_VERSION: current (1)
        header[6] = 1

        # EI_OSABI: System V (0)
        header[7] = 0

        # e_type: ET_DYN (shared object, 3)
        header[16:18] = (3).to_bytes(2, byteorder="little")

        # e_machine: EM_AARCH64 (ARM64, 0xB7 = 183)
        header[18:20] = (0xB7).to_bytes(2, byteorder="little")

        # e_version
        header[20:24] = (1).to_bytes(4, byteorder="little")

        # e_entry (minimal)
        header[24:32] = (0x1000).to_bytes(8, byteorder="little")

        # e_phoff (program header offset)
        header[32:40] = (64).to_bytes(8, byteorder="little")

        # e_shoff (section header offset)
        header[40:48] = (0x1000).to_bytes(8, byteorder="little")

        # e_flags
        header[48:52] = (0).to_bytes(4, byteorder="little")

        # e_ehsize (ELF header size)
        header[52:54] = (64).to_bytes(2, byteorder="little")

        # e_phentsize (program header entry size)
        header[54:56] = (56).to_bytes(2, byteorder="little")

        # e_phnum (number of program headers)
        header[56:58] = (1).to_bytes(2, byteorder="little")

        # e_shentsize (section header entry size)
        header[58:60] = (64).to_bytes(2, byteorder="little")

        # e_shnum (number of section headers)
        header[60:62] = (5).to_bytes(2, byteorder="little")

        # e_shstrndx (section header string table index)
        header[62:64] = (1).to_bytes(2, byteorder="little")

        return header

    def test_valid_elf64_arm64(self):
        """Valid 64-bit ARM64 ELF should PASS validation."""
        header = self._create_valid_elf64_header()

        # Create minimal ELF file with section headers
        elf_file = self.temp_path / "valid.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            # Add padding to reach section header offset
            f.write(b"\x00" * (0x1000 - 64))
            # Add minimal section headers
            f.write(b"\x00" * 320)  # 5 sections * 64 bytes each

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertTrue(result)

        # Check evidence
        atlas = builder.atlas
        l3_closure = [c for c in atlas["closures"] if c["gap_id"] == "L3"]
        self.assertEqual(len(l3_closure), 1)

        evidence = l3_closure[0]["evidence"]
        self.assertTrue(evidence["elf_magic_valid"])
        self.assertTrue(evidence["is_arm64"])
        self.assertTrue(evidence["is_64bit"])
        self.assertTrue(evidence["is_little_endian"])
        self.assertTrue(evidence["endianness_valid"])
        self.assertEqual(l3_closure[0]["status"], "PASS")

    def test_corrupted_elf_magic(self):
        """Corrupted ELF magic bytes should FAIL."""
        header = self._create_valid_elf64_header()

        # Corrupt magic bytes
        header[0:4] = b"XXXX"

        elf_file = self.temp_path / "corrupted_magic.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertFalse(result)

    def test_big_endian_rejected(self):
        """Big-endian ELF should be rejected for ARM64 little-endian target."""
        header = self._create_valid_elf64_header()

        # Change to big-endian
        header[5] = 2

        elf_file = self.temp_path / "big_endian.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertFalse(result)

        # Check that endianness validation failed
        atlas = builder.atlas
        l3_closure = [c for c in atlas["closures"] if c["gap_id"] == "L3"]
        evidence = l3_closure[0]["evidence"]
        self.assertFalse(evidence["endianness_valid"])

    def test_wrong_machine_type(self):
        """Wrong machine type should FAIL."""
        header = self._create_valid_elf64_header()

        # Change machine type to x86_64 (0x3E)
        header[18:20] = (0x3E).to_bytes(2, byteorder="little")

        elf_file = self.temp_path / "wrong_machine.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertFalse(result)

    def test_32bit_elf_rejected(self):
        """32-bit ELF should be rejected."""
        header = self._create_valid_elf64_header()

        # Change class to 32-bit
        header[4] = 1

        elf_file = self.temp_path / "32bit.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertFalse(result)

    def test_missing_file(self):
        """Missing ELF file should return TOKEN_VAZIO."""
        elf_file = self.temp_path / "nonexistent.so"

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        result = builder.close_l3_elf_validation(elf_file)

        self.assertFalse(result)

        # Check closure status
        atlas = builder.atlas
        l3_closure = [c for c in atlas["closures"] if c["gap_id"] == "L3"]
        self.assertEqual(l3_closure[0]["status"], "TOKEN_VAZIO")

    def test_symbol_table_validation(self):
        """Symbol table validation should work correctly."""
        header = self._create_valid_elf64_header()

        elf_file = self.temp_path / "with_symtab.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")

        # Test symbol table validation
        with open(elf_file, "rb") as f:
            f.seek(0)
            valid = builder._validate_elf_symbol_table(f, header)
            self.assertTrue(valid)

    def test_relocation_validation(self):
        """Relocation entries validation should work correctly."""
        header = self._create_valid_elf64_header()

        elf_file = self.temp_path / "with_relocs.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")

        # Test relocation validation
        with open(elf_file, "rb") as f:
            f.seek(0)
            valid = builder._validate_elf_relocations(f, header)
            self.assertTrue(valid)

    def test_evidence_recorded(self):
        """Evidence should be properly recorded in atlas."""
        header = self._create_valid_elf64_header()

        elf_file = self.temp_path / "with_evidence.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas.json")
        builder.close_l3_elf_validation(elf_file)

        # Write and read atlas
        atlas_path = builder.write_atlas()
        atlas_json = json.loads(atlas_path.read_text())

        # Verify evidence chain
        evidence_chain = atlas_json["evidence_chain"]
        self.assertGreater(len(evidence_chain), 0)

        elf_evidence = [e for e in evidence_chain if e["type"] == "elf_validation"]
        self.assertEqual(len(elf_evidence), 1)

        evidence = elf_evidence[0]["data"]
        self.assertIn("elf_magic_valid", evidence)
        self.assertIn("is_arm64", evidence)
        self.assertIn("endianness_valid", evidence)
        self.assertIn("symbol_table_valid", evidence)
        self.assertIn("relocations_valid", evidence)

    def test_hash_consistency(self):
        """File hash should be consistent across validations."""
        header = self._create_valid_elf64_header()

        elf_file = self.temp_path / "consistent.so"
        with open(elf_file, "wb") as f:
            f.write(header)
            f.write(b"\x00" * 0x1000)

        builder1 = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas1.json")
        builder1.close_l3_elf_validation(elf_file)

        builder2 = GapAtlasBuilder(self.temp_path, self.temp_path / "atlas2.json")
        builder2.close_l3_elf_validation(elf_file)

        # Extract hashes from both builders
        closure1 = builder1.atlas["closures"][0]["evidence"]["file_hash"]
        closure2 = builder2.atlas["closures"][0]["evidence"]["file_hash"]

        self.assertEqual(closure1, closure2)


if __name__ == "__main__":
    unittest.main()
