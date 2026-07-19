#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "apkc_first_part_gate", ROOT / "scripts/apkc_first_part_gate.py"
)
assert SPEC and SPEC.loader
GATE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GATE)


class ApkCFirstPartGateTests(unittest.TestCase):
    def test_artifact_state_distinguishes_empty_failure_and_pass(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            base = Path(td)
            token = base / "token.txt"
            failed = base / "failed.txt"
            passed = base / "passed.txt"
            token.write_text("TOKEN_VAZIO: não executado\n", encoding="utf-8")
            failed.write_text("gcc: error: unrecognized command-line option '-target'\n", encoding="utf-8")
            passed.write_text("[A64] STATUS: PASS\n", encoding="utf-8")

            self.assertEqual(GATE.artifact_state(token)[0], "TOKEN_VAZIO")
            self.assertEqual(GATE.artifact_state(failed)[0], "CONTRADICTION")
            self.assertEqual(GATE.artifact_state(passed)[0], "PASS")

    def test_parse_gap_claims(self) -> None:
        text = """
| Gap | Estado atual | Evidência |
|---|---|---|
| ELF ARM64 dentro do APK | **TOKEN_VAZIO** | ausente |
| DEX SHA-1 do APK atual | PASS | prova |
"""
        claims = GATE.parse_gap_claims(text)
        self.assertEqual(claims["elf arm64 dentro do apk"], "TOKEN_VAZIO")
        self.assertEqual(claims["dex sha-1 do apk atual"], "PASS")

    def test_browser_tls_requires_correlated_web_network_and_tls(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            src = root / "browser.s"
            src.write_text(
                '/* TLS 1.3 X509 */\n/* HTTP/1.1 */\n/* socket( connect( */\n.global _start\n',
                encoding="utf-8",
            )
            report = GATE.scan_browser_tls(root, [src])
            self.assertEqual(report["web_browser"]["state"], "IMPLEMENTED")
            self.assertEqual(report["asm_web_browser"]["state"], "IMPLEMENTED")
            self.assertEqual(report["tls_1_2_1_3_x509"]["state"], "IMPLEMENTED")

    def test_current_repository_code_gates_have_no_fail_state(self) -> None:
        checks = GATE.check_code_markers(ROOT)
        failed = [check.check_id for check in checks if check.state == "FAIL"]
        self.assertEqual(failed, [])

    def test_current_gap_document_does_not_promote_token_artifacts(self) -> None:
        _, contradictions = GATE.reconcile_proofs(ROOT)
        self.assertEqual(contradictions, [])


if __name__ == "__main__":
    unittest.main()
