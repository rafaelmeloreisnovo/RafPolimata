import copy
import unittest

from tools.custody_qr_envelope import (
    build_envelope,
    decode_qr_payload,
    default_checkpoint_body,
    encode_qr_payload,
    verify_envelope,
)


class CustodyQrEnvelopeTests(unittest.TestCase):
    def test_unsigned_roundtrip(self):
        envelope = build_envelope(default_checkpoint_body())
        payload = encode_qr_payload(envelope)
        decoded = decode_qr_payload(payload)
        valid, errors = verify_envelope(decoded)
        self.assertTrue(valid, errors)
        self.assertLess(len(payload), 900)

    def test_digest_detects_body_change(self):
        envelope = build_envelope(default_checkpoint_body())
        changed = copy.deepcopy(envelope)
        changed["body"]["epoch_id"] = "E2"
        valid, errors = verify_envelope(changed)
        self.assertFalse(valid)
        self.assertIn("sha256 mismatch", errors)

    def test_hmac_detects_wrong_key(self):
        envelope = build_envelope(default_checkpoint_body(), b"correct-key")
        valid, errors = verify_envelope(envelope, b"wrong-key")
        self.assertFalse(valid)
        self.assertIn("HMAC mismatch", errors)

    def test_checkpoint_has_no_personal_or_secret_fields(self):
        body = default_checkpoint_body()
        text = str(body).lower()
        for term in ("cpf", "email", "password", "private_key", "raw_record"):
            self.assertNotIn(term, text)
        self.assertFalse(body["claim_allowed"])


if __name__ == "__main__":
    unittest.main()
