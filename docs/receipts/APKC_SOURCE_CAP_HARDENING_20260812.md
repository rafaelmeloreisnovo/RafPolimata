# ApkC source-cap hardening receipt — 2026-08-12

Status: VERIFIED_LIMITED
claim_allowed: false
branch: `audit/apkc-mem-failclosed-20260812`
base: `a68aa9093e35f9ed2e332501425b2e0f5a33d99b`

## Gap
`Apkc/apkc.c` reads source into `_src_local[0x100000]` while `src_len < sizeof(_src_local)-1`; when the buffer fills it closes the file without probing for remaining input. A source larger than the accepted capacity can therefore be silently truncated before `build_apk()`.

## Change
Added `scripts/patch_apkc_source_cap.py`, an exact-anchor fail-closed transformer. The transformed read path:

- distinguishes read errors (`n < 0`) from EOF (`n == 0`);
- reserves the legacy NUL terminator byte;
- when the buffer reaches `sizeof(_src_local)-1`, probes exactly one additional byte;
- rejects additional input with `source exceeds SRC_CAP`;
- rejects a failed overflow probe;
- refuses transformation if the vulnerable anchor is absent or duplicated.

Added `tests/test_apkc_source_cap_patch.py` to falsify anchor drift and verify the overflow/read-error guards are present.

## Local evidence
Executed in the audit environment:

`python /mnt/data/test_apkc_source_cap_patch.py`

Result: `PASS source-cap transformer: exact anchor + overflow probe + read-error guard`

SHA-256:

- transformer: `238736d72ee7288b31b2994c78c62838779603371b9f914d4ad829eadbc0d0dc`
- test: `176689a7793c38c4b47d910ab783c355735d401571cee10883960f447edac982`

## CI state on head c414e919a7e03f9cb2b3d897065efea13e5ead1e
GitHub Actions runs `CI` (31590269711), `ApkC First Part Closure` (31590269645), and `Formal Science Orchestrator` (31590269595) concluded `failure`, but each job reported `steps: []` and `runner_id: 0`. This is classified as `TOKEN_VAZIO_RUNNER`, not evidence of a code failure.

## Claim boundary
This commit proves the guarded transformation and its local falsifier. It does **not** yet prove that every canonical build path consumes the transformed translation unit, nor does it prove ARM physical runtime or end-to-end APK generation. Those remain `TOKEN_VAZIO` and `claim_allowed=false`.

## Closure gate
Integrate this transform into the canonical ApkC runtime-hardening/build proof, then execute a negative test with source size `SRC_CAP` (one byte beyond accepted payload) and confirm non-zero exit with no output APK; also test exact accepted size and smaller input as controls.
