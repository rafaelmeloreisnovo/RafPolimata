# Freestanding Failsafe / Failover / Watchdog / Rollback Contract V1

Status: `REFERENCE`
Lifecycle: `ACTIVE_CANDIDATE`
Area: `freestanding | safety | watchdog | rollback | cross-environment`
Parent envelope: `spec/FREESTANDING_PLATFORM_DEPENDENCY_ENVELOPE_V1.md`
Receipt: `receipts/2026-10-08_FREESTANDING_FAILSAFE_WATCHDOG_ROLLBACK_V1.md`

## Intent

Define how freestanding work can be prepared, tested and observed across environments without depending on external runtimes, native classes, hosted wrappers or source mutation.

The contract is conservative:

```text
observed_source == observed_source
observer != runtime_dependency
failsafe != proof_of_success
rollback != hidden_mutation
```

## Core rule

A freestanding target may be exercised in any environment only when the environment is treated as an observer/harness, not as part of the freestanding core.

| Layer | Allowed role | Forbidden promotion |
|---|---|---|
| L0 freestanding core | pure source/ELF/object/ASM contract | depends on Java/Kotlin/native class/Python/shell/libc/syscall |
| Harness | build, package, run, timeout, capture, compare | counted as L0 dependency |
| Watchdog | timeout, heartbeat, process/app observation | counted as algorithmic success |
| Failover | choose next target or previous artifact | silently edits source to pass |
| Rollback | restore previous known artifact/state | erases negative evidence |

## No-mutation policy

A test harness must not modify the code under observation. Any adaptation belongs outside the source under test.

Allowed:

| Action | Status | Condition |
|---|---|---|
| compile source as-is | allowed | command and toolchain recorded |
| package artifact as-is | allowed | embedded identity/hash recorded |
| timeout a run | allowed | timeout state recorded as `FAIL` or `TOKEN_VAZIO`, not PASS |
| retry same artifact | allowed | retry count and reason recorded |
| switch to previous artifact | rollback | previous identity must be recorded |
| create follow-up patch | allowed | new commit/parent/supersedes relation required |

Blocked:

| Action | Reason |
|---|---|
| patching source inside watchdog | hidden mutation |
| treating hosted class/native wrapper as L0 proof | dependency confusion |
| swallowing failures to keep pipeline green | false evidence |
| replacing artifact without hash/provenance | broken custody |
| deleting negative logs/receipts | evidence loss |

## Failsafe states

| State | Meaning | Claim boundary |
|---|---|---|
| `SAFE_STOP` | execution stopped before unsafe or unknown state | not PASS |
| `FAIL_CLOSED` | failure preserved and no promotion occurred | not PASS |
| `FAILOVER_USED` | alternative route/artifact selected | original route still FAIL/TOKEN_VAZIO |
| `ROLLBACK_READY` | previous artifact/state is known | rollback not yet executed |
| `ROLLBACK_DONE` | previous artifact/state restored | new receipt required |
| `WATCHDOG_TIMEOUT` | heartbeat/deadline failed | runtime success denied |
| `OBSERVER_ONLY` | host saw/captured state only | no L0 proof by itself |

## Watchdog minimums

A watchdog receipt should include:

| Field | Required meaning |
|---|---|
| `artifact_identity` | hash/path/ref of the exact artifact observed |
| `source_identity` | commit/path/ref that produced it |
| `observer_identity` | harness/tool/device used to observe |
| `deadline` | timeout or heartbeat limit |
| `exit_state` | PASS/FAIL/TOKEN_VAZIO/timeout state |
| `mutation_policy` | must state `NO_SOURCE_MUTATION` for this contract |
| `rollback_pointer` | previous artifact/ref or `TOKEN_VAZIO` |

## Cross-environment rule

Android, Linux, macOS, Windows, iPhone, emulators and CI providers are execution surfaces. They may provide evidence, but they do not become the freestanding authority.

```text
same source + different harnesses -> comparison evidence
same harness + changed source     -> new source claim boundary
same artifact + different device  -> runtime/environment evidence
```

## ARM32/ARM64 application

For `armeabi-v7a` and `arm64-v8a`, the first safe chain is:

```text
manifest -> source ref -> object/ELF -> hash -> package/embed -> signature -> install/run -> watchdog -> receipt
```

If any link is absent, the corresponding claim remains `TOKEN_VAZIO` or `NOT_RUN`.

## R3

`F_ok`: failsafe/failover/watchdog/rollback are now defined without mutating freestanding source.
`F_gap`: no watchdog run or rollback execution is claimed by this document.
`F_next`: create a minimal watchdog receipt schema or instantiate one ARM target with `NO_SOURCE_MUTATION`.
