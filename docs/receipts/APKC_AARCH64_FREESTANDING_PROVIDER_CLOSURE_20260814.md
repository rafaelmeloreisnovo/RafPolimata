# ApkC — AArch64 Freestanding Provider Closure — 2026-08-14

State: `VERIFIED_LIMITED_PROVIDER / claim_allowed=false`

## Scope

This receipt records only the source→binary AArch64 blocker that was observed as an external compiler-generated `memcpy` libcall under the freestanding/no-libc link. It does not promote Android, installation, launch, TLS runtime, or full C04 closure.

## Before

Provider run `31798804681`, artifact `9218419408` (`sha256:25f18f16bfc82ac30f878d564902a4b38716a462c46c2f70b37083e647be77ae`) preserved:

- AArch64 source proof build: `FAIL`
- linker root: `undefined symbol: memcpy`, referenced from `codegen_select`
- ARM32 build/identity/reproducibility: `PASS`
- `claim_allowed=false`

## Correction

`Apkc/mem.h` now exports a freestanding compiler-runtime `memcpy` ABI shim backed by a noinline volatile byte-copy implementation. This keeps ApkC independent of hosted libc while satisfying backend-generated aggregate-copy libcalls.

An adversarial probe was added at `tests/test_apkc_freestanding_memcpy_aarch64.sh`: the positive path requires a linked ELF64/AArch64 artifact with no unresolved `memcpy`; a negative control removes the shim and requires the same aggregate-copy probe to fail with `undefined symbol: memcpy`.

## Provider evidence

Provider run `31803471997`, source head `5aca39939705cd80affe058861b66dd2f8a206bd`, checkout merge ref `b8f07a40d87c71dba2181527f5f77260e8e9c0ac`, executed the real source→binary proof and reported:

`source->binary proof: PASS (Apkc/proofs/runs/20260814T131028Z/source-to-binary)`

Artifact `9220212050`:

- size: `355240` bytes
- SHA-256: `7ef34981b4e8bd6e95fc15ee7d8cf5b37b081211d04b8152425cbda481376faa`
- schema: `raf.apkc.source-to-binary-proof.v3`
- source-cap hardening: `PASS`
- AArch64: build=`PASS`, identity=`PASS`, reproducibility=`PASS`, SHA-256=`eb4d023a12fcded22828598d44a88ba7c5f50caf1ef980b89d69df48f7fec05e`
- ARM32: build=`PASS`, identity=`PASS`, reproducibility=`PASS`, SHA-256=`f5d01edff241a17f590ff5d215f71788dbb2f76fd7db0d744a239a75446f83b4`
- state=`PASS`
- claim_allowed=`false`

Therefore:

`TOKEN_VAZIO_AARCH64_FREESTANDING_MEMCPY_LIBCALL -> RESOLVED_PROVIDER_SOURCE_PROOF_PASS`

`TOKEN_VAZIO_AARCH64_SOURCE_PROOF_COMPLETE -> RESOLVED_PROVIDER_BUILD_IDENTITY_REPRO_PASS`

## Newly isolated blockers

The same provider artifact proved that the AArch64 ELF was a PIE with `PT_INTERP /lib/ld-linux-aarch64.so.1`. QEMU then failed before ApkC execution because that loader was unavailable. A later append-only delta changes the proof to require static/non-PIE AArch64 and rejects any `INTERP`; this new head still requires exact provider execution.

The same artifact also proved malformed generated C in runtime hardening: RH-008, RH-009 and RH-013 replacement payloads converted Python `\n` escapes into literal source newlines. A compatibility repair now preserves C `\\n` and keeps all 13 exact transform IDs/`replace_once` contracts. This also remains pending exact provider execution on the latest head.

TLS static audit still has an independent failing unit test and is not covered by this closure.

## F_ok

- provider-executed AArch64 source proof PASS;
- ELF64/AArch64 identity PASS;
- byte reproducibility PASS;
- ARM32 remains PASS;
- source-cap remains PASS;
- no hosted libc claim introduced;
- claim_allowed=false.

## F_gap

- `TOKEN_VAZIO_AARCH64_STATIC_NO_INTERP_HEAD_EXACT_EXECUTION`
- `TOKEN_VAZIO_RUNTIME_C_ESCAPE_HEAD_EXACT_EXECUTION`
- `TOKEN_VAZIO_GENERATE_APK_QEMU_EXECUTION_AFTER_STATIC_LINK`
- `TOKEN_VAZIO_HTTPS_STATIC_CA_AUDITOR_RECONCILIATION`
- `TOKEN_VAZIO_C04_STRUCTURAL_CLOSURE`
- all Android/Termux physical runtime gaps remain open.

## F_next

Execute the latest PR head and require, in order: source-proof static/no-INTERP PASS; QEMU candidate generation; runtime-hardened cross-ABI compile; then reconcile TLS static audit and C04 provenance. Do not promote physical Android evidence from provider Linux runs.
