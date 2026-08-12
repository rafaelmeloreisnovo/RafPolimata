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

## Change C — validator raw-source bypass removed
Commit `f7830498bd4a77f59fb4269c4b09262d70b99d20` updates `scripts/apkc_validate.sh`.

Before this change, every compile fallback in the validator consumed raw `Apkc/apkc.c`, so validation could bypass the source-cap mitigation even though `make proof` was hardened.

The validator now:

- requires the source-cap transformer and falsifier before compilation;
- emits `H0 TOKEN_VAZIO` and exits non-zero when Python is unavailable, rather than compiling an unverified raw source;
- applies the transformer to a run-scoped `apkc_hardened.c`;
- checks the overflow guard and absence of the legacy vulnerable anchor;
- records raw and hardened SHA-256 values;
- routes native, cross-AArch64, ARM64-object and syntax fallback compilation only through the hardened source;
- preserves each compiler attempt and exit code in `apkc-compile.txt` rather than overwriting prior attempts;
- separates ARM64/ARM32 ELF gates as `F6A` / `F6B`, eliminating the duplicated F6 identifier.

Audit-environment shell parse of the exact replacement: `sh -n` PASS.
Local replacement-text SHA-256: `f49e3cd59d0038d68e8f1df86ec9d8fe65f19f3fb097ec8acc179cbebe1adaab`.
GitHub content blob after write: `8ca2d690e4a207cfb2bde9a2ad9815ee96c8a102`.

## Change D — Termux hermetic build raw-source bypass removed
Commit `c5d939c67e5016c02dfc5428dff2637c1d1b3e2a` updates `scripts/apkc_termux_hermetic_build.sh`, the physical/on-device build route.

Before this change the script explicitly compiled raw `Apkc/apkc.c` into `apkc-host`. It now fails closed before any compiler command unless the source-cap transformer/falsifier can be executed successfully.

The hermetic build now:

- requires Python3 plus the transformer/falsifier;
- creates and preserves `build/apkc-hermetic/apkc-hardened.c` as chain-of-custody material;
- rejects missing guard or surviving vulnerable anchor;
- records raw and hardened source SHA-256 values;
- compiles only the hardened translation unit with `-I Apkc`;
- records `hardening=SOURCE_CAP_VERIFIED` in the build log and receipt;
- advances receipt schema to `raf.apkc.hermetic-build.v2`;
- binds raw source SHA → hardened source SHA → ApkC host ELF SHA → unsigned/signed APK SHA when those later artifacts exist;
- keeps `claim_allowed=false`, `structural_validation=TOKEN_VAZIO`, and `install_and_runtime=TOKEN_VAZIO` until their respective evidence exists.

Audit-environment shell parse of the exact replacement: `sh -n` PASS.
Local replacement-text SHA-256: `f93f777e5321f5deaa8e066506a90be50c31af9385310c549de2438131881fd2`.
GitHub content blob after write: `4d90f0092c8a30307026316a8762970d6210415d`.

## Prior local evidence
Executed in the audit environment:

`python /mnt/data/test_apkc_source_cap_patch.py`

Result: `PASS source-cap transformer: exact anchor + overflow probe + read-error guard`

Recorded SHA-256 from the earlier local execution:

- transformer: `238736d72ee7288b31b2994c78c62838779603371b9f914d4ad829eadbc0d0dc`
- test: `176689a7793c38c4b47d910ab783c355735d401571cee10883960f447edac982`

## CI / runner provenance
For earlier head `c730a6d7597bfa34de7705064479fdfbb1a2388e`, CI `31593003144`, ApkC First Part Closure `31593003181`, Formal Science Orchestrator `31593003292`, and Document Governance `31593003301` all concluded `failure`, while their jobs reported `steps: []`, `runner_id: 0`, and empty runner names. Classification: `TOKEN_VAZIO_RUNNER`, not code failure.

For head `c5d939c67e5016c02dfc5428dff2637c1d1b3e2a`, GitHub reported six workflow runs. Formal Science Orchestrator run `31593551034` concluded `failure`; its single job again reported `steps: []`, `runner_id: 0`, and empty runner name. This independently confirms the runner/infrastructure condition persists on the new head. No repository proof execution is claimed from that workflow conclusion.

## Contract / epistemic state

- `SOURCE_CAP_TRANSFORMER`: VERIFIED.
- `TRANSFORMER_FALSIFIER_LOCAL`: VERIFIED.
- `CANONICAL_PROOF_WIRING`: VERIFIED_STATIC.
- `VALIDATOR_HARDENED_WIRING`: VERIFIED_STATIC.
- `HERMETIC_TERMUX_HARDENED_WIRING`: VERIFIED_STATIC.
- `CANONICAL_PROOF_EXECUTION`: TOKEN_VAZIO_RUNNER.
- `VALIDATOR_EXECUTION_ON_BRANCH`: TOKEN_VAZIO_RUNNER.
- `HERMETIC_TERMUX_EXECUTION_ON_BRANCH`: TOKEN_VAZIO_PHYSICAL_ARM.
- `DIRECT_CANONICAL_SOURCE_HARDENED`: TOKEN_VAZIO — `Apkc/apkc.c` itself still contains the vulnerable anchor; hardened consumers transform it before compilation.
- `ARM_BOUNDARY_RUNTIME`: TOKEN_VAZIO.
- `END_TO_END_APK_FROM_HARDENED_SOURCE`: TOKEN_VAZIO.
- `claim_allowed`: false for end-to-end/runtime conclusions.

## Urgency / provenance / closure gates
The largest bypasses identified in the canonical proof, validator, and hermetic Termux build paths are now mitigated statically. This does **not** establish repository-wide absence of every possible direct compile of raw `Apkc/apkc.c`; code search still returns additional references that require classification as documentation, historical evidence, test-only use, or executable build path.

Priority order:

1. enumerate remaining executable references to `Apkc/apkc.c` and either route them through the hardened entrypoint or prove they are non-build/document-only;
2. harden `Apkc/apkc.c` directly once all consumers of the exact-anchor transformer are migrated, preventing the transformer itself from becoming a regression dependency;
3. run `make proof` and `scripts/apkc_validate.sh` on a runner that actually starts steps and archive hardening logs, hashes and exit codes;
4. on ARM/Termux execute the boundary triad: `< 1048575` bytes PASS; `1048575` bytes PASS; `1048576` bytes FAIL with non-zero exit and no APK output;
5. execute the hardened hermetic build and bind raw source SHA → hardened source SHA → ApkC ELF SHA → APK SHA → unzip/readelf/sign/install/runtime receipts.

No gap is closed by narrative, workflow conclusion, screenshot alone, or inference. Evidence from an earlier APK installation remains historical evidence until its exact source/ELF/APK hashes are bound to this branch.
