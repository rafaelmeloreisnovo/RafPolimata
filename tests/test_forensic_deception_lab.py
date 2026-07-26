import dataclasses
import unittest

from tools.forensic_deception_lab import (
    StationContext,
    detect_decoy_touch,
    make_manifest,
    project_records,
    recover_fingerprint,
    score_candidate,
    simulate_leak,
    synthetic_records,
    verify_manifest,
)


class ForensicDeceptionLabTests(unittest.TestCase):
    def setUp(self):
        self.key = b"test-key-not-production"
        self.context = StationContext(
            tenant_id="T",
            station_id="S-01",
            user_scope="U",
            epoch_id="E-01",
        )
        self.canonical = synthetic_records(64)
        self.projected = project_records(
            self.canonical, self.context, self.key, decoy_count=4
        )

    def test_business_values_are_preserved(self):
        expected = sum(record.amount_cents for record in self.canonical)
        observed = sum(
            record.amount_cents
            for record in self.projected
            if not record.is_decoy
        )
        self.assertEqual(expected, observed)

    def test_projected_ids_are_unique_and_parent_relations_resolve(self):
        ids = {record.projected_id for record in self.projected}
        self.assertEqual(len(ids), len(self.projected))
        non_decoy_ids = {
            record.projected_id for record in self.projected if not record.is_decoy
        }
        for record in self.projected:
            if record.projected_parent_id is not None:
                self.assertIn(record.projected_parent_id, non_decoy_ids)

    def test_manifest_authentication_detects_change(self):
        manifest = make_manifest(
            self.canonical, self.projected, self.context, self.key
        )
        self.assertTrue(verify_manifest(manifest, self.key))
        changed = dict(manifest)
        changed["projected_record_count"] = 999
        self.assertFalse(verify_manifest(changed, self.key))

    def test_fingerprint_survives_bounded_record_loss(self):
        leak = simulate_leak(self.projected, seed=9, drop_fraction=0.10)
        recovered, confidence = recover_fingerprint(leak)
        correct = score_candidate(recovered, self.context, self.key)
        wrong = score_candidate(
            recovered,
            dataclasses.replace(self.context, station_id="S-02"),
            self.key,
        )
        self.assertGreaterEqual(confidence, 0.90)
        self.assertGreater(correct, wrong)

    def test_decoy_touch_is_observable(self):
        leak = simulate_leak(self.projected, seed=1, drop_fraction=0.0)
        self.assertEqual(len(detect_decoy_touch(leak)), 4)


if __name__ == "__main__":
    unittest.main()
