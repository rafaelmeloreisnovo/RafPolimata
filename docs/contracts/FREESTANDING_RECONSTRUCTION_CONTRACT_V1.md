# RAFAELIA Freestanding Reconstruction Contract V1

Status: `CANONICAL_CANDIDATE / DOCUMENTED / FAIL_CLOSED`  
Evidence state: `DOCUMENTED`  
claim_allowed: `false`  
Closure: `CLOSURE_L13`  
Baseline: `rafaelmeloreisnovo/RafPolimata@be3e1e3cdf11a6f99104f94915b7b47562090a61`  
Created: `2026-10-11T15:40:00-03:00`

This contract turns the current freestanding direction into a falsifiable reconstruction route. It does not promote runtime, provider, device, scientific or license claims. It binds the next development step to source, manifest, falsifiers and receipts so a human or AI navigator can rebuild the evidence chain without friction.

## Scope

The scope is the minimum safe delta for the current RAFAELIA route:

1. preserve the existing L0 freestanding contract in `freestanding/`;
2. make the reconstruction boundary explicit across source, adapters, toolchain, CI, receipts and claims;
3. define a machine-readable manifest companion;
4. define falsifiers that can invalidate over-strong claims before promotion;
5. leave all external/provider/device gates open until measured receipts exist.

Out of scope for this document:

- modifying Android, Termux, Frida, Gradle, R8, SDK, JNI or NDK runtime code;
- relicensing third-party or upstream material;
- declaring CI, physical device, cross-host, security, academic novelty or production readiness;
- using documentation existence as execution evidence.

## Observed baseline

| Surface | Observed artifact | Evidence |
| --- | --- | --- |
| Branch | `main@be3e1e3cdf11a6f99104f94915b7b47562090a61` | GitHub branch readback; protected=true |
| Freestanding core | `freestanding/CANONICAL.md` | source readback |
| L0 contract | `freestanding/README.md` | source readback |
| Header core | `freestanding/include/raf_fs_core.h` | source readback |
| Contract gate | `freestanding/tests/verify_contract.sh` | source readback |
| Zero-dependency gate | `freestanding/tests/verify_authorial_zero_dep.sh` | source readback |
| Language gate | `scripts/audit_language_freestanding_contract.sh` | source readback |
| Custody bridge | `docs/RAFGITTOOLS_CUSTODY_BRIDGE_V1.md` | source readback |
| Root license | `LICENSE` | `TOKEN_VAZIO`; GitHub contents read returned 404 |

## Boundary model

| Layer | May be freestanding? | Allowed role | Claim boundary |
| --- | ---: | --- | --- |
| L0 core | yes | authorial hot core, no hosted runtime, caller-owned memory | source/codegen only until exact execution receipt |
| ABI/profile metadata | yes, when audited | register and calling-contract description | no physical runtime claim by metadata alone |
| Tests/falsifiers | hosted shell may exist | detect forbidden source/binary/evidence states | PASS is scoped to exact ref/env/log |
| Adapters | no, unless separately isolated | Java/Kotlin/JNI/Python/JS/Lua/Shell bridge | adapter dependency does not contaminate L0 if boundary is explicit |
| Toolchain | no | Make/CMake/Gradle/R8/JVM/SDK/NDK/CI orchestration | tool availability is not proof of runtime |
| Receipts | no | bind ref, command, environment, hash and result | receipt does not prove beyond its declared scope |
| Documentation | no | route, governance and reconstruction | documentation is not execution |

## Hard invariants

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
DOCUMENTED != EXECUTED
IMPLEMENTED_UNTESTED != PASS
TOKEN_VAZIO != FAIL != PASS
provider_receipt != consumer_validation
codegen != physical_runtime
license_presence != relicensing_authority
```

If a required source, command, hash, environment, artifact, reviewer or runtime is absent, the state remains `TOKEN_VAZIO` or `DOCUMENTED`; it must not be promoted by prose.

## Acceptance gates

| Gate | Purpose | Minimum evidence | Current state |
| --- | --- | --- | --- |
| G0 source identity | exact repo/ref/path/SHA bound | GitHub readback | PARTIAL: branch/ref/path observed |
| G1 provenance/license | SPDX/license/author boundary per surface | license file or explicit TOKEN_VAZIO | PARTIAL: root LICENSE is TOKEN_VAZIO |
| G2 dependency scan | no hosted/runtime dependency in claimed L0 | `make authorial-freestanding` log | TOKEN_VAZIO in this transaction |
| G3 zero-dependency object | no unresolved helpers for claimed slice | `make authorial-zero-dependency` log + object symbols | TOKEN_VAZIO in this transaction |
| G4 language/toolchain contract | strict language matrix auditable | `make language-contract` log | TOKEN_VAZIO in this transaction |
| G5 artifact/ABI | emitted artifact and ABI stable | hash + `strict-elf`/ABI receipt | TOKEN_VAZIO |
| G6 falsifier suite | negative/adversarial tests run | failing cases preserve logs | TOKEN_VAZIO |
| G7 CI/provider | exact-head provider run | run id, URL, commit, status | external gate open |
| G8 physical runtime | real ARM32/ARM64/device execution | same-artifact device log | external gate open |
| G9 independent reproduction | separate host/provider/reviewer | independent receipt | external gate open |

Promotion is allowed only for the smallest claim whose gate passed. A PASS in one gate never upgrades another gate.

## Falsifiability contract

Every future claim must include:

```yaml
claim_id: ""
claim_text: ""
source_ref: ""
artifact_hash: ""
command: ""
environment: ""
expected_observation: ""
falsifier: ""
invalidates_if: ""
evidence_class: DOCUMENTED|MEASURED_LOCAL|REPRODUCED|CROSS_HOST|PROVEN|INVALIDATED|TOKEN_VAZIO
claim_allowed: false
```

A claim is invalidated or held at `TOKEN_VAZIO` when any of these occur:

- the source path or commit cannot be resolved;
- the command cannot be run in the declared environment;
- a forbidden hosted dependency appears in a claimed L0 path;
- an object contains unresolved helper symbols under the claimed zero-dependency profile;
- the artifact hash differs from the receipt;
- runtime/device proof uses a different artifact than the source claim;
- license/provenance is asserted without a license/notice chain;
- CI/provider state cannot be bound to exact head;
- independent reproduction cannot see enough source, command, input, artifact and expected result to rerun.

## Repository federation rule

RafPolimata may define and consume contracts. RafGitTools may provide custody envelopes and execution/receipt control. Actions, Android, Termux, Frida, SDK, Gradle, R8, JVM and package repositories are provider or adapter surfaces unless their own producer contract proves otherwise.

No repository inherits PASS from another repository. Cross-repository movement must carry:

```text
producer repo/ref/path/hash -> artifact -> execution env -> receipt -> consumer validation -> claim scope
```

## License and authorship rule

Root `LICENSE` is currently `TOKEN_VAZIO` for this observed baseline because `LICENSE` returned 404. Until a per-surface license map is read and bound, this contract authorizes no relicensing and no upstream authorship rewrite.

Required next license probe:

1. enumerate `LICENSE*`, `COPYING*`, `NOTICE*`, `CITATION.cff`, package metadata and upstream notices;
2. classify each path as authorial, derived, vendored, provider, generated, or TOKEN_VAZIO;
3. preserve third-party license text and notices;
4. attach the license map to the manifest before any release or external claim.

## Human/AI navigation

The reconstruction route is:

```text
START HERE -> CURRENT_STATE -> RafPolimata@exact-ref -> this contract -> manifest -> falsifiers -> command log -> receipt -> R3
```

A navigator should stop at the first missing source, command, hash, artifact, environment, reviewer or device log and write the missing item as `TOKEN_VAZIO`, not as a guessed result.

## R3

F_ok:
- baseline branch and key freestanding source files are bound by GitHub readback;
- the contract/manifest/falsifier route is now explicit and fail-closed;
- external gates remain separated from documentation.

F_gap:
- root license file unresolved at `LICENSE`;
- no local or CI execution was performed in this transaction;
- provider dispatch, runner visibility, physical ARM32/ARM64 runtime and independent reproduction remain open.

F_next:
- run the minimal command sequence on an authorized checkout: `make authorial-freestanding`, `make authorial-zero-dependency`, `make language-contract`;
- attach logs, object hashes and environment metadata as the next receipt;
- then update the manifest from `TOKEN_VAZIO` to the exact measured evidence class only for passed gates.
