"""Repack negative tests: exact inventory, ABI identity, size and original bytes.

CLOSURE_L2 binds loader/runtime TOKEN_VAZIO states as unresolved physical evidence;
CLOSURE_L10 distinguishes unsigned structural APKs from verified signed APKs.
"""
import importlib.util
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
MOD=ROOT/"scripts/field_lab_repack.py"
spec=importlib.util.spec_from_file_location("field_lab_repack",MOD)
lab=importlib.util.module_from_spec(spec)
spec.loader.exec_module(lab)

def elf(cls,machine):
    b=bytearray(96)
    b[:4]=b"\x7fELF";b[4]=cls;b[5]=1;b[6]=1
    b[16:18]=(3).to_bytes(2,"little")
    b[18:20]=machine.to_bytes(2,"little")
    return bytes(b)

class ApkRepackTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.dir=Path(self.tmp.name)
        self.apk=self.dir/"before.apk"
        self.a32=self.dir/"a32.so"
        self.a64=self.dir/"a64.so"
        self.out=self.dir/"after.apk"
        self.receipt=self.dir/"receipt.json"
        self.a32.write_bytes(elf(1,40))
        self.a64.write_bytes(elf(2,183))
        with zipfile.ZipFile(self.apk,"w") as z:
            for name in sorted(lab.REQUIRED):
                z.writestr(name,b"original-byte-identity")

    def build(self):
        return lab.assemble(self.apk,self.a32,self.a64,self.out,self.receipt)

    def test_positive_boundaries_and_exact_bytes(self):
        r=self.build()
        self.assertFalse(r["claim_allowed"])
        self.assertEqual(r["loader_binding"],"TOKEN_VAZIO_NOT_CONNECTED")
        self.assertEqual(r["result"],"PASS_STRUCTURAL_MULTIMODULE_ONLY")
        with zipfile.ZipFile(self.out) as z:
            self.assertEqual(set(z.namelist()),lab.REQUIRED|set(lab.EXTRA))
            self.assertEqual(z.read("classes.dex"),b"original-byte-identity")

    def test_wrong_abi_machine_rejected(self):
        self.a32.write_bytes(elf(2,183))
        with self.assertRaisesRegex(ValueError,"class/endian"):
            self.build()
        self.assertFalse(self.out.exists())

    def test_corrupt_elf_rejected(self):
        self.a64.write_bytes(b"not-an-elf")
        with self.assertRaisesRegex(ValueError,"ELF source"):
            self.build()

    def test_duplicate_zip_name_rejected(self):
        with zipfile.ZipFile(self.apk,"a") as z:
            z.writestr("classes.dex",b"evil-duplicate")
        with self.assertRaisesRegex(ValueError,"unexpected original"):
            self.build()

    def test_unexpected_base_entry_rejected(self):
        with zipfile.ZipFile(self.apk,"a") as z:
            z.writestr("assets/private.txt",b"secret")
        with self.assertRaisesRegex(ValueError,"unexpected original"):
            self.build()

    def test_source_equals_target_rejected(self):
        with self.assertRaisesRegex(ValueError,"in-place"):
            lab.assemble(self.apk,self.a32,self.a64,self.apk,self.receipt)

    def test_size_ceiling_rejected(self):
        self.a64.write_bytes(elf(2,183)+b"x"*131073)
        with self.assertRaisesRegex(ValueError,"ceiling"):
            self.build()

if __name__=="__main__":
    unittest.main()
