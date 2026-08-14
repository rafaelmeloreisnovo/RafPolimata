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
    "audit_browser_tls_capability", ROOT / "scripts/audit_browser_tls_capability.py"
)
assert SPEC and SPEC.loader
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)


class BrowserTLSCapabilityTests(unittest.TestCase):
    def make_config(self, root: Path) -> Path:
        config = {
            "capability_levels": {
                "TUI_FILE_BROWSER": [
                    "local_directory_listing", "keyboard_navigation", "terminal_rendering"
                ],
                "HTTPS_TRANSPORT_ADAPTER": [
                    "dns_resolution", "tcp_transport", "https_only_policy",
                    "tls_1_2_request", "tls_1_3_request", "system_ca_validation",
                    "hostname_verification", "redirect_limit", "timeout",
                    "atomic_output", "evidence_report"
                ],
                "WEB_BROWSER_TLS": [
                    "url_parser", "dns_resolution", "tcp_transport", "http_response_parser",
                    "https_transport", "tls_1_2_handshake", "tls_1_3_handshake",
                    "x509_chain_validation", "hostname_verification",
                    "certificate_time_validation", "trusted_root_store", "redirect_policy",
                    "content_renderer", "navigation_history", "runtime_integration_test"
                ],
                "ASM_WEB_BROWSER_TLS": [
                    "web_browser_tls", "assembly_source_core", "abi_contract",
                    "memory_bounds_evidence", "crypto_known_answer_tests",
                    "tls_transcript_tests", "certificate_chain_fixtures",
                    "independent_security_review"
                ]
            }
        }
        path = root / "config.json"
        path.write_text(json.dumps(config), encoding="utf-8")
        return path

    def write_fixture(self, root: Path, insecure_line: str | None = None) -> None:
        (root / "raf_shell").mkdir(exist_ok=True)
        (root / "scripts").mkdir(exist_ok=True)
        (root / "raf_shell/raf_shell.c").write_text(
            "VT100 terminal UI TUI file browser dirbrowse TAB=Panel ENTER=Run",
            encoding="utf-8",
        )
        active_insecure = f"{insecure_line}\n" if insecure_line else ""
        (root / "scripts/raf_https_fetch.sh").write_text(
            """#!/bin/sh
curl --proto '=https' --proto-redir '=https' --tlsv1.2 --tlsv1.3 \\
  --tls-max 1.3 --max-redirs 5 --connect-timeout 10 --max-time 60
# No -k/--insecure: documentation only.
""" + active_insecure + """certificate_and_hostname_validation_enabled
remote_ip ssl_verify_result
mv -f candidate output
raf.https-fetch-evidence.v1
""",
            encoding="utf-8",
        )

    def test_tui_and_https_adapter_do_not_become_web_browser(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            self.write_fixture(root)
            report = MOD.audit(root, self.make_config(root))
            adapter = report["levels"]["HTTPS_TRANSPORT_ADAPTER"]
            self.assertTrue(report["facts"]["tui_file_browser_static_evidence"])
            self.assertTrue(report["facts"]["https_transport_adapter_static_evidence"])
            self.assertFalse(report["facts"]["web_browser_tls_static_evidence"])
            self.assertFalse(report["facts"]["asm_web_browser_tls_static_evidence"])
            self.assertFalse(adapter["unsafe_option_active"])
            self.assertTrue(adapter["static_contract"]["system_ca_validation"])
            self.assertEqual(report["truth"]["web_browser_tls"], "TOKEN_VAZIO")
            self.assertEqual(report["truth"]["certified_tls"], "TOKEN_VAZIO")
            self.assertFalse(report["claim_allowed"])

    def assert_active_insecure_fails(self, line: str) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            self.write_fixture(root, line)
            report = MOD.audit(root, self.make_config(root))
            adapter = report["levels"]["HTTPS_TRANSPORT_ADAPTER"]
            self.assertTrue(adapter["unsafe_option_active"])
            self.assertFalse(adapter["static_contract"]["system_ca_validation"])
            self.assertIn("system_ca_validation", adapter["missing"])
            self.assertFalse(report["facts"]["https_transport_adapter_static_evidence"])
            self.assertEqual(report["truth"]["https_adapter_classification"], "TOKEN_VAZIO")
            self.assertEqual(report["truth"]["certified_tls"], "TOKEN_VAZIO")
            self.assertFalse(report["claim_allowed"])

    def test_active_short_insecure_flag_fails_closed(self) -> None:
        self.assert_active_insecure_fails("curl -k https://example.invalid")

    def test_active_long_insecure_flag_fails_closed(self) -> None:
        self.assert_active_insecure_fails("curl --insecure https://example.invalid")

    def test_comment_alone_never_counts_as_active_insecure_option(self) -> None:
        text = "# No -k/--insecure\ncurl --proto '=https' https://example.invalid\n"
        self.assertFalse(MOD.has_insecure_curl_flag(MOD.executable_shell_text(text)))
        self.assertFalse(MOD.active_shell_option(text, r"-k|--insecure"))
        self.assertTrue(MOD.active_shell_option("  --insecure \\\n", r"-k|--insecure"))
        self.assertTrue(MOD.active_shell_option("  -k \\\n", r"-k|--insecure"))

    def test_empty_tree_is_token_vazio(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            report = MOD.audit(root, self.make_config(root))
            self.assertEqual(report["state"], "TOKEN_VAZIO")
            self.assertEqual(report["levels"]["TUI_FILE_BROWSER"]["state"], "ABSENT")
            self.assertEqual(report["levels"]["HTTPS_TRANSPORT_ADAPTER"]["state"], "ABSENT")


if __name__ == "__main__":
    unittest.main()
