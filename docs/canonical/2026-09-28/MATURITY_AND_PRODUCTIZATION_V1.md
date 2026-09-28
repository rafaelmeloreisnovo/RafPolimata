# Maturity and Productization V1

## Purpose

This cut converts eight previously open engineering gaps into executable, falsifiable surfaces while preserving the repository evidence ladder.

| Aspect | Materialized now | Gate / evidence | Remaining boundary |
|---|---|---|---|
| Functional equivalence | public reference SDK + comparison harness | specialized host CRC32C vs reference; internal Q16 vs reference; arena boundary behavior | runtime equivalence of every ARM32/ARM64/vector specialist remains separate |
| Property/fuzz testing | deterministic PRNG suite with 2,000,000 cases plus boundary vectors | alignment offsets, seeds, zero length, Q16 arithmetic, arena overflow and invalid FSM labels | not exhaustive formal proof; sanitizer/provider/device campaigns may extend it |
| Reproducible benchmarks | SHA-bound hosted benchmark receipt with 31 raw samples | ns, RDTSC cycles on x86, bytes/s, cycles/byte, variance, binary size, max RSS | calibrated energy is TOKEN_VAZIO (CLOSURE_L12) until measured |
| ABI/API stability | API 0.1.0 / ABI 1 machine contract | exact exported symbol set checked with nm | shared-object SONAME, language bindings and long-term support policy not yet declared |
| Supply chain | deterministic SPDX 2.3 SBOM for SDK slice + toolchain/artifact receipt | source hashes, compiler/ar/python identity and hashes, artifact SHA256 | repository-wide license compatibility remains REVIEW_REQUIRED |
| Threat model | explicit assets, trust boundaries, threats, mitigations and non-goals | mapped to existing/new gates | side-channel, compromised kernel and physical fault resistance are not claimed |
| Productization | buildable static library, public C header and smoke consumer | sdk/rafpolimata_v1/build.sh + ABI check | shared library/AAR/APK/package registries remain later layers |
| Reproduction | clean-checkout workflow matrix on Ubuntu 22.04 and 24.04 | same maturity gates run without local state | independent-provider reproduction is TOKEN_VAZIO (CLOSURE_L12) |

## Equivalence law

The SDK reference is intentionally portable and conservative. Optimized/internal implementations are allowed to differ in instruction selection, storage strategy and timing, but not in declared outputs.

Current executable equivalence scope:

CRC32C byte-stream + seed semantics:
  sdk reference == x86 specialized host path for generated/known vectors

Q16:
  mul / alpha=0.25 IIR / saturating abs:
  sdk reference == Benchmark implementation

Arena:
  allocation success/failure + used offset:
  sdk caller-owned arena == Benchmark bounded arena for matched capacity

Compile-only evidence on another ISA is not promoted to runtime equivalence.

## Property campaign

tests/maturity/test_equivalence_properties.c executes **2,000,000 deterministic generated cases**. Each case varies CRC seed, payload bytes, length and pointer alignment offset and also compares bounded Q16 operations.

Targeted boundaries include zero-length data, the standard CRC32C vector 123456789, arena capacities around alignment/capacity limits, 0xffffffff allocation requests and invalid FSM/class indices.

This is a high-volume falsifier, not a mathematical proof over all possible machine states.

## Benchmark receipt

scripts/run_maturity_benchmark.py builds the specialized host CRC path and preserves all 31 raw samples. The receipt is bound to git rev-parse HEAD and records:

- compiler identity;
- kernel/architecture;
- bytes per sample;
- min/median/p95/max nanoseconds;
- population variance;
- bytes/s at median;
- RDTSC cycles and cycles/byte where available;
- benchmark binary size;
- static input bytes;
- hosted runner max RSS;
- calibrated energy state.

No hard-coded performance number is part of the source claim.

## Version contract

ABI 1 is broken by removing/renaming a public symbol, changing signature/return type, changing rafp_arena_v1 field semantics/order, Q16 scale or CRC seed semantics. Such a change requires an ABI increment.

Documentation, private implementation changes and optimizations with identical public behavior do not require an ABI increment. New symbols may be added within ABI 1 while existing behavior remains intact; API minor version should advance.

## Supply-chain scope

The generated SPDX document is intentionally scoped to the SDK product slice. It must not be interpreted as a complete legal inventory of the whole repository.

The supply-chain receipt hashes the SDK artifact and direct source contract and captures the resolved compiler/ar/python executable identities. Runtime external dependencies for the static SDK are declared empty; build tools remain explicit.

## Reproduction semantics

The CI matrix proves a stronger property than a single-run CI:

fresh checkout
+ same source commit
+ two runner OS images
+ SDK build
+ ABI gate
+ two-million-case property/equivalence gate
+ deterministic double-build
+ SBOM/supply-chain receipt

Both images are still GitHub-hosted environments. Therefore this closes cross-environment/local-state reproduction only, not independent-provider reproduction.

## Promotion ladder

REFERENCE
→ IMPLEMENTED
→ BUILD_PROVEN
→ EQUIVALENCE_PROVEN(scope)
→ PROPERTY_CAMPAIGN_PASS(scope)
→ REPRODUCIBLE_BUILD(scope)
→ CROSS_ENV_REPRODUCED
→ DEVICE_PROVEN
→ INDEPENDENT_PROVIDER_REPRODUCED
→ RELEASE_CANDIDATE

Each arrow needs its own receipt. No later state is inferred from an earlier one.

R3 = ⟨F_ok: eight maturity surfaces have executable artifacts/gates; F_gap: cross-ISA runtime, energy/device, full-repo legal closure and independent provider remain separate; F_next: run exact-head matrix and consume the receipts before promotion⟩.
