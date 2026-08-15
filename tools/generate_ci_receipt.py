#!/usr/bin/env python3
"""
Hotfix H2: CI Receipt Generation

Generates deterministic receipts for CI job execution.
Records actual command, environment, exit code, outputs to prove execution vs status.

Protocol: RAFAELIA-PSC-1 (proof recording requirement #2)
Scope: CI job step execution, GitHub Actions integration
"""

import json
import os
import sys
import hashlib
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict, field


@dataclass
class ExecutedCommand:
    """Record of a command execution."""
    command: str
    exit_code: int
    working_directory: str
    environment_keys: List[str]  # Only record key names, not values
    stdout_hash: str
    stdout_size: int
    stderr_hash: str
    stderr_size: int
    execution_time_ms: float


@dataclass
class CIReceipt:
    """Complete CI job execution receipt."""
    schema: str = "rafaelia.ci_receipt.v1"
    job_id: str = ""
    job_name: str = ""
    github_run_id: str = ""
    github_run_number: str = ""
    github_sha: str = ""
    github_ref: str = ""
    github_actor: str = ""
    timestamp_utc: str = ""
    runner_os: str = ""
    runner_arch: str = ""

    # Execution records
    commands: List[ExecutedCommand] = field(default_factory=list)

    # Outputs
    artifacts: List[Dict[str, str]] = field(default_factory=list)  # path, hash, size

    # Summary
    overall_exit_code: int = 0
    status: str = "PENDING"  # PENDING, PASS, FAIL

    # Integrity
    receipt_hash: str = ""

    def add_command(self, cmd_record: ExecutedCommand):
        """Add a command execution record."""
        self.commands.append(cmd_record)

    def add_artifact(self, path: str, file_hash: str, size: int):
        """Record a generated artifact."""
        self.artifacts.append({
            "path": path,
            "hash": file_hash,
            "size": size,
        })

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            "schema": self.schema,
            "job_id": self.job_id,
            "job_name": self.job_name,
            "github": {
                "run_id": self.github_run_id,
                "run_number": self.github_run_number,
                "sha": self.github_sha,
                "ref": self.github_ref,
                "actor": self.github_actor,
            },
            "timestamp_utc": self.timestamp_utc,
            "environment": {
                "os": self.runner_os,
                "arch": self.runner_arch,
            },
            "commands": [asdict(cmd) for cmd in self.commands],
            "artifacts": self.artifacts,
            "overall_exit_code": self.overall_exit_code,
            "status": self.status,
            "receipt_hash": self.receipt_hash,
        }

    def compute_hash(self) -> str:
        """Compute deterministic receipt hash."""
        # Hash over all fields except receipt_hash itself
        hashable = {
            "schema": self.schema,
            "job_id": self.job_id,
            "job_name": self.job_name,
            "github_sha": self.github_sha,
            "github_ref": self.github_ref,
            "commands": [asdict(cmd) for cmd in self.commands],
            "artifacts": self.artifacts,
            "overall_exit_code": self.overall_exit_code,
            "status": self.status,
        }
        content = json.dumps(hashable, indent=2, sort_keys=True)
        return hashlib.sha256(content.encode()).hexdigest()


class CommandExecutor:
    """Execute commands and record receipts."""

    def __init__(self, receipt: CIReceipt):
        self.receipt = receipt
        self.start_time = None

    def run_command(
        self,
        command: str,
        capture_output: bool = True,
        cwd: Optional[str] = None,
        env_override: Optional[Dict[str, str]] = None,
    ) -> int:
        """Execute command and record it in receipt."""
        import time

        cwd = cwd or os.getcwd()
        start = time.time()

        try:
            # Prepare environment
            env = os.environ.copy()
            if env_override:
                env.update(env_override)

            # Run command
            if capture_output:
                result = subprocess.run(
                    command,
                    shell=True,
                    cwd=cwd,
                    capture_output=True,
                    text=False,
                    env=env,
                )
                stdout = result.stdout if result.stdout else b""
                stderr = result.stderr if result.stderr else b""
                exit_code = result.returncode
            else:
                exit_code = os.system(f"cd {cwd} && {command}")
                stdout = b"(not captured)"
                stderr = b""

            elapsed = (time.time() - start) * 1000  # ms

            # Record command
            cmd_record = ExecutedCommand(
                command=command,
                exit_code=exit_code,
                working_directory=cwd,
                environment_keys=list(env.keys()),
                stdout_hash=hashlib.sha256(stdout).hexdigest(),
                stdout_size=len(stdout),
                stderr_hash=hashlib.sha256(stderr).hexdigest(),
                stderr_size=len(stderr),
                execution_time_ms=elapsed,
            )
            self.receipt.add_command(cmd_record)

            return exit_code

        except Exception as e:
            print(f"Error executing command: {e}", file=sys.stderr)
            self.receipt.overall_exit_code = 1
            self.receipt.status = "FAIL"
            raise

    def record_artifact(self, artifact_path: str):
        """Record a file artifact in receipt."""
        path = Path(artifact_path)
        if not path.exists():
            print(f"Warning: artifact not found: {artifact_path}")
            return

        if path.is_file():
            content = path.read_bytes()
            file_hash = hashlib.sha256(content).hexdigest()
            self.receipt.add_artifact(artifact_path, file_hash, len(content))
        else:
            print(f"Warning: {artifact_path} is not a file")

    def finalize(self) -> Dict[str, Any]:
        """Finalize receipt and compute hash."""
        self.receipt.timestamp_utc = datetime.utcnow().isoformat() + "Z"
        self.receipt.receipt_hash = self.receipt.compute_hash()

        if all(cmd.exit_code == 0 for cmd in self.receipt.commands):
            self.receipt.overall_exit_code = 0
            self.receipt.status = "PASS"
        else:
            self.receipt.overall_exit_code = 1
            self.receipt.status = "FAIL"

        return self.receipt.to_dict()


def create_receipt_from_env() -> CIReceipt:
    """Create receipt from GitHub Actions environment variables."""
    receipt = CIReceipt(
        job_id=os.getenv("GITHUB_JOB", "unknown"),
        job_name=os.getenv("GITHUB_JOB_NAME", "unknown"),
        github_run_id=os.getenv("GITHUB_RUN_ID", ""),
        github_run_number=os.getenv("GITHUB_RUN_NUMBER", ""),
        github_sha=os.getenv("GITHUB_SHA", ""),
        github_ref=os.getenv("GITHUB_REF", ""),
        github_actor=os.getenv("GITHUB_ACTOR", ""),
        runner_os=os.getenv("RUNNER_OS", ""),
        runner_arch=os.getenv("RUNNER_ARCH", ""),
    )
    return receipt


def main():
    """CLI for receipt generation."""
    import argparse

    parser = argparse.ArgumentParser(description="Generate CI job execution receipt")
    parser.add_argument("--job-name", default="CI Job", help="Human-readable job name")
    parser.add_argument("--output", type=Path, default=Path("ci_receipt.json"),
                        help="Output receipt file")
    parser.add_argument(
        "--command", nargs="+", help="Commands to execute and record"
    )
    parser.add_argument(
        "--record-artifact", nargs="*", help="Artifact paths to record in receipt"
    )

    args = parser.parse_args()

    # Create receipt
    receipt = create_receipt_from_env()
    receipt.job_name = args.job_name

    executor = CommandExecutor(receipt)

    # Execute commands if provided
    if args.command:
        cmd_str = " ".join(args.command)
        print(f"Executing: {cmd_str}")
        exit_code = executor.run_command(cmd_str)
        if exit_code != 0:
            print(f"Command failed with exit code {exit_code}")

    # Record artifacts if provided
    if args.record_artifact:
        for artifact in args.record_artifact:
            executor.record_artifact(artifact)

    # Finalize and write
    receipt_dict = executor.finalize()

    # Ensure output directory exists
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(receipt_dict, indent=2))

    print(f"Receipt written to {args.output}")
    print(f"Status: {receipt.status}")
    print(f"Hash: {receipt.receipt_hash}")

    return receipt.overall_exit_code


if __name__ == "__main__":
    sys.exit(main())
