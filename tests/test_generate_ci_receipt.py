#!/usr/bin/env python3
"""
Tests for CI Receipt Generator (Hotfix H2)
"""

import json
import os
import tempfile
import unittest
from pathlib import Path
from sys import path as sys_path
import subprocess

# Add tools to path
sys_path.insert(0, str(Path(__file__).parent.parent / "tools"))

from generate_ci_receipt import (
    CIReceipt,
    ExecutedCommand,
    CommandExecutor,
    create_receipt_from_env,
)


class TestCIReceipt(unittest.TestCase):
    """Test CI receipt data structure."""

    def test_receipt_creation(self):
        """Receipt should have required schema."""
        receipt = CIReceipt(job_id="test-job", job_name="Test Job")
        self.assertEqual(receipt.schema, "rafaelia.ci_receipt.v1")
        self.assertEqual(receipt.job_id, "test-job")
        self.assertEqual(receipt.status, "PENDING")

    def test_receipt_add_command(self):
        """Receipt should accumulate commands."""
        receipt = CIReceipt(job_id="test")

        cmd1 = ExecutedCommand(
            command="echo hello",
            exit_code=0,
            working_directory="/tmp",
            environment_keys=["PATH", "HOME"],
            stdout_hash="abc123",
            stdout_size=6,
            stderr_hash="",
            stderr_size=0,
            execution_time_ms=10.5,
        )
        receipt.add_command(cmd1)

        self.assertEqual(len(receipt.commands), 1)
        self.assertEqual(receipt.commands[0].command, "echo hello")

    def test_receipt_add_artifact(self):
        """Receipt should record artifacts."""
        receipt = CIReceipt(job_id="test")
        receipt.add_artifact("output.json", "hash123", 1024)

        self.assertEqual(len(receipt.artifacts), 1)
        self.assertEqual(receipt.artifacts[0]["path"], "output.json")
        self.assertEqual(receipt.artifacts[0]["size"], 1024)

    def test_receipt_to_dict(self):
        """Receipt should serialize to dictionary."""
        receipt = CIReceipt(
            job_id="test-job",
            job_name="Test",
            github_sha="abc123",
            runner_os="Linux",
        )
        receipt.add_artifact("test.txt", "hash", 100)

        receipt_dict = receipt.to_dict()

        self.assertIn("schema", receipt_dict)
        self.assertIn("job_id", receipt_dict)
        self.assertIn("artifacts", receipt_dict)
        self.assertEqual(receipt_dict["environment"]["os"], "Linux")

    def test_receipt_hash_determinism(self):
        """Receipt hash should be deterministic."""
        receipt1 = CIReceipt(job_id="test", github_sha="abc")
        receipt1.add_artifact("file.txt", "hash", 100)

        hash1 = receipt1.compute_hash()

        receipt2 = CIReceipt(job_id="test", github_sha="abc")
        receipt2.add_artifact("file.txt", "hash", 100)

        hash2 = receipt2.compute_hash()

        self.assertEqual(hash1, hash2)

    def test_receipt_hash_changes_with_content(self):
        """Receipt hash should change if content differs."""
        receipt1 = CIReceipt(job_id="test1", github_sha="abc")
        hash1 = receipt1.compute_hash()

        receipt2 = CIReceipt(job_id="test2", github_sha="abc")
        hash2 = receipt2.compute_hash()

        self.assertNotEqual(hash1, hash2)


class TestCommandExecutor(unittest.TestCase):
    """Test command execution and recording."""

    def setUp(self):
        """Create temporary directory for tests."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_path = Path(self.temp_dir.name)

    def tearDown(self):
        """Clean up."""
        self.temp_dir.cleanup()

    def test_run_successful_command(self):
        """Executor should record successful command."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        exit_code = executor.run_command("echo 'hello world'")

        self.assertEqual(exit_code, 0)
        self.assertEqual(len(receipt.commands), 1)
        self.assertEqual(receipt.commands[0].command, "echo 'hello world'")
        self.assertEqual(receipt.commands[0].exit_code, 0)
        self.assertGreater(receipt.commands[0].stdout_size, 0)

    def test_run_failing_command(self):
        """Executor should record failed command."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        exit_code = executor.run_command("false")

        self.assertNotEqual(exit_code, 0)
        self.assertEqual(receipt.commands[0].exit_code, exit_code)

    def test_record_artifact(self):
        """Executor should record file artifacts."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        # Create test file
        test_file = self.temp_path / "test.txt"
        test_file.write_text("test content")

        executor.record_artifact(str(test_file))

        self.assertEqual(len(receipt.artifacts), 1)
        self.assertGreater(receipt.artifacts[0]["size"], 0)

    def test_finalize_receipt(self):
        """Finalize should set status based on exit codes."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        executor.run_command("echo ok")
        executor.run_command("true")

        receipt_dict = executor.finalize()

        self.assertEqual(receipt_dict["status"], "PASS")
        self.assertEqual(receipt_dict["overall_exit_code"], 0)
        self.assertIn("receipt_hash", receipt_dict)
        self.assertNotEqual(receipt_dict["receipt_hash"], "")

    def test_finalize_with_failures(self):
        """Finalize should mark FAIL if any command fails."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        executor.run_command("true")
        executor.run_command("false")

        receipt_dict = executor.finalize()

        self.assertEqual(receipt_dict["status"], "FAIL")
        self.assertNotEqual(receipt_dict["overall_exit_code"], 0)


class TestReceiptIntegration(unittest.TestCase):
    """Integration tests for receipt generation."""

    def setUp(self):
        """Create temporary directory."""
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_path = Path(self.temp_dir.name)

    def tearDown(self):
        """Clean up."""
        self.temp_dir.cleanup()

    def test_receipt_json_serialization(self):
        """Receipt should be JSON-serializable."""
        receipt = CIReceipt(
            job_id="test",
            job_name="Test Job",
            github_sha="abc123",
            runner_os="Linux",
        )
        receipt.add_artifact("output.json", "hash", 1024)

        executor = CommandExecutor(receipt)
        receipt_dict = executor.finalize()

        # Should be serializable
        json_str = json.dumps(receipt_dict, indent=2)
        self.assertIsInstance(json_str, str)
        self.assertGreater(len(json_str), 0)

        # Should be deserializable
        restored = json.loads(json_str)
        self.assertEqual(restored["job_id"], "test")
        self.assertEqual(len(restored["artifacts"]), 1)

    def test_receipt_file_output(self):
        """Receipt should be writable to file."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        executor.run_command("echo test")

        output_file = self.temp_path / "receipt.json"
        receipt_dict = executor.finalize()

        output_file.write_text(json.dumps(receipt_dict, indent=2))

        self.assertTrue(output_file.exists())
        content = json.loads(output_file.read_text())
        self.assertEqual(content["schema"], "rafaelia.ci_receipt.v1")

    def test_multi_command_receipt(self):
        """Receipt should track multiple commands with distinct hashes."""
        receipt = CIReceipt(job_id="test")
        executor = CommandExecutor(receipt)

        executor.run_command("echo first")
        executor.run_command("echo second")
        executor.run_command("echo third")

        self.assertEqual(len(receipt.commands), 3)

        # Each command should have its own hashes
        for cmd in receipt.commands:
            self.assertGreater(len(cmd.stdout_hash), 0)

    def test_create_receipt_from_env(self):
        """Should create receipt from environment variables."""
        # Set some test env vars
        test_env = {
            "GITHUB_JOB": "test-job",
            "GITHUB_SHA": "abc123def",
            "RUNNER_OS": "Linux",
        }

        for key, val in test_env.items():
            os.environ[key] = val

        receipt = create_receipt_from_env()

        self.assertEqual(receipt.job_id, "test-job")
        self.assertEqual(receipt.github_sha, "abc123def")
        self.assertEqual(receipt.runner_os, "Linux")

        # Clean up
        for key in test_env:
            del os.environ[key]


class TestExecutedCommand(unittest.TestCase):
    """Test ExecutedCommand data structure."""

    def test_command_record_creation(self):
        """Should create command record with all fields."""
        cmd = ExecutedCommand(
            command="test-cmd",
            exit_code=0,
            working_directory="/tmp",
            environment_keys=["PATH"],
            stdout_hash="abc",
            stdout_size=100,
            stderr_hash="def",
            stderr_size=0,
            execution_time_ms=15.5,
        )

        self.assertEqual(cmd.command, "test-cmd")
        self.assertEqual(cmd.exit_code, 0)
        self.assertEqual(cmd.execution_time_ms, 15.5)

    def test_command_to_dict(self):
        """Command should serialize to dict."""
        cmd = ExecutedCommand(
            command="test",
            exit_code=1,
            working_directory="/home",
            environment_keys=["A", "B"],
            stdout_hash="hash",
            stdout_size=50,
            stderr_hash="err",
            stderr_size=10,
            execution_time_ms=20.0,
        )

        cmd_dict = {
            "command": "test",
            "exit_code": 1,
            "working_directory": "/home",
            "environment_keys": ["A", "B"],
            "stdout_hash": "hash",
            "stdout_size": 50,
            "stderr_hash": "err",
            "stderr_size": 10,
            "execution_time_ms": 20.0,
        }

        self.assertEqual(cmd.__dict__, cmd_dict)


if __name__ == "__main__":
    unittest.main()
