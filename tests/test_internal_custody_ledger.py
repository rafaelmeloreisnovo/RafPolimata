#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def load(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module

ICE = load("internal_custody_event", ROOT / "tools/internal_custody_event.py")
VIC = load("verify_internal_custody", ROOT / "tools/verify_internal_custody.py")

class InternalCustodyLedgerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.ledger = self.root / "custody.jsonl"

    def tearDown(self):
        self.tmp.cleanup()

    def record(self, **overrides):
        args = dict(ledger_path=self.ledger, repository="rafaelmeloreisnovo/RafPolimata", path="artifact.bin", symbol="unit_test", result="PASS", repo_root=self.root)
        args.update(overrides)
        event = ICE.record_event(**args)
        self.assertIsNotNone(event)
        return event

    def verify(self, **kwargs):
        verifier = VIC.LedgerVerifier(self.ledger, **kwargs)
        return verifier, verifier.verify_ledger()

    def test_genesis_and_manifest(self):
        event = self.record()
        self.assertEqual(event["parent_event_id"], ICE.GENESIS_PARENT_ID)
        manifest = json.loads(ICE.manifest_path_for(self.ledger).read_text())
        self.assertEqual(manifest["event_count"], 1)
        self.assertEqual(manifest["head_event_id"], event["event_id"])
        verifier, valid = self.verify()
        self.assertTrue(valid, verifier.report())

    def test_all_19_contract_fields_are_emitted(self):
        self.assertEqual(set(self.record()), set(VIC.REQUIRED_FIELDS))

    def test_canonical_serialization_utf8_and_key_order(self):
        raw1 = ICE.canonical_json_bytes({"z": "龍", "a": "∆Rafael"})
        raw2 = ICE.canonical_json_bytes({"a": "∆Rafael", "z": "龍"})
        self.assertEqual(raw1, raw2)
        self.assertTrue(raw1.endswith(b"\n"))
        self.assertIn("龍".encode("utf-8"), raw1)

    def test_event_id_is_deterministic_given_identical_event(self):
        event = {
            "event_version": "1.1.0", "parent_event_id": ICE.GENESIS_PARENT_ID,
            "timestamp_utc": "2026-08-15T12:00:00Z", "repository": "r",
            "commit_sha": "TOKEN_VAZIO", "path": "x", "blob_sha": "TOKEN_VAZIO",
            "symbol": "s", "toolchain": {"compiler":"TOKEN_VAZIO","linker":"TOKEN_VAZIO","platform":"linux"},
            "parameters": {"state":"TOKEN_VAZIO"}, "seed": "TOKEN_VAZIO", "environment": {},
            "input_sha256": "TOKEN_VAZIO", "output_sha256": "TOKEN_VAZIO",
            "stdout_hash": "TOKEN_VAZIO", "stderr_hash": "TOKEN_VAZIO", "exit_code": 0, "result": "PASS"
        }
        self.assertEqual(ICE.calculate_event_id(event, ICE.GENESIS_PARENT_ID), ICE.calculate_event_id(event, ICE.GENESIS_PARENT_ID))

    def test_one_byte_tamper_fails(self):
        self.record()
        self.ledger.write_text(self.ledger.read_text().replace('"result":"PASS"', '"result":"FAIL"', 1))
        verifier, valid = self.verify()
        self.assertFalse(valid)
        self.assertIn("event_id verification failed", verifier.report())

    def test_reordered_events_fail(self):
        self.record(symbol="first"); self.record(symbol="second")
        lines = self.ledger.read_text().splitlines()
        self.ledger.write_text(lines[1] + "\n" + lines[0] + "\n")
        self.assertFalse(self.verify()[1])

    def test_duplicate_event_fails(self):
        self.record(); line = self.ledger.read_text(); self.ledger.write_text(line + line)
        self.assertFalse(self.verify()[1])

    def test_terminal_event_removal_detected_by_manifest(self):
        self.record(symbol="first"); self.record(symbol="second")
        lines = self.ledger.read_text().splitlines(); self.ledger.write_text(lines[0] + "\n")
        verifier, valid = self.verify()
        self.assertFalse(valid)
        self.assertIn("Manifest event_count mismatch", verifier.report())

    def test_truncation_without_final_newline_fails(self):
        self.record(); self.ledger.write_bytes(self.ledger.read_bytes()[:-1])
        verifier, valid = self.verify()
        self.assertFalse(valid); self.assertIn("missing final newline", verifier.report())

    def test_missing_required_field_fails(self):
        self.record(); event = json.loads(self.ledger.read_text()); event.pop("seed")
        self.ledger.write_text(json.dumps(event, separators=(",", ":"), sort_keys=True) + "\n")
        verifier, valid = self.verify()
        self.assertFalse(valid); self.assertIn("missing required field 'seed'", verifier.report())

    def test_empty_file_hash_is_real_sha256(self):
        empty = self.root / "empty.bin"; empty.write_bytes(b"")
        self.assertEqual(ICE.sha256_file(empty), hashlib.sha256(b"").hexdigest())

    def test_streaming_hash_large_file(self):
        data = b"0123456789abcdef" * 131072
        large = self.root / "large.bin"; large.write_bytes(data)
        self.assertEqual(ICE.sha256_file(large), hashlib.sha256(data).hexdigest())

    def test_utf8_path_round_trip(self):
        path = "dados/龍_∆_φ.txt"; event = self.record(path=path)
        self.assertEqual(event["path"], path)
        verifier, valid = self.verify(); self.assertTrue(valid, verifier.report())

    def test_artifact_hash_verification(self):
        artifact = self.root / "artifact.bin"; artifact.write_bytes(b"output")
        self.record(path="artifact.bin", output_file=artifact)
        verifier, valid = self.verify(artifact_root=self.root); self.assertTrue(valid, verifier.report())
        artifact.write_bytes(b"tampered")
        verifier, valid = self.verify(artifact_root=self.root); self.assertFalse(valid)
        self.assertIn("artifact SHA-256 mismatch", verifier.report())

    def test_actual_concurrent_process_writes_are_serialized(self):
        writer = ROOT / "tools/internal_custody_event.py"; processes = []
        for idx in range(12):
            cmd = [sys.executable, str(writer), "--ledger", str(self.ledger), "--repository", "rafaelmeloreisnovo/RafPolimata", "--path", f"artifact-{idx}.bin", "--symbol", f"concurrent-{idx}", "--result", "PASS", "--repo-root", str(self.root)]
            processes.append(subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE))
        errors = []
        for process in processes:
            _, stderr = process.communicate(timeout=20)
            if process.returncode != 0: errors.append(stderr.decode("utf-8", "replace"))
        self.assertEqual(errors, [])
        self.assertEqual(len(self.ledger.read_text().splitlines()), 12)
        verifier, valid = self.verify(); self.assertTrue(valid, verifier.report())

    def test_invalid_existing_ledger_fails_closed_on_append(self):
        self.ledger.write_text('{"broken":')
        event = ICE.record_event(ledger_path=self.ledger, repository="r", path="x", symbol="s", result="PASS", repo_root=self.root)
        self.assertIsNone(event)

    def test_manifest_ledger_hash_detects_direct_rewrite(self):
        self.record(); old = json.loads(ICE.manifest_path_for(self.ledger).read_text())
        self.ledger.write_text(self.ledger.read_text().replace('"symbol":"unit_test"', '"symbol":"changed"'))
        verifier, valid = self.verify(); self.assertFalse(valid)
        self.assertNotEqual(old["ledger_sha256"], VIC.sha256_file(self.ledger))

if __name__ == "__main__": unittest.main()
