#!/usr/bin/env python3
"""QR-CUSTODY-RECEIPT.v1 — bounded, synthetic custody receipt transport.

The core uses Python stdlib only. A QR renderer is an optional adapter: when the
third-party ``qrcode`` package is available, ``render_svg`` emits a standards-
compliant SVG. The security boundary is the authenticated payload, not the
visual QR symbol.

This module is a laboratory/reference implementation:
- no production keys;
- no personal data in the payload;
- HMAC is not a public digital signature;
- claim_allowed is always false.
"""

from __future__ import annotations

import argparse
import base64
import dataclasses
import datetime as dt
import hashlib
import hmac
import json
import os
import re
import secrets
import zlib
from pathlib import Path
from typing import Any, Mapping

VERSION = "QR-CUSTODY-RECEIPT.v1"
PREFIX = "RAFCUST1:"
AUTH_METHOD = "HMAC-SHA-256-LAB"
CLAIM_ALLOWED = False
MAX_COMPRESSED_BYTES = 4096
MAX_DECOMPRESSED_BYTES = 16384
SHA256_RE = re.compile(r"^[a-f0-9]{64}$")
ALLOWED_KEYS = frozenset(
    {
        "version",
        "receipt_id",
        "edition_id",
        "epoch_id",
        "manifest_sha256",
        "canonical_root_sha256",
        "projection_root_sha256",
        "source_registry_root_sha256",
        "recipient_scope_commitment",
        "issued_at",
        "expires_at",
        "nonce",
        "authentication_method",
        "key_id",
        "authenticator",
        "claim_allowed",
        "limitations",
    }
)
REQUIRED_KEYS = ALLOWED_KEYS - {"expires_at", "key_id", "limitations"}


class ReceiptError(ValueError):
    """Raised when a receipt is malformed, oversized, or unauthenticated."""


@dataclasses.dataclass(frozen=True)
class ReceiptInput:
    edition_id: str
    epoch_id: str
    manifest_sha256: str
    canonical_root_sha256: str
    projection_root_sha256: str
    source_registry_root_sha256: str
    recipient_scope: str
    recipient_salt: bytes
    key_id: str = "LAB-HMAC-01"
    expires_at: str | None = None


def _canonical_json(value: Mapping[str, Any]) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def _b64u_encode(raw: bytes) -> str:
    return base64.urlsafe_b64encode(raw).rstrip(b"=").decode("ascii")


def _b64u_decode(text: str) -> bytes:
    if not text or not re.fullmatch(r"[A-Za-z0-9_-]+", text):
        raise ReceiptError("invalid base64url transport")
    padding = "=" * ((4 - len(text) % 4) % 4)
    try:
        return base64.urlsafe_b64decode(text + padding)
    except Exception as exc:  # binascii.Error differs across Python versions
        raise ReceiptError("invalid base64url transport") from exc


def _require_identifier(name: str, value: str, maximum: int = 128) -> str:
    value = value.strip()
    if not value or len(value) > maximum:
        raise ReceiptError(f"{name} must contain 1..{maximum} characters")
    if any(ord(ch) < 32 for ch in value):
        raise ReceiptError(f"{name} contains control characters")
    return value


def _require_sha256(name: str, value: str) -> str:
    value = value.lower()
    if not SHA256_RE.fullmatch(value):
        raise ReceiptError(f"{name} must be a lowercase SHA-256 hex digest")
    return value


def _parse_time(value: str) -> dt.datetime:
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ReceiptError("invalid RFC3339 datetime") from exc
    if parsed.tzinfo is None:
        raise ReceiptError("datetime must include timezone")
    return parsed.astimezone(dt.timezone.utc)


def _utc_now_text(now: dt.datetime | None = None) -> str:
    current = now or dt.datetime.now(dt.timezone.utc)
    if current.tzinfo is None:
        raise ReceiptError("now must include timezone")
    return current.astimezone(dt.timezone.utc).isoformat(timespec="seconds").replace(
        "+00:00", "Z"
    )


def recipient_scope_commitment(scope: str, salt: bytes, edition_id: str) -> str:
    scope = _require_identifier("recipient_scope", scope, maximum=512)
    edition_id = _require_identifier("edition_id", edition_id)
    if len(salt) < 16:
        raise ReceiptError("recipient_salt must contain at least 16 bytes")
    payload = b"\x00".join(
        (scope.encode("utf-8"), edition_id.encode("utf-8"), salt)
    )
    return hashlib.sha256(payload).hexdigest()


def _unsigned_body(
    receipt_input: ReceiptInput,
    *,
    nonce: str,
    issued_at: str,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "version": VERSION,
        "edition_id": _require_identifier("edition_id", receipt_input.edition_id),
        "epoch_id": _require_identifier("epoch_id", receipt_input.epoch_id),
        "manifest_sha256": _require_sha256(
            "manifest_sha256", receipt_input.manifest_sha256
        ),
        "canonical_root_sha256": _require_sha256(
            "canonical_root_sha256", receipt_input.canonical_root_sha256
        ),
        "projection_root_sha256": _require_sha256(
            "projection_root_sha256", receipt_input.projection_root_sha256
        ),
        "source_registry_root_sha256": _require_sha256(
            "source_registry_root_sha256",
            receipt_input.source_registry_root_sha256,
        ),
        "recipient_scope_commitment": recipient_scope_commitment(
            receipt_input.recipient_scope,
            receipt_input.recipient_salt,
            receipt_input.edition_id,
        ),
        "issued_at": issued_at,
        "expires_at": receipt_input.expires_at,
        "nonce": _require_identifier("nonce", nonce),
        "authentication_method": AUTH_METHOD,
        "key_id": _require_identifier("key_id", receipt_input.key_id),
        "claim_allowed": CLAIM_ALLOWED,
        "limitations": [
            "QR payload is publicly readable",
            "HMAC is not a public digital signature",
            "receipt requires an independent issuance ledger",
        ],
    }
    _parse_time(issued_at)
    if receipt_input.expires_at is not None:
        if _parse_time(receipt_input.expires_at) <= _parse_time(issued_at):
            raise ReceiptError("expires_at must be later than issued_at")
    return body


def issue_receipt(
    receipt_input: ReceiptInput,
    key: bytes,
    *,
    nonce: str | None = None,
    now: dt.datetime | None = None,
) -> dict[str, Any]:
    if len(key) < 16:
        raise ReceiptError("laboratory HMAC key must contain at least 16 bytes")
    nonce_value = nonce or secrets.token_urlsafe(18)
    body = _unsigned_body(
        receipt_input,
        nonce=nonce_value,
        issued_at=_utc_now_text(now),
    )
    receipt_id = "RCPT-" + hashlib.sha256(_canonical_json(body)).hexdigest()[:24].upper()
    body["receipt_id"] = receipt_id
    authenticator = hmac.new(key, _canonical_json(body), hashlib.sha256).digest()
    body["authenticator"] = _b64u_encode(authenticator)
    return body


def encode_transport(receipt: Mapping[str, Any]) -> str:
    validate_structure(receipt)
    compressed = zlib.compress(_canonical_json(dict(receipt)), level=9)
    if len(compressed) > MAX_COMPRESSED_BYTES:
        raise ReceiptError("compressed receipt exceeds transport limit")
    return PREFIX + _b64u_encode(compressed)


def _bounded_decompress(compressed: bytes) -> bytes:
    if len(compressed) > MAX_COMPRESSED_BYTES:
        raise ReceiptError("compressed receipt exceeds transport limit")
    decoder = zlib.decompressobj()
    raw = decoder.decompress(compressed, MAX_DECOMPRESSED_BYTES + 1)
    if len(raw) > MAX_DECOMPRESSED_BYTES or decoder.unconsumed_tail:
        raise ReceiptError("decompressed receipt exceeds limit")
    raw += decoder.flush()
    if len(raw) > MAX_DECOMPRESSED_BYTES:
        raise ReceiptError("decompressed receipt exceeds limit")
    if not decoder.eof or decoder.unused_data:
        raise ReceiptError("invalid or concatenated compressed stream")
    return raw


def decode_transport(payload: str) -> dict[str, Any]:
    if not payload.startswith(PREFIX):
        raise ReceiptError("unsupported QR custody prefix")
    compressed = _b64u_decode(payload[len(PREFIX) :])
    raw = _bounded_decompress(compressed)
    try:
        value = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ReceiptError("invalid receipt JSON") from exc
    if not isinstance(value, dict):
        raise ReceiptError("receipt root must be an object")
    validate_structure(value)
    return value


def validate_structure(receipt: Mapping[str, Any]) -> None:
    keys = set(receipt)
    missing = REQUIRED_KEYS - keys
    extra = keys - ALLOWED_KEYS
    if missing:
        raise ReceiptError(f"missing fields: {sorted(missing)}")
    if extra:
        raise ReceiptError(f"unexpected fields: {sorted(extra)}")
    if receipt.get("version") != VERSION:
        raise ReceiptError("unsupported receipt version")
    if receipt.get("authentication_method") != AUTH_METHOD:
        raise ReceiptError("unsupported authentication method")
    if receipt.get("claim_allowed") is not False:
        raise ReceiptError("claim_allowed must remain false")
    for field in (
        "manifest_sha256",
        "canonical_root_sha256",
        "projection_root_sha256",
        "source_registry_root_sha256",
        "recipient_scope_commitment",
    ):
        _require_sha256(field, str(receipt.get(field, "")))
    for field in ("receipt_id", "edition_id", "epoch_id", "nonce"):
        _require_identifier(field, str(receipt.get(field, "")))
    _parse_time(str(receipt.get("issued_at", "")))
    expires_at = receipt.get("expires_at")
    if expires_at is not None:
        _parse_time(str(expires_at))
    authenticator = str(receipt.get("authenticator", ""))
    decoded_auth = _b64u_decode(authenticator)
    if len(decoded_auth) != hashlib.sha256().digest_size:
        raise ReceiptError("authenticator must contain one SHA-256 HMAC")


def verify_receipt(
    payload: str,
    key: bytes,
    *,
    now: dt.datetime | None = None,
) -> dict[str, Any]:
    if len(key) < 16:
        raise ReceiptError("laboratory HMAC key must contain at least 16 bytes")
    receipt = decode_transport(payload)
    supplied = str(receipt["authenticator"])
    unsigned = dict(receipt)
    unsigned.pop("authenticator", None)
    expected = _b64u_encode(hmac.new(key, _canonical_json(unsigned), hashlib.sha256).digest())
    authentic = hmac.compare_digest(supplied, expected)

    current = now or dt.datetime.now(dt.timezone.utc)
    if current.tzinfo is None:
        raise ReceiptError("now must include timezone")
    expired = False
    if receipt.get("expires_at") is not None:
        expired = current.astimezone(dt.timezone.utc) > _parse_time(
            str(receipt["expires_at"])
        )

    states: list[str] = []
    states.append("VALID_AUTHENTICATOR" if authentic else "INVALID_AUTHENTICATOR")
    if expired:
        states.append("EXPIRED")
    states.append("LEDGER_LOOKUP_REQUIRED")
    states.append("ROOT_COMPARISON_REQUIRED")

    return {
        "valid": authentic and not expired,
        "states": states,
        "receipt_id": receipt["receipt_id"],
        "edition_id": receipt["edition_id"],
        "epoch_id": receipt["epoch_id"],
        "payload_sha256": hashlib.sha256(payload.encode("ascii")).hexdigest(),
        "receipt": receipt,
        "claim_allowed": False,
    }


def render_svg(payload: str, output: str | os.PathLike[str]) -> Path:
    """Render a standards-compliant QR SVG when the optional adapter exists."""
    try:
        import qrcode  # type: ignore[import-not-found]
        import qrcode.image.svg  # type: ignore[import-not-found]
    except ImportError as exc:
        raise ReceiptError(
            "optional QR renderer unavailable; install the audited qrcode adapter"
        ) from exc

    image = qrcode.make(
        payload,
        image_factory=qrcode.image.svg.SvgPathImage,
        error_correction=qrcode.constants.ERROR_CORRECT_Q,
        box_size=6,
        border=4,
    )
    path = Path(output)
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)
    return path


def _sha(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def run_demo(output_dir: str | os.PathLike[str]) -> dict[str, Any]:
    key = b"LAB-QR-CUSTODY-KEY-REPLACE-ME"
    salt = b"LAB-RECIPIENT-SALT-20260726"
    now = dt.datetime(2026, 7, 26, 6, 30, tzinfo=dt.timezone.utc)
    receipt_input = ReceiptInput(
        edition_id="ED-LAB-20260726-001",
        epoch_id="EPOCH-LAB-001",
        manifest_sha256=_sha("manifest-lab"),
        canonical_root_sha256=_sha("canonical-lab"),
        projection_root_sha256=_sha("projection-lab"),
        source_registry_root_sha256=_sha("source-registry-lab"),
        recipient_scope="TENANT-LAB|ESTACAO-07|FISCAL-OPERADOR",
        recipient_salt=salt,
    )
    receipt = issue_receipt(
        receipt_input,
        key,
        nonce="LAB-NONCE-20260726-0001",
        now=now,
    )
    payload = encode_transport(receipt)
    verification = verify_receipt(payload, key, now=now)

    output = Path(output_dir)
    output.mkdir(parents=True, exist_ok=True)
    (output / "receipt.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    (output / "payload.txt").write_text(payload + "\n", encoding="ascii")
    (output / "verification.json").write_text(
        json.dumps(verification, ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    renderer_state = "TOKEN_VAZIO_RENDERER"
    try:
        render_svg(payload, output / "receipt.svg")
        renderer_state = "SVG_RENDERED_OPTIONAL_ADAPTER"
    except ReceiptError:
        pass

    return {
        "status": "VERIFIED_LIMITED_LOCAL" if verification["valid"] else "FAIL",
        "receipt_id": receipt["receipt_id"],
        "payload_sha256": verification["payload_sha256"],
        "payload_length": len(payload),
        "recipient_cleartext_present": receipt_input.recipient_scope in payload,
        "verification_states": verification["states"],
        "renderer_state": renderer_state,
        "claim_allowed": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", default="build/qr-custody")
    args = parser.parse_args()
    result = run_demo(args.output_dir)
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))
    return 0 if result["status"] == "VERIFIED_LIMITED_LOCAL" else 1


if __name__ == "__main__":
    raise SystemExit(main())
