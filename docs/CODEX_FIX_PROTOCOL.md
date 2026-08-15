# Codex / AI Compilation-Fix Protocol — RafPolimata

> Subordinate to `AGENTS.md` and `docs/AGENTES.md`.

This is a **surgical compilation-diagnosis protocol**, not a second repository-wide instruction source. OpenAI Codex should start at `AGENTS.md`; other agents may use this file when the task is specifically a compiler/build failure.

## Objective

Diagnose and repair compilation/build defects without weakening architecture, evidence, error semantics, ABI/layout contracts, or tests merely to obtain a green result.

## Non-negotiable rules

1. Distinguish hosted and freestanding paths before changing includes, allocation, entrypoints or syscalls.
2. Do not use the obsolete blanket rule “no libc anywhere in `Apkc/`”. Freestanding targets keep their no-libc/no-heap contract; explicitly hosted development paths may use libc inside that boundary.
3. Do not change a public API, persisted format, status code, error meaning or claim boundary as a side effect of a compile fix.
4. NULL/unknown lookup paths must be handled according to the current API contract.
5. `TOKEN_VAZIO` is an evidence state, **not a universal C integer value**. Never assume `TOKEN_VAZIO == 0` unless a concrete schema/API explicitly defines that encoding.
6. Missing hardware/tool/device is not automatically PASS. Represent it using the contract's explicit unavailable/skip/TOKEN_VAZIO state.
7. Never weaken warnings, tests or assertions only to hide a causal defect.
8. Never merge without explicit human authorization.

## Diagnostic flow

### Step 1 — Identify the exact failing contract

Record:

```sh
git branch --show-current
git rev-parse HEAD
git status --short
```

Then identify:

```text
source path
target architecture
hosted | freestanding | shared
compiler/toolchain
exact command/target
first causal error
related test/gate
```

Prefer current repository entrypoints over commands copied from historical documents.

Examples:

```sh
make syntax
make compiler-contract
make compiler-selftest
```

Run only the gate applicable to the failing path.

### Step 2 — Minimize the failure without changing semantics

Use the project's current command when possible. If a direct compiler invocation is required for diagnosis, preserve the target flags and include paths from the canonical script/Makefile rather than inventing a new build profile.

Capture the **first causal error**, not only the last cascade.

Useful diagnostic classes:

| Class | Typical symptom | Required check |
|---|---|---|
| declaration/type | incompatible/implicit declaration | header/API contract |
| target boundary | hosted symbol in freestanding build | target-specific guard |
| layout/ABI | size/offset/link failure | schema/format + validator |
| capacity | overflow/truncation diagnostic | explicit bounds |
| toolchain | command/path unavailable | provenance + TOKEN_VAZIO if unavailable |
| architecture | unsupported asm/instruction | target/compiler flags |
| warning-as-error | unused/sign/format | fix cause, do not globally silence |

### Step 3 — Choose the smallest semantic fix

A compile fix may be larger than five lines when the causal defect genuinely spans several files. The correct limit is **scope**, not line count.

Escalate when the repair requires any of:

- public API/ABI change;
- persisted format/schema change;
- deletion/migration;
- security/license change;
- altered error/status semantics;
- weakened freestanding boundary;
- new architecture/language backend;
- contradiction with a current closure or receipt.

### Step 4 — Re-run the failing gate and nearest regression tests

Do not claim repository-wide PASS from one compiler invocation.

For ApkC-related work, applicable gates may include:

```sh
make syntax
make compiler-contract
make compiler-selftest
make hotfix-audit
```

Select the minimum sufficient matrix for the changed paths and state exactly what was not run.

## Approved repair patterns

### Guard optional/unknown lookup

```c
if (!prof) {
    pr_err("unknown language\n");
    return 1;
}
```

Use the actual function/API's error convention; do not copy this return value into unrelated APIs.

### Hosted/freestanding split

```c
#if defined(__x86_64__) || defined(__i386__)
/* explicitly hosted development support */
#include <unistd.h>
#else
/* target-specific freestanding path */
#endif
```

This is acceptable only when the current architecture intentionally defines that boundary. Do not use it to smuggle hosted dependencies into the freestanding object.

### Ignored result

Prefer checking a result when failure matters. If the API contract intentionally ignores it, make that choice explicit and warning-clean without changing behavior.

### Feature macro guard

```c
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
```

Use only where the hosted target requires it and where it does not change a freestanding translation unit unexpectedly.

## Forbidden “fixes”

- `|| true` around a blocking gate;
- unconditional `return 0` to make CI green;
- disabling `-Werror` globally instead of fixing a new warning;
- deleting a falsifier because it exposes a regression;
- changing `FAIL`/unavailable into `PASS`;
- replacing a real current-artifact gate with file-existence checks;
- editing generated results manually;
- introducing unbounded copy/allocation into bounded/freestanding code;
- claiming current runtime from historical receipts.

## Evidence package

A useful compilation-fix receipt/handoff records:

```text
base commit
changed paths
failing command and first causal error
repair rationale
post-fix command(s)
exit code(s)
compiler/tool version when available
PASS / FAIL / TOKEN_VAZIO by gate
risks and rollback
```

## Close

```text
F_ok   = compile defect actually repaired and gates actually observed
F_gap  = architectures/tools/tests/runtime not observed or contradictions remaining
F_next = smallest reproducible next validation
```

A green local compiler command proves only the scope of that command.
