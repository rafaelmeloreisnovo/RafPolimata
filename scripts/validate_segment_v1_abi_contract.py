#!/usr/bin/env python3
"""Validate the frozen segment.v1 ABI and emit an auditable reconciliation report."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any

SCHEMA = "raf.segment-abi-reconciliation-report.v1"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument(
        "--contract",
        type=Path,
        default=Path("runtime/conversation_indexer/SEGMENT_V1_ABI_CONTRACT.json"),
    )
    parser.add_argument(
        "--header",
        type=Path,
        default=Path("runtime/conversation_indexer/raf_segment_v1.h"),
    )
    parser.add_argument(
        "--codec",
        type=Path,
        default=Path("runtime/conversation_indexer/raf_segment_v1.c"),
    )
    parser.add_argument(
        "--proposal",
        type=Path,
        default=Path("docs/copilot/TASK_02_CONVERSATION_SEGMENTS_V1.md"),
    )
    parser.add_argument("--output", required=True, type=Path)
    return parser.parse_args()


def repo_path(repo: Path, path: Path) -> Path:
    return path if path.is_absolute() else repo / path


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def macro_value(text: str, name: str) -> int | None:
    match = re.search(rf"^#define\s+{re.escape(name)}\s+([^\s/]+)", text, re.MULTILINE)
    if match is None:
        return None
    token = match.group(1).rstrip("uUlL")
    try:
        return int(token, 0)
    except ValueError:
        return None


def required_codec_pattern(width: int, offset: int, field: str) -> re.Pattern[str]:
    function = "put_u32le" if width == 4 else "put_u64le"
    return re.compile(
        rf"{function}\(out\s*\+\s*{offset}u,\s*record->{re.escape(field)}\)"
    )


def inspect_record(
    codec: str,
    contract_record: dict[str, Any],
    record_name: str,
) -> tuple[list[dict[str, Any]], list[str]]:
    checks: list[dict[str, Any]] = []
    errors: list[str] = []
    for field in contract_record["fields"]:
        name = field["name"]
        offset = int(field["offset"])
        width = int(field["width"])
        if name in {"record_crc32c", "reserved"}:
            if name == "record_crc32c":
                pattern = re.compile(rf"put_u32le\(out\s*\+\s*{offset}u,\s*crc\)")
            else:
                pattern = re.compile(rf"put_u32le\(out\s*\+\s*{offset}u,\s*0u\)")
        else:
            pattern = required_codec_pattern(width, offset, name)
        passed = pattern.search(codec) is not None
        checks.append(
            {
                "record": record_name,
                "field": name,
                "offset": offset,
                "width": width,
                "encode_pattern_present": passed,
            }
        )
        if not passed:
            errors.append(f"{record_name}.{name} encoding offset/width mismatch")
    return checks, errors


def main() -> int:
    args = parse_args()
    repo = args.repo.resolve()
    paths = {
        "contract": repo_path(repo, args.contract),
        "header": repo_path(repo, args.header),
        "codec": repo_path(repo, args.codec),
        "proposal": repo_path(repo, args.proposal),
    }
    errors: list[str] = []
    for label, path in paths.items():
        if not path.is_file():
            errors.append(f"missing {label}: {path}")

    contract: dict[str, Any] = {}
    header = ""
    codec = ""
    proposal = ""
    if not errors:
        try:
            contract = json.loads(paths["contract"].read_text(encoding="utf-8"))
            header = paths["header"].read_text(encoding="utf-8")
            codec = paths["codec"].read_text(encoding="utf-8")
            proposal = paths["proposal"].read_text(encoding="utf-8")
        except (OSError, UnicodeError, json.JSONDecodeError) as error:
            errors.append(f"input parse error: {error}")

    macro_checks: list[dict[str, Any]] = []
    role_checks: list[dict[str, Any]] = []
    field_checks: list[dict[str, Any]] = []
    proposal_conflicts: list[dict[str, Any]] = []

    if not errors:
        expected_macros = {
            "RAF_SEGMENT_V1_HEADER_SIZE": contract["file_header"]["serialized_size"],
            "RAF_SEGMENT_V1_CONVERSATION_SIZE": contract["conversation_record"]["serialized_size"],
            "RAF_SEGMENT_V1_MESSAGE_SIZE": contract["message_record"]["serialized_size"],
            "RAF_SEGMENT_V1_VERSION": int(contract["file_header"]["version_hex"], 0),
        }
        for name, expected in expected_macros.items():
            observed = macro_value(header, name)
            passed = observed == expected
            macro_checks.append(
                {"macro": name, "expected": expected, "observed": observed, "passed": passed}
            )
            if not passed:
                errors.append(f"macro mismatch {name}: expected {expected}, got {observed}")

        for role, expected in contract["roles"].items():
            macro = f"RAF_SEGMENT_ROLE_{role}"
            observed = macro_value(header, macro)
            passed = observed == expected
            role_checks.append(
                {"role": role, "expected": expected, "observed": observed, "passed": passed}
            )
            if not passed:
                errors.append(f"role mismatch {role}: expected {expected}, got {observed}")

        magic_pattern = re.compile(
            r"'R'\s*,\s*'A'\s*,\s*'F'\s*,\s*'S'\s*,\s*'E'\s*,\s*'G'\s*,\s*'1'\s*,\s*0"
        )
        if magic_pattern.search(codec) is None:
            errors.append("RAFSEG1 magic mismatch")

        conversation_checks, conversation_errors = inspect_record(
            codec, contract["conversation_record"], "conversation"
        )
        message_checks, message_errors = inspect_record(
            codec, contract["message_record"], "message"
        )
        field_checks.extend(conversation_checks)
        field_checks.extend(message_checks)
        errors.extend(conversation_errors)
        errors.extend(message_errors)

        conflict_needles = {
            "proposal_32_byte_identity": "uint8_t  id_hash[32]",
            "proposal_parent_32_byte_identity": "uint8_t  parent_id_hash[32]",
            "proposal_role_system_1": "1 SYSTEM",
            "proposal_role_user_2": "2 USER",
            "proposal_role_assistant_3": "3 ASSISTANT",
            "proposal_message_count_u64": "uint64_t message_count",
            "proposal_source_start_end": "source_start",
        }
        for conflict, needle in conflict_needles.items():
            present = needle in proposal
            proposal_conflicts.append(
                {"conflict": conflict, "needle": needle, "present": present}
            )
            if not present:
                errors.append(f"expected documented proposal conflict missing: {conflict}")

        if contract["compatibility_decision"]["segment_v1"] != "PRESERVE_CURRENT_CODEC_AND_WIRE_LAYOUT":
            errors.append("contract does not preserve frozen segment.v1")
        if contract["compatibility_decision"]["task_02_32_byte_identity_layout"] != "RECLASSIFY_AS_SEGMENT_V2_CANDIDATE":
            errors.append("32-byte proposal is not explicitly reclassified")

    state = "FAIL" if errors else "PASS_WITH_RECONCILED_SPEC_CONFLICTS"
    report = {
        "schema": SCHEMA,
        "cycle_id": "C03",
        "state": state,
        "claim_allowed": False,
        "inputs": {
            label: {
                "path": str(path.relative_to(repo)) if path.is_file() else str(path),
                "sha256": sha256_file(path) if path.is_file() else "TOKEN_VAZIO",
            }
            for label, path in paths.items()
        },
        "macro_checks": macro_checks,
        "role_checks": role_checks,
        "field_checks": field_checks,
        "proposal_conflicts": proposal_conflicts,
        "decision": contract.get("compatibility_decision", {}),
        "identity_boundary": contract.get("identity", {}),
        "extractor_gate": {
            "segment_v1_wire_layout": "FROZEN" if not errors else "NOT_VERIFIED",
            "full_blake3_256_identity_in_v1_records": "FORBIDDEN_BY_WIDTH",
            "opaque_128_identity": "CALLER_SUPPLIED_ONLY_UNTIL_CRYPTO_GATE",
            "segment_v2_32_byte_identity": "DESIGN_CANDIDATE_NOT_IMPLEMENTED",
            "streaming_extractor": "TOKEN_VAZIO_NOT_IMPLEMENTED",
            "atomic_writer": "TOKEN_VAZIO_NOT_IMPLEMENTED",
            "checkpoint_resume": "TOKEN_VAZIO_NOT_IMPLEMENTED",
        },
        "errors": sorted(set(errors)),
        "falsifiers": [
            "codec_macro_differs_from_contract",
            "record_field_offset_differs_from_contract",
            "role_number_differs_from_contract",
            "writer_labels_128_bit_identity_as_full_blake3_256",
            "in_place_v1_relayout",
            "in_place_v1_role_renumbering",
        ],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"{state} segment.v1 ABI reconciliation: {args.output}")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
