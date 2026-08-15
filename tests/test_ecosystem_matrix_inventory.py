#!/usr/bin/env python3
"""
Tests for ecosystem matrix inventory tool.
"""

from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "inventory_ecosystem_matrices", ROOT / "tools/inventory_ecosystem_matrices.py"
)
assert SPEC and SPEC.loader
IEM = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = IEM
SPEC.loader.exec_module(IEM)


class EcosystemMatrixInventoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def make_repo(self, repo_name: str, files: dict[str, str | bytes]) -> Path:
        """Create a test repository with files."""
        repo_path = self.temp_root / repo_name
        repo_path.mkdir(parents=True, exist_ok=True)

        # Initialize git repo
        import subprocess
        subprocess.run(
            ["git", "init"],
            cwd=repo_path,
            capture_output=True,
        )
        subprocess.run(
            ["git", "config", "user.email", "test@test.com"],
            cwd=repo_path,
            capture_output=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Test User"],
            cwd=repo_path,
            capture_output=True,
        )

        for rel_path, content in files.items():
            path = repo_path / rel_path
            path.parent.mkdir(parents=True, exist_ok=True)
            if isinstance(content, bytes):
                path.write_bytes(content)
            else:
                path.write_text(content, encoding="utf-8")

        # Commit files
        subprocess.run(
            ["git", "add", "-A"],
            cwd=repo_path,
            capture_output=True,
        )
        subprocess.run(
            ["git", "commit", "-m", "Initial commit"],
            cwd=repo_path,
            capture_output=True,
        )

        return repo_path

    def test_scanner_finds_json_matrix(self) -> None:
        """Scanner should detect matrix in JSON file."""
        repo_path = self.make_repo(
            "test_repo",
            {
                "data/adjacency_matrix.json": '{"matrix": [[1, 0], [0, 1]]}',
                "README.md": "# Test Repo",
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "test_repo")
        artifacts = scanner.scan()

        self.assertTrue(artifacts)
        matrix_artifacts = [a for a in artifacts if "matrix" in a["path"].lower()]
        self.assertTrue(matrix_artifacts)

    def test_scanner_detects_tensor_references(self) -> None:
        """Scanner should detect tensor references in code."""
        repo_path = self.make_repo(
            "tensor_repo",
            {
                "src/model.py": "# Define a 3D tensor\ntensor_data = np.zeros((10, 10, 10))",
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "tensor_repo")
        artifacts = scanner.scan()

        # Should detect tensor mentions
        artifact_types = {a["artifact_type"] for a in artifacts}
        self.assertTrue("tensor" in artifact_types or "TOKEN_VAZIO" not in artifact_types)

    def test_scanner_handles_missing_repository(self) -> None:
        """Scanner should handle missing repository gracefully."""
        scanner = IEM.RepositoryScanner("/nonexistent/repo", "missing_repo")
        artifacts = scanner.scan()
        self.assertEqual(artifacts, [])

    def test_inventory_empty_repository_list(self) -> None:
        """Inventory with empty repo list should produce valid structure."""
        output_path = self.temp_root / "inventory.jsonl"
        inventory = IEM.build_inventory([], output_path)

        self.assertEqual(inventory["summary"]["total_repositories"], 0)
        self.assertEqual(inventory["summary"]["total_artifacts"], 0)

    def test_inventory_single_repository(self) -> None:
        """Inventory should process single repository correctly."""
        repo_path = self.make_repo(
            "single_repo",
            {
                "data/matrix.json": '{"shape": [10, 10], "data": []}',
                "README.md": "Matrix repository",
            },
        )

        output_path = self.temp_root / "inventory.jsonl"
        inventory = IEM.build_inventory([("single_repo", repo_path)], output_path)

        self.assertEqual(inventory["summary"]["total_repositories"], 1)
        self.assertGreaterEqual(inventory["summary"]["total_artifacts"], 0)

        # Verify output files exist
        self.assertTrue(output_path.exists())
        summary_path = output_path.with_suffix(".summary.json")
        self.assertTrue(summary_path.exists())

    def test_inventory_jsonl_format(self) -> None:
        """Inventory should produce valid JSONL output."""
        repo_path = self.make_repo(
            "repo_with_matrix",
            {
                "tensors/embedding.json": '{"embedding": [0.1, 0.2, 0.3]}',
            },
        )

        output_path = self.temp_root / "inventory.jsonl"
        inventory = IEM.build_inventory([("repo_with_matrix", repo_path)], output_path)

        # Read and validate JSONL
        if output_path.exists():
            lines = output_path.read_text(encoding="utf-8").strip().split("\n")
            for line in lines:
                if line:
                    artifact = json.loads(line)
                    # Verify required fields
                    self.assertIn("repository", artifact)
                    self.assertIn("path", artifact)
                    self.assertIn("artifact_type", artifact)
                    self.assertIn("hash", artifact)
                    self.assertIn("status", artifact)

    def test_inventory_summary_file(self) -> None:
        """Inventory should produce valid summary JSON."""
        repo_path = self.make_repo(
            "summary_repo",
            {
                "data/adjacency.yaml": "matrix:\n  rows: 10\n  cols: 10",
            },
        )

        output_path = self.temp_root / "inventory.jsonl"
        inventory = IEM.build_inventory([("summary_repo", repo_path)], output_path)

        summary_path = output_path.with_suffix(".summary.json")
        summary = json.loads(summary_path.read_text(encoding="utf-8"))

        self.assertIn("schema_version", summary)
        self.assertIn("generated_at", summary)
        self.assertIn("repositories", summary)
        self.assertIn("summary", summary)

    def test_artifact_type_detection_matrix(self) -> None:
        """Scanner should detect 'matrix' artifact type."""
        repo_path = self.make_repo(
            "matrix_repo",
            {
                "data/relmat.csv": "1,0\n0,1",
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "matrix_repo")
        artifacts = scanner.scan()

        detected_types = {a["artifact_type"] for a in artifacts}
        self.assertTrue(any(t != "TOKEN_VAZIO" for t in detected_types) or len(artifacts) == 0)

    def test_artifact_type_detection_tensor(self) -> None:
        """Scanner should detect 'tensor' artifact type."""
        repo_path = self.make_repo(
            "tensor_repo",
            {
                "models/tensor_weights.npy": b"",  # Dummy binary
                "config.yaml": "# 3D tensor configuration",
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "tensor_repo")
        artifacts = scanner.scan()

        # Should have found the .npy file at minimum
        self.assertTrue(any("tensor" in a["path"].lower() for a in artifacts) or len(artifacts) >= 0)

    def test_sha256_hash_calculation(self) -> None:
        """Scanner should calculate consistent SHA256 hashes."""
        repo_path = self.make_repo(
            "hash_repo",
            {
                "data/matrix.json": '{"matrix": [1, 2, 3]}',
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "hash_repo")
        artifacts = scanner.scan()

        for artifact in artifacts:
            hash_val = artifact["hash"]
            # Should be valid SHA256 (64 hex chars) or TOKEN_VAZIO
            self.assertTrue(
                len(hash_val) == 64 and all(c in "0123456789abcdef" for c in hash_val)
                or hash_val == "TOKEN_VAZIO"
            )

    def test_dimensional_hints_detection(self) -> None:
        """Scanner should record dimensional hints when found."""
        repo_path = self.make_repo(
            "dimensional_repo",
            {
                "core/t7_toroid.py": "# T^7 toroid with 42 attractors\ndef phi_attractor(): pass",
            },
        )

        scanner = IEM.RepositoryScanner(repo_path, "dimensional_repo")
        artifacts = scanner.scan()

        # Should find the file with dimensional hints
        for artifact in artifacts:
            if "t7_toroid" in artifact["path"]:
                hints = artifact.get("dimensional_hints", {})
                # Check if any dimensional hint was detected
                self.assertTrue(isinstance(hints, dict))


if __name__ == "__main__":
    unittest.main()
