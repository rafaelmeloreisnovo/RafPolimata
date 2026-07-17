import copy
import json
from pathlib import Path
import unittest

from scripts.validate_zipraf_ao42_governance import validate

ROOT = Path(__file__).resolve().parents[1]
BASE = json.loads((ROOT / "configs" / "zipraf-ao42-governance.json").read_text())


class GovernanceTests(unittest.TestCase):
    def test_canonical_contract_passes(self):
        self.assertEqual(validate(BASE)["status"], "PASS")

    def test_body_copy_is_rejected(self):
        cfg = copy.deepcopy(BASE)
        cfg["privacy"]["public_mirror_may_copy_body"] = True
        self.assertEqual(validate(cfg)["status"], "FAIL")

    def test_wrong_hyperform_count_is_rejected(self):
        cfg = copy.deepcopy(BASE)
        cfg["geometry"]["hyperform_count"] = 41
        self.assertEqual(validate(cfg)["status"], "FAIL")

    def test_weak_cache_key_is_rejected(self):
        cfg = copy.deepcopy(BASE)
        cfg["cache_promotion_gate"]["required"].remove("code_hash")
        self.assertEqual(validate(cfg)["status"], "FAIL")

    def test_crc_cannot_be_promoted(self):
        cfg = copy.deepcopy(BASE)
        cfg["cache_promotion_gate"]["crc32c_is_cryptographic"] = True
        self.assertEqual(validate(cfg)["status"], "FAIL")


if __name__ == "__main__": unittest.main()
