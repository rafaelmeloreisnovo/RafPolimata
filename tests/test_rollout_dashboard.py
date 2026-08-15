#!/usr/bin/env python3
"""
Tests for RolloutDashboardGenerator (Phase 4: Status Dashboard)

Covers dashboard generation, component tracking, and repository metrics.
"""

import unittest
import json
import tempfile
from pathlib import Path
from datetime import datetime

import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))

from rollout_dashboard import (
    RolloutDashboardGenerator,
    RolloutPhase,
    ComponentStatus,
    ComponentReport,
    RepositoryMetrics,
)


class TestDashboardGeneration(unittest.TestCase):
    """Test dashboard generation."""

    def setUp(self):
        self.generator = RolloutDashboardGenerator()
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_initial_dashboard_not_started(self):
        """Test that initial dashboard has NOT_STARTED phase."""
        dashboard = self.generator.generate_dashboard({})

        self.assertEqual(dashboard.rollout_phase, RolloutPhase.NOT_STARTED)
        self.assertEqual(dashboard.progress_percent, 0)

    def test_all_phases_complete_dashboard(self):
        """Test that all phases complete sets COMPLETE phase."""
        completed = {
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 32,
            "Phase 3: CI Gates": 18,
            "Phase 4: Dashboard": 15,
            "Phase 5: Parable Compiler": 20,
        }

        dashboard = self.generator.generate_dashboard(completed)

        self.assertEqual(dashboard.rollout_phase, RolloutPhase.COMPLETE)
        self.assertEqual(dashboard.progress_percent, 100)

    def test_partial_completion_in_progress(self):
        """Test that partial completion shows IN_PROGRESS phase."""
        completed = {
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 32,
            "Phase 3: CI Gates": 18,
            "Phase 4: Dashboard": 0,
            "Phase 5: Parable Compiler": 0,
        }

        dashboard = self.generator.generate_dashboard(completed)

        self.assertEqual(dashboard.rollout_phase, RolloutPhase.IN_PROGRESS)
        self.assertGreater(dashboard.progress_percent, 0)
        self.assertLess(dashboard.progress_percent, 100)

    def test_component_status_based_on_tests(self):
        """Test that component status reflects test completion."""
        completed = {
            "Phase 1: Claim Verification": 26,  # All tests pass
            "Phase 2: Federated Protocol": 16,  # Partial tests
            "Phase 3: CI Gates": 0,  # No tests yet
            "Phase 4: Dashboard": 0,
            "Phase 5: Parable Compiler": 0,
        }

        dashboard = self.generator.generate_dashboard(completed)

        # Phase 1 should be TESTED (all tests passing)
        phase1 = next(c for c in dashboard.components if "Phase 1" in c.name)
        self.assertEqual(phase1.status, ComponentStatus.TESTED)

        # Phase 2 should be IN_PROGRESS (partial tests)
        phase2 = next(c for c in dashboard.components if "Phase 2" in c.name)
        self.assertEqual(phase2.status, ComponentStatus.IMPLEMENTED)

        # Phase 3 should be PLANNED (no tests)
        phase3 = next(c for c in dashboard.components if "Phase 3" in c.name)
        self.assertEqual(phase3.status, ComponentStatus.PLANNED)

    def test_dashboard_has_all_repositories(self):
        """Test that dashboard includes all 10 trusted repositories."""
        dashboard = self.generator.generate_dashboard({})

        self.assertEqual(len(dashboard.repositories), 10)

    def test_repositories_have_correct_levels(self):
        """Test that repositories have correct authority levels."""
        dashboard = self.generator.generate_dashboard({})

        # Count by level
        level_counts = {}
        for repo in dashboard.repositories:
            if repo.level not in level_counts:
                level_counts[repo.level] = 0
            level_counts[repo.level] += 1

        # Verify expected distribution
        self.assertEqual(level_counts.get("FORMAL_PROOF", 0), 3)
        self.assertEqual(level_counts.get("DOMAIN_EXPERT", 0), 4)
        self.assertEqual(level_counts.get("PEER_REVIEW", 0), 1)
        self.assertEqual(level_counts.get("CROSS_REFERENCE", 0), 1)
        self.assertEqual(level_counts.get("NARRATIVE", 0), 1)


class TestComponentReport(unittest.TestCase):
    """Test component reporting."""

    def test_component_to_dict(self):
        """Test component can be converted to dict."""
        component = ComponentReport(
            name="Phase 1: Test",
            status=ComponentStatus.TESTED,
            description="Test phase",
            tests_passing=10,
            tests_total=10,
        )

        data = component.to_dict()

        self.assertEqual(data["name"], "Phase 1: Test")
        self.assertEqual(data["status"], "TESTED")
        self.assertEqual(data["tests_passing"], 10)
        self.assertEqual(data["tests_total"], 10)
        self.assertEqual(data["test_coverage"], "100.0%")

    def test_component_test_coverage_calculation(self):
        """Test that test coverage is correctly calculated."""
        component = ComponentReport(
            name="Phase 2: Test",
            status=ComponentStatus.IMPLEMENTED,
            description="Test phase",
            tests_passing=16,
            tests_total=32,
        )

        data = component.to_dict()
        self.assertEqual(data["test_coverage"], "50.0%")

    def test_component_blockers_tracked(self):
        """Test that component blockers are tracked."""
        component = ComponentReport(
            name="Phase 3: Test",
            status=ComponentStatus.UNKNOWN,
            description="Blocked phase",
            blockers=["CI failing", "Signature verification not implemented"],
        )

        data = component.to_dict()
        self.assertEqual(len(data["blockers"]), 2)
        self.assertIn("CI failing", data["blockers"])


class TestRepositoryMetrics(unittest.TestCase):
    """Test repository metrics."""

    def test_repository_to_dict(self):
        """Test repository metrics can be converted to dict."""
        repo = RepositoryMetrics(
            name="rafaelmeloreisnovo/Matem-tica-",
            level="FORMAL_PROOF",
            status=ComponentStatus.ACTIVE,
            receipt_count=5,
            verified_count=4,
            rejected_count=1,
            timeout_count=0,
            success_rate=0.8,
        )

        data = repo.to_dict()

        self.assertEqual(data["name"], "rafaelmeloreisnovo/Matem-tica-")
        self.assertEqual(data["level"], "FORMAL_PROOF")
        self.assertEqual(data["receipt_count"], 5)
        self.assertEqual(data["success_rate"], "80.0%")

    def test_repository_success_rate(self):
        """Test repository success rate calculation."""
        repo = RepositoryMetrics(
            name="test-repo",
            level="PEER_REVIEW",
            status=ComponentStatus.ACTIVE,
            receipt_count=10,
            verified_count=9,
            rejected_count=1,
            success_rate=0.9,
        )

        data = repo.to_dict()
        self.assertEqual(data["success_rate"], "90.0%")


class TestDashboardSummary(unittest.TestCase):
    """Test dashboard summary generation."""

    def test_summary_includes_test_statistics(self):
        """Test that summary includes test statistics."""
        generator = RolloutDashboardGenerator()
        completed = {
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 32,
            "Phase 3: CI Gates": 18,
            "Phase 4: Dashboard": 0,
            "Phase 5: Parable Compiler": 0,
        }

        dashboard = generator.generate_dashboard(completed)

        self.assertEqual(dashboard.summary["total_tests"], 26 + 32 + 18)
        self.assertEqual(dashboard.summary["passing_tests"], 26 + 32 + 18)
        self.assertEqual(dashboard.summary["test_coverage"], "100.0%")

    def test_summary_includes_component_status_distribution(self):
        """Test that summary includes component status distribution."""
        generator = RolloutDashboardGenerator()
        completed = {
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 16,  # Partial
            "Phase 3: CI Gates": 0,  # Not started
            "Phase 4: Dashboard": 0,
            "Phase 5: Parable Compiler": 0,
        }

        dashboard = generator.generate_dashboard(completed)

        status_dist = dashboard.summary["components_by_status"]
        self.assertIn("TESTED", status_dist)
        self.assertIn("IMPLEMENTED", status_dist)
        self.assertIn("PLANNED", status_dist)

    def test_summary_identifies_next_phase(self):
        """Test that summary identifies next incomplete phase."""
        generator = RolloutDashboardGenerator()
        completed = {
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 32,
            "Phase 3: CI Gates": 0,  # Next to start
            "Phase 4: Dashboard": 0,
            "Phase 5: Parable Compiler": 0,
        }

        dashboard = generator.generate_dashboard(completed)

        self.assertIn("Phase 3", dashboard.summary["next_phase"])

    def test_summary_lists_critical_blockers(self):
        """Test that summary lists critical blockers."""
        generator = RolloutDashboardGenerator()
        dashboard = generator.generate_dashboard({})

        # New rollout should identify uninitialized repos as blocker
        blockers = dashboard.summary["critical_blockers"]
        self.assertGreater(len(blockers), 0)


class TestDashboardHash(unittest.TestCase):
    """Test dashboard hash computation."""

    def test_hash_deterministic(self):
        """Test that dashboard hash is deterministic."""
        generator = RolloutDashboardGenerator()
        dashboard = generator.generate_dashboard({"Phase 1: Claim Verification": 26})

        hash1 = dashboard.dashboard_hash

        # Recompute hash
        hash2 = generator._compute_hash(dashboard)

        self.assertEqual(hash1, hash2)

    def test_hash_changes_with_progress(self):
        """Test that hash changes when progress changes."""
        generator = RolloutDashboardGenerator()

        dashboard1 = generator.generate_dashboard({"Phase 1: Claim Verification": 26})
        hash1 = dashboard1.dashboard_hash

        dashboard2 = generator.generate_dashboard({
            "Phase 1: Claim Verification": 26,
            "Phase 2: Federated Protocol": 32,
        })
        hash2 = dashboard2.dashboard_hash

        self.assertNotEqual(hash1, hash2)


class TestDashboardExport(unittest.TestCase):
    """Test dashboard export."""

    def setUp(self):
        self.generator = RolloutDashboardGenerator()
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_dashboard_exported_to_json(self):
        """Test that dashboard is exported to JSON."""
        dashboard = self.generator.generate_dashboard(
            {"Phase 1: Claim Verification": 26}
        )

        output_path = Path(self.temp_dir.name) / "dashboard.json"
        result = self.generator.export_dashboard(dashboard, output_path)

        self.assertTrue(output_path.exists())
        self.assertEqual(result, output_path)

    def test_exported_json_is_valid(self):
        """Test that exported JSON is valid."""
        dashboard = self.generator.generate_dashboard(
            {"Phase 1: Claim Verification": 26}
        )

        output_path = Path(self.temp_dir.name) / "dashboard.json"
        self.generator.export_dashboard(dashboard, output_path)

        data = json.loads(output_path.read_text())

        # Validate schema
        self.assertEqual(
            data["schema"],
            "rafaelia.rollout_dashboard.routing.v1"
        )

        # Validate required fields
        self.assertIn("dashboard_id", data)
        self.assertIn("rollout_phase", data)
        self.assertIn("progress_percent", data)
        self.assertIn("components", data)
        self.assertIn("repositories", data)
        self.assertIn("summary", data)


if __name__ == "__main__":
    unittest.main()
