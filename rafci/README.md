# RafCI — Authorial CI Contract V1

**Area:** execution-governance / CI federation  
**Logical authority:** `rafaelmeloreisnovo/RafPolimata`  
**Lifecycle:** `ACTIVE / source-contract`  
**Claim state:** `claim_allowed=false`

RafCI is the small authorial contract that connects existing producers without copying them into RafPolimata.

```text
RafPolimata        = AUTHORITY (contract, graph, gate dictionary, receipt wire)
RafGitTools        = OPERATOR (dispatch/UI/receipt consumption)
actions            = ACTION_PROVIDER (provider/fork implementation surface)
runner-images      = HOST_PROVIDER (host image surface)
termux-packages    = TOOLCHAIN_PROVIDER (toolchain/distribution surface)
freestanding code  = EXECUTION_TARGET / ARTIFACT DOMAIN
```

The provider repositories keep their own implementation authority. RafCI does not transfer authority and does not make an upstream-derived fork an authorial core.

## Truth boundary

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
IMPLEMENTED_UNTESTED != PASS
```

**Closure routing:** structural/operational unknowns in this subsystem bind to `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY`; runtime/device evidence gaps bind to `CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE`. A closure classifies the open evidence obligation; it does not convert an unknown into PASS.

A CI YAML, an ELF object, a linker section, a provider ruleset readback, a receipt file, and a physical-device observation are different evidence classes.

## Small route

```text
source identity
    -> artifact identity
    -> linker/ELF structural gates
    -> hosted or physical execution
    -> evidence receipt
    -> claim gate
```

`graph.v1.json` is the machine-readable route. `contracts/rafci_wire_v1.h` is the fixed binary vocabulary. `contracts/rafci_anchors_v1.c` materializes route metadata as symbols and ELF sections. `tools/verify_rafci_v1.py` proves that the graph, bit assignments, source comments, closures and anchors do not silently drift.

## Comments as semantic databits

Comments are engineering metadata only when they follow the bounded block below and the verifier can resolve them:

```text
RAFCI-BIT
ID=<globally unique id inside rafci C/H sources>
KIND=<contract|anchor|gate|route>
ROUTE=<graph node/edge ids or bounded route>
AUTHORITY=<authority id>
EVIDENCE=<what this source can and cannot establish>
```

The same meaning is not duplicated as free prose into the binary. Instead:

1. the graph owns semantic IDs and bit numbers;
2. the header exposes those bit numbers to the C preprocessor/compiler;
3. the anchor translation unit exports stable symbol names into `.rafci.*` ELF sections;
4. CI checks graph ↔ header ↔ source comments ↔ linker-visible sections.

This makes the route inspectable by humans, IAs, compilers and binary tooling without claiming that a comment executes.

## Linker-visible composition

The anchor object intentionally contains only constant metadata:

```text
.rafci.authority  -> rafci_authority_anchor_v1
.rafci.route      -> rafci_route_anchor_v1
.rafci.gates      -> rafci_gate_anchor_v1
```

There is no heap, libc call, syscall, background process, hidden runtime loop, or external helper symbol in this anchor layer. Structural compilation/linking is not physical execution.

## Bit law

Every capability bit and gate bit has one stable semantic ID. Reusing or renumbering an existing bit is a breaking contract change and must use a new schema/version.

Capability bits describe **who/what a module can be**. Gate bits describe **what evidence was actually checked**. A receipt carries independent PASS / FAIL / TOKEN_VAZIO masks; absence is never promoted to PASS.

## Gate commands

Repository-local source validation:

```sh
python3 rafci/tools/verify_rafci_v1.py --selftest \
  --receipt build/rafci/contract-receipt.json
```

Freestanding-compatible metadata object, example:

```sh
clang -std=c11 -ffreestanding -fno-builtin -fno-stack-protector \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -I rafci/contracts -c rafci/contracts/rafci_anchors_v1.c -o anchors.o
ld -r anchors.o -o anchors.linked.o
nm -u anchors.linked.o
readelf -SW anchors.linked.o
readelf -sW anchors.linked.o
```

The GitHub workflow compiles and inspects the anchor object for x86_64, ARMv7 and AArch64, then separately proves relocatable-link survival on hosted x86_64 with the runner's available system linker. Cross-target linking is toolchain-dependent and is not inferred from object compilation. A green structural workflow does **not** establish physical ARM32/ARM64 runtime, provider enforcement, scientific validity, or production readiness.

## Provider boundary

The authorial contract references providers by repository + role. Exact provider SHAs belong in execution receipts, not in this long-lived semantic dictionary, so the contract does not become stale merely because a provider advances.

`RafGitTools` may dispatch and display the route but does not silently rewrite authority. `termux-packages` may provide a toolchain but does not become evidence authority. `runner-images_RAFCODE` may provide a host but does not make the host freestanding. `actions` may provide action implementations but does not define RafCI truth semantics.

### Provider enforcement observation

`provider_policy.v1.json` defines the configuration needed before provider enforcement can even be considered ready: active branch ruleset, target-ref coverage, zero bypass actors, deletion and non-fast-forward protection, pull-request enforcement, at least one required approval and at least one required status check.

`tools/audit_provider_rules_v1.py` performs a live GitHub ruleset readback using only Python stdlib, normalizes the provider state, emits a deterministic observation digest and records the missing controls. The tool is intentionally incapable of promoting `gate.provider-enforcement` to PASS. Even a fully configured readback still requires a **separate zero-approval rejection receipt** against the protected promotion path.

This distinction is deliberate:

```text
provider configuration readback
    != rejected unauthorized/zero-approval promotion attempt
    != provider-enforcement PASS
```

The Actions job preserves the normalized readback and receipt and asserts that `claim_allowed=false`, `provider_enforcement_gate=TOKEN_VAZIO`, and closure L11 remain intact.

## Current explicit gaps

The graph keeps these fail-closed:

- provider/admin enforcement and zero-approval rejection evidence;
- ARM32 physical execution receipt;
- ARM64 physical execution receipt;
- provider-independent runtime reproduction.

They stay `TOKEN_VAZIO` until their own evidence rules are satisfied.

## Rollback

This subsystem is additive. Rollback is repository-local: revert the RafCI commit/PR or supersede the schema. Do not rewrite history in other producer repositories and do not delete contradictory receipts.

## R3

`F_ok` = source-level contract, semantic graph, unique bits, linker anchors, closure discipline and live provider readback machinery are defined.  
`F_gap` = provider negative rejection evidence and physical/provider-independent execution remain separate until executed.  
`F_next` = consume exact-head receipts; provider configuration may be observed, but promotion remains closed until an authorized negative rejection test exists.
