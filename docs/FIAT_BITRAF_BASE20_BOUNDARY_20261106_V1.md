# FIAT ↔ BITRAF: strict base20 boundary (2026-11-06)

State: DRAFT/HOLD, claim_allowed=false. Producer: rafaelmeloreisnovo/RafPolimata.

## Source
Existing bitraf_matrix.h = 8000 indexed states (10³×4×2), 4096 core, 3904 shell. Existing W20 = 256000 (state,subvalue) pairs, parity not ECC. Existing EV3 = P/N/A masks, not durable provenance. Existing RAFDB zipraf adapter = RDB1 boundary, not a ZIPRAF archive writer.

## Concrete API gap and correction
Legacy bitraf_to_base20_3 is void and has no range guard: index 8000 produces K00, outside 0123456789ABCDEFGHIJ. The legacy decoder rejects K00; existing stored data is not proven corrupted. New opt-in wrapper bitraf_base20_checked_v1.h validates the index and output before encoding, rejects malformed/non-canonical lowercase input, and leaves output unchanged on errors. Existing headers and binary format remain byte-for-byte untouched.

## Epistemic boundary
The user-provided symbolic BITRAF64 has 64 Unicode glyphs; provided JS emits 60 distinct deterministic glyph positions. No reversible equivalence to BITRAF8000, W20, base60 or ZIPRAF is defined. 6000 (10×10×10×6) is not 8000 (10³×4×2); they are separate domains. The strict wrapper only reduces an input-validation hazard in base20.

## Gates
- Tests exhaustively check all 8000 indices, state roundtrip, digits20 EV3 consistency, negative index 8000/65535, null, lowercase, length, reserved glyph, outputs unchanged.
- Hosted GCC/Clang exact HEAD plus ARMv7 freestanding object are proposed CI gates, not device measurements.
- Original Termux file bytes, actual BLAKE3, ZIPRAF persistence, source-to-format mapping, base60, human approval and physical ARM32 remain TOKEN_VAZIO.
- No source files outside these additions altered; no merge, release, database or device changes. Rollback = close draft branch, retain predecessor and issue successor receipt.
