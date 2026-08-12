# TEST RECEIPT — RAFAELIA V4.1 Hardened

Date: 2026-08-12  
Environment: host/container reference only; **not** physical Android proof.  
Compiler: `clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)`  
Java: `javac 21.0.11`  
Kernel: `Linux x86_64 / container reference`

## Canonical reconstruction gate

`./MATERIALIZE_AND_VERIFY.sh /path/RAFAELIA_COMPLETE_v4.zip /tmp/rafaelia-v4.1`

## Result

- source SHA-256 gate: PASS
- reassembled patch SHA-256 gate: PASS
- patch application: PASS
- reconstructed hardened files vs audited working copy: byte-match PASS
- shell syntax: PASS
- `rafaelia_core.c`: ASan+UBSan PASS; sanitizer stderr 0 bytes
- `rafaelia_orchestrator.c`: ASan+UBSan PASS; `OK=42`; `CRC_ERR=0`; sanitizer stderr 0 bytes
- `baremetal_nomalloc.c`: `-Wall -Wextra -Werror` PASS
- `RafaeliaCore.java`: `javac -Xlint:all` PASS
- ARMv7 assembly probe: `0 PASS / 9 FAIL`

Observed core: `GPU=CPU-SCALAR`, `COMMITS=42`, `ROLLBACKS=0`, `TOTAL_PTS=1008`.  
Observed orchestrator: `OK=42`, `CRC_ERR=0`, `GPU=cpu`.

## TOKEN_VAZIO

- ARMv7 assembly closure
- ARMv7 physical-device execution
- Android NDK ABI/device execution
- real OpenCL/Vulkan compute execution

`STATUS=VERIFIED_LIMITED`  
`CLAIM_ALLOWED=false`

This receipt proves only the gates executed in the reference environment. It does not substitute for architecture-specific device evidence.
