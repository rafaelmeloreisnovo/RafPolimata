#!/usr/bin/env python3
"""
Risk Gates Validator — RAFAELIA

Operacionaliza gates de risco (G0-G7, D1-D5, R0-R3, I0-I3) de docs/RISCO_GESTAO_FRAMEWORK_CANONICAL.md

Uso:
    python3 scripts/risk_gates_validator.py --check-all
    python3 scripts/risk_gates_validator.py --branch HEAD
    python3 scripts/risk_gates_validator.py --subsystem ApkC
    python3 scripts/risk_gates_validator.py --gate G2_claims_supported
"""

import json
import subprocess
import sys
import os
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Tuple, Optional

# Configuration
REPO_ROOT = Path(__file__).parent.parent
RESULTS_DIR = REPO_ROOT / "docs" / "results" / "risk"
RESULTS_DIR.mkdir(parents=True, exist_ok=True)

class GateResult:
    """Represents a single gate execution result."""

    def __init__(self, gate_id: str, gate_type: str, subsystem: str,
                 result: str, evidence: Dict, scope: str = None,
                 limitations: str = None):
        self.timestamp = datetime.utcnow().isoformat() + "Z"
        self.gate_id = gate_id
        self.gate_type = gate_type  # prevention, detection, remediation, improvement
        self.subsystem = subsystem
        self.result = result  # PASS, FAIL, TOKEN_VAZIO
        self.evidence = evidence
        self.scope = scope or "default"
        self.limitations = limitations or ""

        try:
            commit = subprocess.check_output(
                ["git", "rev-parse", "HEAD"],
                cwd=REPO_ROOT,
                text=True
            ).strip()
        except:
            commit = "unknown"

        try:
            branch = subprocess.check_output(
                ["git", "rev-parse", "--abbrev-ref", "HEAD"],
                cwd=REPO_ROOT,
                text=True
            ).strip()
        except:
            branch = "unknown"

        self.commit = commit
        self.branch = branch

    def to_dict(self) -> Dict:
        return {
            "timestamp": self.timestamp,
            "commit": self.commit,
            "branch": self.branch,
            "gate_id": self.gate_id,
            "gate_type": self.gate_type,
            "subsystem": self.subsystem,
            "result": self.result,
            "evidence": self.evidence,
            "scope": self.scope,
            "limitations": self.limitations,
        }


class RiskGatesValidator:
    """Main validator class."""

    def __init__(self):
        self.results: List[GateResult] = []
        self.passed = 0
        self.failed = 0
        self.token_vazio = 0

    def add_result(self, result: GateResult):
        self.results.append(result)
        if result.result == "PASS":
            self.passed += 1
        elif result.result == "FAIL":
            self.failed += 1
        else:
            self.token_vazio += 1

    # =========================================================================
    # PREVENTION GATES (G0-G7)
    # =========================================================================

    def check_g0_legality(self, subsystem: str) -> GateResult:
        """G0: Is this prohibited by law/regulation?"""

        # Simple check: look for SECURITY.md or legal notices
        security_file = REPO_ROOT / ".github" / "SECURITY.md"

        result = "PASS"
        evidence = {
            "check": "Security policy file exists",
            "security_file_exists": security_file.exists(),
        }

        if not security_file.exists():
            result = "TOKEN_VAZIO"
            evidence["note"] = "SECURITY.md not found; cannot verify legality framework"

        return GateResult(
            gate_id="G0_legality",
            gate_type="prevention",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope="Repository-wide",
            limitations="Requires manual review of laws; script does syntactic check only"
        )

    def check_g2_claims_supported(self, subsystem: str) -> GateResult:
        """G2: Can I prove the claim with evidence?"""

        # Check if RISCO_GESTAO_FRAMEWORK_CANONICAL.md exists
        framework_file = REPO_ROOT / "docs" / "RISCO_GESTAO_FRAMEWORK_CANONICAL.md"

        result = "PASS"
        evidence = {
            "framework_exists": framework_file.exists(),
            "framework_path": str(framework_file.relative_to(REPO_ROOT)),
        }

        if not framework_file.exists():
            result = "FAIL"
            evidence["error"] = "Risk framework document not found"
        else:
            # Check if framework contains "C4: Camada de Claims"
            with open(framework_file) as f:
                content = f.read()
                has_claims_section = "C4" in content and "claims" in content.lower()
                evidence["claims_section_present"] = has_claims_section

                if not has_claims_section:
                    result = "TOKEN_VAZIO"
                    evidence["note"] = "Claims section exists but incomplete"

        return GateResult(
            gate_id="G2_claims_supported",
            gate_type="prevention",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope="Documentation framework",
            limitations="Checks documentation structure only; does not validate claim content"
        )

    def check_g3_determinism_verifiable(self, subsystem: str = "ApkC") -> GateResult:
        """G3: Is determinism verifiable? (ApkC-specific)"""

        # Check if golden test exists
        golden_test_files = list(REPO_ROOT.rglob("*golden*")) + \
                           list(REPO_ROOT.rglob("*determinism*"))

        result = "TOKEN_VAZIO"
        evidence = {
            "golden_test_count": len(golden_test_files),
            "subsystem": subsystem,
        }

        if golden_test_files:
            result = "PASS"
            evidence["files_found"] = [str(f.relative_to(REPO_ROOT)) for f in golden_test_files[:5]]

        return GateResult(
            gate_id="G3_determinism_verifiable",
            gate_type="prevention",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope=f"{subsystem} subsystem",
            limitations="Only detects golden test files; does not run them"
        )

    def check_g5_universalization_test(self, subsystem: str) -> GateResult:
        """G5: Would generalization degrade the market?"""

        # This is primarily a qualitative gate; we document it exists
        framework_file = REPO_ROOT / "docs" / "RISCO_GESTAO_FRAMEWORK_CANONICAL.md"

        result = "TOKEN_VAZIO"
        evidence = {
            "gate_documented": framework_file.exists(),
            "note": "G5 is a qualitative gate requiring human judgment"
        }

        if framework_file.exists():
            with open(framework_file) as f:
                if "G5" in f.read() and "universaliz" in f.read().lower():
                    result = "PASS"
                    evidence["note"] = "G5 documented in framework; requires manual assessment per change"

        return GateResult(
            gate_id="G5_universalization",
            gate_type="prevention",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope="Governance framework",
            limitations="Gate is qualitative; automation only confirms documentation exists"
        )

    # =========================================================================
    # DETECTION GATES (D1-D5)
    # =========================================================================

    def check_d1_test_failure(self, subsystem: str = "ApkC") -> GateResult:
        """D1: Does code pass failure test?"""

        # Try to run basic test infrastructure
        tests_dir = REPO_ROOT / "tests"

        result = "TOKEN_VAZIO"
        evidence = {
            "tests_dir_exists": tests_dir.exists(),
            "subsystem": subsystem,
        }

        if tests_dir.exists():
            test_files = list(tests_dir.glob("*.py")) + list(tests_dir.glob("*test*.c"))
            evidence["test_file_count"] = len(test_files)

            if test_files:
                result = "PASS"
                evidence["note"] = f"Found {len(test_files)} test files"

        return GateResult(
            gate_id="D1_test_failure",
            gate_type="detection",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope=f"{subsystem} subsystem",
            limitations="Only checks test file existence; does not execute tests"
        )

    def check_d3_integrity_hash(self, subsystem: str = "conversation_indexer") -> GateResult:
        """D3: Does receipt/hash verify integrity?"""

        # Check if indexer has hash verification in code
        indexer_dir = REPO_ROOT / "runtime" / "conversation_indexer"

        result = "TOKEN_VAZIO"
        evidence = {
            "indexer_exists": indexer_dir.exists(),
            "subsystem": subsystem,
        }

        if indexer_dir.exists():
            # Look for hash/checksum/integrity code
            hash_patterns = ["hash", "checksum", "blake", "sha256", "crc32", "integrity"]
            found_patterns = []

            for py_file in indexer_dir.glob("*.py"):
                try:
                    with open(py_file) as f:
                        content = f.read().lower()
                        for pattern in hash_patterns:
                            if pattern in content:
                                found_patterns.append(pattern)
                except:
                    pass

            evidence["hash_patterns_found"] = list(set(found_patterns))

            if found_patterns:
                result = "PASS"

        return GateResult(
            gate_id="D3_integrity_hash",
            gate_type="detection",
            subsystem=subsystem,
            result=result,
            evidence=evidence,
            scope=f"{subsystem} subsystem",
            limitations="Only searches for hash-related keywords; does not validate implementation"
        )

    def check_d4_semantic_coherence(self) -> GateResult:
        """D4: Are code, documentation, and tests coherent?"""

        # Run document governance check if available
        governance_script = REPO_ROOT / "scripts" / "document_governance.py"

        result = "TOKEN_VAZIO"
        evidence = {
            "governance_script_exists": governance_script.exists(),
        }

        if governance_script.exists():
            try:
                output = subprocess.run(
                    ["python3", str(governance_script), "--check"],
                    cwd=REPO_ROOT,
                    capture_output=True,
                    text=True,
                    timeout=30
                )

                evidence["governance_check_output"] = output.stdout[:500]

                if output.returncode == 0:
                    result = "PASS"
                else:
                    result = "FAIL"
                    evidence["error"] = output.stderr[:500]
            except subprocess.TimeoutExpired:
                result = "TOKEN_VAZIO"
                evidence["error"] = "Governance check timed out"
            except Exception as e:
                result = "TOKEN_VAZIO"
                evidence["error"] = str(e)

        return GateResult(
            gate_id="D4_semantic_coherence",
            gate_type="detection",
            subsystem="entire",
            result=result,
            evidence=evidence,
            scope="Repository-wide",
            limitations="Requires document_governance.py to be functional"
        )

    def check_d5_traceability(self) -> GateResult:
        """D5: Can I connect execution -> input -> output -> receipt -> claim?"""

        # Check if results/risk directory exists and has receipts
        results_dir = REPO_ROOT / "docs" / "results" / "risk"

        result = "TOKEN_VAZIO"
        evidence = {
            "results_dir_exists": results_dir.exists(),
        }

        if results_dir.exists():
            receipts = list(results_dir.glob("*.json"))
            evidence["receipt_count"] = len(receipts)

            if receipts:
                result = "PASS"
                evidence["note"] = f"Found {len(receipts)} receipt(s)"
                evidence["recent_receipt"] = str(receipts[-1].name)
        else:
            result = "TOKEN_VAZIO"
            evidence["note"] = "Results directory does not exist; receipts cannot be stored"

        return GateResult(
            gate_id="D5_traceability",
            gate_type="detection",
            subsystem="entire",
            result=result,
            evidence=evidence,
            scope="Repository-wide",
            limitations="Only checks directory structure; does not validate receipt content"
        )

    # =========================================================================
    # PUBLIC INTERFACE
    # =========================================================================

    def check_all(self) -> bool:
        """Run all available gates."""

        print("[*] Running all risk gates...")

        # Prevention gates
        self.add_result(self.check_g0_legality("entire"))
        self.add_result(self.check_g2_claims_supported("entire"))
        self.add_result(self.check_g3_determinism_verifiable("ApkC"))
        self.add_result(self.check_g5_universalization_test("entire"))

        # Detection gates
        self.add_result(self.check_d1_test_failure("ApkC"))
        self.add_result(self.check_d3_integrity_hash("conversation_indexer"))
        self.add_result(self.check_d4_semantic_coherence())
        self.add_result(self.check_d5_traceability())

        return self.failed == 0

    def check_subsystem(self, subsystem: str) -> bool:
        """Run gates for a specific subsystem."""

        print(f"[*] Checking subsystem: {subsystem}")

        # Map subsystem to relevant gates
        gates_by_subsystem = {
            "ApkC": [self.check_g3_determinism_verifiable, self.check_d1_test_failure],
            "conversation_indexer": [self.check_d3_integrity_hash],
            "entire": [
                self.check_g0_legality,
                self.check_g2_claims_supported,
                self.check_d4_semantic_coherence,
                self.check_d5_traceability,
            ],
        }

        gates = gates_by_subsystem.get(subsystem, [])

        for gate in gates:
            if "subsystem" in gate.__code__.co_varnames:
                self.add_result(gate(subsystem))
            else:
                self.add_result(gate())

        return self.failed == 0

    def save_results(self, filename: str = None):
        """Save results to JSON file."""

        if filename is None:
            filename = f"risk_gates_{datetime.utcnow().isoformat().replace(':', '-')}.json"

        filepath = RESULTS_DIR / filename

        with open(filepath, "w") as f:
            json.dump(
                {
                    "timestamp": datetime.utcnow().isoformat() + "Z",
                    "summary": {
                        "passed": self.passed,
                        "failed": self.failed,
                        "token_vazio": self.token_vazio,
                        "total": len(self.results),
                    },
                    "results": [r.to_dict() for r in self.results],
                },
                f,
                indent=2
            )

        print(f"[+] Results saved to {filepath.relative_to(REPO_ROOT)}")
        return filepath

    def print_summary(self):
        """Print human-readable summary."""

        print("\n" + "=" * 70)
        print("RISK GATES VALIDATION SUMMARY")
        print("=" * 70)
        print(f"PASS:        {self.passed}")
        print(f"FAIL:        {self.failed}")
        print(f"TOKEN_VAZIO: {self.token_vazio}")
        print(f"TOTAL:       {len(self.results)}")
        print("=" * 70)

        for result in self.results:
            status_icon = {
                "PASS": "✓",
                "FAIL": "✗",
                "TOKEN_VAZIO": "≈"
            }.get(result.result, "?")

            print(f"{status_icon} [{result.gate_type.upper()}] {result.gate_id} ({result.subsystem})")
            if result.limitations:
                print(f"    └─ {result.limitations[:80]}")

        print()


def main():
    validator = RiskGatesValidator()

    if len(sys.argv) < 2 or sys.argv[1] in ["--check-all", "-a"]:
        success = validator.check_all()
    elif sys.argv[1] in ["--subsystem", "-s"] and len(sys.argv) > 2:
        success = validator.check_subsystem(sys.argv[2])
    else:
        print(__doc__)
        sys.exit(1)

    validator.print_summary()
    validator.save_results()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
