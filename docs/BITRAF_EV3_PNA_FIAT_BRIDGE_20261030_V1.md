# BITRAF EV3 — FIAT LUX × FIAT ∅ — Evidence overlay v1

**2026-10-30 · DRAFT / NÃO PROMOVER · claim_allowed=false**  
Owner of producer: `rafaelmeloreisnovo/RafPolimata`. Federation routes are owned by `Mapa`; private custody/longitudinal receipts belong in Drive NEXUS. This file is an ADDITIVE experiment, not a modification of the BITRAF64 storage format.

## 1. Historical compatibility boundary

Exact source of truth: `rafaelia/bitraf_matrix.h` and `rafaelia/bitraf_w20_bridge_v1.h` at the PR base commit `87c074ba5bf1fe364e634d87591aa4ec2874a0f1`.

- Historical state is `S=(x,y,z,vertex,parity)`, with `x,y,z∈[0,9]`, `vertex∈[0,3]`, `parity∈[0,1]`. `1000×4×2=8000=20³` is a cardinality equality, NOT equality of geometries.
- Geometric CORE4096 is `8³×4×2` with coordinates 1..8. SHELL3904 covers the remaining historical states. `index8000` and `core_index4096` are unchanged.
- A new `20³` numeral conversion of `index8000` is reversible: `(index/400, (index/20)%20, index%20)`. This is *not* a new location `(x,y,z)` and it is *not* the historical core index.
- Historical 20-glyph alphabet: `0123456789ABCDEFGHIJ`. QWERTY/ASDF/custom 20-glyph sets may be explicit versioned **display codebooks**, but must not silently change persisted data. Three sets of 20 typed symbols are 60 labels; that alone is not a base-60 wire protocol.
- W20 remains `A10 || V8 || pA || pV` and represents `(state8000, subvalue5)` with 256000 combinations. A parity bit is NOT error-correction capability. Double-bit errors within a field can escape.

## 2. Overlay, NOT core replacement

`rafaelia/bitraf_evidence_planes_v1.h` defines three bitplanes for each index 0..7999: `P=4` positive, `N=2` negative, `A=1` explicitly verified absence. These masks yield 8 states `000..111`; `000` is UNKNOWN / `TOKEN_VAZIO` and is not A, not zero-valued observation, not FAIL and not PASS.

There are **2 serialization pages** of **4096 bits per plane**, each with 3 planes. Raw buffer = `2×3×512=3072 bytes`. First page indexes 0..4095; second 4096..7999. The remaining 192 positions are unavailable and MUST NOT represent new valid historical states. Serialization page 0 is **not the geometric CORE4096**. The overlay accumulates masks with monotonic bitwise OR; no negative result is erased by a positive result, and conflicting masks 110,101,011,111 remain inspectable.

Call `raf_ev3_zero` ONLY on fresh instances before recording observations. This in-memory overlay is **not** an event ledger and cannot show provenance, identity, clock, order or authentication. External append-only receipts and an access-control policy remain required. Single-writer/single-thread test scope only; thread-safety not proven. No libc, heap, syscalls, I/O, compression or cryptography inside the new C header.

## 3. Executable falsifiers

Command on an exact producer checkout:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic tests/bitraf_evidence_planes_v1_test.c -o /tmp/raf_ev3_test
/tmp/raf_ev3_test
clang -std=c11 -O2 -Wall -Wextra -Werror -pedantic tests/bitraf_evidence_planes_v1_test.c -o /tmp/raf_ev3_clang
/tmp/raf_ev3_clang
```

- Exhaust 8000 historical states ↔ 20³ numeral mappings and CORE4096/SHELL3904 partition.
- Exhaust 256000 W20 pack/unpack pairs against the **producer header**.
- Test all eight PNA masks; unknown ≠ explicit absence; contradictions and idempotence remain.
- Test invalid index 8000, invalid digit 20, zero evidence injection, four 90° rotations, double opposite, non-mutating errors and known undetected two-bit W20 counterexample.
- Test 20 single-bit changes on one W20 vector. The pre-existing `tests/bitraf_w20_bridge_test.c` has an independent 5,120,000 single-bit mutation loop; do not claim that the new test reproduces that entire older loop.
- Discrete antiderivative and gcd octagon belong to a separate Python experiment; neither is claimed to be integrated into this freestanding header.

## 4. Eight gates and remaining typed gaps

`C1` alphabet namespace → `C2` numeral bijection → `C3` page boundaries → `C4` masks/contradictions → `C5` W20 parity and counterexample → `C6` historical core/rotation → `C7` inverse/derivative remains a separate mathematical test → `C8` receipt/fail-closed.

**LOCAL:** GCC and Clang with UBSan passed the additive test with locally reconstructed *shims* matching the observed historical formulas. **NOT YET PROVEN:** compiling this new test against the exact original GitHub headers; current PR exact-HEAD CI; ARM32 physical runtime; codebook human approval; full BITRAF64 `BRAF` wire-format interoperability; event-level timestamps/provenance, multiwriter safety, ECC/SECDED, independent review and the full historical eight-cycle lineage. All remain `TOKEN_VAZIO` or `FAIL_FOR_GENERAL_ECC` as applicable, not positive claims.

**Rollback:** close/discard this draft branch or revert ONLY the additive files. No changes to `main`, existing header bytes, secrets, accounts, user data, archival input, APK or production hardware. GitHub merge remains human-authorized. An adverse result creates a successor receipt; it does not delete prior observations.

**R3:** `F_ok` source-scoped additive candidate and local shim tests; `F_gap` exact-head provider/CI/device/human gates; `F_next` compile at immutable GitHub HEAD and compare receipts, then human review before any promotion.
