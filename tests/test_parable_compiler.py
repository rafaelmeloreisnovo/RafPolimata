#!/usr/bin/env python3
"""
Tests for ParableCompiler (Phase 5: Parable Compiler)

Covers parable compilation, element extraction, and claim generation.
"""

import unittest
import json
import tempfile
from pathlib import Path
from datetime import datetime

import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))

from parable_compiler import (
    ParableCompiler,
    ParableType,
    CompilationStatus,
    ParableElement,
)


class TestParableCompilation(unittest.TestCase):
    """Test parable compilation."""

    def setUp(self):
        self.compiler = ParableCompiler()
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_simple_parable_compiles(self):
        """Test that simple parable compiles successfully."""
        parable = (
            "A hero faced a mountain. "
            "The mountain was an obstacle to overcome. "
            "Through courage, the hero reached the summit. "
            "The lesson was that obstacles test character."
        )

        claim = self.compiler.compile_parable(parable)

        self.assertEqual(claim.status, CompilationStatus.COMPILED)
        self.assertGreater(len(claim.elements), 0)
        self.assertIsNotNone(claim.formal_claim)

    def test_parable_type_set(self):
        """Test that parable type is preserved."""
        parable = "A wise person learns from stories."

        claim = self.compiler.compile_parable(
            parable,
            parable_type=ParableType.ANALOGY
        )

        self.assertEqual(claim.parable_type, ParableType.ANALOGY)

    def test_parable_id_generated(self):
        """Test that parable ID is generated or set."""
        parable = "A journey begins with a single step."

        claim = self.compiler.compile_parable(
            parable,
            parable_id="TEST-001"
        )

        self.assertEqual(claim.parable_id, "TEST-001")

    def test_confidence_score_computed(self):
        """Test that confidence score is computed."""
        parable = (
            "A hero faced challenges. "
            "The hero persevered. "
            "The hero succeeded. "
            "The lesson is that persistence pays."
        )

        claim = self.compiler.compile_parable(parable)

        self.assertGreater(claim.confidence_score, 0.0)
        self.assertLessEqual(claim.confidence_score, 1.0)

    def test_formal_claim_generated(self):
        """Test that formal claim is generated with required fields."""
        parable = (
            "A character faced adversity. "
            "Through wisdom, they found a solution. "
            "The moral is that understanding conquers obstacles."
        )

        claim = self.compiler.compile_parable(parable)

        formal = claim.formal_claim
        self.assertIn("id", formal)
        self.assertIn("source_status", formal)
        self.assertIn("epistemic_status", formal)
        self.assertIn("domain", formal)
        self.assertIn("limitations", formal)
        self.assertIn("falsifier", formal)


class TestElementExtraction(unittest.TestCase):
    """Test narrative element extraction."""

    def setUp(self):
        self.compiler = ParableCompiler()

    def test_protagonist_extracted(self):
        """Test that protagonist is extracted."""
        parable = "The hero was brave and strong."

        claim = self.compiler.compile_parable(parable)

        protagonists = [e for e in claim.elements if e.category == "protagonist"]
        self.assertGreater(len(protagonists), 0)

    def test_setting_extracted(self):
        """Test that setting is extracted."""
        parable = "In a distant kingdom, there was a forest."

        claim = self.compiler.compile_parable(parable)

        settings = [e for e in claim.elements if e.category == "setting"]
        self.assertGreater(len(settings), 0)

    def test_conflict_extracted(self):
        """Test that conflict is extracted."""
        parable = "A challenge arose that tested the warrior."

        claim = self.compiler.compile_parable(parable)

        conflicts = [e for e in claim.elements if e.category == "conflict"]
        self.assertGreater(len(conflicts), 0)

    def test_resolution_extracted(self):
        """Test that resolution is extracted."""
        parable = "Finally, the problem was solved through wisdom."

        claim = self.compiler.compile_parable(parable)

        resolutions = [e for e in claim.elements if e.category == "resolution"]
        self.assertGreater(len(resolutions), 0)

    def test_moral_extracted(self):
        """Test that moral is extracted."""
        parable = "The moral of the story is that truth prevails."

        claim = self.compiler.compile_parable(parable)

        morals = [e for e in claim.elements if e.category == "moral"]
        self.assertGreater(len(morals), 0)

    def test_element_confidence_in_range(self):
        """Test that element confidence scores are in valid range."""
        parable = "A hero on a journey faced obstacles and achieved victory."

        claim = self.compiler.compile_parable(parable)

        for element in claim.elements:
            self.assertGreaterEqual(element.confidence, 0.0)
            self.assertLessEqual(element.confidence, 1.0)


class TestSymbolicMapping(unittest.TestCase):
    """Test symbolic element mapping."""

    def setUp(self):
        self.compiler = ParableCompiler()

    def test_journey_maps_to_method(self):
        """Test that journey maps to method."""
        parable = "The hero's journey was long and difficult."

        claim = self.compiler.compile_parable(parable)

        journey_elements = [e for e in claim.elements if "journey" in e.content.lower()]
        if journey_elements:
            self.assertEqual(journey_elements[0].symbolic_mapping, "method")

    def test_obstacle_maps_to_limitation(self):
        """Test that obstacle maps to limitation."""
        parable = "The obstacle in the path was overcome by courage."

        claim = self.compiler.compile_parable(parable)

        obstacle_elements = [e for e in claim.elements if "obstacle" in e.content.lower()]
        if obstacle_elements:
            self.assertEqual(obstacle_elements[0].symbolic_mapping, "limitation")

    def test_test_maps_to_verification(self):
        """Test that test maps to verification."""
        parable = "The test of character revealed the hero's virtue."

        claim = self.compiler.compile_parable(parable)

        test_elements = [e for e in claim.elements if "test" in e.content.lower()]
        if test_elements:
            self.assertEqual(test_elements[0].symbolic_mapping, "verification")


class TestFormalClaimGeneration(unittest.TestCase):
    """Test formal claim generation."""

    def setUp(self):
        self.compiler = ParableCompiler()

    def test_epistemic_status_is_analogy_only(self):
        """Test that narrative epistemic status is ANALOGY_ONLY."""
        parable = "A story teaches through analogy."

        claim = self.compiler.compile_parable(parable)

        self.assertEqual(claim.formal_claim["epistemic_status"], "ANALOGY_ONLY")

    def test_source_status_is_narrative(self):
        """Test that source status is NARRATIVE."""
        parable = "A narrative proof shows the way."

        claim = self.compiler.compile_parable(parable)

        self.assertEqual(claim.formal_claim["source_status"], "NARRATIVE")

    def test_domain_inferred(self):
        """Test that domain is inferred from content."""
        parable = "Virtue and wisdom are the foundations of good character."

        claim = self.compiler.compile_parable(parable)

        # Should infer ethics or philosophy domain
        self.assertIn(claim.formal_claim["domain"],
                      ["PHILOSOPHY", "ETHICS", "SPIRITUALITY", "HUMAN_NATURE", "SOCIETY"])

    def test_limitations_include_analogy_disclaimer(self):
        """Test that limitations include analogy disclaimer."""
        parable = "A metaphorical proof of eternal truth."

        claim = self.compiler.compile_parable(
            parable,
            parable_type=ParableType.METAPHOR
        )

        limitations = claim.formal_claim["limitations"]
        self.assertIn("analogical", limitations.lower())

    def test_falsifier_defined(self):
        """Test that falsifier is defined."""
        parable = "A simple story with a message."

        claim = self.compiler.compile_parable(parable)

        falsifier = claim.formal_claim["falsifier"]
        self.assertGreater(len(falsifier), 0)


class TestInterpretationGeneration(unittest.TestCase):
    """Test interpretation generation."""

    def setUp(self):
        self.compiler = ParableCompiler()

    def test_interpretation_generated(self):
        """Test that interpretation is generated."""
        parable = (
            "A wise person lived in the mountains. "
            "They learned that knowledge brings peace."
        )

        claim = self.compiler.compile_parable(parable)

        self.assertGreater(len(claim.interpretation), 0)

    def test_caveats_generated_when_low_confidence(self):
        """Test that caveats are generated for low confidence claims."""
        parable = "X."  # Minimal parable

        claim = self.compiler.compile_parable(parable)

        if claim.confidence_score < 0.7:
            self.assertGreater(len(claim.caveats), 0)


class TestClaimHash(unittest.TestCase):
    """Test claim hash computation."""

    def setUp(self):
        self.compiler = ParableCompiler()

    def test_hash_deterministic(self):
        """Test that claim hash is deterministic."""
        parable = "A consistent parable always yields the same proof."

        claim1 = self.compiler.compile_parable(parable, parable_id="TEST-001")
        hash1 = claim1.claim_hash

        compiler2 = ParableCompiler()
        claim2 = compiler2.compile_parable(parable, parable_id="TEST-001")
        hash2 = claim2.claim_hash

        self.assertEqual(hash1, hash2)

    def test_hash_changes_with_parable_type(self):
        """Test that hash changes when parable type changes."""
        parable = "A story of transformation and growth."

        claim1 = self.compiler.compile_parable(
            parable,
            parable_type=ParableType.ANALOGY
        )
        hash1 = claim1.claim_hash

        compiler2 = ParableCompiler()
        claim2 = compiler2.compile_parable(
            parable,
            parable_type=ParableType.METAPHOR
        )
        hash2 = claim2.claim_hash

        self.assertNotEqual(hash1, hash2)


class TestClaimExport(unittest.TestCase):
    """Test claim export."""

    def setUp(self):
        self.compiler = ParableCompiler()
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_claim_exported_to_json(self):
        """Test that claim is exported to JSON."""
        parable = "A simple narrative about truth."

        claim = self.compiler.compile_parable(parable)

        output_path = Path(self.temp_dir.name) / "claim.json"
        result = self.compiler.export_claim(claim, output_path)

        self.assertTrue(output_path.exists())
        self.assertEqual(result, output_path)

    def test_exported_json_is_valid(self):
        """Test that exported JSON is valid and complete."""
        parable = "A narrative proof of wisdom."

        claim = self.compiler.compile_parable(parable)

        output_path = Path(self.temp_dir.name) / "claim.json"
        self.compiler.export_claim(claim, output_path)

        data = json.loads(output_path.read_text())

        # Validate schema
        self.assertEqual(
            data["schema"],
            "rafaelia.parable_compiled_claim.routing.v1"
        )

        # Validate required fields
        self.assertIn("claim_id", data)
        self.assertIn("formal_claim", data)
        self.assertIn("elements", data)
        self.assertIn("confidence_score", data)


if __name__ == "__main__":
    unittest.main()
