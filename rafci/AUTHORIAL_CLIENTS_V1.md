# RafCI Authorial Clients V1

**Authority:** `rafaelmeloreisnovo/RafPolimata` for this control contract only  
**Consumer authority:** each producer repository keeps implementation/test authority  
**Lifecycle:** `DRAFT_CANDIDATE / claim_allowed=false`  
**License state for this new file:** `TOKEN_VAZIO_FILE_LEVEL_SCOPE` until the repository's file-level licensing authority explicitly classifies it.

## Purpose

Reduce external dependency and integration friction without copying, renaming, relicensing or falsely promoting provider implementations as authorial code.

The model is:

```text
authorial algorithm / authorial control client
        != external factory tool
        != platform ABI
        != runtime dependency
        != runtime evidence
```

The machine-readable authority is `rafci/authorial_clients.v1.json`.

## Why this is not a provider rewrite

RafCI does not attempt to recreate Android SDK, Android NDK, Gradle, Android Gradle Plugin, CMake, R8, JDK or Kotlin merely to remove their names. That would increase maintenance and copyright/provenance risk without reducing the platform boundary.

Instead, the authorial layer owns:

- intent and admissibility;
- exact version/hash expectations where available;
- transformation and ABI boundaries;
- dependency/runtime classification;
- negative gates and falsifiers;
- receipts, rollback and evidence ceilings;
- clean-room replacement prerequisites when replacement is genuinely useful.

Provider bodies remain provider bodies. Repository/fork control does not create upstream authorship.

## Twelve client surfaces

| Surface | Authorial ownership | External/platform boundary | Default |
|---|---|---|---|
| `C` | algorithms, fixed layouts, freestanding core contracts | compiler is a factory tool | preferred for bounded low-level core |
| `Java` | thin app/platform edge source | Android ART/platform APIs | allowed without third-party app libraries |
| `Kotlin` | optional source surface | Kotlin compiler/runtime implications | disabled unless total friction decreases |
| `Python` | stdlib-only audit/control scripts | host interpreter | build/audit only, never app runtime |
| `Gradle` | project-owned task names/order/receipts | external build orchestrator | pin/wrap; do not copy provider body |
| `AGP` | project-owned configuration contract | Android Gradle Plugin | pin and audit transform boundary |
| `SDK` | capability/version client | Android SDK/platform/build tools | platform/build edge only |
| `NDK` | target/ABI client | Android native factory toolchain | Android-link edge; does not prove freestanding |
| `CMake` | project-owned target graph/config | external build orchestrator | minimal explicit graph |
| `JNI` | narrow project-owned bridge implementation | Android platform JNI ABI | one canonical edge by default |
| `R8` | enablement/rules/equivalence contract | external DEX/resource transform | OFF until separate semantic gate |
| `assemble` | build-target/receipt contract | composition of declared factory tools | artifact construction only |

## Freestanding boundary

`freestanding` applies to an execution/source contract that can satisfy its own freestanding gates. It is **not** a marketing label for every build tool involved in producing an Android APK.

```text
C core with freestanding gates     -> may reach FREESTANDING_STRUCTURAL
NDK-produced Android component      -> ANDROID_NATIVE_HOSTED
JNI bridge                          -> MANAGED_NATIVE_BOUNDARY
Java/Kotlin app edge                -> MANAGED_PLATFORM_EDGE
SDK/Gradle/AGP/CMake/R8/assemble    -> factory/transform evidence
```

Therefore:

```text
NDK_BUILD != FREESTANDING_EXECUTION
SDK_PACKAGE != RUNTIME_PASS
R8_SUCCESS != NATIVE_PASS
ASSEMBLE_SUCCESS != PHYSICAL_PASS
```

## Consumer split: Est-dio-de-udio

The audio repository remains the producer and owns:

- DSP/audio algorithms;
- C headers and implementations;
- the concrete JNI bridge;
- Android/Java UI and platform services;
- app build configuration;
- local tests, ABI manifests and receipts;
- visual design and user interaction.

RafPolimata owns only the reusable control vocabulary and validator. It points to the consumer; it does not copy the consumer body.

## Copyright and license preservation

The guardrail is fail-closed:

1. existing file/path notices remain authoritative for those files;
2. no third-party notice may be removed by this contract;
3. no blanket relicense is inferred from repository ownership;
4. a clean-room replacement needs a behavioral/format specification, independent implementation record, provenance, tests/falsifiers and an explicit file-level license decision;
5. until that last decision exists, the new replacement license field remains `TOKEN_VAZIO`.

This is engineering governance, not legal certification or legal advice.

## Dependency reduction law

A dependency is a replacement candidate only when the replacement reduces the **total** burden:

```text
runtime coupling
+ supply-chain exposure
+ maintenance cost
+ compatibility risk
+ licensing/provenance uncertainty
+ verification cost
```

A provider used only at build time is not silently counted as an app-runtime dependency. Conversely, a platform ABI used at runtime is not made freestanding by placing an authorial wrapper around it.

## DMAIC-style method

- **Define:** intended capability and the boundary that currently creates friction.
- **Measure:** imports, dependency declarations, tool identities, ABI edges, byte identities and receipts.
- **Analyze:** runtime dependency vs factory tool vs platform ABI vs optional transform.
- **Improve:** smallest reversible authorial client or clean-room algorithm that actually removes coupling.
- **Control:** exact source/artifact pins, negative tests, license/provenance checks, evidence ceilings and rollback.

This is a process mapping, not a Six Sigma certification claim.

## Gate

```sh
python3 rafci/tools/verify_authorial_clients_v1.py \
  --selftest \
  --receipt build/rafci/authorial-clients/receipt.json
```

A PASS here proves the **control contract only**. Runtime, physical-device behavior, upstream license interpretation and independent reproduction remain separate evidence classes.

## R3

`F_ok` = twelve surfaces are classified without provider-body copying or authority transfer; freestanding and factory/runtime boundaries are explicit.  
`F_gap` = file-level license for this new RafCI document/validator is intentionally unresolved; consumer execution evidence is not produced by this contract.  
`F_next` = bind one consumer through a local no-network gate, then execute exact-head CI; preserve `TOKEN_VAZIO` for any unobserved runtime or physical state.
