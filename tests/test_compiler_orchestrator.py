from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "compiler_orchestrator.py"
SPEC = importlib.util.spec_from_file_location("compiler_orchestrator", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
compiler = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(compiler)


class CompilerOrchestratorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = compiler.load_json(ROOT / "compiler" / "targets.v1.json")

    def test_registry_is_fail_closed(self) -> None:
        compiler.validate_manifest(self.manifest)
        self.assertFalse(self.manifest["claim_allowed"])
        self.assertEqual(self.manifest["policy"]["install"], "FORBIDDEN")
        self.assertEqual(self.manifest["policy"]["launch"], "FORBIDDEN")

    def test_three_repository_families_are_present(self) -> None:
        targets = self.manifest["targets"]
        self.assertIn("rafpolimata-apkc-native", targets)
        self.assertIn("rafgittools-full-dev-debug", targets)
        self.assertIn("vectras-full-debug", targets)

    def test_full_targets_are_not_bootstrap_claims(self) -> None:
        targets = self.manifest["targets"]
        self.assertEqual(
            targets["rafgittools-full-dev-debug"]["level"],
            "FULL_ANDROID_APPLICATION",
        )
        self.assertEqual(
            targets["vectras-full-debug"]["level"],
            "FULL_ANDROID_APPLICATION",
        )
        self.assertEqual(
            targets["rafgittools-apkc-native"]["level"],
            "NATIVE_BOOTSTRAP_APK",
        )

    def test_install_task_is_rejected(self) -> None:
        with self.assertRaises(compiler.GateError):
            compiler.reject_forbidden_actions(
                ["./gradlew", ":app:installDebug"], "test"
            )

    def test_placeholder_expansion_is_argument_safe(self) -> None:
        command = compiler.expand_command(
            ["tool", "--out", "{out}", "--abi", "{abi}"],
            {"out": "/tmp/a path", "abi": "armeabi-v7a"},
        )
        self.assertEqual(command[2], "/tmp/a path")
        self.assertEqual(command[4], "armeabi-v7a")

    def test_apk_structure_validation(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            apk = Path(raw) / "sample.apk"
            with zipfile.ZipFile(apk, "w") as archive:
                archive.writestr("AndroidManifest.xml", b"axml")
                archive.writestr("classes.dex", b"dex\n035\0")
                archive.writestr("lib/armeabi-v7a/libsample.so", b"\x7fELF")
            result = compiler.validate_android_archive(apk)
            self.assertEqual(result["state"], "PASS")
            self.assertEqual(result["abis"], ["armeabi-v7a"])

    def test_apk_without_native_library_fails(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            apk = Path(raw) / "broken.apk"
            with zipfile.ZipFile(apk, "w") as archive:
                archive.writestr("AndroidManifest.xml", b"axml")
                archive.writestr("classes.dex", b"dex\n035\0")
            result = compiler.validate_android_archive(apk)
            self.assertEqual(result["state"], "FAIL")


if __name__ == "__main__":
    unittest.main()
