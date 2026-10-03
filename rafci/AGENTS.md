# AGENTS.md — RafCI scoped contract

This file scopes `rafci/**`.

## Read order

1. `rafci/README.md`
2. `rafci/graph.v1.json`
3. `rafci/contracts/rafci_wire_v1.h`
4. `rafci/contracts/rafci_anchors_v1.c`
5. `rafci/tools/verify_rafci_v1.py`

The repository-root `AGENTS.md` and `docs/AGENTES.md` remain authoritative above this file.

## Boundary

RafCI is a contract/federation layer, not a replacement GitHub runner and not a physical-runtime simulator.

Preserve:

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
IMPLEMENTED_UNTESTED != PASS
```

Closure routing is explicit: structural/operational gaps bind to `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY`; runtime/device evidence gaps bind to `CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE`. Closure linkage classifies an unresolved obligation and never promotes it.

The provider repo keeps implementation authority. Relationship never implies authority transfer.

## RAFCI-BIT rule

Every `RAFCI-BIT` block in C/H source must contain exactly these semantic fields:

```text
ID
KIND
ROUTE
AUTHORITY
EVIDENCE
```

`ID` must be unique across `rafci/**/*.c` and `rafci/**/*.h`.

Comments are navigational metadata. They never count as execution evidence.

## Bit stability

Capability and gate bit numbers are ABI-like identifiers.

- Never reuse a bit for a different meaning.
- Never silently renumber an existing bit.
- Update `graph.v1.json`, the C header and the verifier in one change.
- A breaking reinterpretation requires a successor schema/version.

## Binary/linker boundary

`rafci_wire_v1.h` and `rafci_anchors_v1.c` must remain freestanding-compatible source:

- no libc calls;
- no heap/GC;
- no syscall;
- no hosted runtime requirement;
- no undefined helper symbol;
- no runtime loop;
- no hidden tail/shadow state.

The `.rafci.*` sections are metadata anchors only. Their presence proves source/build structure when the gate executes, not runtime/device behavior.

## Evidence

`PASS` is valid only for an executed named gate and scope. Physical-device and provider-admin gaps remain `TOKEN_VAZIO` unless a same-scope receipt exists.

Minimum source gate:

```sh
python3 rafci/tools/verify_rafci_v1.py --selftest
```

Minimum binary gate:

```text
compile -> relocatable link -> no undefined symbols
        -> .rafci.authority/.rafci.route/.rafci.gates present
        -> expected anchor symbols present
```

## Change discipline

Keep changes local to `rafci/**`, its workflow, and the existing federation discovery pointer unless a concrete dependency requires more.

Do not:
- copy provider repositories into RafPolimata;
- weaken a provider/protection gate to obtain green CI;
- infer physical execution from cross-compilation;
- edit unrelated science/runtime code;
- merge without explicit human authorization.

Close with `F_ok / F_gap / F_next`.
