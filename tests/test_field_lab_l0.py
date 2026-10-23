"""Exercise L0 binary on host, cross-validate SHA3/ZIP/WAV with stdlib oracles."""
import ctypes
import hashlib
import io
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
import wave
import zipfile

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/"field_lab/l0/raf_field_lab_core.c"
U8=ctypes.c_ubyte
U32=ctypes.c_uint
I16=ctypes.c_short


class LabL0(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory()
        lib=Path(cls.tmp.name)/"lab_host.so"
        subprocess.run(["clang","-std=c11","-O2","-Wall","-Wextra","-Werror",
                        "-ffreestanding","-fno-builtin","-nostdinc","-nostdlib",
                        "-fPIC","-shared","-Wl,--no-undefined",str(SRC),"-o",str(lib)],
                       check=True,capture_output=True)
        cls.lib=ctypes.CDLL(str(lib))
        cls.lib.raf_lab_sha3_256.restype=ctypes.c_int
        cls.lib.raf_lab_pcm16_48k_square.restype=ctypes.c_int
        cls.lib.raf_lab_wav48k_header.restype=ctypes.c_int
        cls.lib.raf_lab_spectrum8_dft16.restype=ctypes.c_int
        cls.lib.raf_lab_zip32_one.restype=ctypes.c_int

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def digest(self,raw):
        arr=(U8*len(raw)).from_buffer_copy(raw)
        out=(U8*32)()
        self.assertEqual(self.lib.raf_lab_sha3_256(arr,len(raw),out),1)
        return bytes(out)

    def test_sha3_256_empty_reference(self):
        self.assertEqual(self.digest(b""),hashlib.sha3_256(b"").digest())

    def test_sha3_256_abc_reference(self):
        self.assertEqual(self.digest(b"abc"),hashlib.sha3_256(b"abc").digest())

    def test_sha3_256_multiblock_reference(self):
        v=bytes(range(256))*4
        self.assertEqual(self.digest(v),hashlib.sha3_256(v).digest())

    def test_sha3_256_null_nonempty_and_bounds_fail_closed(self):
        out=(U8*32)(*([0x7b]*32))
        self.assertEqual(self.lib.raf_lab_sha3_256(None,1,out),0)
        self.assertEqual(bytes(out),b"\x7b"*32)
        self.assertEqual(self.lib.raf_lab_sha3_256(None,1048577,out),0)

    def test_pcm48k_window_and_state(self):
        buf=(I16*512)()
        phase=U32(0)
        self.assertEqual(self.lib.raf_lab_pcm16_48k_square(buf,512,ctypes.byref(phase)),1)
        self.assertTrue(all(x in (-12000,12000) for x in buf))
        self.assertEqual(phase.value,(440*512)%48000)
        self.assertEqual(self.lib.raf_lab_pcm16_48k_square(buf,48001,ctypes.byref(phase)),0)

    def test_wav_48k_reference_reader(self):
        out=(U8*44)()
        self.assertEqual(self.lib.raf_lab_wav48k_header(out,44,4),1)
        wav=bytes(out)+(b"\0\0"*4)
        with wave.open(io.BytesIO(wav),"rb") as w:
            self.assertEqual((w.getframerate(),w.getnchannels(),w.getsampwidth(),w.getnframes()),(48000,1,2,4))
        self.assertEqual(self.lib.raf_lab_wav48k_header(out,43,4),0)

    def test_dft16_zero_and_impulse(self):
        inp=(I16*16)()
        o=(U32*8)()
        self.assertEqual(self.lib.raf_lab_spectrum8_dft16(inp,o),1)
        self.assertEqual(list(o),[0]*8)
        inp[0]=1000
        self.assertEqual(self.lib.raf_lab_spectrum8_dft16(inp,o),1)
        self.assertEqual(list(o),[1000]*8)

    def test_zip32_store_round_trip_and_path_guard(self):
        name=b"capture.wav"
        content=b"RIFF"+bytes(range(64))
        n=(U8*len(name)).from_buffer_copy(name)
        payload=(U8*len(content)).from_buffer_copy(content)
        out=(U8*1024)()
        wrote=U32(0)
        self.assertEqual(self.lib.raf_lab_zip32_one(n,len(name),payload,len(content),out,1024,ctypes.byref(wrote)),1)
        with zipfile.ZipFile(io.BytesIO(bytes(out)[:wrote.value])) as z:
            self.assertEqual(z.namelist(),["capture.wav"])
            self.assertEqual(z.read("capture.wav"),content)
        traversal=b"../unsafe"
        bad=(U8*len(traversal)).from_buffer_copy(traversal)
        before=wrote.value
        self.assertEqual(self.lib.raf_lab_zip32_one(bad,len(traversal),payload,len(content),out,1024,ctypes.byref(wrote)),0)
        self.assertEqual(wrote.value,before)

    def test_zip32_short_buffer_no_modification(self):
        name=(U8*4)(*b"a.wav"[:4])
        data=(U8*3)(1,2,3)
        out=(U8*16)(*([77]*16))
        w=U32(33)
        self.assertEqual(self.lib.raf_lab_zip32_one(name,4,data,3,out,16,ctypes.byref(w)),0)
        self.assertEqual(bytes(out),bytes([77]*16))
        self.assertEqual(w.value,33)


if __name__=="__main__":
    unittest.main()
