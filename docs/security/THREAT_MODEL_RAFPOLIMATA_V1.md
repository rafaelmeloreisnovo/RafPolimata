# RafPolimata Threat Model V1

**State:** engineering threat model; not a certification or penetration-test report.

## Assets

- source and public API semantics;
- produced ELF/static-library/APK artifacts;
- ABI layouts and symbol contracts;
- hashes, receipts and custody records;
- benchmark integrity;
- device/runtime evidence;
- license/provenance metadata.

## Trust boundaries

1. untrusted external input → kernel/reference functions;
2. source tree → compiler/assembler/linker;
3. toolchain → produced artifact;
4. CI runner → uploaded receipt;
5. OS/kernel → raw-syscall userspace;
6. artifact → downstream consumer;
7. device observation → evidence ledger.

## Threats and current controls

| Threat | Consequence | Current control |
|---|---|---|
| length/integer overflow | OOB/corruption | bounded arena arithmetic + property boundaries |
| unaligned input | target fault/semantic drift | alignment-rotating CRC property cases; ARM32 byte path |
| ABI drift | downstream binary/source breakage | machine ABI contract + exact nm symbol gate |
| hidden runtime/helper | undeclared dependency | ARM32 full-TU undefined-symbol audit + freestanding gates |
| optimized/reference divergence | silent wrong answer | public reference implementation + equivalence campaign |
| forged/stale performance claim | misleading engineering decision | raw samples + SHA/toolchain/environment-bound benchmark receipt |
| source/artifact substitution | supply-chain compromise | source hashes, artifact SHA256, tool executable hashes, SPDX SBOM |
| stale local state | irreproducible success | fresh checkout cross-environment CI matrix |
| fake device promotion | unsupported physical claim | SOURCE ≠ BUILD ≠ DEVICE and CLOSURE_L12 receipt gate |
| stub/fallback reporting success | false capability | fail-closed stub policy and claim-gate discipline |

## Explicit non-goals in this cut

The following are not claimed as solved:

- resistance to a malicious or compromised kernel/hypervisor;
- constant-time behavior for secret-dependent workloads;
- electromagnetic/power side-channel resistance;
- physical fault injection/tamper resistance;
- formal memory-safety proof of the complete repository;
- vulnerability-free status;
- independent-provider reproduction;
- calibrated physical energy measurement.

Where runtime/device or independent-provider evidence is required and unavailable, the state remains TOKEN_VAZIO (CLOSURE_L12).

## Security invariants to keep executable

1. Public SDK runtime external dependencies remain empty.
2. Public ABI symbol set equals the versioned contract.
3. Specialized result equals reference result for the declared equivalence scope.
4. Oversized arena requests fail closed without cursor mutation.
5. Generated SBOM/source/toolchain receipts bind to the exact commit/artifact.
6. Benchmark source never contains a fixed comparative performance verdict.
7. Cross-environment reproduction never promotes itself to device or independent-provider proof.

## Next security gates

- sanitizer/fuzz campaigns for hosted adapters where sanitizers are semantically applicable;
- side-channel review for any future secret-bearing cryptographic API;
- signed release provenance after a release policy is authorized;
- independent external reproduction of the SDK gate bundle;
- device-bound receipts for ARM32/ARM64 physical paths.

R3 = ⟨F_ok: threats are mapped to concrete invariants/gates; F_gap: kernel/side-channel/physical/external-provider assurance remains outside this cut; F_next: execute exact-head maturity workflow and then external/device reproduction independently⟩.
