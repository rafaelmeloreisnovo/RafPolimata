#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "ecosystem_build_doctor", ROOT / "scripts/ecosystem_build_doctor.py"
)
assert SPEC and SPEC.loader
EBD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = EBD
SPEC.loader.exec_module(EBD)


class EcosystemBuildDoctorTests(unittest.TestCase):
    def make_repo(self, files: dict[str, str | bytes]) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        td = tempfile.TemporaryDirectory()
        root = Path(td.name)
        for relative, payload in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            if isinstance(payload, bytes):
                path.write_bytes(payload)
            else:
                path.write_text(payload, encoding="utf-8")
        return td, root

    def codes(self, root: Path) -> set[str]:
        findings = EBD.analyze_repo(EBD.RepoTarget("fixture", root), 1_000_000)
        return {item.code for item in findings}

    def test_c_only_project_rejects_cxx_flags_and_static_link_contract(self) -> None:
        td, root = self.make_repo(
            {
                "CMakeLists.txt": """
                    cmake_minimum_required(VERSION 3.22)
                    project(sample C ASM)
                    add_library(core STATIC src/core.c)
                    target_compile_options(core PRIVATE -O2 -O3 -fno-rtti -fno-exceptions)
                    target_link_options(core PRIVATE -nostdlib -Wl,--gc-sections)
                """,
                "src/core.c": "int core(void) { return 1; }\n",
            }
        )
        try:
            codes = self.codes(root)
            self.assertIn("conflicting_optimization_flags", codes)
            self.assertIn("cxx_only_flag_in_c_project", codes)
            self.assertIn("link_options_on_static_library", codes)
        finally:
            td.cleanup()

    def test_dead_cmake_variable_candidate(self) -> None:
        td, root = self.make_repo(
            {
                "CMakeLists.txt": "project(sample C)\nset(UNUSED_PLATFORM_FLAGS -O2)\n",
            }
        )
        try:
            findings = EBD.analyze_repo(EBD.RepoTarget("fixture", root), 1_000_000)
            dead = [item for item in findings if item.code == "dead_cmake_variable_candidate"]
            self.assertEqual([item.evidence for item in dead], ["UNUSED_PLATFORM_FLAGS"])
        finally:
            td.cleanup()

    def test_zombie_source_is_reported_but_manifested_source_is_not(self) -> None:
        td, root = self.make_repo(
            {
                "CMakeLists.txt": "project(sample C)\nadd_library(core src/live.c)\n",
                "src/live.c": "int live(void) { return 1; }\n",
                "src/zombie.c": "int zombie(void) { return 0; }\n",
            }
        )
        try:
            findings = EBD.analyze_repo(EBD.RepoTarget("fixture", root), 1_000_000)
            zombies = [item.path for item in findings if item.code == "zombie_source_candidate"]
            self.assertEqual(zombies, ["src/zombie.c"])
        finally:
            td.cleanup()

    def test_binary_without_provenance_and_sidecar_exception(self) -> None:
        td, root = self.make_repo(
            {
                "artifacts/untracked.apk": b"APK",
                "artifacts/tracked.apk": b"APK",
                "artifacts/tracked.apk.sha256": "deadbeef  tracked.apk\n",
            }
        )
        try:
            findings = EBD.analyze_repo(EBD.RepoTarget("fixture", root), 1_000_000)
            binaries = [item.path for item in findings if item.code == "binary_without_provenance"]
            self.assertEqual(binaries, ["artifacts/untracked.apk"])
        finally:
            td.cleanup()

    def test_warning_and_shell_failure_masks_are_visible(self) -> None:
        td, root = self.make_repo(
            {
                ".github/workflows/ci.yml": """
                    jobs:
                      build:
                        continue-on-error: true
                        steps:
                          - run: cc -Wno-error source.c || true
                """,
            }
        )
        try:
            codes = self.codes(root)
            self.assertIn("ci_continue_on_error", codes)
            self.assertIn("warnings_not_blocking", codes)
            self.assertIn("shell_failure_masked", codes)
        finally:
            td.cleanup()

    def test_linker_log_becomes_high_priority_instruction(self) -> None:
        td, root = self.make_repo(
            {
                "logs/link.log": "ld.lld: error: undefined reference to rmr_missing_symbol\n",
            }
        )
        try:
            findings = EBD.analyze_repo(EBD.RepoTarget("fixture", root), 1_000_000)
            linker = [item for item in findings if item.code == "linker_diagnostic_recorded"]
            self.assertTrue(linker)
            self.assertTrue(all(item.severity == "high" for item in linker))
        finally:
            td.cleanup()

    def test_source_markers_are_not_promoted_to_implementation(self) -> None:
        td, root = self.make_repo(
            {
                "CMakeLists.txt": "project(sample C)\nadd_library(core src/core.c)\n",
                "src/core.c": "/* TOKEN_VAZIO: runtime bridge */\nint core(void) { return 0; }\n",
            }
        )
        try:
            codes = self.codes(root)
            self.assertIn("executable_source_marker", codes)
        finally:
            td.cleanup()

    def test_report_is_deterministic_and_json_serializable(self) -> None:
        td, root = self.make_repo(
            {
                "CMakeLists.txt": "project(sample C)\nadd_library(core src/core.c)\n",
                "src/core.c": "int core(void) { return 0; }\n",
            }
        )
        try:
            targets = [EBD.RepoTarget("fixture", root)]
            report_a = EBD.build_report(targets, 1_000_000)
            report_b = EBD.build_report(targets, 1_000_000)
            self.assertEqual(report_a, report_b)
            self.assertEqual(
                json.dumps(report_a, sort_keys=True),
                json.dumps(report_b, sort_keys=True),
            )
            self.assertIn(report_a["summary"]["state"], {"PASS", "PASS_LIMITED", "REVIEW_REQUIRED"})
        finally:
            td.cleanup()

    def test_fail_threshold(self) -> None:
        findings = [
            {"severity": "low"},
            {"severity": "high"},
        ]
        self.assertFalse(EBD.should_fail(findings, "critical"))
        self.assertTrue(EBD.should_fail(findings, "high"))
        self.assertTrue(EBD.should_fail(findings, "medium"))
        self.assertFalse(EBD.should_fail(findings, "none"))


if __name__ == "__main__":
    unittest.main()
