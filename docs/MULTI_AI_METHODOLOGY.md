# Multi-AI Methodology — RafPolimata

> Canonical authority: `AGENTS.md` + `docs/AGENTES.md`.

This document defines collaboration among Codex, GitHub Copilot, ChatGPT, Claude Code, other coding agents, and humans **without assigning truth or architecture authority by vendor/model name**.

## 1. Principle

```text
agent identity != authority
claim quality = scope clarity + reproducible evidence + review
```

A tool may be better suited to a task because of available repository context, terminal access, review UI, or connector permissions. That does not make its prose more authoritative than executed evidence.

## 2. Common entrypoints

| Surface | Repository instruction entry |
|---|---|
| OpenAI Codex | `AGENTS.md` and any nearer scoped `AGENTS.md` |
| GitHub Copilot | `.github/copilot-instructions.md`, `AGENTS.md`, matching `.github/instructions/*.instructions.md` |
| Claude Code | `CLAUDE.md`, which imports/routes to the common protocol |
| ChatGPT with GitHub context | `AGENTS.md` + `docs/AGENTES.md` when the repository content is available in the task context |
| Human developer/reviewer | `README.md`, `AGENTS.md`, `docs/INDEX.md`, subsystem docs |

Do not assume a product automatically loaded a repository instruction file unless the surface actually provides it. When uncertain, explicitly read the file before acting.

## 3. Functional roles

Roles are task-scoped and can be performed by any capable agent or human.

| Role | Responsibility | Required output |
|---|---|---|
| navigator | locate canonical source, tests, receipts, conflicts | file/commit map |
| implementer | make scoped code/docs change | diff + rationale |
| falsifier | search for counterexample/regression | negative/positive test result |
| evidence custodian | bind commands, environment, hashes, receipts | provenance package |
| semantic reviewer | detect stale/overbroad claims | claim corrections |
| security/license reviewer | assess secrets, permissions, dependencies, terms | bounded risk finding |
| integrator | reconcile independent changes | conflict/compatibility map |
| human authorizer | decide exception/merge | explicit decision |

One session may perform multiple roles, but it should not pretend independent replication if the same agent generated and validated the same claim.

## 4. Handoff protocol

Every handoff should answer:

```text
BASE      = repository + branch + commit
INTENT    = what was requested
SCOPE     = files/subsystems touched
INVARIANT = what must not regress
EXECUTED  = exact commands/actions actually performed
OBSERVED  = PASS / FAIL / TOKEN_VAZIO by gate
CLAIMS    = what the evidence supports, no more
RISKS     = remaining failure modes
ROLLBACK  = how to revert safely
F_next    = smallest reproducible next step
```

Do not write `tests pass` when tests were not executed in that environment.

## 5. Branch and PR lifecycle

The repository default branch is `main` unless Git metadata says otherwise. Do not use historical text such as “Main (capital M)” as authority.

Default agent workflow:

```text
read -> baseline -> branch -> scoped edits -> applicable gates -> draft PR -> human review -> merge decision
```

- Do not commit directly to protected/default branch by default.
- Do not merge without explicit human authorization.
- Keep a PR draft while a material gate required by its stated claim is open.
- A merged PR is repository history, not proof that unrelated runtime/scientific claims are true.

## 6. Non-collision rules

1. Read `AGENTS.md` and the nearest scoped instructions before editing.
2. Do not modify generated outputs by hand.
3. Do not silently change API/ABI/schema/format/hash/timestamp/order semantics.
4. Do not delete contradictions, failed experiments, or falsifiers to make a narrative cleaner.
5. Do not create parallel “canonical” instructions when one already exists; add a thin adapter or scoped instruction.
6. Keep tool-specific details out of the canonical truth model unless they are required for that tool's discovery mechanism.
7. Rebase/reconcile against current `main` before claiming a conflict is solved.
8. If two agents produce incompatible changes, preserve both proposals until a test, spec, or human decision resolves them.

## 7. Conflict resolution

Record durable conflicts in `docs/AGENTES_DECISAO_LOG.md` rather than creating another decision-log filename.

For each conflict capture:

```text
proposal A
proposal B
shared invariant
evidence A
evidence B
TOKEN_VAZIO
falsifier/decision criterion
human decision if required
```

Escalate when conflict involves public interface, persisted format, security/privacy/license, deletion/migration, scientific claim promotion, or a required merge exception.

## 8. Independent validation

Use at least one independent dimension when stakes justify it:

- different compiler/toolchain;
- different architecture/device;
- independent reader/validator;
- negative/corruption test;
- separately implemented oracle;
- blind/out-of-sample dataset;
- human domain review.

A second AI repeating the same unchecked assumption is not independent validation.

## 9. Current cross-agent truth guards

### ApkC

Hosted x86/x86_64 development support and ARM/freestanding paths are separate contracts. Do not enforce “no libc everywhere” and do not weaken freestanding gates because a hosted wrapper exists.

### T^7 / 42

Do not restore “42 fixed-point attractors” as a proved statement. Read `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md`; the strong fixed-point convergence claim was falsified as stated.

### Android

```text
ELF built != APK packaged != signature verified != installed != runtime proven
```

### Evidence snapshots

A state JSON or receipt has a time/commit scope. If it predates HEAD, report it as a snapshot/historical observation until reconciled.

## 10. Documentation behavior

Prefer:

```text
one canonical rule
+ short tool adapter
+ path-specific instructions
+ links to evidence/closures
```

over copying the same technical narrative into five agent files.

When a narrative becomes stale, correct the adapter and canonical rule; preserve historical receipts rather than rewriting history.

## 11. Suggested division for parallel agents

When multiple agents are available, divide by orthogonal responsibility rather than vendor:

```text
A: repository/navigation + provenance
B: implementation
C: tests/falsifier
D: docs/claim reconciliation
E: security/license review
```

Integration happens only after comparing diffs and evidence against the same base commit.

## 12. Session close

All agents use the same closeout:

```text
F_ok   = what was actually completed/demonstrated
F_gap  = uncertainty, failure, unavailable evidence, or contradiction
F_next = smallest verifiable next action
```

`TOKEN_VAZIO` is a valid audit state and must not be cosmetically removed.
