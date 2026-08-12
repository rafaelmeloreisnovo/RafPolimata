# TEST RECEIPT — RAFAELIA V4.2 ARMv7 Closure

Date: 2026-08-12
Environment: Linux x86_64 reference container; clang 17.0.0; javac 21.0.11.
Evidence class: reference/cross execution, **not physical Android evidence**.

## Reconstruction

```text
SOURCE_SHA256=d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454
V4_1_TO_V4_2_PATCH_SHA256=50aa730140f0577f2672c205f3294ce6acf02b07302cec57c8dc847e071a2a56
V42_PATCH_BYTE_MATCH=PASS
```

## Verification result

```text
PASS shell-syntax
PASS core-asan-ubsan
PASS orchestrator-asan-ubsan-42of42
PASS baremetal-werror
PASS opencl-smoketest-host-build
PASS java-xlint
PASS armv7-assembly-closure-9of9
PASS armv7-static-link-closure-9of9
TOKEN_VAZIO android-ndk-cross-build:ndk-build-missing
TOKEN_VAZIO armv7-real-device-execution
TOKEN_VAZIO android-jni-device-load-and-call
TOKEN_VAZIO gpu-opencl-kernel-real-device-execution
CLAIM_ALLOWED=false
STATUS=VERIFIED_LIMITED_ARMV7_CROSS_CLOSED
```

All nine static ARM outputs were independently inspected as `ELF32 / ARM / EABI5 /
soft-float` with no `PT_INTERP`. Link gate used `-nostdlib -static -Wl,-e,_start
-Wl,--no-undefined`.

Host execution of `rafaelia_opencl_smoketest` returned:

```text
rc=77
TOKEN_VAZIO gpu-opencl-library-not-found
```

That is an expected capability-missing state and is not promoted to GPU evidence.

## Negative gate

Running `omega_final_gate.sh` on the x86_64 reference host returned exit `1` with:

```text
FAIL requires-physical-arm32-userspace:got-x86_64
```

This is deliberate fail-closed behavior.
