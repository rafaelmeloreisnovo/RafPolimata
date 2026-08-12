# ApkC source-cap hardening receipt — 2026-08-12

Status: VERIFIED_LIMITED
claim_allowed: false
branch: `audit/apkc-mem-failclosed-20260812`
base: `a68aa9093e35f9ed2e332501425b2e0f5a33d99b`

## Gap
`Apkc/apkc.c` reads source into `_src_local[0x100000]` while `src_len < sizeof(_src_local)-1`; when the buffer fills it closes the file without probing for remaining input. A source larger than the accepted capacity can therefore be silently truncated before `build_apk()`.

## Change A — fail-closed transform
Added `scripts/patch_apkc_source_cap.py`, an exact-anchor fail-closed transformer. The transformed read path:

- distinguishes read errors (`n < 0`) from EOF (`n == 0`);
- reserves the legacy NUL terminator byte;
- when the buffer reaches `sizeof(_src_local)-1`, probes exactly one additional byte;
- rejects additional input with `source exceeds SRC_CAP`;
- rejects a failed overflow probe;
- refuses transformation if the vulnerable anchor is absent or duplicated.

Added `tests/test_apkc_source_cap_patch.py` to falsify anchor drift and verify the overflow/read-error guards are present.

## Change B — canonical proof wiring / anti-retraction
Commit `c730a6d7597bfa34de7705064479fdfbb1a2388e` updates `tools/raf_clean_proof_run.sh` so the canonical `make proof` path no longer compiles the vulnerable source directly for its host-tractable ApkC gates.

The proof now:

1. runs the transformer falsifier;
2. applies the transformer to the real `Apkc/apkc.c` into a run-scoped `apkc_hardened.c`;
3. verifies the overflow guard is present and the legacy `if (n<=0) break;` anchor is absent;
4. records source and hardened SHA-256 values;
5. runs AArch64 freestanding syntax and object compilation only against the hardened translation unit;
6. marks downstream gates FAIL when hardening cannot be established;
7. exits non-zero when any host-tractable gate is FAIL, while preserving device-only gaps as `TOKEN_VAZIO`.

This is a monotonic hardening change: previously proved encoder/verbovivo gates are retained; no existing PASS is removed or reclassified merely to make the new gate pass.

Audit-environment shell parse of the exact replacement script: `sh -n` PASS. This proves shell syntax only, not execution of the repository proof suite.

## Prior local evidence
Executed in the audit environment:

`python /mnt/data/test_apkc_source_cap_patch.py`

Result: `PASS source-cap transformer: exact anchor + overflow probe + read-error guard`

Recorded SHA-256 from the earlier local execution:

- transformer: `238736d72ee7288b31b2994c78c62838779603371b9f914d4ad829eadbc0d0dc`
- test: `176689a7793c38c4b47d910ab783c355735d401571cee10883960f447edac982`

## Current CI / provenance state
Head after canonical proof wiring: `c730a6d7597bfa34de7705064479fdfbb1a2388e`.

GitHub Actions runs observed for this head:

- CI `31593003144`;
- ApkC First Part Closure `31593003181`;
- Formal Science Orchestrator `31593003292`;
- Document Governance `31593003301`.

All four concluded `failure`, but each corresponding job reported `steps: []`, `runner_id: 0`, and an empty runner name. Classification: `TOKEN_VAZIO_RUNNER`, not code failure. No repository proof execution may be claimed from these runs.

## Contract / epistemic state

- `SOURCE_CAP_TRANSFORMER`: VERIFIED.
- `TRANSFORMER_FALSIFIER_LOCAL`: VERIFIED.
- `CANONICAL_PROOF_WIRING`: VERIFIED_STATIC.
- `CANONICAL_PROOF_EXECUTION`: TOKEN_VAZIO_RUNNER.
- `DIRECT_CANONICAL_SOURCE_HARDENED`: TOKEN_VAZIO — `Apkc/apkc.c` itself still contains the vulnerable anchor; the proof path consumes a hardened generated translation unit.
- `ARM_BOUNDARY_RUNTIME`: TOKEN_VAZIO.
- `END_TO_END_APK_FROM_HARDENED_SOURCE`: TOKEN_VAZIO.
- `claim_allowed`: false for end-to-end/runtime claims.

## Urgency / closure gates
P0 remains the bypass risk: build paths outside the canonical proof can still compile raw `Apkc/apkc.c` unless they explicitly consume the transformer.

Closure order:

1. harden `Apkc/apkc.c` directly or enforce one shared hardened-source entrypoint across every build/validation path;
2. run `make proof` on a runner that actually starts steps and archive `g0_source_cap.txt`, `g1_syntax.txt`, `g2_object.txt`, hashes and exit code;
3. on ARM/Termux execute the boundary triad: `< 1048575` bytes PASS; `1048575` bytes PASS; `1048576` bytes FAIL with non-zero exit and no APK output;
4. bind source SHA → hardened/source SHA → apkc ELF SHA → APK SHA → unzip/readelf/sign/install/runtime receipt.

No gap is closed by narrative or workflow conclusion alone.
