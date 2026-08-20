from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.validate_android_proof_bundle import TV, validate

HEX64 = "a" * 64
SHA40 = "b" * 40


def template():
    return {
        "schema": "rafpolimata.android_proof_chain.v1",
        "status": "TEST",
        "date_utc": TV,
        "git_commit": TV,
        "artifacts": {
            "apkc_binary": {"sha256": TV, "compile_log": "01_compile_apkc.txt"},
            "apk": {"sha256": TV, "generate_log": "02_generate_apk.txt", "zip_list": "03_unzip_list.txt"},
            "arm64_elf": {"readelf_log": "05_readelf_arm64.txt", "status": TV},
            "install": {"log": "08_adb_install.txt", "status": TV},
            "launch": {"log": "09_launch.txt", "status": TV},
            "logcat": {"log": "10_logcat_nativeactivity.txt", "fatal_exception": TV, "dlopen_failure": TV, "status": TV}
        },
        "promotion": {
            "source_to_binary": TV,
            "binary_to_apk": TV,
            "arm64_real": TV,
            "android_runtime": TV,
            "single_run_reproducibility": TV
        }
    }


class ProofBundleTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.manifest = self.root / "manifest.json"

    def tearDown(self):
        self.temp.cleanup()

    def write(self, data):
        self.manifest.write_text(json.dumps(data), encoding="utf-8")

    def host_chain(self):
        data = template()
        data["git_commit"] = SHA40
        data["artifacts"]["apkc_binary"]["sha256"] = HEX64
        data["artifacts"]["apk"]["sha256"] = "c" * 64
        (self.root / "01_compile_apkc.txt").write_text("compile PASS")
        (self.root / "02_generate_apk.txt").write_text("generate PASS")
        (self.root / "03_unzip_list.txt").write_text("AndroidManifest.xml\nclasses.dex\nlib/armeabi-v7a/libhello.so\n")
        return data

    def full_chain(self):
        data = self.host_chain()
        data["date_utc"] = "2026-08-20T12:00:00Z"
        data["artifacts"]["arm64_elf"]["status"] = "PASS"
        (self.root / "05_readelf_arm64.txt").write_text("Class: ELF64\nMachine: AArch64\n")
        data["artifacts"]["install"] = {"log": "08_adb_install.txt", "status": "PASS"}
        data["artifacts"]["launch"] = {"log": "09_launch.txt", "status": "PASS"}
        data["artifacts"]["logcat"] = {"log": "10_logcat_nativeactivity.txt", "fatal_exception": False, "dlopen_failure": False, "status": "PASS"}
        (self.root / "08_adb_install.txt").write_text("Success")
        (self.root / "09_launch.txt").write_text("launch ok")
        (self.root / "10_logcat_nativeactivity.txt").write_text("clean")
        for key in data["promotion"]:
            data["promotion"][key] = "PASS"
        return data

    def test_empty_template_is_token_vazio(self):
        self.write(template())
        self.assertEqual(validate(self.manifest)["status"], TV)

    def test_host_chain_is_pass_limited(self):
        self.write(self.host_chain())
        report = validate(self.manifest)
        self.assertEqual(report["status"], "PASS_LIMITED")
        self.assertFalse(report["claim_allowed"])

    def test_false_promotion_is_rejected(self):
        data = template()
        data["promotion"]["android_runtime"] = "PASS"
        self.write(data)
        self.assertEqual(validate(self.manifest)["status"], "FAIL")

    def test_full_chain_passes(self):
        self.write(self.full_chain())
        report = validate(self.manifest)
        self.assertEqual(report["status"], "PASS")
        self.assertTrue(report["claim_allowed"])

    def test_runtime_failure_is_fail(self):
        data = self.full_chain()
        data["artifacts"]["logcat"]["fatal_exception"] = True
        self.write(data)
        self.assertEqual(validate(self.manifest)["status"], "FAIL")

    def test_missing_required_log_rejects_claimed_pass(self):
        data = self.full_chain()
        (self.root / "08_adb_install.txt").unlink()
        self.write(data)
        self.assertEqual(validate(self.manifest)["status"], "FAIL")

    def test_bad_hash_fails(self):
        data = self.host_chain()
        data["artifacts"]["apk"]["sha256"] = "bad"
        self.write(data)
        self.assertEqual(validate(self.manifest)["status"], "FAIL")


if __name__ == "__main__":
    unittest.main()
