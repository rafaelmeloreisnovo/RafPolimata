# RAFAELIA L0 — stub policy

A stub is permitted only as an explicit structural boundary. It is never evidence that the behavior exists.

## Allowed

1. Compile-time unsupported-target gate (`#error`).
2. Declaration-only contract used to reserve a stable interface before an implementation lands.
3. Capability record that says unavailable/unsupported.
4. Test/probe symbol under `tests/` used only to inspect codegen or ABI.
5. Documentation anchor that binds an open gap to a closure/evidence path.

## Forbidden

```text
return 0;              // when 0 means success but nothing happened
return true;           // fake capability
empty production body // silent no-op
hosted fallback        // leaks libc/runtime into L0
scalar compatibility tail hidden behind vector API
silent retry/loop that changes latency semantics
shadow copy without declared ownership/lifetime
```

## Required stub annotation

A structural stub must state next to the declaration/body:

```text
STUB_KIND
IMPLEMENTED_BEHAVIOR
UNAVAILABLE_BEHAVIOR
FAIL_CLOSED_BEHAVIOR
PROMOTION_GATE
```

If the missing item is runtime/device evidence, use `TOKEN_VAZIO (CLOSURE_L12)` and name the receipt needed to promote it.

## Promotion

A stub is removed or promoted only when all of these are true:

1. implementation replaces the placeholder;
2. the relevant compile/equivalence gate passes;
3. comments and capability metadata are updated in the same change;
4. any stronger runtime/device claim has its own evidence.

No agent may weaken a stub's fail-closed behavior merely to make CI green.
