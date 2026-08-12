# ApkC QEMU runtime hardening receipt — 2026-08-12

status: `VERIFIED_STATIC`
claim_allowed: `false`
base_main: `3f2873b6c8cf56a4a8e116eab40523af03354e92`
branch: `audit/apkc-runtime-proof-hardened-20260812`
implementation_commit: `8ad5c995f4daa45688d2d6d99a39728c379ba1de`
file: `scripts/arm64_apk_qemu_proof.sh`
content_blob: `d2410aac7f811e85c6e3e8f5c24eaef836391c0c`

## P0 closed statically

Before this change, the QEMU proof compiled `Apkc/apkc.c` directly. The proof path now refuses to invoke the compiler until a runtime-hardened translation unit is generated and verified.

Mandatory precompile gate:

1. require `python3`;
2. require `scripts/patch_apkc_runtime_source.py`;
3. generate `/tmp/apkc.runtime-hardened.qemu.c`;
4. generate `Apkc/proofs/out/apkc-runtime-source-hardening-qemu.json`;
5. require source marker `source exceeds 1MiB bounded input`;
6. require manifest marker `APKC-RH-013`;
7. require absence of legacy `if (n<=0) break;`;
8. record raw and runtime-hardened SHA-256;
9. compile only the generated runtime-hardened TU.

The cross-compile stage also removes stale output first and uses `--build-id=none` for cleaner custody/reproducibility semantics.

## Boundary falsifier added

The QEMU proof now creates exact byte-length assembly fixtures by padding the known `Apkc/hello.s.txt` program with trailing ASCII spaces:

- 1,048,574 bytes: expected PASS + non-empty APK;
- 1,048,575 bytes: expected PASS + non-empty APK;
- 1,048,576 bytes: expected non-zero exit + **no APK** + overflow marker.

Any violation makes the QEMU proof exit non-zero. The validation summary can promote the boundary gate only when `BOUNDARY_PROVEN=1` from actual execution.

## Chain-of-custody fields

The runtime proof transcript now binds, when executed:

`raw Apkc SHA → runtime-hardened TU SHA → AArch64 ApkC ELF SHA → APK SHA → classes.dex internal SHA-1 → embedded arm64 SO SHA → signed APK SHA`.

The existing structural/aapt/signature gates remain in place.

## Evidence boundary

The GitHub file was read back after the write and the runtime-hardening block, `APKC-RH-013` manifest check, hardened compile input and `claim_allowed:false` transcript marker are present.

Not yet proved in this environment:

- shell execution / syntax by a started runner;
- runtime transformer success on this exact head;
- AArch64 cross-link success;
- QEMU execution;
- boundary triad outcomes;
- signed APK result.

Therefore:

- `QEMU_RAW_SOURCE_BYPASS = MITIGATED_STATIC`
- `QEMU_RUNTIME_HARDENED_EXECUTION = TOKEN_VAZIO`
- `QEMU_BOUNDARY_TRIAD = TOKEN_VAZIO`
- `claim_allowed = false`

F_next: execute this exact branch on a runner with steps/logs or a local environment with python3+clang+lld+qemu-aarch64-static; archive transcript and hashes. In parallel, migrate `capture_android_proof_chain.sh` away from raw source before the next physical proof.
