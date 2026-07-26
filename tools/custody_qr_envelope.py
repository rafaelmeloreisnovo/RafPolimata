#!/usr/bin/env python3
"""RAFAELIA custody QR envelope.

The QR is a portable checkpoint/pointer for a custody chain. It MUST NOT contain
PII, secrets, credentials, raw business records, or a complete database.

Security boundary:
- SHA-256 + CRC32 detect accidental or unsophisticated modification.
- Optional HMAC-SHA256 authenticates an internal checkpoint.
- HMAC is not a third-party-verifiable digital signature.
- A QR code is not proof by itself; it points to manifests, commits and gates.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import hmac
import json
import os
from pathlib import Path
import zlib
from typing import Any

PREFIX = "RQC1:"
SCHEMA = "RAFCUSTODY-QR/1"
PROHIBITED_KEY_FRAGMENTS = (
    "cpf", "email", "phone", "address", "password", "secret",
    "credential", "private_key", "raw_data", "raw_record",
)


def canonical_json(value: object) -> bytes:
    return json.dumps(
        value, ensure_ascii=True, sort_keys=True, separators=(",", ":")
    ).encode("ascii")


def default_checkpoint_body() -> dict[str, Any]:
    """Public-safe checkpoint for the first forensic-deception implementation."""
    return {
        "schema": SCHEMA,
        "snapshot_id": "FDV1-20260726-QR01",
        "epoch_id": "E1",
        "scope": "FORENSIC-DECEPTION-CUSTODY",
        "claim_allowed": False,
        "gates": {
            "local": "VERIFIED_LIMITED_LOCAL",
            "remote": "TOKEN_VAZIO_RUNNER",
            "production": "BLOCKED",
        },
        "baseline_heads": {
            "Mapa#65": "dd853548d58b68177b25dd85be7028343b3475da",
            "papers#24": "0c82658fb5069c71a79fb057e7e8c6cd163b9c27",
            "RafPolimata#166": "1fb036a3d124a111abfd08356f6b4d6ce49329c5",
            "RafGitTools#309": "dfed9ed546c4e86f8bf499d63b548e2e96b7a835",
        },
        "privacy": "NO_PII_NO_SECRET_NO_RAW_DATA",
        "auth_state": "UNSIGNED_DIGEST_CHECKPOINT",
    }


def _walk_keys(value: object) -> list[str]:
    keys: list[str] = []
    if isinstance(value, dict):
        for key, child in value.items():
            keys.append(str(key).lower())
            keys.extend(_walk_keys(child))
    elif isinstance(value, list):
        for child in value:
            keys.extend(_walk_keys(child))
    return keys


def validate_public_safe_body(body: dict[str, Any]) -> None:
    if body.get("schema") != SCHEMA:
        raise ValueError(f"schema must be {SCHEMA!r}")
    if body.get("claim_allowed") is not False:
        raise ValueError("claim_allowed must remain false for this checkpoint")
    for key in _walk_keys(body):
        if any(fragment in key for fragment in PROHIBITED_KEY_FRAGMENTS):
            raise ValueError(f"prohibited sensitive key in QR body: {key}")


def build_envelope(
    body: dict[str, Any], hmac_key: bytes | None = None
) -> dict[str, Any]:
    validate_public_safe_body(body)
    body_bytes = canonical_json(body)
    envelope: dict[str, Any] = {
        "body": body,
        "sha256": hashlib.sha256(body_bytes).hexdigest(),
        "crc32": f"{zlib.crc32(body_bytes) & 0xffffffff:08x}",
        "auth": {"kind": "NONE", "tag": None},
    }
    if hmac_key is not None:
        envelope["auth"] = {
            "kind": "HMAC-SHA256",
            "tag": hmac.new(hmac_key, body_bytes, hashlib.sha256).hexdigest(),
        }
    return envelope


def verify_envelope(
    envelope: dict[str, Any], hmac_key: bytes | None = None
) -> tuple[bool, list[str]]:
    errors: list[str] = []
    body = envelope.get("body")
    if not isinstance(body, dict):
        return False, ["body is missing or is not an object"]

    try:
        validate_public_safe_body(body)
    except ValueError as exc:
        errors.append(str(exc))

    body_bytes = canonical_json(body)
    expected_sha = hashlib.sha256(body_bytes).hexdigest()
    expected_crc = f"{zlib.crc32(body_bytes) & 0xffffffff:08x}"
    if not hmac.compare_digest(str(envelope.get("sha256", "")), expected_sha):
        errors.append("sha256 mismatch")
    if not hmac.compare_digest(str(envelope.get("crc32", "")), expected_crc):
        errors.append("crc32 mismatch")

    auth = envelope.get("auth")
    if not isinstance(auth, dict):
        errors.append("auth object missing")
    else:
        kind = auth.get("kind")
        tag = auth.get("tag")
        if kind == "NONE":
            if tag is not None:
                errors.append("NONE auth must not carry a tag")
        elif kind == "HMAC-SHA256":
            if hmac_key is None:
                errors.append("HMAC key required")
            else:
                expected_tag = hmac.new(
                    hmac_key, body_bytes, hashlib.sha256
                ).hexdigest()
                if not hmac.compare_digest(str(tag or ""), expected_tag):
                    errors.append("HMAC mismatch")
        else:
            errors.append(f"unsupported auth kind: {kind!r}")

    return not errors, errors


def encode_qr_payload(envelope: dict[str, Any]) -> str:
    compressed = zlib.compress(canonical_json(envelope), level=9)
    token = base64.urlsafe_b64encode(compressed).rstrip(b"=").decode("ascii")
    return PREFIX + token


def decode_qr_payload(payload: str) -> dict[str, Any]:
    if not payload.startswith(PREFIX):
        raise ValueError("invalid QR custody prefix")
    token = payload[len(PREFIX):]
    token += "=" * (-len(token) % 4)
    try:
        raw = zlib.decompress(base64.urlsafe_b64decode(token.encode("ascii")))
        value = json.loads(raw.decode("ascii"))
    except (ValueError, zlib.error, json.JSONDecodeError) as exc:
        raise ValueError("invalid QR custody payload") from exc
    if not isinstance(value, dict):
        raise ValueError("decoded envelope must be an object")
    return value


def render_svg(payload: str, output: Path) -> None:
    try:
        import qrcode
        from qrcode.image.svg import SvgPathImage
    except ImportError as exc:
        raise RuntimeError(
            "SVG rendering is optional and requires the 'qrcode' package"
        ) from exc

    qr = qrcode.QRCode(
        version=None,
        error_correction=qrcode.constants.ERROR_CORRECT_M,
        box_size=4,
        border=4,
    )
    qr.add_data(payload)
    qr.make(fit=True)
    image = qr.make_image(image_factory=SvgPathImage)
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)


def _hmac_key_from_env(name: str | None) -> bytes | None:
    if not name:
        return None
    value = os.environ.get(name)
    if value is None:
        raise SystemExit(f"environment variable {name!r} is not set")
    return value.encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)

    build = sub.add_parser("build")
    build.add_argument("--body-json", type=Path)
    build.add_argument("--output-envelope", type=Path, required=True)
    build.add_argument("--output-payload", type=Path, required=True)
    build.add_argument("--output-svg", type=Path)
    build.add_argument("--hmac-key-env")

    verify = sub.add_parser("verify")
    verify.add_argument("--payload", type=Path, required=True)
    verify.add_argument("--hmac-key-env")

    args = parser.parse_args()

    if args.command == "build":
        body = (
            json.loads(args.body_json.read_text(encoding="utf-8"))
            if args.body_json
            else default_checkpoint_body()
        )
        key = _hmac_key_from_env(args.hmac_key_env)
        envelope = build_envelope(body, key)
        payload = encode_qr_payload(envelope)
        args.output_envelope.parent.mkdir(parents=True, exist_ok=True)
        args.output_payload.parent.mkdir(parents=True, exist_ok=True)
        args.output_envelope.write_text(
            json.dumps(envelope, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        args.output_payload.write_text(payload + "\n", encoding="ascii")
        if args.output_svg:
            render_svg(payload, args.output_svg)
        print(json.dumps({
            "schema": SCHEMA,
            "snapshot_id": body["snapshot_id"],
            "payload_length": len(payload),
            "sha256": envelope["sha256"],
            "crc32": envelope["crc32"],
            "auth_kind": envelope["auth"]["kind"],
            "claim_allowed": False,
        }, sort_keys=True))
        return 0

    key = _hmac_key_from_env(args.hmac_key_env)
    payload = args.payload.read_text(encoding="ascii").strip()
    envelope = decode_qr_payload(payload)
    valid, errors = verify_envelope(envelope, key)
    print(json.dumps({"valid": valid, "errors": errors}, sort_keys=True))
    return 0 if valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
